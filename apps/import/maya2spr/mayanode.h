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

#ifndef MAYANODE_H
#define MAYANODE_H

#include <csutil/array.h>
#include <csgeom/vector3.h>
#include <ivideo/graph3d.h>

#include "mayafile.h"

struct DisplacementGroup
{
    int vertex;
    int startframe;
    int stopframe;
};

struct Animation
{
    int startframe;
    csString name;
    int duration;
    csArray<DisplacementGroup> displacements;
};

class DAGNode
{
protected:
    csString name;
    csString parentname;
    DAGNode *parentnode;
    csArray<DAGNode*> children;

    void PrintSpaces(FILE *s,int level)
    {
        for (int i=level; i>0; i--)
            csFPrintf(s," ");
    };

public:
    virtual ~DAGNode () { }

    virtual const char *Parent() { return parentname; };

    void SetParentNode(DAGNode *which)   { parentnode = which; };
    void SetParentName(const char *name) { parentname = name; };
    void SetName(const char *setname)    { name = setname; };

    virtual bool Load(MayaInputFile& )     { return false; };
    virtual bool Write(FILE *)             { return false; };

    DAGNode *Find(const char *)            { return 0; };
    void AddChild(DAGNode *child)
    {
        children.Push (child);
        child->SetParentNode(this);
        child->SetParentName(name);
    };
    virtual void PrintStats(FILE *s,int level);

    virtual bool IsType(const char *) { return false; };
};

class NodeTransform : public DAGNode
{
protected:

    csVector3 trans, rot, scale;

    bool LoadAttr(MayaInputFile& file);
    bool LoadTransformAttr(MayaInputFile& file,csVector3& vec);
    bool LoadRotationAttr(MayaInputFile& file);

public:

    NodeTransform();
    virtual bool Load(MayaInputFile& file);
    virtual bool IsType(const char *type) { return (!strcmp("NodeTransform",type)); };

    void GetTransform(csVector3& t)
    {
	t = trans;
    }
    void GetScale(csVector3& s)
    {
	s = scale;
    }
};


struct VertUV
{
    csVector3 vertex,normal;
    float    u,v;
};

struct VertUVCombo
{
    int vert;
    int uvmap;
    int index;

    bool operator==(VertUVCombo& other)
    {
        return (vert==other.vert && uvmap==other.uvmap);
    };
    bool operator<(VertUVCombo& other)
    {
        if (uvmap<other.uvmap)
            return true;
        if (uvmap>other.uvmap)
            return false;
        if (vert<other.vert)
            return true;
        return false;
    };
};

class NodeAnimCurveTL;

class NodeMesh : public DAGNode
{
protected:

    int      countelem;  // count comes before enumeration
    csString countname;  // name of counted object comes first
    int      type;       // indicator of using double, int, string, etc.

    int     count_uv;    //  how many u,v's are there?
    float   *u,*v;       //  u,v mapping set for all vertices

    int     count_vert;     //  how many x,y,z's follow?
    csVector3 *vertex;      //  x,y,z coords for all vertices
    csVector3 *animvertex;  //  x,y,z coords for all vertices
    csVector3 *vertnorms;   //  Vertex normals for all vertices

    int     count_edge;            // how many edges?
    int     *edgestart,*edgestop;  // edge definitions (index to vertices)

    int     count_face;      // how many faces are there (both e1,e2,e3 and uv1, etc.)
    csTriangle *faceedge;    // edges of triangle faces (index to edges)
    csVector3  *tri_normals; // normals to each poly
    int     *uv1,*uv2,*uv3;  // indices into u[] and v[] for each point
    
    int     countcsverts;
    VertUV  *CSVerts;
    int     *CSPoly[3];

    csVector3 translate;    // Vector to subtract from each vertex point to center object
    csVector3 scale;        // Vector to scale each vertex by once it is centered on 0,0,0

    bool GetArrayRange(csString& token,int& start,int& stop);

    bool LoadAttr(MayaInputFile& file);
    bool LoadUVMap(MayaInputFile& file,csString& token);
    bool LoadVertexList(MayaInputFile& file,csString& token);
    bool LoadEdgeList(MayaInputFile& file,csString& token);
    bool LoadFaceList(MayaInputFile& file,csString& token);

    int  GetUV(int edge,int face);
    int  GetStartVertex(int edge,int face);
    int  GetStopVertex(int edge,int face);

public:
    
    NodeMesh(csVector3& trans,csVector3& scale);

    virtual bool Load(MayaInputFile& file);
    bool WriteVertices(FILE *f);
    bool WriteTriangles(FILE *f);

    virtual void PrintStats(FILE *s,int level);
    virtual bool IsType(const char *type) { return (!strcmp("NodeMesh",type)); };

    int CountVertices(void) { return count_vert; };

    void ApplyAnimDeltas(NodeAnimCurveTL *anim,int frame);
    bool FixupUniqueVertexMappings(void);
    void ClearCS(void);

    float GetDisplacement(NodeAnimCurveTL *anim,int frame,int frame2,int vert);
};



class NodeFile : public DAGNode
{
protected:
    csString filename;

    bool LoadAttr(MayaInputFile& file);
    bool LoadFilenameAttr(MayaInputFile& file);

public:

    virtual bool Load(MayaInputFile& file);
    virtual bool Write(FILE *f);
    virtual void PrintStats(FILE *s,int level);
    virtual bool IsType(const char *type) { return (!strcmp("NodeFile",type)); };
};


class NodeAnimCurveTL : public DAGNode
{
protected:
    int  index,coord,frames,expected_verts;

    bool GetIndexCoord(csString& tok,int& index,int& coord);
    
    bool LoadAttr(MayaInputFile& file);
    bool LoadCurveAttr(MayaInputFile& file);

public:
    int  totalcurves;
    float   **animX,**animY,**animZ;

    NodeAnimCurveTL(int vertices);
    void CreateDefault();
    virtual bool Load(MayaInputFile& file);
    virtual void PrintStats(FILE *s,int level);
    virtual bool IsType(const char *type) { return (!strcmp("NodeAnimCurveTL",type)); };

    int GetFrames(void) { return frames; };
};


#endif
