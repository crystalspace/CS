
/*
 *  CS Object output
 *  Author: Luca Pancallo 2000.09.28
 */

#include "3dsout.h"
#include "3ds2lev.h"
#include <stdio.h>
#include <string.h>
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

/**
 * Outputs the header with TEXTURES, MATERIALS, PLUGINS.
 */
void OutpHeadCS (FILE *o, Lib3dsFile *p3dsFile)
{
  if (flags & FLAG_SPRITE)
  {
    fprintf (o, "MESHFACT 'sprite' (\n");
    fprintf (o, "  PLUGIN ('crystalspace.mesh.loader.factory.sprite.3d')\n");
    fprintf (o, "  PARAMS (\n");
  }
  else
  {

    // extracts all unique textures
    char *textures[100];
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

    fprintf (o, "WORLD (\n");

    fprintf (o, "  TEXTURES (\n");

    // set the current mesh to the first in the file
    for (j=0; j<numTextures; j++)
        fprintf (o, "    TEXTURE '%s' (FILE (%s)) \n",textures[j], textures[j]);

    fprintf (o, "  )\n\n");

    fprintf (o, "  MATERIALS (\n");

    for (j=0; j<numTextures; j++)
        fprintf (o, "    MATERIAL '%s' (TEXTURE ('%s')) \n",textures[j], textures[j]);

    fprintf (o, "  )\n\n");

    fprintf (o, "  PLUGINS (\n");
    fprintf (o, "    PLUGIN 'thing' ('crystalspace.mesh.loader.thing')\n");
    fprintf (o, "    )\n");
    fprintf (o, "  SECTOR 'room' (\n");
  }
}


/**
 * Outputs all the objects present in the 3ds file.
 * Based on the object name we create MESHOBJ or PART.
 *
 */
void OutpObjectsCS (FILE * o, Lib3dsFile *p3dsFile, bool lighting)
{

  Lib3dsMesh *p3dsMesh = p3dsFile->meshes;

  if (flags & FLAG_SPRITE)
  {
    fprintf (o, "  ; '%s'\n", p3dsMesh->name);
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
                fprintf (o, "    MESHOBJ 'static' (\n");
                fprintf (o, "      PLUGIN ('thing')\n");
                fprintf (o, "      ZFILL ()\n");
                fprintf (o, "      PRIORITY('wall')\n");
                fprintf (o, "      PARAMS (\n");
                fprintf (o, "      VISTREE()\n");
                fprintf (o, "; Object Name : %s ", p3dsMesh->name);
                fprintf (o, " Faces: %6d faces \n", (int)p3dsMesh->faces);
                fprintf (o, "      PART '%s' (\n", p3dsMesh->name);
                staticObj = true;
            }
            else
	    {
                fprintf (o, "      PART '%s' (\n", p3dsMesh->name);
            }
            part = true;
        // else always MESHOBJ
        }
	else
	{
            // close previous PART and MESHOBJ if present
            if (part)
	    {
                fprintf (o, "      )\n\n");
                fprintf (o, "    )\n\n");
                part = false;
            }
            fprintf (o, "; Object Name : %s ", p3dsMesh->name);
            fprintf (o, " Faces: %6d faces \n", (int)p3dsMesh->faces);

            fprintf (o, "    MESHOBJ '%s' (\n", p3dsMesh->name);
            fprintf (o, "      PLUGIN ('thing')\n");
            fprintf (o, "      ZUSE ()\n");
            // handles transparent objects
            if (strstr(p3dsMesh->name, "_t_"))
	    {
                fprintf (o, "      PRIORITY('alpha')\n");
            }
	    else
	    {
                fprintf (o, "      PRIORITY('object')\n");
            }
            fprintf (o, "      PARAMS (\n");
        }
    }
    fprintf (o, "        MATERIAL ('%s')\n", p3dsMesh->faceL->material);


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

        fprintf (o, "        V(%g,%g,%g:", xyz[0], xyz[1], xyz[2]);
        fprintf (o, "%g,%g)\n",u, (flags & FLAG_SWAP_V ? 1.-v : v));
      }
      else
      {
        fprintf (o, "        VERTEX (%g,%g,%g)    ; %d\n",
        	  xyz[0], xyz[1], xyz[2], i);
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
        fprintf (o, "        TRIANGLE (%d,%d,%d)\n",
		pCurFace->points[0], pCurFace->points[1], pCurFace->points[2]);
      }
      else
      {
        // if a light is present in the 3ds file force LIGHTNING (yes)
        if (p3dsFile->lights)
            lighting = true;

        fprintf (o, "        POLYGON 'x%d_%d'  (VERTICES (%d,%d,%d)%s\n",
	             numMesh, i, pCurFace->points[0],
		     pCurFace->points[1], pCurFace->points[2],
	             lighting ? "" : " LIGHTING (no)");
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
          fprintf (o, "          TEXTURE (UV (0,%g,%g,1,%g,%g,2,%g,%g))\n",
                 u0, (flags & FLAG_SWAP_V ? 1.-v0 : v0),
                 u1, (flags & FLAG_SWAP_V ? 1.-v1 : v1),
                 u2, (flags & FLAG_SWAP_V ? 1.-v2 : v2));
	}

        fprintf (o, "        )\n"); // close polygon
      }

      // go to next face
      pCurFace++;
    }

    // close last PART
    fprintf (o, "      )\n\n");

    if (part)
    {
       // close PARAMS tag
       fprintf (o, "      )\n\n");

       // close MESHOBJ tag
       fprintf (o, "    )\n\n");
       part = false;
    }

    // increment mesh count
    numMesh++;
    p3dsMesh = p3dsMesh->next;

  } // ~end while (p3dsMesh)

  if (flags & FLAG_SPRITE)
  {
    fprintf (o, "ACTION 'default' (F (f1,1000))\n");
    fprintf (o, "  )\n");
  }
  else
  {
    fprintf (o, "    CULLER ('static')\n");

    Lib3dsLight *pCurLight = p3dsFile->lights;
    // output lights
    while (pCurLight) {

      // discart spot-lights
      if (pCurLight->spot_light) {
        fprintf (stderr, "Spotlight are not supported. Light '%s' will not be imported in CS\n", pCurLight->name);
      // convert omni-lights
      } else {
        fprintf (o, "    LIGHT (\n");
        fprintf (o, "      CENTER (%g,%g,%g) \n",pCurLight->position[0], pCurLight->position[1], pCurLight->position[2]);
        fprintf (o, "      RADIUS (%g) \n",pCurLight->outer_range);
        fprintf (o, "      COLOR (%g,%g,%g)\n",pCurLight->color[0], pCurLight->color[1], pCurLight->color[2]);
        fprintf (o, "    )\n");
      }

      pCurLight = pCurLight->next;
    }

    fprintf (o, "  )\n");
    fprintf (o, ")\n");
  }

}

