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
}

CSWriter::CSWriter(FILE* f, Lib3dsFile* data3d)
    : Writer(f), p3dsFile(data3d)
{
}

CSWriter::~CSWriter()
{
}

/**
 * Outputs the header with TEXTURES, MATERIALS, PLUGINS.
 */
void CSWriter::OutpHeaderCS()
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

/**
 * Outputs all the objects present in the 3ds file.
 * Based on the object name we create MESHOBJ or PART.
 *
 */
void CSWriter::OutpObjectsCS (bool lighting)
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
                Write ("PART '%s' (", p3dsMesh->name);
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
                WriteL (")"); WriteL (""); // end PART
		UnIndent();
                WriteL (")"); WriteL (""); // end MESHOBJ
		UnIndent();               		
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

    // get the number of vertices in the current mesh
    int numVertices = p3dsMesh->points;
    int i;

    // vertexes pointer
    Lib3dsPoint *pCurPoint = p3dsMesh->pointL;
    Lib3dsTexel *pCurTexel = p3dsMesh->texelL;

    for (i = 0 ; i < numVertices ; i++ )
    {
      // index to the position on the list using index
      float *xyz = pCurPoint->pos;
      if (flags & FLAG_SPRITE)
      {
        float u = 0;
        float v = 0;
        if (pCurTexel != NULL)
        {
          u = pCurTexel[0][0];
          v = pCurTexel[0][1];
          pCurTexel++;
        }

        Write ("V(%g,%g,%g:", xyz[0], xyz[1], xyz[2]);
        WriteL ("%g,%g)",u, (flags & FLAG_SWAP_V ? 1.-v : v));
      }
      else
      {
        WriteL ("VERTEX (%g,%g,%g)    ; %d", xyz[0], xyz[1], xyz[2], i);
      }

      // go to next vertex and texel
      pCurPoint++;
    }

    // <--output faces-->

    // get the number of faces in the current mesh
    int numFaces = p3dsMesh->faces;
    Lib3dsFace *pCurFace = p3dsMesh->faceL;
    Lib3dsTexel *pTexelList = p3dsMesh->texelL;

    // output faces
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

        WriteL ("POLYGON 'x%d_%d'  (VERTICES (%d,%d,%d)%s",
	             numMesh, i, pCurFace->points[0],
		     pCurFace->points[1], pCurFace->points[2],
	             lighting ? "" : " LIGHTING (no)");
	Indent();
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

        WriteL (")"); // close polygon
	UnIndent();
      }

      // go to next face
      pCurFace++;
    }

    if (meshobj) {
	WriteL(")"); WriteL(""); // close PARAMS tag
	UnIndent();
	WriteL(")"); WriteL(""); // close MESHOBJ
	UnIndent();             	
        meshobj = false;
    } else {
	WriteL (")"); WriteL("");
	UnIndent();
    }

    // increment mesh count
    numMesh++;
    p3dsMesh = p3dsMesh->next;
  } // ~end while (p3dsMesh)

  // if working on static object closes MESHOBJECT
  if (part)
  {
     WriteL("("); WriteL(""); // close PARAMS
     UnIndent();
     WriteL("("); WriteL(""); // close MESHOBJ
     UnIndent();                                  
     part = false;
  }

  if (flags & FLAG_SPRITE)
  {
    WriteL ("ACTION 'default' (F (f1,1000))");
    WriteL (")");
    UnIndent();
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
        WriteL (")");
	UnIndent();
      }

      pCurLight = pCurLight->next;
    }

    WriteL (")");
    UnIndent();
    WriteL (")");
    UnIndent();
  }
}

