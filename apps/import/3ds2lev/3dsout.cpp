
/*
 *  Object output
 */


/*

   Raw object structure 


   +---------------+
   |    header     |  sizeof(header)   
   +---------------+
   |               |
   |   vertices    |  verts*3*sizeof(word) if no mapping
   |               |  verts*5*sizeof(word) if mapping
   +---------------+
   |               |
   | sub objects   |  if only one object: nothing here
   | center points |  else: objects*3*sizeof(word)
   |               |
   +---------------+
   | main object   |  3*sizeof(word)
   | center point  |  
   +---------------+
   |               |  each object starts with an objheader sizeof(objheader)
   |   subobject   |  then follows all faces
   |   faces       |  objheader.faces*3*sizeof(word)
   |               |
   +---------------+
   |               |
   | next subobject|  ....
   |               |
   +---------------+

*/
#include "cssysdef.h"
#include "3dsload.h"
#include "3dsco.h"
#include <stdio.h>
#include <ctype.h>

typedef struct
{
  int id;             // 0x49524d24  '$MRI'
  int flag;           // Type of data, bitfield
  int verts;          // Number of vertices (not including centre points)
  int objects;        // Number of objects
  int vtxptr;         // Offset to verices (from start of header)
  int centreptr;      // Offset to centre points (from start of header)
  int objptr;         // Offset to objects (from start of header)
  int reserved;
} header;

typedef struct
{
  short faces;        // Number of faces in object
  char  color;        // Object color
  char  reserved;
} objheader;



// Added by LucaPancallo 2000.09.28

extern int flags;

void OutpHeadCS (FILE * o, H3dsScene * scene, int verts, char * name)
{
  (void)scene; (void)verts;
  if (flags & FLAG_SPRITE)
  {
    fprintf (o, "MATERIAL ('white')\n");
  }
  else
  {
    fprintf (o, "WORLD (\n");
    fprintf (o, "  PLUGINS (\n");
    fprintf (o, "    PLUGIN 'thing' ('crystalspace.mesh.loader.thing')\n");
    fprintf (o, "    )\n");
    fprintf (o, "  SECTOR 'room' (\n");
    fprintf (o, "    MESHOBJ '%s' (\n", name);
    fprintf (o, "      PLUGIN ('thing')\n");
    fprintf (o, "      ZFILL ()\n");
    fprintf (o, "      PARAMS (\n");
  }
}

// CS FORMAT:
//     VERTEX (10.000000,-1.800000,5.700000)  ; 1
void OutpVertsCS (FILE * o, H3dsMapVert * vrtmap, int verts,char * name)
{
  (void)name;
  if (flags & FLAG_SPRITE)
  {
    fprintf (o, "FRAME '%s' (\n", "f1");
  }
  for (int n=0; n<verts; n++)
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

void OutpCentresCS (FILE * o, H3dsScene * scene, char * name)
{
  (void)o; (void)scene; (void)name;
  //fprintf(o, "CS CENTRES: What's This???? \n", name);
}

// CS FORMAT:
// POLYGON 'x0_0' (VERTICES (2,3,6) LIGHTING (no))
void OutpObjectsCS (FILE * o, H3dsScene * scene, H3dsMapVert* vrtmap,
		    int /*verts*/, char * name, bool lighting)
{
  if (flags & FLAG_SPRITE)
  {
    fprintf (o, "  ; '%s'\n", name);
  }

  for (int n=0; n<scene->meshobjs; n++)
  {
    H3dsMeshObj * mo = &scene->meshobjlist[n];
    fprintf (o, "; %s ", mo->name);
    fprintf (o, ": %6d faces - %d + 256 * %d \n", (int)mo->faces, 0, 0);
    for (int i=0; i<mo->faces; i++)
    {
      H3dsFace * fa = &mo->facelist[i];
      if (flags & FLAG_SPRITE)
      {
        fprintf (o, "TRIANGLE (%d,%d,%d)\n", fa->p0, fa->p1, fa->p2);
      }
      else
      {
        fprintf (o, "      POLYGON 'x%d_%d'  (VERTICES (%d,%d,%d)%s\n",
	  n, i, fa->p0, fa->p1, fa->p2,
	  lighting ? "" : " LIGHTING (no)");
        fprintf (o, "        TEXTURE (UV (0,%g,%g,1,%g,%g,2,%g,%g))\n",
          vrtmap[fa->p0].iu, vrtmap[fa->p0].iv,
          vrtmap[fa->p1].iu, vrtmap[fa->p1].iv,
          vrtmap[fa->p2].iu, vrtmap[fa->p2].iv
      	  );
        fprintf (o, "      )\n");
      }
    }
  }
  if (flags & FLAG_SPRITE)
  {
    fprintf (o, "ACTION 'default' (F (f1,1000))\n");
  }
  else
  {
    fprintf (o, "    )\n");
    fprintf (o, "    CULLER ()\n");
    fprintf (o, "  )\n");
    fprintf (o, ")\n");
  }
}

