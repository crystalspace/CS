/*
 *  Object converter/optimizer
 *  Author: Luca Pancallo
 */

#include "cssysdef.h"
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "3dsout.h"
#include "3ds2lev.h"


#include "csutil/datastrm.h"

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

// ~end new implementation --------- 

CS_IMPLEMENT_APPLICATION

// Default object name to set as public label
#define DEFNAME "world"

float roundFloat (float f);

int flags = 0;
int curmodel = 0;
int modelnum = -1;

#define MODE_XYZ 0
#define MODE_XZY 1
#define MODE_YXZ 2
#define MODE_YZX 3
#define MODE_ZXY 4
#define MODE_ZYX 5
int mode_xyz = MODE_XYZ;


/*
 * Moves (or relocates) the whole scene.
 * All meshes are moved.
 * All lights are moved.
 */
void Lib3dsMove (Lib3dsFile* p3dsFile, float32 x, float32 y, float32 z)
{

  // a vertex
  Lib3dsPoint *pCurPoint;

  // used for coords of vertex
  float *xyz;

  // set the current mesh to the first in the file
  Lib3dsMesh *p3dsMesh = p3dsFile->meshes;

  // as long as we have a valid mesh...
  while( p3dsMesh )
  {
    // get the number of vertices in the current mesh
    int numVertices = p3dsMesh->points;

    // vertexes pointer
    pCurPoint = p3dsMesh->pointL;

    for ( int i = 0 ; i < numVertices ; i++ )
    {
        // index to the position on the list using index
        xyz = pCurPoint->pos;

        // Move the vertex
        xyz[0]+=x;
        xyz[1]+=y;
        xyz[2]+=z;

        // go to next vertex
        pCurPoint++;
    }
    p3dsMesh = p3dsMesh->next;
  }

  // move lights
  Lib3dsLight *pCurLight = p3dsFile->lights;

  while (pCurLight) {

    pCurLight->position[0]+=x;
    pCurLight->position[1]+=y;
    pCurLight->position[2]+=z;

    pCurLight = pCurLight->next;
  }
}

/*
 * Scales the whole scene.
 * All meshes are scaled.
 * All lights are scaled.
 */
void Lib3dsScale (Lib3dsFile* p3dsFile, float32 x, float32 y, float32 z)
{

  // a vertex
  Lib3dsPoint *pCurPoint;

  // used for coords of vertex
  float *xyz;

  // set the current mesh to the first in the file
  Lib3dsMesh *p3dsMesh = p3dsFile->meshes;

  // as long as we have a valid mesh...
  while( p3dsMesh )
  {
    // get the number of vertices in the current mesh
    int numVertices = p3dsMesh->points;

    // vertexes pointer
    pCurPoint = p3dsMesh->pointL;

    for ( int i = 0 ; i < numVertices ; i++ )
    {
        // index to the position on the list using index
        xyz = pCurPoint->pos;

        // Scale the vertex
        xyz[0]*=x;
        xyz[1]*=y;
        xyz[2]*=z;

        // go to next vertex
        pCurPoint++;
    }
    // go to next mesh
    p3dsMesh = p3dsMesh->next;
  }

  // scale lights
  Lib3dsLight *pCurLight = p3dsFile->lights;

  while (pCurLight) {

    pCurLight->position[0]*=x;
    pCurLight->position[1]*=y;
    pCurLight->position[2]*=z;

    // scale range of light on 'x' scaling
    pCurLight->outer_range*=x;
    pCurLight->inner_range*=x;

    pCurLight = pCurLight->next;
  }


}

/**
 * Simply switch x,y,z coord depending on the selected MODE_XYZ
 */
void ConvertXYZ (float& x, float& y, float& z)
{
  float sw;
  switch (mode_xyz)
  {
    case MODE_XYZ: return;
    case MODE_XZY: sw = y; y = z; z = sw; return;
    case MODE_YXZ: sw = x; x = y; y = sw; return;
    case MODE_YZX: sw = x; x = y; y = z; z = sw; return;
    case MODE_ZXY: sw = x; x = z; z = y; y = sw; return;
    case MODE_ZYX: sw = x; x = z; z = sw; return;
  }
}

/**
 * Swap x,y,z coord depending on the selected MODE_XYZ
 * All vertexes and triangles are affected.
 * All lights are affected.
 */
void Lib3dsConvertXYZ (Lib3dsFile* p3dsFile)
{

  // TODO: should be done for -center option. actually not supported.
  //ConvertXYZ (scene->centre.x, scene->centre.y, scene->centre.z);

  // a vertex
  Lib3dsPoint *pCurPoint;

  // used for coords of vertex
  float *xyz;
  
  // set the current mesh to the first in the file
  Lib3dsMesh *p3dsMesh = p3dsFile->meshes;

  // as long as we have a valid mesh...
  while( p3dsMesh )
  {
    // get the number of vertices in the current mesh
    int numVertices = p3dsMesh->points;
    int i;
    // vertexes pointer
    pCurPoint = p3dsMesh->pointL;
    for ( i = 0 ; i < numVertices ; i++ )
    {
        // index to the position on the list using index
        xyz = pCurPoint->pos;

        // Convert the vertex coords
        ConvertXYZ (xyz[0], xyz[1], xyz[2]);

        // go to next vertex
        pCurPoint++;
    }

    // we must convert also triangles or the texture will be flipped
    // get the triangle count and go to the first triangle
    int numTriangles = p3dsMesh->faces;
    Lib3dsFace *pCurFace = p3dsMesh->faceL;

    // convert each triangle
    for ( i = 0 ; i < numTriangles ; i++ ) {
        float v1 = pCurFace->points[0];
        float v2 = pCurFace->points[1];
        float v3 = pCurFace->points[2];
        ConvertXYZ (v1, v2, v3);
        pCurFace->points[0] = (short unsigned int)v1;
        pCurFace->points[1] = (short unsigned int)v2;
        pCurFace->points[2] = (short unsigned int)v3;

        // go to next triangle
        pCurFace++;
    }

    // go to next mesh
    p3dsMesh = p3dsMesh->next;
  }

  // swap coords of lights
  Lib3dsLight *pCurLight = p3dsFile->lights;

  while (pCurLight) {

    ConvertXYZ (pCurLight->position[0], pCurLight->position[1], pCurLight->position[2]);

    pCurLight = pCurLight->next;
  }

}


/**
 * Actually not used: must be redone for Lib3dsFile
void FindCentrePoints (H3dsScene * scene, Lib3dsFile* p3dsFile)
{

  float32 xmino= 1e30, ymino= 1e30, zmino= 1e30;
  float32 xmaxo=-1e30, ymaxo=-1e30, zmaxo=-1e30;
  int n, v;
  for (n=0; n<scene->meshobjs; n++)
  {
    float32 xmin= 1e30, ymin= 1e30, zmin= 1e30;
    float32 xmax=-1e30, ymax=-1e30, zmax=-1e30;
    H3dsMeshObj * mo = &scene->meshobjlist[n];
    for (v=0; v<mo->verts; v++)
    {
      H3dsVert * vrt=&mo->vertlist[v];
      if (vrt->x > xmax) xmax=vrt->x;
      if (vrt->x < xmin) xmin=vrt->x;
      if (vrt->y > ymax) ymax=vrt->y;
      if (vrt->y < ymin) ymin=vrt->y;
      if (vrt->z > zmax) zmax=vrt->z;
      if (vrt->z < zmin) zmin=vrt->z;
    }
    mo->centre.x = xmax-(xmax-xmin)*0.5;
    mo->centre.y = ymax-(ymax-ymin)*0.5;
    mo->centre.z = zmax-(zmax-zmin)*0.5;

    if (mo->centre.x > xmaxo) xmaxo=mo->centre.x;
    if (mo->centre.x < xmino) xmino=mo->centre.x;
    if (mo->centre.y > ymaxo) ymaxo=mo->centre.y;
    if (mo->centre.y < ymino) ymino=mo->centre.y;
    if (mo->centre.z > zmaxo) zmaxo=mo->centre.z;
    if (mo->centre.z < zmino) zmino=mo->centre.z;
  }
  scene->centre.x = xmaxo-(xmaxo-xmino)*0.5;
  scene->centre.y = ymaxo-(ymaxo-ymino)*0.5;
  scene->centre.z = zmaxo-(zmaxo-zmino)*0.5;
}
*/

/**
 * Used by FindCentrePoints()
void FindExchange (H3dsScene * scene, int find, int exchange)
{
  // Find all references to the 'find' vertice and replace
  // them with references to the 'exchange' vertice
  int o, f;
  for (o=0; o<scene->meshobjs; o++)
  {
    H3dsMeshObj * mo = &scene->meshobjlist[o];
    for (f=0; f<mo->faces; f++)
    {
      H3dsFace * fa = &mo->facelist[f];
      if(fa->p0 == find) fa->p0 = exchange;
      if(fa->p1 == find) fa->p1 = exchange;
      if(fa->p2 == find) fa->p2 = exchange;
    }
  }
}
 */

/**
 * Actually not used: must be redone for Lib3dsFile
int RemoveDupVerts (H3dsScene * scene, H3dsMapVert * vrtmap, int verts)
{
  int vrttop=0, dot=0, currvtx, runvtx;
  for (currvtx=0; currvtx<verts; currvtx++)
  {
    // Only process those vertices that has not been
    // processed already.
    if (vrtmap[currvtx].marked == 0)
    {
      // OK, we have a vertex, currvtx. Try to find all other
      // vertices that have the same x,y,z values.
      for (runvtx=currvtx+1; runvtx<verts; runvtx++)
      {
        // Skip all vertices that has been processed already.
        // We already know that they don't have the same values.
        if (vrtmap[runvtx].marked == 1)
          continue;
    
        // If we find another vertex with the same x,y,z values
        // we must find and adjust all the indexes that point
        // to that vertex so that they point to currvtx.
        if (vrtmap[runvtx].ix == vrtmap[currvtx].ix &&
            vrtmap[runvtx].iy == vrtmap[currvtx].iy &&
            vrtmap[runvtx].iz == vrtmap[currvtx].iz)
        {
          // Make them point to the top of our optimized array
          FindExchange (scene, runvtx, vrttop);
    
          // Mark it so we don't process it again.
          vrtmap[runvtx].marked=1;
        }
      }
    
      // Now find all other indexes that points to currvtx
      // and adjust them to the top of our optimized array, vrttop.
      FindExchange (scene, currvtx, vrttop);
    
      // Put currvtx on top of our optimized array.
      vrtmap[vrttop] = vrtmap[currvtx];
      vrttop++;
    }

    // Print some dots so that the user don't fall asleep
    if ((flags & FLAG_VERBOSE) && dot++>20)
    {
      fprintf (stderr, ".");
      dot=0;
    }
  }
  return vrttop;
}
 */

/**
 * Actually not used: must be redone for Lib3dsFile
void AdjustFaceIndexes (H3dsScene * scene)
{
  int m=0, f, n;
  for (n=0; n<scene->meshobjs; n++)
  {
    H3dsMeshObj * mo = &scene->meshobjlist[n];
    for (f=0; f<mo->faces; f++)
    {
      H3dsFace * fa = &mo->facelist[f];
      fa->p0 += m;
      fa->p1 += m;
      fa->p2 += m;
    }
    m+=mo->verts;
  }
}
 */

/**
 * Actually not used: must be redone for Lib3dsFile
void CollectVertsAndMaps (H3dsScene * scene, H3dsMapVert * vrtmap)
{
  int vn=0, mn, v, n, m, mmn;
  for (n=0; n<scene->meshobjs; n++)
  {
    H3dsMeshObj * mo = &scene->meshobjlist[n];
    mn=vn;
    for (v=0; v<mo->verts; v++)
    {
      vrtmap[vn].ix=mo->vertlist[v].ix;
      vrtmap[vn].iy=mo->vertlist[v].iy;
      vrtmap[vn].iz=mo->vertlist[v].iz;
      vn++;
    }
    for (m=0; m<mo->maps; m++)
    {
      vrtmap[mn].iu=mo->maplist[m].u;
      vrtmap[mn].iv=
        (flags & FLAG_SWAP_V ? 1.-mo->maplist[m].v : mo->maplist[m].v);
      mn++;
    }
    if (mn<vn)
    {
      if (flags & FLAG_VERBOSE)
        fprintf(stderr, "%-14s missing mapping, set to zero...\n",
            mo->name);
      for (mmn=mn; mmn<vn; mmn++)
      {
        vrtmap[mmn].iu=0;
        vrtmap[mmn].iv=0;
      }
    }
  }
}
 */

/**
 * Main function
 */
int main (int argc, char * argv[])
{
  char * infn=0, * outfn=0, * name=DEFNAME;
  FILE * outf;
  int n;
  float32 xscale = 0, yscale = 0, zscale = 0;
  float32 xrelocate = 0, yrelocate = 0, zrelocate = 0;

  flags = 0;
  curmodel = 0;
  modelnum = -1;
  mode_xyz = MODE_XZY;
  flags |= FLAG_SWAP_V; // default to lower left texture origin

  argc--;
  argv++;

  if(!argc)
  {
    fprintf (stderr,
         "3D Studio native objectfile converter v2.0\n"
         "originally by Mats Byggmastar 1996 Espoo, Finland.\n"
	 "This version is for Crystal Space and heavily modified by\n"
	 "Jorrit Tyberghein and Luca Pancallo.\n"
         "Use:  %s [params...] inputfile.3DS [outputfile]\n"
               "params:\n"
               " -o name   Object name (default "DEFNAME")\n"
               " -v        Verbose mode on\n"
               " -vv       Very verbose mode on\n"
               " -l        Don't convert but list objects in 3ds file\n"
               " -c        Centre objects\n"
               " -s x y z  Scale objects (x,y,z = floats)\n"
               " -r x y z  Relocate objects (x,y,z = floats)\n"
               " -pl       Make polygons lit\n"
               " -f        Don't ask for file overwrite (force)\n"
               " -d        Don't remove duplicated vertices\n"
               " -3        Output 3D sprite instead of level\n"
               " -m num    Output only one object from 3DS (use -l to list)\n"
	       " -tl       Make texture origin lower left (default)\n"
	       " -xyz      Convert model xyz -> CS xyz\n"
	       " -xzy      Convert model xyz -> CS xzy (default)\n"
	       " -yxz      Convert model xyz -> CS yxz\n"
	       " -yzx      Convert model xyz -> CS yzx\n"
	       " -zxy      Convert model xyz -> CS zxy\n"
	       " -zyx      Convert model xyz -> CS zyx\n",
               argv[-1]);

    return 1;
  }

  // Get the parameters and filenames

  for (n=0; n<argc; n++)
  {
    if (argv[n][0] == '-' || argv[n][0] == '/')
    {
      switch (toupper(argv[n][1]))
      {
        case 'O': if (n+1<argc) name=argv[++n];
                  else { fprintf (stderr, "Missing object name!\n");
                    return 1; }
		  break;
        case 'R': if (n+3<argc)
		  {
		    flags |= FLAG_RELOCATE;
                    xrelocate = atof(argv[++n]);
                    yrelocate = atof(argv[++n]);
                    zrelocate = atof(argv[++n]);
		  }
                  else
		  {
		    fprintf(stderr, "Missing relocate value!\n");
		    return 1;
		  }
		  break;
        case 'S': if (n+3<argc)
		  {
		    flags |= FLAG_SCALE;
                    xscale = atof(argv[++n]);
                    yscale = atof(argv[++n]);
                    zscale = atof(argv[++n]);
		  }
                  else
		  {
		    fprintf(stderr, "Missing scale value!\n");
		    return 1;
		  }
		  break;
        case 'P':
	          if (toupper (argv[n][2]) == 'L')
		  {
		    flags |= FLAG_LIGHTING;
		  }
		  break;
        case 'V': flags |= FLAG_VERBOSE;
	          if (toupper (argv[n][2]) == 'V')
		  {
		    flags |= FLAG_VERYVERBOSE;
		  }
		  break;
        case 'T': if (toupper (argv[n][2]) == 'L')
		    flags |= FLAG_SWAP_V;
		  break;
	case 'C': flags |= FLAG_CENTRE; break;
	case 'F': flags |= FLAG_OVERWR; break;
	case 'D': flags |= FLAG_NORMDUP; break;
	case '3': flags |= FLAG_SPRITE; break;
	case 'L': flags |= FLAG_LIST; break;
        case 'M': if (n+1<argc) sscanf (argv[++n], "%d", &modelnum);
                  else { fprintf (stderr, "Missing model number!\n");
                    return 1; }
		  flags |= FLAG_MODEL;
		  break;
	case 'X': if (toupper (argv[n][2]) == 'Y')
		    mode_xyz = MODE_XYZ;
		  else
		    mode_xyz = MODE_XZY;
		  break;
	case 'Y': if (toupper (argv[n][2]) == 'X')
		    mode_xyz = MODE_YXZ;
		  else
		    mode_xyz = MODE_YZX;
		  break;
	case 'Z': if (toupper (argv[n][2]) == 'X')
		    mode_xyz = MODE_ZXY;
		  else
		    mode_xyz = MODE_ZYX;
		  break;
	default:  fprintf (stderr, "Bad param: %s\n",argv[n]);
		  return 1;
      }
    }
    else
    {
      if (!infn)
        infn = argv[n];
      else if (!outfn)
        outfn = argv[n];
      else
      {
	fprintf (stderr, "Too many filenames!\n");
        return 1;
      }
    }
  }

  if (!infn)
  {
    fprintf (stderr, "No inputfile specified!\n");
    return 1;
  }

  // <--------- finished parsing parameters ----------->


  // <--------- opening input and output files--------->

  // Read inputfile
  cs3ds2LevConverter *converter = new cs3ds2LevConverter();
  Lib3dsFile *p3dsFile = converter->LoadFile(infn);
  if (!p3dsFile ) {
    fprintf (stderr, "Failed to open %s\n", infn);
    return 1;
  }

  // Open, create or redirect outputfile
  if (!(flags & FLAG_LIST) && outfn)
  {
    if ((outf = fopen(outfn, "r+b")) != 0)
    {
      if ((flags & FLAG_OVERWR) == 0)
      {
        fprintf (stderr, "%s exist, overwrite [y/n] ", outfn);
        fflush (stdout);
        if (toupper(getc(stdin)) != 'Y')
        {
          fclose (outf);
          //fclose (inf);
          return 0;
        }
        fprintf (stderr, "\n");
        fclose (outf);
        if ((outf = fopen(outfn, "w+b")) == 0)
        {
          fprintf (stderr, "Unable to reopen %s\n", outfn);
          //fclose (inf);
          return 1;
        }
      }
    }
    else
    {
      if ((outf = fopen(outfn, "w+b")) == 0)
      {
        fprintf (stderr, "Unable to create %s\n", outfn);
        //fclose (inf);
        return 1;
      }
    }
  }
  else
  {
    // Use stdout if not binary output and no outputfile was specified.
    outf = stdout;
  }

  // <--------- finished opening input and output files--------->

  /* Centering: currently not supported
  FindCentrePoints (scene);
  if (flags & FLAG_CENTRE)
    Move (scene, -scene->centre.x, -scene->centre.y, -scene->centre.z);
  */

  // scale model if requested
  if (flags & FLAG_SCALE)
    Lib3dsScale (p3dsFile, xscale, yscale, zscale);

  // move (relocate) model if requested
  if (flags & FLAG_RELOCATE)
    Lib3dsMove (p3dsFile, -xrelocate, -yrelocate, -zrelocate);

  // swap xyz if requested
  if (mode_xyz != MODE_XYZ)
    Lib3dsConvertXYZ (p3dsFile);

  // Print some interesting information about what we have
  if (flags & FLAG_VERBOSE)
  {
    // fprintf (stderr, "3DS data size: %ld byte\n", size);
    fprintf (stderr, "3DS name: %s\n", p3dsFile->name);
    fprintf (stderr, "lights: %s\n", p3dsFile->lights ? "yes" : "no");
    fprintf (stderr, "object-name     faces vertices  maps  matrix\n");

    // set the current mesh to the first in the file
    Lib3dsMesh *p3dsMesh = p3dsFile->meshes;
    // as long as we have a valid mesh...
    while( p3dsMesh )
    {
        // get the numbers in the current mesh
        fprintf(stderr, "===================================================\n");
        fprintf(stderr, "%-14s  %5ld  %5ld  %5d    %s\n",
              p3dsMesh->name, p3dsMesh->faces, p3dsMesh->points, -1, " ");

        // go to next mesh
        p3dsMesh = p3dsMesh->next;
    }
  }

  // Output data in CS format
  if (!(flags & FLAG_LIST))
  {
    fprintf (stderr, "Writing output in CS format...");
    OutpHeadCS (outf, p3dsFile);
    OutpObjectsCS (outf, p3dsFile, flags & FLAG_LIGHTING);

    fprintf (stderr, "done! \n");
  }

  return 0;
}


/**
 * Is it better to have it as a class?
 */
cs3ds2LevConverter::cs3ds2LevConverter() {

}


cs3ds2LevConverter::~cs3ds2LevConverter() {

}

Lib3dsFile *cs3ds2LevConverter::LoadFile( char *filename )
{

  Lib3dsFile *pFile = lib3ds_file_load(filename);

  return pFile;
}

