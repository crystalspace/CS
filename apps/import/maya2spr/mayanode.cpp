/*
  Crystal Space Maya .ma Convertor
  Copyright (C) 2002 by Keith Fulton <keith@paqrat.com>
    (loosely based on "mdl2spr" by Nathaniel Saint Martin <noote@bigfoot.com>
                     and Eric Sunshine <sunshine@sunshineco.com>)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "mayanode.h"
#include "binarytree.h"

#include "csgeom/tri.h"

void DAGNode::PrintStats(FILE *s,int level) 
{
  for (size_t i=0;i<children.Length();i++)
    children[i]->PrintStats (s, level+1);
}

NodeTransform::NodeTransform()
{
    trans.Set(0,0,0);
    scale.Set(1,1,1);
    rot.Set(0,0,0);
}

bool NodeTransform::Load(MayaInputFile& file)
{
    csString tok;

    bool stop = false;

    while (!stop)
    {
        file.GetToken(tok);
        
        if (tok == "-n")  // set name
        {
            file.GetToken(tok);
            SetName(tok);
            csPrintf("Transform Name is '%s'\n",(const char *)tok);
        }
        else if (tok == "setAttr")
        {
            if (!LoadAttr(file))
                return false;
        }
        else if (tok == "createNode")
        {
            file.PushToken(tok);
            break;
        }
    }
    return true;
}

bool NodeTransform::LoadAttr(MayaInputFile& file)
{
    csString tok;
    bool stop = false;

    while (!stop)
    {
        file.GetToken(tok);
        
        if (tok == ".v")
        {
            // not sure about this one
            return true;
        }
        else if (tok == ".t")
        {
            return LoadTransformAttr(file,trans);
        }
        else if (tok == ".s")
        {
            return LoadTransformAttr(file,scale);
        }
        else if (tok == ".r")
        {
            return LoadTransformAttr(file,rot);
        }
	else if (tok == ".sp")
	{
            return LoadTransformAttr(file,trans);
	}
        else if (tok == "createNode")
        {
            file.PushToken(tok);
            return true;
        }
    }
    return false;
}

bool NodeTransform::LoadTransformAttr(MayaInputFile& file,csVector3& vec)
{
    csString tok;

    file.GetToken(tok);

    if (tok != "-type")
    {
        file.SetError("Expected -type token in Transform Attr.");
        return false;
    }

    file.GetToken(tok);

    if (tok != "double3")
    {
        file.SetError("-type in Transform Attr must be double3.");
        return false;
    }
    file.GetToken(tok);
    vec.x = atof(tok);

    file.GetToken(tok);
    vec.y = atof(tok);

    file.GetToken(tok);
    vec.z = -atof(tok);

    file.GetToken(tok);
    if (tok != ";")
    {
        file.SetError("Missing semicolon (;) at end of Transform Attr.");
        return false;
    }
    return true;
}


/*--------------------------------------------------------------------------*/

NodeMesh::NodeMesh(csVector3& trans,csVector3& s)
{
    translate = trans;
    scale     = s;

    countelem = 0;  // count comes before enumeration
    type = 0;       // indicator of using double, int, string, etc.

    count_uv=0;    //  how many u,v's are there?
    u = v = 0;       //  u,v mapping set for all vertices

    count_vert=0;     //  how many x,y,z's follow?
    vertex = 0;      //  x,y,z coords for all vertices
    animvertex = 0;  //  x,y,z coords for all vertices
    vertnorms = 0;   //  Vertex normals for all vertices

    count_edge=0;            // how many edges?
    edgestart=0;
    edgestop=0;  // edge definitions (index to vertices)

    count_face=0;      // how many faces are there (both e1,e2,e3 and uv1, etc.)
    faceedge=0;    // edges of triangle faces (index to edges)
    tri_normals=0; // normals to each poly
    uv1 = uv2 = uv3 = 0;  // indices into u[] and v[] for each point
    
    countcsverts=0;
    CSVerts=0;
    CSPoly[0] = CSPoly[1] = CSPoly[2] = 0;
}

void NodeMesh::PrintStats(FILE *s,int level)
{
    PrintSpaces(s,level);
    csFPrintf(s,"Mesh '%s':\n",(const char *)name);

    PrintSpaces(s,level);
    csFPrintf(s," (%d uv mappings)\n",count_uv);
    PrintSpaces(s,level);
    csFPrintf(s," (%d vertices)\n",count_vert);
    PrintSpaces(s,level);
    csFPrintf(s," (%d edges)\n",count_edge);
    PrintSpaces(s,level);
    csFPrintf(s," (%d polys)\n",count_face);

    DAGNode::PrintStats(s,level+1);
}

bool NodeMesh::Load(MayaInputFile& file)
{
    csPrintf("In NodeMesh::Load.\n");
    csString tok;

    bool stop = false;

    while (!stop)
    {
        file.GetToken(tok);
        
        if (tok == "-n")  // set name
        {
            file.GetToken(tok);
            SetName(tok);
            csPrintf("Mesh Name is '%s'\n",(const char *)tok);
        }
        else if (tok == "-p") // parent
        {
            file.GetToken(tok);
            this->SetParentName(tok);
            csPrintf("Parent of this Mesh is '%s'.\n",(const char *)tok);
        }
        else if (tok == "setAttr")
        {
            if (!LoadAttr(file))
                return false;
        }
        else if (tok == "createNode")
        {
            file.PushToken(tok);
            break;
        }
    }
    return true;
}

bool NodeMesh::LoadAttr(MayaInputFile& file)
{
    csString tok;
    bool stop = false;

    while (!stop)
    {
        file.GetToken(tok);
        
        if (tok == "-s") // element count is next
        {
            // save these values for the next attribute
            file.GetToken(tok);
            countelem = atoi(tok);
            file.GetToken(tok);
            countname = tok;

            if (!strncmp(countname,".uvst[0].uvsp",13)) // UV mapping for vertices
            {
                csPrintf(" Need %d elements of %s.\n",countelem,(const char *)countname );

                u = new float[countelem];
                v = new float[countelem];

                count_uv = countelem;
		int i;
		for (i=0; i<countelem; i++)
		{
		    u[i] = 0;
		    v[i] = 0;
		}
                if (strlen(countname)>13) // if subscript is here
                {
                    return LoadUVMap(file,tok); // actually only partial for large models per line
                }
                return true;
            }
            if (!strncmp(countname,".vt",3))           // Vertex list
            {
                csPrintf(" Need %d elements of %s.\n",countelem,(const char *)countname );

                /**
                 * We allocate as many vertices as there are UV mappings because
                 * that is the theoretical maximum of how many unique vertex-UV
                 * maps there could be.  Normally the number will be much closer
                 * to the vertex count if the UVs have been welded.
                 */
                vertex     = new csVector3[count_uv];
                animvertex = new csVector3[count_uv];
                vertnorms  = new csVector3[count_uv];

                count_vert = countelem;

                if (strlen(countname)>3)  // if subscript is here
                {
                    return LoadVertexList(file,tok);  // also partial for large models
                }
                return true;
            }
            if (!strncmp(countname,".ed",3))           // Edge list
            {
                csPrintf(" Need %d elements of %s.\n",countelem,(const char *)countname );

                edgestart = new int[countelem];
                edgestop  = new int[countelem];

                count_edge = countelem;

                if (strlen(countname)>3)  // if subscript is here
                {
                    return LoadEdgeList(file,tok);    // also partial for large models
                }
                return true;
            }
            if (!strncmp(countname,".fc",3))           // Face list
            {
                csPrintf(" Need %d elements of %s.\n",countelem,(const char *)countname );

                faceedge = new csTriangle[countelem];
                tri_normals = new csVector3[countelem];
                uv1 = new int[countelem];
                uv2 = new int[countelem];
                uv3 = new int[countelem];

                count_face = countelem;

                if (strlen(countname)>3)  // if subscript is here
                {
                    return LoadFaceList(file,tok);    // also partial for large models
                }

                return true;
            }
        }
        else if ((const char *)countname && !strncmp(tok,countname,strlen(countname)) ) // often 2nd attr is enumeration of count of first attr
        {
            // we used partial equals above because counted attribute will have [subscript:range] after it.
            
            if (countname == ".uvst[0].uvsp") // UV mapping for vertices
            {
                return LoadUVMap(file,tok); // actually only partial for large models per line
            }
            if (countname == ".vt")           // Vertex list
            {
                return LoadVertexList(file,tok);  // also partial for large models
            }
            if (countname == ".ed")             // Edge list
            {
                return LoadEdgeList(file,tok);    // also partial for large models
            }
            if (countname == ".fc")             // Poly face list
            {
                return LoadFaceList(file,tok);    // also partial for large models
            }
        }
        else if (tok == ";" || tok == "")
        {
            return true;  // completed attribute
        }
    }
    return false;
}

bool NodeMesh::GetArrayRange(csString& token,int& startindex,int& stopindex)
{
    char buff[1000];
    strcpy(buff,token);  // need writeable copy

    char *start = strrchr(buff,'[');
    if (!start)
        return false;

    char *colon = strchr(start,':');
    if (!colon)
        return false;

    *colon = 0;
    startindex = atoi(start+1);
    stopindex  = atoi(colon+1);

    if (startindex < stopindex)
        return true;
    else
        return false;
}

bool NodeMesh::LoadUVMap(MayaInputFile& file,csString& token)
{
    csPrintf("  UV Map:\n");
    csString tok;
    int start,stop;

    // format expected here is [start:stop] n1 n2 n3...

    if (!GetArrayRange(token,start,stop))
    {
        file.SetError("Could not get UV Map array range.");
        return false;
    }

    file.GetToken(tok);
    if (tok == "-type")
    {
        type = file.GetType();
    }
    else
        file.PushToken(tok);

    csPrintf("   Loading UV Map values from %d to %d.\n",start,stop);

    for (int i=start; i<=stop; i++)
    {
        if (!file.GetFloat(type,u[i]))
        {
            file.SetError("Could not read number from file in UV Map.");
            return false;
        }
        if (!file.GetFloat(type,v[i]))
        {
            file.SetError("Could not read 2nd number from file in UV Map.");
            return false;
        }
    }
    return true;
}

bool NodeMesh::LoadVertexList(MayaInputFile& file,csString& token)
{
    csString tok;
    int start,stop;

    // format expected here is [start:stop] x1 y1 z1 x2 y2...

    if (!GetArrayRange(token,start,stop))
    {
        file.SetError("Could not get Vertex List array range.");
        return false;
    }
    
    file.GetToken(tok);
    if (tok == "-type")
    {
        type = file.GetType();
    }
    else
        file.PushToken(tok);

    csPrintf("   Loading Vertex Coordinates from %d to %d.\n",start,stop);

    for (int i=start; i<=stop; i++)
    {
        if (!file.GetFloat(type,vertex[i].x))
        {
            file.SetError("Could not read first number from file in Vertex List.");
            return false;
        }
        if (!file.GetFloat(type,vertex[i].y))
        {
            file.SetError("Could not read 2nd number from file in Vertex List.");
            return false;
        }
        if (!file.GetFloat(type,vertex[i].z))
        {
            file.SetError("Could not read 3rd number from file in Vertex List.");
            return false;
        }
        else
        {
            // Must reverse because Maya uses opposite Z orientation
            vertex[i].z = -vertex[i].z;
        }
	vertex[i] -= translate;
	vertex[i].x *= scale.x;
	vertex[i].y *= scale.y;
	vertex[i].z *= scale.z;
    }
    return true;
}

bool NodeMesh::LoadEdgeList(MayaInputFile& file,csString& token)
{
    csString tok;
    int start,stop;

    // format expected here is [start:stop] x1 y1 z1 x2 y2...

    if (!GetArrayRange(token,start,stop))
    {
        file.SetError("Could not get Edge List array range.");
        return false;
    }
    
    file.GetToken(tok);
    if (tok == "-type")
    {
        type = file.GetType();
    }
    else
        file.PushToken(tok);  // if not type then let the loop see this one

    csPrintf("   Loading Edge Vertices from %d to %d.\n",start,stop);

    for (int i=start; i<=stop; i++)
    {
        if (!file.GetToken(tok))
        {
            file.SetError("Could not read first number from file in Edge List.");
            return false;
        }

        edgestart[i] = atoi(tok);

        if (!file.GetToken(tok))
        {
            file.SetError("Could not read 2nd number from file in Edge List.");
            return false;
        }

        edgestop[i] = atoi(tok);

        if (!file.GetToken(tok))    // this 3rd token is required because Maya edges have 3 numbers per edge for some reason.
        {
            file.SetError("Could not read 3rd token from file in Edge List.");
            return false;
        }  // don't save it, just skip it.
    }

    return true;
}

bool NodeMesh::LoadFaceList(MayaInputFile& file,csString& token)
{
    csString tok;
    int start,stop;

    if (!GetArrayRange(token,start,stop))
    {
        file.SetError("Could not get Face List array range.");
        return false;
    }
    
    file.GetToken(tok);
    if (tok == "-type")
    {
        file.GetToken(tok);
        if (tok != "polyFaces")
        {
            file.SetError("-type specifier on Faces must be 'polyFaces'.");
            return false;
        }
    }
    else
        file.PushToken(tok);  // needed in for loop

    csPrintf("   Loading Face values from %d to %d.\n",start,stop);

    for (int i=start; i<=stop; i++)
    {
        /**
         * format here is:
         * 	f 3 0 1 2           // face, sides, edgeindex, edgeindex,edgeindex
         *  mu 0 3 0 1 2        // uv, index (always 0), sides, uvindex, uvindex,uvindex
         *  ...
         */

        if (!file.GetToken(tok))
        {
            file.SetError("Unexpected end of file in poly face list.");
            return false;
        }
        if (tok != "f")
        {
            file.SetError("Required token 'f' next in poly face list.");
            return false;
        }

        if (!file.GetToken(tok))
        {
            file.SetError("Unexpected end of file in poly face list.");
            return false;
        }
        if (atoi(tok) != 3)
        {
            file.SetError("maya2spr only supports triangles as faces right now.");
            return false;
        }

        if (!file.GetToken(tok))
        {
            file.SetError("Unexpected end of file in poly face list.");
            return false;
        }
        faceedge[i].a = atoi(tok);

        if (!file.GetToken(tok))
        {
            file.SetError("Unexpected end of file in poly face list.");
            return false;
        }
        faceedge[i].b = atoi(tok);

        if (!file.GetToken(tok))
        {
            file.SetError("Unexpected end of file in poly face list.");
            return false;
        }
        faceedge[i].c = atoi(tok);

        if (!file.GetToken(tok))
        {
            file.SetError("Unexpected end of file in poly face list.");
            return false;
        }
        if (tok != "mu")
        {
            file.SetError("Required token 'mu' next in poly face list.");
            return false;
        }

        if (!file.GetToken(tok))
        {
            file.SetError("Unexpected end of file in poly face list.");
            return false;
        }
        if (atoi(tok) != 0)
            return false;

        if (!file.GetToken(tok))
        {
            file.SetError("Unexpected end of file in poly face list.");
            return false;
        }
        if (atoi(tok) != 3)
        {
            file.SetError("maya2spr only supports triangles as faces right now.");
            return false;
        }

        if (!file.GetToken(tok))
        {
            file.SetError("Unexpected end of file in poly face list.");
            return false;
        }
        uv1[i] = atoi(tok);

        if (!file.GetToken(tok))
        {
            file.SetError("Unexpected end of file in poly face list.");
            return false;
        }
        uv2[i] = atoi(tok);

        if (!file.GetToken(tok))
        {
            file.SetError("Unexpected end of file in poly face list.");
            return false;
        }
        uv3[i] = atoi(tok);
    }

    return true;
}

int  NodeMesh::GetStartVertex(int edge,int face)
{
    switch(edge)
    {
        case 0:
        {
            if (faceedge[face].a >= 0)
                return edgestart[faceedge[face].a];
            else
                return edgestop[-faceedge[face].a-1];  // negative indexes are reversed edges
        }
        case 1:
        {
            if (faceedge[face].b >= 0)
                return edgestart[faceedge[face].b];
            else
                return edgestop[-faceedge[face].b-1];  // negative indexes are reversed edges
        }
        case 2:
        default:
        {
            if (faceedge[face].c >= 0)
                return edgestart[faceedge[face].c];
            else
                return edgestop[-faceedge[face].c-1];  // negative indexes are reversed edges
        }
    }
}

int  NodeMesh::GetStopVertex(int edge,int face)
{
    switch(edge)
    {
        case 0:
        {
            if (faceedge[face].a >= 0)
                return edgestop[faceedge[face].a];
            else
                return edgestart[-faceedge[face].a-1];  // negative indexes are reversed edges
        }
        case 1:
        {
            if (faceedge[face].b >= 0)
                return edgestop[faceedge[face].b];
            else
                return edgestart[-faceedge[face].b-1];  // negative indexes are reversed edges
        }
        case 2:
        default:
        {
            if (faceedge[face].c >= 0)
                return edgestop[faceedge[face].c];
            else
                return edgestart[-faceedge[face].c-1];  // negative indexes are reversed edges
        }
    }
}

int  NodeMesh::GetUV(int edge,int face)
{
    switch(edge)
    {
        case 0: return uv1[face];
        case 1: return uv2[face];
        case 2: return uv3[face];
        default:
            return -1000000; // cause crash
    }
}

bool NodeMesh::FixupUniqueVertexMappings()
{
    BinaryTree<VertUVCombo> verts;

    CSVerts = new VertUV[count_uv];  // count_uv is theoretical max # of verts

    CSPoly[0] = new int[count_face];
    CSPoly[1] = new int[count_face];
    CSPoly[2] = new int[count_face];
    countcsverts = 0;

    for (int face=0; face<count_face; face++)
    {
        for (int edge=0; edge<3; edge++)
        {
            VertUVCombo next,*found;
            
            next.vert  = GetStartVertex(edge,face);
            next.uvmap = GetUV(edge,face);
            
            found = verts.Find(&next);
            if (found)
            {
                CSPoly[edge][face]=found->index;
            }
            else
            {
                CSVerts[countcsverts].vertex.x = animvertex[next.vert].x;
                CSVerts[countcsverts].vertex.y = animvertex[next.vert].y;
                CSVerts[countcsverts].vertex.z = animvertex[next.vert].z;
                CSVerts[countcsverts].normal.x = vertnorms[next.vert].x;
                CSVerts[countcsverts].normal.y = vertnorms[next.vert].y;
                CSVerts[countcsverts].normal.z = vertnorms[next.vert].z;
                CSVerts[countcsverts].u = u[next.uvmap];
                CSVerts[countcsverts].v = v[next.uvmap];
                
                VertUVCombo *combo = new VertUVCombo;
                combo->vert  = next.vert;
                combo->uvmap = next.uvmap;
                combo->index = countcsverts;
                
                CSPoly[edge][face] = combo->index;
                
                verts.Add(combo,true);
                countcsverts++;
            }
        }
    }
    return true;
}

bool NodeMesh::WriteVertices(FILE *f)
{
    for (int i=0; i<countcsverts; i++)
    {
        csFPrintf(f,"        <v x=\"%f\" y=\"%f\" z=\"%f\" u=\"%f\" v=\"%f\"  nx=\"%f\" ny=\"%f\" nz=\"%f\" />\n",
                CSVerts[i].vertex.x,
                CSVerts[i].vertex.y,
                CSVerts[i].vertex.z,
                CSVerts[i].u,
                CSVerts[i].v,
                CSVerts[i].normal.x,
                CSVerts[i].normal.y,
                CSVerts[i].normal.z);
    }
    return true;
}

bool NodeMesh::WriteTriangles(FILE *f)
{
    //  We write out the vertices for each polygon in reverse order
    //  so the face normals calculated by CS are facing the correct
    //  direction, due to the Z axis reversal in other places in the code here.
    for (int i=0; i<count_face; i++)
    {
        csFPrintf(f,"        <t v1=\"%d\" v2=\"%d\" v3=\"%d\" />\n",
                CSPoly[2][i],
                CSPoly[1][i],
                CSPoly[0][i]);
    }
    return true;
}

void NodeMesh::ClearCS()
{
    countcsverts = 0;

    delete CSVerts;

    delete CSPoly[0];
    delete CSPoly[1];
    delete CSPoly[2];
}

void NodeMesh::ApplyAnimDeltas(NodeAnimCurveTL *anim,int frame)
{
    if (!anim)
        return;

    if (count_vert != anim->totalcurves/3)
        return;

    if (frame > anim->GetFrames())
        return;

    int i;

    // Maya stores anim curves as deltas relative to the first frame.
    for (i=0; i<count_vert; i++)
    {
        animvertex[i].x = vertex[i].x + anim->animX[i][frame];
        animvertex[i].y = vertex[i].y + anim->animY[i][frame];
        animvertex[i].z = vertex[i].z + anim->animZ[i][frame];
    }

    // calculate triangle normals
    // get the cross-product of 2 edges of the triangle and normalize it
    for (i = 0; i < count_face; i++)
    {
        csVector3 ab = vertex[GetStartVertex(1,i)] - vertex[GetStartVertex(0,i)];
        csVector3 bc = vertex[GetStartVertex(2,i)] - vertex[GetStartVertex(1,i)];
        tri_normals[i] = bc % ab;

        float norm = tri_normals[i].Norm ();
        if (norm)
            tri_normals[i] /= norm;

    }

    // calculate vertex normals, by averaging connected triangle normals
    for (i = 0; i < count_vert; i++)
    {
        csVector3 n;
        n.Set (0,0,0);
                
        for (int face = 0; face < count_face ; face++)
        {
            if (GetStartVertex(0,face) == i ||
                GetStartVertex(1,face) == i ||
                GetStartVertex(2,face) == i)    // this poly uses this vertex
            {
                n += tri_normals [face];
            }
        }
        // Now normalize poly.
        float norm = n.Norm ();
        if (norm)
            n /= norm;

        if (n.IsZero())    // Arbitrary normal for vertex not used by ANY poly
        {
            n.Set(1,0,0);
            csPrintf("Warning:  Vertex %d is not used in any face or normals are not specified.\n",i);
        }

        vertnorms[i] = n;  // Save so it is written out with the frame.
    }
}

float NodeMesh::GetDisplacement(NodeAnimCurveTL *anim,int frame,int frame2,int vert)
{
    return anim->animZ[vert][frame2] - anim->animZ[vert][frame];
}







/*--------------------------------------------------------------------------*/

void NodeFile::PrintStats(FILE *s,int level)
{
    PrintSpaces(s,level);
    csFPrintf(s,"File '%s':\n",(const char *)name);

    PrintSpaces(s,level);
    csFPrintf(s," (%s)\n",(const char *)filename);

    DAGNode::PrintStats(s,level+1);
}

bool NodeFile::Write(FILE *f)
{
    char name[500];
    
    const char *start = strrchr(filename,'/');
    if (!start)
        start = (const char *)filename;

    strcpy(name,start);

    char *end = strchr(name,'.');
    if (end)
        *end = 0;  // null terminate at dot.

    csFPrintf(f,"      <material>%s</material> /* %s */\n",name+1,(const char *)filename);

    return true;
}

bool NodeFile::Load(MayaInputFile& file)
{
    csString tok;

    bool stop = false;

    while (!stop)
    {
        file.GetToken(tok);
        
        if (tok == "-n")  // set name
        {
            file.GetToken(tok);
            SetName(tok);
            csPrintf("File Name is '%s'\n",(const char *)tok);
        }
        else if (tok == "setAttr")
        {
            if (!LoadAttr(file))
                return false;
        }
        else if (tok == "createNode")
        {
            file.PushToken(tok);
            break;
        }
    }
    return true;
}


bool NodeFile::LoadAttr(MayaInputFile& file)
{
    csString tok;
    bool stop = false;

    while (!stop)
    {
        file.GetToken(tok);
        
        if (tok == ".ftn")
        {
            return LoadFilenameAttr(file);
        }
        else if (tok == "createNode")
        {
            file.PushToken(tok);
            return true;
        }
    }
    return false;
}


bool NodeFile::LoadFilenameAttr(MayaInputFile& file)
{
    csString tok;

    file.GetToken(tok);

    if (tok == "-type")
    {
        file.GetToken(tok);
        if (tok != "string")
        {
            file.SetError("Filenames must be of type 'string'.");
            return false;
        }

        file.GetToken(tok);
    }
    else
        file.PushToken(tok);

    if (tok != "(" && tok != "\"")
    {
        file.SetError("Filenames for textures must be within ()'s or in quotes.");
        return false;
    }

    file.GetToken(tok);  // This is filename, within parentheses.
    filename = tok;
    csPrintf(" Filename was <%s>\n",(const char *)filename);

    file.GetToken(tok);
    if (tok == ":") // check for special case of path including drive letter
    {
	filename.Append(tok);   // append the colon to the drive letter
	file.GetToken(tok);	// get the rest of the filename
	filename.Append(tok);   // append the part of the filename behind the colon
	file.GetToken(tok);     // prefetch the next token
    }

    if (tok != ")" && tok != "\"")
    {
        file.SetError("Filenames for textures must be within ()'s or quotes.");
        return false;
    }
    else
    {
        return true;
    }
}








/*--------------------------------------------------------------------------*/

NodeAnimCurveTL::NodeAnimCurveTL(int vertices)
{
    totalcurves = 0;
    expected_verts = vertices;

    animX = new float* [ vertices ];
    animY = new float* [ vertices ];
    animZ = new float* [ vertices ];
}

void NodeAnimCurveTL::PrintStats(FILE *s,int level)
{
    PrintSpaces(s,level);
    csFPrintf(s,"Animation '%s':\n",(const char *)name);

    PrintSpaces(s,level);
    csFPrintf(s," (%d animated vertices)\n",totalcurves/3);
    PrintSpaces(s,level);
    csFPrintf(s," (%d frames)\n",frames);

    DAGNode::PrintStats(s,level+1);
}

void NodeAnimCurveTL::CreateDefault()
{
    frames = 1;

    for (int i=0; i<expected_verts; i++)
    {
	animX[i] = new float[frames]; 
	animX[i][0] = 0;
	animY[i] = new float[frames]; 
	animY[i][0] = 0;
	animZ[i] = new float[frames];
	animZ[i][0] = 0;
    }
    totalcurves = expected_verts * 3;
}

bool NodeAnimCurveTL::Load(MayaInputFile& file)
{
    csString tok;

    bool stop = false;

    while (!stop)
    {
        file.GetToken(tok);
        
        if (tok == "-n")  // set name
        {
            file.GetToken(tok);
            SetName(tok);
//            csPrintf("Anim Curve Name is '%s'...",(const char *)tok);

            GetIndexCoord(tok,index,coord);
        }
        else if (tok == "setAttr")
        {
            if (!LoadAttr(file))
                return false;
        }
        else if (tok == "createNode" || tok=="select")
        {
            file.PushToken(tok);
            break;
        }
    }
    totalcurves++;

    return true;
}

bool NodeAnimCurveTL::GetIndexCoord(csString& tok,int& index,int& coord)
{
    char buff[1000];
    strcpy(buff,tok);  // writeable copy

    char *start = strstr(buff,"pnts_");
    if (!start)
        return false;

    index = atoi(start+5);

    switch(buff[strlen(buff)-1])
    {
    case 'x': coord = 0;    return true;
    case 'y': coord = 1;    return true;
    case 'z': coord = 2;    return true;
    default:                return false;
    }
}

bool NodeAnimCurveTL::LoadAttr(MayaInputFile& file)
{
    csString tok;
    bool stop = false;

    while (!stop)
    {
        file.GetToken(tok);
        
        if (tok == "-s")    // size attribute
        {
            file.GetToken(tok);
            frames = atoi(tok);

            switch(coord)
            {
            case 0: animX[index] = new float[frames]; break;
            case 1: animY[index] = new float[frames]; break;
            case 2: animZ[index] = new float[frames]; break;
            }

            return LoadCurveAttr(file);
        }
        else if (tok == "createNode" || tok=="select")
        {
            file.PushToken(tok);
            return true;
        }
    }
    return false;
}

bool NodeAnimCurveTL::LoadCurveAttr(MayaInputFile& file)
{
    char *coords = "xyz";
    csPrintf("Processing %d frame anim curve for %c coord of vertex %d.\n",frames,coords[coord],index);

    csString tok;
    file.GetToken(tok);

    if (strncmp(tok,".ktv[",5))
    {
        file.SetError("AnimCurveTL path attributes must have .ktv[x] in front of them.");
        return false;
    }
    for (int i=0; i<frames; i++)
    {
        file.GetToken(tok);
        if (i+1 != atoi(tok) )
        {
            file.SetError("AnimCurveTL frame number does not match specified number.");
            return false;
        }
        float fl;
        file.GetFloat(2,fl);

        switch(coord)
        {
            case 0: animX[index][i] = fl; break;
            case 1: animY[index][i] = fl; break;
            case 2: animZ[index][i] = -fl; break;
            default: 
            {
                file.SetError("AnimCurveTL coord not x,y or z.");
                return false;
            }
        }
    }
    return true;
}

