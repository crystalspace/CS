/*
    Crystal Space 3ds2lev xml writer
    Copyright (C) 2001,2002 by Luca Pancallo and Matze Braun

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

#include <stdlib.h>
#include <ctype.h>
#include <math.h>

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

#include "csutil/scfstr.h"
#include "csutil/sysfunc.h"

#include "levelwriter.h"

CS_IMPLEMENT_APPLICATION

enum
{
  FLAG_VERBOSE	= 0x0001,
  FLAG_LIST     = 0x0002,    /* List all objects in this 3ds     */
  FLAG_SPRITE	= 0x0004
};

enum xyzmode
{
  MODE_XYZ = 0,
  MODE_XZY = 1,
  MODE_YXZ = 2,
  MODE_YZX = 3,
  MODE_ZXY = 4,
  MODE_ZYX = 5
};

/**
 * Simply switch x,y,z coord depending on the selected MODE_XYZ
 */
void ConvertXYZ (xyzmode mode_xyz, float& x, float& y, float& z)
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
void Lib3dsConvertXYZ (Lib3dsFile* p3dsFile, xyzmode mode)
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
        ConvertXYZ (mode, xyz[0], xyz[1], xyz[2]);

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
      ConvertXYZ (mode, v1, v2, v3);
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
  Lib3dsLight* light;
  for (light = p3dsFile->lights; light; light = light->next)
  {
    ConvertXYZ (mode, light->position[0],
	              light->position[1],
		      light->position[2]);
  }

  // swap coords of cameras
  Lib3dsCamera *camera;
  for (camera = p3dsFile->cameras; camera; camera = camera->next)
  {
    ConvertXYZ (mode, camera->position[0],
		      camera->position[1],
		      camera->position[2]);
    ConvertXYZ (mode, camera->target[0],
		      camera->target[1],
		      camera->target[2]);
  }
}

/**
 * Main function
 */
int main (int argc, char * argv[])
{
  char* infn=0, *outfn=0;
  char* spritename=0;
  float xscale = 1, yscale = 1, zscale = 1;
  float xrelocate = 0, yrelocate = 0, zrelocate = 0;
  xyzmode mode_xyz = MODE_XZY;
  int flags = 0;
  LevelWriter writer;

  flags = 0;
  
  // default to lower left texture origin
  writer.SetFlags (LevelWriter::FLAG_SWAP_V);

  argc--;
  argv++;

  if(!argc)
  {
    csFPrintf (stderr,
         "3D Studio native objectfile converter v2.0\n"
         "originally by Mats Byggmastar 1996 Espoo, Finland.\n"
	 "This version is for Crystal Space and heavily modified by\n"
	 "Jorrit Tyberghein, Luca Pancallo and Matze Braun.\n"
         "Use:  %s [params...] inputfile.3ds\n"
               "params:\n"
	       " -o file   Name of the output file\n"
               " -v        Verbose mode on\n"
               " -l        Don't convert but list objects in 3ds file\n"
	       " -c	   optimize (combine every two triangles into polys)\n"
               " -r x y z  Relocate objects (x,y,z = floats)\n"
	       " -s x y t  Scale objects (x,y,z = floats)\n"
               " -pl       Make polygons lit\n"
               " -3 name   Output 3D sprite instead of level\n"
	       " -tl       Make texture origin lower left (default)\n"
	       " -b	   Use clearzbuf and clearscreen settings\n"
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
  for (int n=0; n<argc; n++)
  {
    if (argv[n][0] == '-' || argv[n][0] == '/')
    {
      switch (toupper(argv[n][1]))
      {
	case 'O':
	  if (n+1<argc) 
	    outfn=argv[++n];
	  else
	  { 
	    csFPrintf (stderr, "Missing outputfile name!\n");
	    return 1;
	  }
	  break;
        case 'R':
	  if (n+3<argc)
	  {
	    xrelocate = atof(argv[++n]);
	    yrelocate = atof(argv[++n]);
	    zrelocate = atof(argv[++n]);
	  }
	  else
	  {
	    csFPrintf(stderr, "Missing relocate value!\n");
	    return 1;
	  }
	  break;
        case 'S':
	  if (n+3<argc)
	  {
	    xscale = atof(argv[++n]);
	    yscale = atof(argv[++n]);
	    zscale = atof(argv[++n]);
	  }
   	  else
	  {
	    csFPrintf(stderr, "Missing scale value!\n");
	    return 1;
	  }
	  break;
        case 'P':
	  if (toupper (argv[n][2]) == 'L')
	  {
	    writer.SetFlags (LevelWriter::FLAG_LIGHTING);
	  }
	  break;
	case 'V':
	  writer.SetFlags(LevelWriter::FLAG_VERBOSE);
	  flags |= FLAG_VERBOSE;
	  break;
        case 'T':
	  if (toupper (argv[n][2]) == 'L')
	    writer.SetFlags (LevelWriter::FLAG_SWAP_V);
	  break;
	case '3':
	  if (n+1<argc)
	    spritename=argv[++n];
	  else
	  {
      	    csFPrintf (stderr, "Missing sprite name!\n");
	    return 1;
	  }
	  flags |= FLAG_SPRITE;
	  break;
	case 'L':
	  flags |= FLAG_LIST;
	  break;
	case 'B':
	  writer.SetFlags (LevelWriter::FLAG_CLEARZBUFCLEARSCREEN);
	  break;
	case 'X':
	  if (toupper (argv[n][2]) == 'Y')
	    mode_xyz = MODE_XYZ;
	  else
	    mode_xyz = MODE_XZY;
	  break;
	case 'Y':
	  if (toupper (argv[n][2]) == 'X')
	    mode_xyz = MODE_YXZ;
	  else
	    mode_xyz = MODE_YZX;
	  break;
	case 'Z':
	  if (toupper (argv[n][2]) == 'X')
	    mode_xyz = MODE_ZXY;
	  else
	    mode_xyz = MODE_ZYX;
	  break;
	case 'C': 
	  writer.SetFlags(LevelWriter::FLAG_COMBINEFACES);
	  writer.SetFlags(LevelWriter::FLAG_REMOVEDOUBLEVERTICES);
	  break;
	default:
	  csFPrintf (stderr, "Bad parameter: %s\n",argv[n]);
	  return 1;
      }
    }
    else
    {
      if (!infn)
        infn = argv[n];
      else 
      {
	csFPrintf (stderr, "Too many filenames (can only convert 1 file at once!\n");
        return 1;
      }
    }
  }

  if (!infn)
  {
    csFPrintf (stderr, "No inputfile specified!\n");
    return 1;
  }

  // <--------- finished parsing parameters ----------->

  // Read inputfile
  Lib3dsFile* file3ds = lib3ds_file_load(infn);
  if (!file3ds ) {
    csFPrintf (stderr, "Failed to open %s\n", infn);
    return 1;
  }

  // swap xyz if requested
  if (mode_xyz != MODE_XYZ)
    Lib3dsConvertXYZ (file3ds, mode_xyz);

  // Print some interesting information about what we have
  if (flags & FLAG_LIST || flags & FLAG_VERBOSE)
  {
    // csFPrintf (stderr, "3DS data size: %ld byte\n", size);
    csFPrintf (stderr, "3DS name: %s\n", file3ds->name);
    csFPrintf (stderr, "lights: %s\n", file3ds->lights ? "yes" : "no");
    csFPrintf (stderr, "object-name     faces vertices  maps  matrix\n");

    // set the current mesh to the first in the file
    Lib3dsMesh* mesh;
    for (mesh = file3ds->meshes; mesh; mesh = mesh->next)
    {
        // get the numbers in the current mesh
        csFPrintf(stderr, "===================================================\n");
        csFPrintf(stderr, "%-14s  %5ld  %5ld  %5d    %s\n",
	    mesh->name, mesh->faces, mesh->points, -1, " ");
    }

    exit(0);
  }

  // setup writer
  writer.Set3dsFile (file3ds);
  writer.SetScale (xscale, yscale, zscale);
  writer.SetTranslate (xrelocate, yrelocate, zrelocate);

  if (flags & FLAG_VERBOSE)
    csFPrintf (stderr, "Writing output in CS format...");
  
  csRef<iDocument> document;
  if (flags & FLAG_SPRITE)
    document = writer.WriteSprite(spritename);
  else
    document = writer.WriteDocument();

  if(!document)
  {
    csFPrintf(stderr, "Problem creating document.\n");
    exit(1);
  }
  
  iString* str = new scfString;
  document->Write (str);
  
  if (outfn)
  {
    // Write file to disk
    FILE* outfile = fopen (outfn, "w");
    if (!outfile)
    {    
      csFPrintf (stderr, "Couldn't open output file '%s'!\n", outfn);
      return 1;
    }
    fwrite (str->GetData(), 1, str->Length(), outfile);
    fclose (outfile);
  }
  else
  {
    // Write file to stdout
    csPrintf ("%s", str->GetData());
  }
  
  if (flags & FLAG_VERBOSE)
    csFPrintf (stderr, "done! \n");

  return 0;
}

