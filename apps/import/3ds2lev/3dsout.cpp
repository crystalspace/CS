/*
 *  CS Object output
 *  Author: Luca Pancallo 2000.09.28
 *   heavily modified by Matze Braun <MatzeBraun@gmx.de>
 */
#include "cssysdef.h"

#include "3dsout.h"
#include "3ds2lev.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "csgeom/vector3.h"
#include "csgeom/plane3.h"
#include "csgeom/math3d.h"

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
  vectors = NULL;
}

CSWriter::CSWriter(FILE* f, Lib3dsFile* data3d)
    : Writer(f), p3dsFile(data3d)
{
  newpointmap = NULL;
  vectors = NULL;
  planes = NULL;
}

CSWriter::~CSWriter()
{
  if (newpointmap)
    delete[] newpointmap;
  if (vectors)
    delete[] vectors;  
  if (planes)
    delete[] planes;
}

void CSWriter::SetScale(float x, float y, float z)
{
  xscale = x;
  yscale = y;
  zscale = z;
}

void CSWriter::SetTranslate(float x, float y, float z)
{
  xrelocate = x;
  yrelocate = y;
  zrelocate = z;
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
        WriteL ("CENTER (%g,%g,%g)",
	    pCurLight->position[0] * xscale + xrelocate, 
	    pCurLight->position[1] * yscale + yrelocate,
	    pCurLight->position[2] * zscale + zrelocate);
        WriteL ("RADIUS (%g)",pCurLight->outer_range * xscale);
        WriteL ("COLOR (%g,%g,%g)",
	    pCurLight->color[0],
	    pCurLight->color[1],
	    pCurLight->color[2]);
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

      Write ("V(%g,%g,%g:", 
	      xyz[0]*xscale + xrelocate, 
	      xyz[1]*yscale + yrelocate,
	      xyz[2]*zscale + zrelocate);
      WriteL ("%g,%g)",u, (flags & FLAG_SWAP_V ? 1.-v : v));
    }
  }
  else
  {
    if (newpointmap)
      delete[] newpointmap;
    newpointmap = new int [mesh->points];
    if (vectors)
	delete[] vectors;
    vectors = new csDVector3 [mesh->points];
    
    for (unsigned int i=0;i<mesh->points;i++)
      newpointmap[i]=-1;

    int newpoint = 0;
    for (unsigned int v = 0; v < mesh->points; v++)
    {
      // doubled point? then do nothing
      if (newpointmap[v] != -1)
	continue;
      
      float* xyz1 = mesh->pointL[v].pos;
      for (unsigned int v2 = v+1; v2 < mesh->points; v2++)
      {
	float* xyz2 = mesh->pointL[v2].pos;

	if (xyz1[0] == xyz2[0] && xyz1[1]==xyz2[1] && xyz1[2] == xyz2[2])
	{
	  newpointmap[v2] = newpoint;
	}
      }
      newpointmap[v] = newpoint;
      vectors[newpoint].Set(xyz1[0], xyz1[1], xyz1[2]);
      WriteL ("VERTEX (%g,%g,%g)    ; %d",
	      xyz1[0]*xscale + xrelocate,
	      xyz1[1]*yscale + yrelocate,
	      xyz1[2]*zscale + zrelocate,
	      newpoint);
      newpoint++;
    }
  }
}

/* This function tries to combine the triangle with number trinum with a
 * polygon (that has been perhaps already combined with other triangles?
 */
typedef unsigned short facenum;

bool RelaxedPlanesEqual(const csDPlane& p1, const csDPlane& p2)
{
    return (( p1.norm - p2.norm) < (double) 13. ) &&
	    ( ABS(p1.DD - p2.DD) < (double) 13. );
}

bool CSWriter::CombineTriangle (Lib3dsMesh* mesh, csDPlane*& plane, int* poly,
	int& plen, int trinum)
{
  facenum* ppoints = mesh->faceL[trinum].points;
  int points[3];
  points[0] = newpointmap[ppoints[0]];
  points[1] = newpointmap[ppoints[1]];
  points[2] = newpointmap[ppoints[2]];

  // this holds the numbers of the 2 shared vertices
  facenum sharedfaces[2];
  // this is the number of the vertice that is left on the triangle
  facenum nonshared;
  
  bool found=false;
  for (int i=0;i<3;i++)
  {
    for (int i2=0; i2<plen; i2++)
    {
      // a point found that is the same on both polys
      // then the next point must correspond to the next (or last) point
      // on the triangle we compare with
      if (poly[i2]==points[i])
      {
	int tp = poly[ (i2+1) % plen ];
	if (tp == points[ (i+1) % 3 ] )
	{
	  found=true;
	  sharedfaces[0]=poly[i2];
	  sharedfaces[1]=tp;
	  // you can write i+2 instead of i-1 (in fact it avoids errors where
	  // i==0
	  nonshared = points[ (i+2) % 3 ];
	  goto pointfound;
	}
	else if (tp == points[ (i+2) % 3 ])
	{
	  found=true;
	  sharedfaces[0]=poly[i2];
	  sharedfaces[1]=tp;
	  nonshared = points[ (i+1) % 3 ];
	  goto pointfound;
	}
      }
    }
  }
  
pointfound:
  if (!found)
    return false;

  if (!RelaxedPlanesEqual (*plane, planes[trinum]) )
  {
    return false;
  }

  // check if the poly shares 3 vertices
  int p;
  for (p=0; p < plen; p++)
  {
    if (poly[p]==nonshared)
    {
      printf ("Warning!!! object '%s' contains a face that overlapps another"
	  "face!\n", mesh->name);
      used[trinum]=true;
      return false;
    }
  }

  // combine the triangle with the poly (insert the nonshared triangle point
  // between the 2 shared points in the poly
  for (p=0; p < plen; p++)
  {
    if (poly[p]==sharedfaces[0])
      break;
  }
  plen++;
  p++;
  for (int p2=plen; p2>p; p2--)
    poly[p2]=poly[p2-1];

  poly[p]=nonshared;

  // check if the 
  return true;
}

void CSWriter::WriteFaces(Lib3dsMesh* mesh, bool lighting, unsigned int numMesh)
{
  if (flags & FLAG_SPRITE)
  {
    for (unsigned int f = 0; f < mesh->faces; f++)
    {
      unsigned short* xyz = mesh->faceL[f].points;
      WriteL ("TRIANGLE (%d,%d,%d)", xyz[0], xyz[1], xyz[2]);
    }
  }
  else
  {
    used = new bool[mesh->faces];
    memset (used, 0, sizeof(bool) * mesh->faces);

    // check if the object is mapped with a method we support
    bool outputuv = true;
    if (!mesh->texelL || mesh->texels!=mesh->points)
      outputuv = false;
    if (mesh->map_data.maptype != LIB3DS_MAP_NONE)
      outputuv = false;
    if (!outputuv)
    {
      printf ("Mesh Object '%s' uses an unsupported mapping type."
	 " Mapping for this object is ignored\n", mesh->name);
    }

    // precalculate normals of the triangles
    if (flags & FLAG_COMBINEFACES)
    {
      if (planes)
	delete[] planes;
      planes = new csDPlane [mesh->faces];
      unsigned int f;
      for (f = 0; f < mesh->faces; f++)
      {
	int p1,p2,p3;
	unsigned short* ppp = mesh->faceL[f].points;
	p1 = newpointmap [ ppp[0] ];
	p2 = newpointmap [ ppp[1] ];
	p3 = newpointmap [ ppp[2] ];
	
	csDMath3::CalcPlane (vectors[p1], vectors[p2], vectors[p3], 
	    planes[f].norm, planes[f].DD);
	planes[f].Normalize();
      }
    }

    for (unsigned int f = 0; f < mesh->faces; f++)
    {
      if (used[f])
	continue;
      
      WriteL ("POLYGON 'x%d_%d'  (", numMesh, f);
      Indent();
      unsigned short* ppp = mesh->faceL[f].points; 
      int poly[1000];
      int plen=3;
      poly[0] = newpointmap[ppp[0]];
      poly[1] = newpointmap[ppp[1]];
      poly[2] = newpointmap[ppp[2]];
      csDPlane* plane = &planes[f];

      // iterate over triangles and try to combine some to make more efficient
      // polgons
      if (flags & FLAG_COMBINEFACES)
      {
  	for (unsigned int f2 = f+1; f2 < mesh->faces; f2++)
    	{
  	  if ( !used[f2] && CombineTriangle(mesh, plane, poly, plen, f2) )
	  {
  	    used[f2]=true;
	    // XXX: It should work without this...
	    break;
	  }
    	}
      }
      Write ("VERTICES (");
      int mappoints[3]; int np=0;
      for (int i=0; i<plen; i++)
      {
	if (np<3 && poly[i] == newpointmap[ppp[np]])
	  mappoints[np++]=i;
	Write ("%d%s", poly[i], i!=plen-1 ? "," : "");
      }
      Write(")");
      
      if (!lighting)
	Write (" LIGHTING (no)");
      WriteL("");

      if (outputuv)
      {
	Write ("TEXTURE (UV (");
	float *uvcoords = mesh->texelL [ ppp[0] ];
	Write ("%d,%g,%g,", mappoints[0],
	    uvcoords[0], flags & FLAG_SWAP_V ? (1.f - uvcoords[1]) : uvcoords[1] );
	uvcoords = mesh->texelL [ ppp[1] ];
	Write ("%d,%g,%g,", mappoints[1],
	    uvcoords[0], flags & FLAG_SWAP_V ? (1.f - uvcoords[1]) : uvcoords[1] );
	uvcoords = mesh->texelL [ ppp[2] ];
	WriteL ("%d,%g,%g))", mappoints[2],
	    uvcoords[0], flags & FLAG_SWAP_V ? (1.f - uvcoords[1]) : uvcoords[1] );
      }
      
      UnIndent();
      WriteL (")"); // close POLYGON
    }

    delete[] used;
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

  // if a light is present in the 3ds file force LIGHTNING (yes)
  if (p3dsFile->lights)
    lighting = true;  

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
                //break;
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
    
    WriteFaces(p3dsMesh, lighting, numMesh);
    
    // move to next object/part
    
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

