/*
 *  Object converter/optimizer
 */

#include "cssysdef.h"
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "3dsload.h"
#include "3dsout.h"
#include "3dsco.h"

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

void FindCentrePoints (H3dsScene * scene)
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

void Move (H3dsScene * scene, float32 x, float32 y, float32 z)
{
  scene->centre.x+=x;
  scene->centre.y+=y;
  scene->centre.z+=z;
  int n, v;
  for (n=0; n<scene->meshobjs; n++)
  {
    H3dsMeshObj * mo = &scene->meshobjlist[n];
    mo->centre.x+=x;
    mo->centre.y+=y;
    mo->centre.z+=z;
    for (v=0; v<mo->verts; v++)
    {
      H3dsVert * vrt=&mo->vertlist[v];
      vrt->x+=x;
      vrt->y+=y;
      vrt->z+=z;
    }
  }
}

void Scale (H3dsScene * scene, float32 x, float32 y, float32 z)
{
  scene->centre.x*=x;
  scene->centre.y*=y;
  scene->centre.z*=z;
  int n, v;
  for (n=0; n<scene->meshobjs; n++)
  {
    H3dsMeshObj * mo = &scene->meshobjlist[n];
    mo->centre.x*=x;
    mo->centre.y*=y;
    mo->centre.z*=z;
    for (v=0; v<mo->verts; v++)
    {
      H3dsVert * vrt=&mo->vertlist[v];
      vrt->x*=x;
      vrt->y*=y;
      vrt->z*=z;
    }
  }
}

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

void ConvertXYZ (H3dsScene * scene)
{
  ConvertXYZ (scene->centre.x, scene->centre.y, scene->centre.z);
  int n, v, f;
  for (n=0; n<scene->meshobjs; n++)
  {
    H3dsMeshObj * mo = &scene->meshobjlist[n];
    ConvertXYZ (mo->centre.x, mo->centre.y, mo->centre.z);
    for (v=0; v<mo->verts; v++)
    {
      H3dsVert * vrt=&mo->vertlist[v];
      ConvertXYZ (vrt->x, vrt->y, vrt->z);
    }
	// we must convert also triangles or the texture will be flippe
	for (f=0; f<mo->faces; f++)
    {
      H3dsFace * fa = &mo->facelist[f];
	  float v1 = fa->p0;
	  float v2 = fa->p1;
	  float v3 = fa->p2;

      ConvertXYZ (v1, v2, v3);

	  fa->p0 = (word)v1;
	  fa->p1 = (word)v2;
	  fa->p2 = (word)v3;
    }
  }
}

void ConvertFloatsToInts (H3dsScene * scene)
{
  int n, v;
  for (n=0; n<scene->meshobjs; n++)
  {
    H3dsMeshObj * mo = &scene->meshobjlist[n];
    for (v=0; v<mo->verts; v++)
    {
      H3dsVert * vrt = &mo->vertlist[v];
      vrt->ix = roundFloat(vrt->x);
      vrt->iy = roundFloat(vrt->y);
      vrt->iz = roundFloat(vrt->z);
    }
    mo->centre.ix = mo->centre.x;
    mo->centre.iy = mo->centre.y;
    mo->centre.iz = mo->centre.z;
  }
  scene->centre.ix = scene->centre.x;
  scene->centre.iy = scene->centre.y;
  scene->centre.iz = scene->centre.z;
}

float roundFloat (float f)
{
  float rem = fabs(fmod(f,1));
  if (rem<0.00001 && rem>0)
  {
    if (f>0)
      return f-rem;
    else
      return f+rem;
  }
  return f;
}

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

int main (int argc, char * argv[])
{
  char * infn=0, * outfn=0, * name=DEFNAME;
  FILE * inf, * outf;
  int n;
  H3dsScene * scene;
  float32 xscale = 0, yscale = 0, zscale = 0;
  float32 xrelocate = 0, yrelocate = 0, zrelocate = 0;

  flags = 0;
  curmodel = 0;
  modelnum = -1;
  mode_xyz = MODE_XYZ;

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
	       " -tl       Make texture origin lower left\n"
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

  mode_xyz = MODE_XZY; // default mode

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

  // Open inputfile

  if (!(inf = fopen(infn, "rb")))
  {
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
          fclose (inf);
          return 0;
        }
        fprintf (stderr, "\n");
        fclose (outf);
        if ((outf = fopen(outfn, "w+b")) == 0)
	{
          fprintf (stderr, "Unable to reopen %s\n", outfn);
          fclose (inf);
          return 1;
        }
      }
    }
    else
    {
      if ((outf = fopen(outfn, "w+b")) == 0)
      {
        fprintf (stderr, "Unable to create %s\n", outfn);
        fclose (inf);
        return 1;
      }
    }
  }
  else
  {
    // Use stdout if not binary output and no outputfile was specified.
    outf = stdout;
  }

  // Here we have both the source and destination files opened as:
  // FILE * inf  -> source file
  // FILE * outf -> destination file (could be stdout)

  long size;
  if (fseek(inf, 0, SEEK_END))
  {
    fprintf (stderr, "Error seeking %s\n", infn);
    if (outf!=stdout) fclose (outf);
    fclose (inf);
    return 1;
  }

  if ((size=ftell(inf)) == -1L)
  {
    fprintf (stderr, "Error seeking %s\n", infn);
    if (outf!=stdout) fclose (outf);
    fclose (inf);
    return 1;
  }
  rewind (inf);

  if ((scene = HRead3dsScene (inf, 0, size)) == 0)
  {
    fprintf (stderr, "Failed to load %s\n", infn);
    if (outf!=stdout) fclose (outf);
    fclose (inf);
    return 1;
  }
  fclose (inf);

  // At this point we have all object's data loaded into memory.
  // H3dsScene * scene  -> object data

  // Do any rotating, moving, scaling here before
  // we convert all floats to integers.

  FindCentrePoints (scene);

  if (flags & FLAG_CENTRE)
    Move (scene, -scene->centre.x, -scene->centre.y, -scene->centre.z);

  if (flags & FLAG_SCALE)
    Scale (scene, xscale, yscale, zscale);

  if (flags & FLAG_RELOCATE)
    Move (scene, -xrelocate, -yrelocate, -zrelocate);

  if (mode_xyz != MODE_XYZ)
    ConvertXYZ (scene);

  ConvertFloatsToInts (scene);

  if (flags & FLAG_VERBOSE)
  {
    // Print some interesting information about what we have
    fprintf (stderr, "3DS data size: %ld byte\n", size);
    fprintf (stderr, "object-name     faces vertices  maps  matrix\n");
    int f=0, v=0, m=0;
    for (n=0; n<scene->meshobjs; n++)
    {
      H3dsMeshObj * mo = &scene->meshobjlist[n];
      char * mtrx="yes";
      if (mo->matrix==0)
        mtrx="no";
      fprintf(stderr, "===================================================\n");
      fprintf(stderr, "%-14s  %5d  %5d  %5d    %s\n",
              mo->name, mo->faces, mo->verts, mo->maps, mtrx);
      f+=mo->faces;
      v+=mo->verts;
      m+=mo->maps;
      int i;
      float minx = 1000000000., miny = 1000000000., minz = 1000000000.;
      float maxx = -1000000000., maxy = -1000000000., maxz = -1000000000.;
      for (i = 0 ; i < mo->verts ; i++)
      {
        H3dsVert& v = mo->vertlist[i];
        if (v.ix < minx) minx = v.ix;
        if (v.iy < miny) miny = v.iy;
        if (v.iz < minz) minz = v.iz;
        if (v.ix > maxx) maxx = v.ix;
        if (v.iy > maxy) maxy = v.iy;
        if (v.iz > maxz) maxz = v.iz;
      }
      fprintf (stderr,
        "    min=(%g,%g,%g)\n    max=(%g,%g,%g)\n    center=(%g,%g,%g)\n",
      	minx, miny, minz, maxx, maxy, maxz,
	(minx+maxx)/2, (miny+maxy)/2, (minz+maxz)/2);
      if (mo->matrix)
      {
        fprintf (stderr, "    local matrix:\n");
        fprintf (stderr, "\t%g\t%g\t%g\n",
		roundFloat (mo->TraMatrix[0]),
		roundFloat (mo->TraMatrix[1]),
		roundFloat (mo->TraMatrix[2]));
        fprintf (stderr, "\t%g\t%g\t%g\n",
		roundFloat (mo->TraMatrix[3]),
		roundFloat (mo->TraMatrix[4]),
		roundFloat (mo->TraMatrix[5]));
        fprintf (stderr, "\t%g\t%g\t%g\n",
		roundFloat (mo->TraMatrix[6]),
		roundFloat (mo->TraMatrix[7]),
		roundFloat (mo->TraMatrix[8]));
        fprintf (stderr, "    translation vector:\n");
        fprintf (stderr, "\t%g\t%g\t%g\n",
		roundFloat (mo->TraMatrix[9]),
		roundFloat (mo->TraMatrix[10]),
		roundFloat (mo->TraMatrix[11]));
      }
    }
    if (scene->meshobjs >= 1)
      fprintf (stderr, "%d faces, %d vertices, %d maps in "
              "%d objects loaded\n", f, v, m, scene->meshobjs);
    fprintf (stderr, "global center=%g,%g,%g\n",
    	scene->centre.x, scene->centre.y, scene->centre.z);
  }

  // Prepare to collect all vertices to one big array

  int verts=0;
  for (n=0; n<scene->meshobjs; n++)
  {
    int v=scene->meshobjlist[n].verts;
    if (scene->meshobjlist[n].maps > v)
    {
      fprintf (stderr, "%-14s more maps than vertices, quitting!\n",
          scene->meshobjlist[n].name);
      if (outf!=stdout) fclose (outf);
      return 1;
    }
    // Get the total number of vertices in all objects
    verts+=v;
  }

  H3dsMapVert * vrtmap = (H3dsMapVert *) malloc(verts*sizeof(H3dsMapVert));
  if (!vrtmap)
  {
    fprintf (stderr, "Failed to allocate mem for vertice array\n");
    HFree3dsScene (scene);
    fclose (outf);
    return 1;
  }
  memset (vrtmap, 0, verts*sizeof(H3dsMapVert));

  // Do it! Collect them

  CollectVertsAndMaps (scene, vrtmap);

  // Adjust the face indexes to the new vertice array
  //AdjustFaceIndexes (scene);

#if 0
// @@@ This corrupts texture mapping!
  if (!(flags & FLAG_NORMDUP))
  {
    if (flags & FLAG_VERBOSE)
      fprintf (stderr, "Removing duplicated vertices ");
    n = RemoveDupVerts (scene, vrtmap, verts);
    if (flags & FLAG_VERBOSE)
      fprintf (stderr, "\nRemoved %d duplicated vertices\n", verts-n);
    verts=n;
  }
#endif

  // Output data

  if (!(flags & FLAG_LIST))
  {
    fprintf (stderr, "CS format!\n");
    OutpHeadCS (outf, scene, verts, name);
    // removed because we output every object with its vertexes
    //OutpVertsCS (outf, vrtmap, verts, name);
    OutpObjectsCS (outf, scene, vrtmap, verts, name, flags & FLAG_LIGHTING);
  }

  free (vrtmap);
  HFree3dsScene (scene);
  if (!(flags & FLAG_LIST) && outf != stdout)
    fclose (outf);

  return 0;
}
