
/*
 *  CS Object output
 *  Author: Luca Pancallo 2000.09.28
 */

#include "cssysdef.h"
#include "3dsload.h"
#include "3dsco.h"
#include "3dsout.h"
#include <stdio.h>
#include <ctype.h>

extern int flags;

/**
 * Outputs the header with TEXTURES, MATERIALS, PLUGINS.
 */
void OutpHeadCS (FILE * o, H3dsScene * scene, int verts, char *)
{
  (void)scene; (void)verts;
  if (flags & FLAG_SPRITE)
  {
    fprintf (o, "MESHFACT 'sprite' (\n");
    fprintf (o, "  PLUGIN ('crystalspace.mesh.loader.factory.sprite.3d')\n");
    fprintf (o, "  PARAMS (\n");
  }
  else
  {
    int n;

    fprintf (o, "WORLD (\n");
    fprintf (o, "  TEXTURES (\n");
    for (n=0; n<scene->meshobjs; n++) {
        H3dsMeshObj * mo = &scene->meshobjlist[n];
        fprintf (o, "    TEXTURE '%s' (FILE (%s.png)) \n",mo->material, mo->material);
    }
    fprintf (o, "  )\n\n");

    fprintf (o, "  MATERIALS (\n");
    for (n=0; n<scene->meshobjs; n++) {
        H3dsMeshObj * mo = &scene->meshobjlist[n];
        fprintf (o, "    MATERIAL '%s' (TEXTURE ('%s')) \n",mo->material, mo->material);
    }
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
void OutpObjectsCS (FILE * o, H3dsScene * scene, H3dsMapVert* ,
		    int /*verts*/, char * name, bool lighting)
{
  int i, n, j;

  if (flags & FLAG_SPRITE)
  {
    fprintf (o, "  ; '%s'\n", name);
  } else {

      // Reorder the objects to have all "_s_" first.
      for (n=0; n<scene->meshobjs; n++) {
        H3dsMeshObj * mo1 = &scene->meshobjlist[n];
        // if not static...
        if (!strstr(mo1->name, "_s_")) {
              // search a static and swap
              for (j=n+1; j<scene->meshobjs; j++) {
                H3dsMeshObj * mo2 = &(scene->meshobjlist[j]);
                if (strstr(mo2->name, "_s_")) {
                        H3dsMeshObj tmp = *mo2;
                        scene->meshobjlist[j] = *mo1;
                        scene->meshobjlist[n] = tmp;
                        break;
                }
              }
        }
      }
  }

  bool staticObj = false;
  bool part = false;
  for (n=0; n<scene->meshobjs; n++)
  {
    H3dsMeshObj * mo = &scene->meshobjlist[n];

    if (flags & FLAG_SPRITE) {
    }
    else {
        // on "_s_" decide if MESHOBJ or PART
        if (strstr(mo->name, "_s_")) {
            if (!staticObj) {
                fprintf (o, "    MESHOBJ 'static' (\n");
                fprintf (o, "      PLUGIN ('thing')\n");
                fprintf (o, "      ZFILL ()\n");
                fprintf (o, "      PRIORITY('wall')\n");
                fprintf (o, "      PARAMS (\n");
                fprintf (o, "      VISTREE()\n");
                fprintf (o, "; Object Name : %s ", mo->name);
                fprintf (o, " Faces: %6d faces \n", (int)mo->faces);
                fprintf (o, "      PART '%s' (\n", mo->name);
                staticObj = true;
            }
            else  {
                fprintf (o, "      PART '%s' (\n", mo->name);
            }
            part = true;
        // else always MESHOBJ
        } else {
            // close previous PART and MESHOBJ if present
            if (part) {
                fprintf (o, "      )\n\n");
                fprintf (o, "    )\n\n");
                part = false;
            }
            fprintf (o, "; Object Name : %s ", mo->name);
            fprintf (o, " Faces: %6d faces \n", (int)mo->faces);

            fprintf (o, "    MESHOBJ '%s' (\n", mo->name);
            fprintf (o, "      PLUGIN ('thing')\n");
            fprintf (o, "      ZUSE ()\n");
            // handles transparent objects
            if (strstr(mo->name, "_t_")) {
                fprintf (o, "      PRIORITY('alpha')\n");
            } else {
                fprintf (o, "      PRIORITY('object')\n");
            }
            fprintf (o, "      PARAMS (\n");
        }
    }
    fprintf (o, "        MATERIAL ('%s')\n", mo->material);

    // output vertexes
    for (int v=0; v<mo->verts; v++)
    {
      if (flags & FLAG_SPRITE)
      {
          fprintf (o, "        V(%g,%g,%g:", mo->vertlist[v].ix, mo->vertlist[v].iy, mo->vertlist[v].iz);
          if (mo->maplist==NULL)
              fprintf (o, "0,0)\n");
          else
            fprintf (o, "%g,%g)\n",mo->maplist[0].u, (flags & FLAG_SWAP_V ? 1.-mo->maplist[0].v : mo->maplist[0].v));
      } else {
          fprintf (o, "        VERTEX (%g,%g,%g)    ; %d\n",
        	mo->vertlist[v].ix, mo->vertlist[v].iy, mo->vertlist[v].iz, v);
      }

    }

    // output faces
    for (i=0; i<mo->faces; i++)
    {
      H3dsFace * fa = &mo->facelist[i];
      H3dsMap* mapV0 = &mo->maplist[fa->p0];
      H3dsMap* mapV1 = &mo->maplist[fa->p1];
      H3dsMap* mapV2 = &mo->maplist[fa->p2];

      if (flags & FLAG_SPRITE)
      {
        fprintf (o, "        TRIANGLE (%d,%d,%d)\n", fa->p0, fa->p1, fa->p2);
      }
      else
      {
        fprintf (o, "        POLYGON 'x%d_%d'  (VERTICES (%d,%d,%d)%s\n",
	             n, i, fa->p0, fa->p1, fa->p2,
	             lighting ? "" : " LIGHTING (no)");
        fprintf (o, "          TEXTURE (UV (0,%g,%g,1,%g,%g,2,%g,%g))\n",
                 mapV0->iu, mapV0->iv,
                 mapV1->iu, mapV1->iv,
                 mapV2->iu, mapV2->iv);
        fprintf (o, "        )\n"); // close polygon
      }
    }
    // close PARAMS tag
    fprintf (o, "      )\n\n");

    // close MESHOBJ tag
    if (!part)
        fprintf (o, "    )\n\n");
  }
  if (flags & FLAG_SPRITE)
  {
    fprintf (o, "ACTION 'default' (F (f1,1000))\n");
    fprintf (o, "  )\n");
  }
  else
  {
    fprintf (o, "    CULLER ('static')\n");
    fprintf (o, "  )\n");
    fprintf (o, ")\n");
  }
}

/**
 * This outputs all the vertexes of the 3ds file,
 * regardless of the objects they belong to
 *
 * es. format: VERTEX (10.000000,-1.800000,5.700000)  ; 1
 */
void OutpVertsCS (FILE * o, H3dsMapVert * vrtmap, int verts,char * name)
{
  (void)name;
  if (flags & FLAG_SPRITE)
  {
    fprintf (o, "FRAME '%s' (\n", "f1");
  }
  int n;
  for (n=0; n<verts; n++)
  {
    H3dsMapVert * vm = &vrtmap[n];
    if (flags & FLAG_SPRITE)
    {
      fprintf (o, "  V(%g,%g,%g:%g,%g)\n",
    	vm->ix, vm->iy, vm->iz, vm->iu, vm->iv);
      	
    }
    else
    {
      fprintf (o, "        VERTEX (%g,%g,%g)    ; %d\n",
    	vm->ix, vm->iy, vm->iz, n);
    }
  }
  if (flags & FLAG_SPRITE)
  {
    fprintf (o, ")\n");
  }
}

