/*
 *  CS Object output
 *  Author: Luca Pancallo 2000.09.28
 */

#include "3dsout.h"
#include "3ds2lev.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

// includes for lib3ds
#include <lib3ds/camera.h>
#include <lib3ds/file.h>
#include <lib3ds/io.h>
#include <lib3ds/light.h>
#include <lib3ds/material.h>
#include <lib3ds/matrix.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <lib3ds/vector.h>

extern int flags;

//---------------------------------------------------------------------------
Writer::Writer (const char* fname)
    : indentlevel(0)
{
    file = fopen(fname, "w");
    indented = false;
}

Writer::Writer(FILE* f)
    : indented(false), indentlevel(0), file(f)
{
}

Writer::~Writer()
{
    fclose(file);
}

void Writer::Indent(int sp)
{
    indentlevel+=sp;
}

void Writer::UnIndent(int sp)
{
    if (indentlevel-sp<0)
	indentlevel=0;
    else
	indentlevel-=sp;
}

void Writer::WriteL(const char* line, ...)
{
    va_list args;
    va_start(args, line);

    WriteV(line,args);    
    
    fputc('\n', file);
    indented = false;

    va_end(args);
}

void Writer::WriteV(const char* line, va_list args)
{
    if (!indented)
    {
	for (int i=0;i<indentlevel;i++)
	    fputc(' ', file);
	indented = true;
    }

    vfprintf(file, line, args);
}

void Writer::Write(const char* line, ...)
{
    va_list args;
    va_start(args, line);

    WriteV(line, args);

    va_end(args);
}

//---------------------------------------------------------------------------

CSWriter::CSWriter(const char* filename, Lib3dsFile* data3d)
    : Writer (filename), p3dsFile(data3d)
{
  newpointmap = NULL;
}

CSWriter::CSWriter(FILE* f, Lib3dsFile* data3d)
    : Writer(f), p3dsFile(data3d)
{
  if (newpointmap)
    delete[] newpointmap;
}

CSWriter::~CSWriter()
{
}

/**
 * Outputs the header with TEXTURES, MATERIALS, PLUGINS.
 */
void CSWriter::WriteHeader ()
{
  if (flags & FLAG_SPRITE)
  {
    WriteL ("MESHFACT 'sprite' (");
    Indent();
    WriteL ("PLUGIN ('crystalspace.mesh.loader.factory.sprite.3d')");
    WriteL ("PARAMS (");
    Indent();
  }
  else
  {
    // extracts all unique textures
    char *textures[10000];
    int numTextures = 0;
    int j;

    // set the current mesh to the first in the file
    Lib3dsMesh *p3dsMesh = p3dsFile->meshes;
    // as long as we have a valid mesh...
    while( p3dsMesh )
    {
      // search if already present
      bool found = false;

      for (j=0; j<numTextures; j++) {
        if (strcmp(p3dsMesh->faceL->material, textures[j])==0) {
          found = true;
          break;
        }
      }
      // if not present add it!
      if (!found) {
        textures[numTextures] = p3dsMesh->faceL->material;
        numTextures++;
      }

      // go to next mesh
      p3dsMesh = p3dsMesh->next;
    }

    WriteL ("WORLD (");
    Indent();

    WriteL ("TEXTURES (");
    Indent();
    // set the current mesh to the first in the file
    for (j=0; j<numTextures; j++)
        WriteL ("TEXTURE '%s' (FILE (%s)) ",textures[j], textures[j]);
    
    UnIndent();
    WriteL (")"); 
    WriteL("");

    WriteL ("MATERIALS (");
    Indent();

    for (j=0; j<numTextures; j++)
        WriteL ("MATERIAL '%s' (TEXTURE ('%s'))",textures[j], textures[j]);
    
    UnIndent();
    WriteL (")");
    WriteL ("");
    
    WriteL ("PLUGINS (");
    Indent();
    WriteL ("PLUGIN 'thing' ('crystalspace.mesh.loader.thing')");
    UnIndent();
    WriteL (")");
    WriteL(""); 
    WriteL ("SECTOR 'room' (");
    Indent();
  }
}

void CSWriter::WriteFooter ()
{
  if (flags & FLAG_SPRITE)
  {
    WriteL ("ACTION 'default' (F (f1,1000))");
    UnIndent();
    WriteL (")"); // close MESHFACT
  }
  else
  {
    WriteL ("CULLER ('static')");

    Lib3dsLight *pCurLight = p3dsFile->lights;
    // output lights
    while (pCurLight) {

      // discart spot-lights
      if (pCurLight->spot_light) {
        fprintf (stderr, "Spotlight are not supported. Light '%s' will not be imported in CS\n", pCurLight->name);
      // convert omni-lights
      } else {
        WriteL ("LIGHT (");
	Indent();
        WriteL ("CENTER (%g,%g,%g)",pCurLight->position[0], pCurLight->position[1], pCurLight->position[2]);
        WriteL ("RADIUS (%g)",pCurLight->outer_range);
        WriteL ("COLOR (%g,%g,%g)",pCurLight->color[0], pCurLight->color[1], pCurLight->color[2]);
	UnIndent();
	WriteL (")");
      }

      pCurLight = pCurLight->next;
    }

    UnIndent();
    WriteL (")"); // close SECTOR
    UnIndent();
    WriteL (")"); // clode WORLD
  }
}

void CSWriter::WriteVertices (Lib3dsMesh* mesh)
{
  if (flags & FLAG_SPRITE)
  {
    for (unsigned int vn = 0; vn < mesh->points; vn++)
    {
      float *xyz = mesh->pointL[vn].pos;
      Lib3dsTexel* texel = &mesh->texelL[vn];
      float u = 0, v = 0;

      if (texel)
      {
	u = texel[0][0];
	v = texel[0][1];
      }

      Write ("V(%g,%g,%g:", xyz[0], xyz[1], xyz[2]);
      WriteL ("%g,%g)",u, (flags & FLAG_SWAP_V ? 1.-v : v));
    }
  }
  else
  {
    if (newpointmap)
      delete[] newpointmap;
    newpointmap = new unsigned int[mesh->points];
    memset (newpointmap, 0, sizeof(unsigned int) * mesh->points);

    unsigned int newpoint = 0;
    for (unsigned int v = 0; v < mesh->points; v++)
    {
      // doubled point? then do nothing
      if (newpointmap[v] != 0)
	continue;
      
      float* xyz1 = mesh->pointL[v].pos;
      for (unsigned int v2 = v+1; v2 < mesh->points; v2++)
      {
	float* xyz2 = mesh->pointL[v2].pos;

	if (xyz1[0] == xyz2[0] && xyz1[1]==xyz2[1] && xyz1[2] == xyz2[2])
	  newpointmap[v2] = newpoint;
      }
      newpointmap[v] = newpoint;
      WriteL ("VERTEX (%g,%g,%g)    ; %d", xyz1[0], xyz1[1], xyz1[2], newpoint);
      newpoint++;
    }
  }
}

/**
 * Outputs all the objects present in the 3ds file.
 * Based on the object name we create MESHOBJ or PART.
 *
 */
void CSWriter::WriteObjects (bool lighting)
{
  Lib3dsMesh *p3dsMesh = p3dsFile->meshes;

  if (flags & FLAG_SPRITE)
  {
    WriteL ("; Spritename: '%s'", p3dsMesh->name);
  }
  else
  {
    // count meshes
    int numMeshes = 0;
    int n;
    while (p3dsMesh)
    {
      numMeshes++;
      p3dsMesh = p3dsMesh->next;
    }
    // build an array with all meshes
    Lib3dsMesh* p3dsMeshArray = new Lib3dsMesh[numMeshes];
    p3dsMesh = p3dsFile->meshes;
    for (n=0; n<numMeshes; n++)
    {
      p3dsMeshArray[n] = *p3dsMesh;
      p3dsMesh = p3dsMesh->next;
    }

    // Reorder the objects to have all "_s_" first.
    // set the current mesh to the first in the file
    for (n=0; n<numMeshes; n++)
    {
        // if not static...
        if (!strstr( ((Lib3dsMesh *)&p3dsMeshArray[n])->name, "_s_"))
	{
          // search a static and swap
          for (int j=n+1; j<numMeshes; j++)
	  {
             if (strstr( ((Lib3dsMesh *)&p3dsMeshArray[j])->name, "_s_"))
	     {
                Lib3dsMesh tmp = p3dsMeshArray[j];
                p3dsMeshArray[j] = p3dsMeshArray[n];
                p3dsMeshArray[n] = tmp;
                break;
             }
          }
        }
    }

    // assign reordered vector to main Lib3ds struct
    p3dsMesh = p3dsFile->meshes;
    for (n=0; n<numMeshes-1; n++) {
      ((Lib3dsMesh *)&p3dsMeshArray[n])->next=&p3dsMeshArray[n+1];
    }
    ((Lib3dsMesh *)&p3dsMeshArray[n])->next=0;

    p3dsFile->meshes = p3dsMeshArray;
  }

  // iterate on all meshes
  p3dsMesh = p3dsFile->meshes;

  bool staticObj = false;
  bool part = false;
  bool meshobj = false;
  int numMesh = 0;
  while (p3dsMesh)
  {

    if (flags & FLAG_SPRITE)
    {
    }
    else
    {
        // on "_s_" decide if MESHOBJ or PART
        if (strstr(p3dsMesh->name, "_s_"))
        {
            if (!staticObj)
            {
                WriteL ("MESHOBJ 'static' (");
		Indent();
		WriteL ("PLUGIN ('thing')");
                WriteL ("ZFILL()");
	        WriteL ("PRIORITY('wall')");
		WriteL ("PARAMS(");
		Indent();
		WriteL ("VISTREE()");
		Write ("; Object Name: '%s'" , p3dsMesh->name);
                WriteL (" Faces: %6d faces ", (int)p3dsMesh->faces);
                WriteL ("PART '%s' (", p3dsMesh->name);
		Indent();
                staticObj = true;
            }
            else
            {
                WriteL ("PART '%s' (", p3dsMesh->name);
		Indent();
            }
            part = true;
        }
        // else always MESHOBJ
        else
        {
            // close previous PART and MESHOBJ if present
            if (part)
            {
		UnIndent();
                WriteL (")"); WriteL (""); // end PART
		UnIndent();
                WriteL (")"); WriteL (""); // end MESHOBJ
		part = false;
            }
            Write ("; Object Name : %s ", p3dsMesh->name);
            WriteL (" Faces: %6d faces ", (int)p3dsMesh->faces);

            WriteL ("MESHOBJ '%s' (", p3dsMesh->name);
	    Indent();
	    WriteL ("PLUGIN ('thing')");
	    WriteL ("ZUSE ()");
            // handles transparent objects
            if (strstr(p3dsMesh->name, "_t_"))
                WriteL ("PRIORITY('alpha')");
            else
                WriteL ("PRIORITY('object')");
            WriteL ("PARAMS (");
	    Indent();

            meshobj = true;
        }
    }
    WriteL ("MATERIAL ('%s')", p3dsMesh->faceL->material);

    // <--output vertexes-->

    WriteVertices(p3dsMesh);

    // <--output faces-->

    // get the number of faces in the current mesh
    int numFaces = p3dsMesh->faces;
    Lib3dsFace *pCurFace = p3dsMesh->faceL;
    Lib3dsTexel *pTexelList = p3dsMesh->texelL;

    // output faces
    int i;
    for (i=0; i<numFaces; i++)
    {
      if (flags & FLAG_SPRITE)
      {
        WriteL ("TRIANGLE (%d,%d,%d)",
		pCurFace->points[0], pCurFace->points[1], pCurFace->points[2]);
      }
      else
      {
        // if a light is present in the 3ds file force LIGHTNING (yes)
        if (p3dsFile->lights)
            lighting = true;

        WriteL ("POLYGON 'x%d_%d'  (", numMesh, i);
	Indent();
	unsigned int p1,p2,p3;
	p1 = newpointmap[pCurFace->points[0]];
	p2 = newpointmap[pCurFace->points[1]];
	p3 = newpointmap[pCurFace->points[2]];
	Write ("VERTICES (%d,%d,%d)", p1, p2, p3);
	if (!lighting)
	  Write (" LIGHTING (no)");
	WriteL("");

        if (pTexelList)
        {
          Lib3dsTexel *mapV0 = (Lib3dsTexel*)pTexelList[pCurFace->points[0]];
          float u0 = mapV0[0][0];
          float v0 = mapV0[0][1];
          Lib3dsTexel *mapV1 = (Lib3dsTexel*)pTexelList[pCurFace->points[1]];
          float u1 = mapV1[0][0];
          float v1 = mapV1[0][1];
          Lib3dsTexel *mapV2 = (Lib3dsTexel*)pTexelList[pCurFace->points[2]];
          float u2 = mapV2[0][0];
          float v2 = mapV2[0][1];
          WriteL ("TEXTURE (UV (0,%g,%g,1,%g,%g,2,%g,%g))",
                 u0, (flags & FLAG_SWAP_V ? 1.-v0 : v0),
                 u1, (flags & FLAG_SWAP_V ? 1.-v1 : v1),
                 u2, (flags & FLAG_SWAP_V ? 1.-v2 : v2));
        }

	UnIndent();
        WriteL (")"); // close polygon
      }

      // go to next face
      pCurFace++;
    }

    if (meshobj) {
	UnIndent();
	WriteL(")"); WriteL(""); // close PARAMS tag
	UnIndent();
	WriteL(")"); WriteL(""); // close MESHOBJ
        meshobj = false;
    } else {
	UnIndent();
	WriteL (")");
    }

    // increment mesh count
    numMesh++;
    p3dsMesh = p3dsMesh->next;
  } // ~end while (p3dsMesh)

  // if working on static object closes MESHOBJECT
  if (part)
  {
    UnIndent();
    WriteL(")"); WriteL(""); // close part
    UnIndent();
    WriteL(")"); WriteL(""); // close MESHOBJ
    part = false;
  }
}

