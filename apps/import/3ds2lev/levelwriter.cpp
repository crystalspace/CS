/*
 *  CS Object output
 *  Author: Luca Pancallo 2000.09.28
 *   heavily modified by Matze Braun <MatzeBraun@gmx.de>
 */
#include "cssysdef.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include <csgeom/vector3.h>
#include <csgeom/plane3.h>
#include <csgeom/math3d.h>
#include <csutil/xmltiny.h>

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

#include "levelwriter.h"

bool GetObjectNameSetting (const char* name, char setting);

LevelWriter::LevelWriter ()
    : p3dsFile(0), newpointmap(0), vectors(0), planes(0)
{
  flags = 0;
  xml = csPtr<csTinyDocumentSystem> (new csTinyDocumentSystem);
}

LevelWriter::~LevelWriter()
{
  delete[] newpointmap;
  delete[] vectors;  
  delete[] planes;
}

void LevelWriter::SetScale(float x, float y, float z)
{
  xscale = x;
  yscale = y;
  zscale = z;
}

void LevelWriter::SetTranslate(float x, float y, float z)
{
  xrelocate = x;
  yrelocate = y;
  zrelocate = z;
}

void LevelWriter::Set3dsFile (Lib3dsFile* file3ds)
{
  p3dsFile = file3ds;
}

void LevelWriter::SetFlags (int newflags)
{
  flags |= newflags;
}

csPtr<iDocument> LevelWriter::WriteDocument ()
{
  if (!p3dsFile)
    return 0;

  csRef<iDocument> document = xml->CreateDocument();
  csRef<iDocumentNode> rootnode = document->CreateRoot ();

  // Generate a normal world file
  csRef<iDocumentNode> worldnode = 
    rootnode->CreateNodeBefore (CS_NODE_ELEMENT);
  worldnode->SetValue ("world");
    
  WriteTexturesMaterials (worldnode);
  WritePlugins (worldnode);
  WriteStartPoints (worldnode);

  csRef<iDocumentNode> sectornode = 
    worldnode->CreateNodeBefore (CS_NODE_ELEMENT);
  sectornode->SetValue ("sector");
  sectornode->SetAttribute ("name", "room");

  // use dynavis culler
  csRef<iDocumentNode> cullerpnode =
    sectornode->CreateNodeBefore (CS_NODE_ELEMENT);
  cullerpnode->SetValue ("cullerp");
  csRef<iDocumentNode> textnode =
    cullerpnode->CreateNodeBefore (CS_NODE_TEXT);
  textnode->SetValue ("crystalspace.culling.dynavis");
    
  WriteObjects (sectornode);
  WriteLights (sectornode);

  // create settings section
  csRef<iDocumentNode> settingsnode =
    worldnode->CreateNodeBefore(CS_NODE_ELEMENT);
  settingsnode->SetValue ("settings");

  if (flags & FLAG_CLEARZBUFCLEARSCREEN)
  {
    csRef<iDocumentNode> clearzbuf =
      settingsnode->CreateNodeBefore (CS_NODE_ELEMENT);
    clearzbuf->SetValue ("clearzbuf");
    textnode = clearzbuf->CreateNodeBefore (CS_NODE_TEXT);
    textnode->SetValue ("yes");

    csRef<iDocumentNode> clearscreen =
      settingsnode->CreateNodeBefore (CS_NODE_ELEMENT);
    clearscreen->SetValue ("clearscreen");
    textnode = clearscreen->CreateNodeBefore (CS_NODE_TEXT);
    textnode->SetValue ("yes");
  }

  return csPtr<iDocument> (document);
}

void LevelWriter::WriteTexturesMaterials (iDocumentNode* worldnode)
{
  // Generate XML tree
  csRef<iDocumentNode> texturesnode = 
    worldnode->CreateNodeBefore (CS_NODE_ELEMENT);
  csRef<iDocumentNode> materialsnode =
    worldnode->CreateNodeBefore (CS_NODE_ELEMENT);

  if (!texturesnode || !materialsnode)
  {
    printf ("FATAL: Couldn't create texture nodes!\n");
    return;
  }

  texturesnode->SetValue ("textures");
  materialsnode->SetValue ("materials");

  Lib3dsMaterial *pMaterial = p3dsFile->materials;

  // iterate through materials
  for (; pMaterial != 0; pMaterial = pMaterial->next )
  {
    csRef<iDocumentNode> texturenode =
      texturesnode->CreateNodeBefore (CS_NODE_ELEMENT);
    texturenode->SetValue ("texture");
    texturenode->SetAttribute ("name", pMaterial->name);
    csRef<iDocumentNode> filenode =
      texturenode->CreateNodeBefore (CS_NODE_ELEMENT);
    filenode->SetValue("file");
    csRef<iDocumentNode> textnode =
      filenode->CreateNodeBefore (CS_NODE_TEXT);

    // set file name to name of texture, whether it's valid or not
    // (this is only to maintain backward compatibility w/old 3ds2lev)
    textnode->SetValue (pMaterial->name);
    size_t pos = strlen(pMaterial->name);

    // if material name does not end with dot and 3 letters
    // try to get it from the filename field of the texture1_map
    if ( ! ( pos > 3 && pMaterial->name[ pos - 3] == '.')  )
    {
      if ( pMaterial->texture1_map.name && 
           strlen(pMaterial->texture1_map.name) > 1 )
      {
        // todo: prefix w/texture path if one was given on command line
        //textnode->SetValue (strlwr (pMaterial->texture1_map.name));
	textnode->SetValue (pMaterial->texture1_map.name); 
      }
    }
    
    // Create materials nodes
    csRef<iDocumentNode> materialnode =
      materialsnode->CreateNodeBefore (CS_NODE_ELEMENT);
    materialnode->SetValue ("material");
    materialnode->SetAttribute ("name", pMaterial->name);

    csRef<iDocumentNode> mtexturenode =
      materialnode->CreateNodeBefore (CS_NODE_ELEMENT);
    mtexturenode->SetValue("texture");

    csRef<iDocumentNode> mtextnode =
      mtexturenode->CreateNodeBefore (CS_NODE_TEXT);
    mtextnode->SetValue (pMaterial->name);
  }
  
} 

void LevelWriter::WritePlugins (iDocumentNode* worldnode)
{
  csRef<iDocumentNode> pluginsnode = 
    worldnode->CreateNodeBefore (CS_NODE_ELEMENT);
  pluginsnode->SetValue("plugins");

  csRef<iDocumentNode> pluginnode =
    pluginsnode->CreateNodeBefore (CS_NODE_ELEMENT);
  pluginnode->SetValue("plugin");
  pluginnode->SetAttribute ("name", "thing");

  csRef<iDocumentNode> textnode =
    pluginnode->CreateNodeBefore (CS_NODE_TEXT);
  textnode->SetValue ("crystalspace.mesh.loader.thing");
}

void LevelWriter::WriteStartPoints (iDocumentNode* worldnode)
{
  Lib3dsCamera* camera;
  for (camera = p3dsFile->cameras; camera; camera = camera->next)
  {
    csRef<iDocumentNode> startnode =
      worldnode->CreateNodeBefore (CS_NODE_ELEMENT);
    startnode->SetValue ("start");
    csRef<iDocumentNode> sectornode =
      startnode->CreateNodeBefore (CS_NODE_ELEMENT);
    sectornode->SetValue ("sector");
    csRef<iDocumentNode> textnode =
      sectornode->CreateNodeBefore (CS_NODE_TEXT);
    textnode->SetValue ("room");

    csRef<iDocumentNode> positionnode =
      startnode->CreateNodeBefore (CS_NODE_ELEMENT);
    positionnode->SetValue ("position");
    positionnode->SetAttributeAsFloat 
      ("x", camera->position[0] * xscale + xrelocate);
    positionnode->SetAttributeAsFloat 
      ("y", camera->position[1] * yscale + yrelocate);
    positionnode->SetAttributeAsFloat 
      ("z", camera->position[2] * zscale + zrelocate);    
  }
}

void LevelWriter::WriteLights (iDocumentNode* sectornode)
{
  Lib3dsLight *pCurLight;
  
  for (pCurLight = p3dsFile->lights; pCurLight; pCurLight = pCurLight->next)
  {
    // discard spot-lights
    if (pCurLight->spot_light) {
      fprintf (stderr, 
	  "Spotlight are not supported. Light '%s' will not be imported in CS\n",
	  pCurLight->name);
      continue;
    } 

    csRef<iDocumentNode> lightnode =
      sectornode->CreateNodeBefore (CS_NODE_ELEMENT);
    lightnode->SetValue ("light");
    
    csRef<iDocumentNode> centernode =
      lightnode->CreateNodeBefore (CS_NODE_ELEMENT);
    centernode->SetValue ("center");
    centernode->SetAttributeAsFloat
      ("x", pCurLight->position[0] * xscale + xrelocate);
    centernode->SetAttributeAsFloat
      ("y", pCurLight->position[1] * yscale + yrelocate);
    centernode->SetAttributeAsFloat
      ("z", pCurLight->position[2] * zscale + zrelocate);

    csRef<iDocumentNode> radiusnode =
      lightnode->CreateNodeBefore (CS_NODE_ELEMENT);
    radiusnode->SetValue ("radius");
    csRef<iDocumentNode> textnode =
      radiusnode->CreateNodeBefore (CS_NODE_TEXT);
    textnode->SetValueAsFloat (pCurLight->outer_range * xscale);
    
    csRef<iDocumentNode> colornode =
      lightnode->CreateNodeBefore (CS_NODE_ELEMENT);
    colornode->SetValue ("color");
    colornode->SetAttributeAsFloat
      ("red", pCurLight->color[0]);
    colornode->SetAttributeAsFloat
      ("green", pCurLight->color[1]);
    colornode->SetAttributeAsFloat
      ("blue", pCurLight->color[2]);
  }
}

void LevelWriter::WriteVertices (iDocumentNode* paramsnode, Lib3dsMesh* mesh,
				 bool writesprite)
{
  if (writesprite)
  {
    for (unsigned int vn = 0; vn < mesh->points; vn++)
    {
      float *xyz = mesh->pointL[vn].pos;
      Lib3dsTexel* texel = 0;
      if (mesh->texelL) texel = &mesh->texelL[vn];
      float u = 0, v = 0;

      if (texel)
      {
	u = texel[0][0];
	v = texel[0][1];
      }

      csRef<iDocumentNode> vnode = 
	paramsnode->CreateNodeBefore (CS_NODE_ELEMENT);
      vnode->SetValue ("v");
      vnode->SetAttributeAsFloat ("x", xyz[0]*xscale + xrelocate);
      vnode->SetAttributeAsFloat ("y", xyz[1]*yscale + yrelocate);
      vnode->SetAttributeAsFloat ("z", xyz[2]*zscale + zrelocate);
      vnode->SetAttributeAsFloat ("u", u);
      vnode->SetAttributeAsFloat ("v", (flags & FLAG_SWAP_V ? 1.-v : v));
    }
  }
  else
  {
    // Create a new set of vertices
    // so that we can eventually remove doubled verrtices later
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

      // Write Out Vertice
      csRef<iDocumentNode> vnode = 
	paramsnode->CreateNodeBefore (CS_NODE_ELEMENT);
      vnode->SetValue ("v");
      vnode->SetAttributeAsFloat ("x", xyz1[0]*xscale + xrelocate);
      vnode->SetAttributeAsFloat ("y", xyz1[1]*yscale + yrelocate);
      vnode->SetAttributeAsFloat ("z", xyz1[2]*zscale + zrelocate);
      // TODO: Eventually add a comment with the number of the vertex for
      // debugging maps...
      newpoint++;
    }
  }
}

/* This function tries to combine the triangle with number trinum with a
 * polygon (that has been perhaps already combined with other triangles?
 */
typedef unsigned short facenum;

bool LevelWriter::CombineTriangle (Lib3dsMesh* mesh, csDPlane*& plane,
				   int* poly, int& plen, int trinum)
{
  facenum* ppoints = mesh->faceL[trinum].points;
  int points[3];
  points[0] = newpointmap[ppoints[0]];
  points[1] = newpointmap[ppoints[1]];
  points[2] = newpointmap[ppoints[2]];

  // this holds the numbers of the 2 shared vertices
  facenum sharedvertices[2];
  // this is the number of the vertex that is left on the triangle
  facenum nonshared = 0;
  // this is the number of the vertex that is left on the first triangle
  facenum nonshared2 = 0;
  
  bool found=false;
  int i, i2;
  for (i=0;i<3;i++)
  {
    for (i2=0; i2<plen; i2++)
    {
      // a point found that is the same on both polys
      // then the next point must correspond to the next (or last) point
      // on the triangle we compare with
      if (poly[i2]==points[i])
      {
	int tp = poly[ (i2+1) % plen ];
	nonshared2 = poly[ (i2+2) % plen];

	if (tp == points[ (i+1) % 3 ] )
	{
	  found=true;
	  sharedvertices[0]=poly[i2];
	  sharedvertices[1]=tp;
	  // you can write i+2 instead of i-1 (in fact it avoids errors where
	  // i==0
	  nonshared = points[ (i+2) % 3 ];
	  goto pointfound;
	}
	else if (tp == points[ (i+2) % 3 ])
	{
	  found=true;
	  sharedvertices[0]=poly[i2];
	  sharedvertices[1]=tp;
	  nonshared = points[ (i+1) % 3 ];
	  goto pointfound;
	}
      }
    }
  }
  
pointfound:
  if (!found)
    return false;

// TODO: The value of 0.01 requires tweaking...
// This value depends on the scale used. So it probably has to be a parameter.
  double dist = plane->Distance (vectors[nonshared]);
  if (flags & FLAG_VERBOSE)
  {
    fprintf (stderr, "\npl:%g,%g,%g,%g\n",
	plane->norm.x, plane->norm.y, plane->norm.z, plane->DD);
    fprintf (stderr, "ve:%g,%g,%g\n",
	vectors[nonshared].x, vectors[nonshared].y, vectors[nonshared].z);
    fprintf (stderr, "dist:%g\n", dist);
  }
  if (dist > 0.01)
  {
    return false;
  }

  // Check if the poly shares 3 vertices.
  int p;
  for (p=0; p < plen; p++)
  {
    if (poly[p]==nonshared)
    {
      fprintf (stderr, "Warning!!! object '%s' contains a face that overlaps"
	  "another face!\n", mesh->name);
      used[trinum]=true;
      return false;
    }
  }

  // For every edge of the original triangle we calculate a plane
  // that is orthogonal to the plane of the triangle itself.
  csDPlane plane0, plane1, plane2;
  csDMath3::CalcNormal (plane0.norm,
	vectors[nonshared2], vectors[sharedvertices[0]],
	vectors[nonshared2]+plane->norm);
  plane0.DD = -plane0.norm * vectors[nonshared2];
  csDMath3::CalcNormal (plane1.norm,
	vectors[sharedvertices[1]], vectors[nonshared2],
	vectors[sharedvertices[1]]+plane->norm);
  plane1.DD = -plane1.norm * vectors[sharedvertices[1]];
  csDMath3::CalcNormal (plane2.norm,
	vectors[sharedvertices[0]], vectors[sharedvertices[1]],
	vectors[sharedvertices[0]]+plane->norm);
  plane2.DD = -plane2.norm * vectors[sharedvertices[0]];

  // Now classify the nonshared vertex to these three planes. Classification
  // will basically return >0 if vertex is on positive side of plane and
  // <0 otherwise.
  double class0 = plane0.Classify (vectors[nonshared]);
  double class1 = plane1.Classify (vectors[nonshared]);
  double class2 = plane2.Classify (vectors[nonshared]);
  int class_sign0 = class0 < 0 ? -1 : 1;
  int class_sign1 = class1 < 0 ? -1 : 1;
  int class_sign2 = class2 < 0 ? -1 : 1;

  // To result in a convex polygon the sign of class0 and class1
  // must be the same. The reason is that we want the nonshared
  // vertex to be in between the two planes that start from nonshared2
  // and go through the two shared vertices. Since these planes
  // are created thus that everything inside the triangle has same
  // sign we also have to check for same sign here.
  if (class_sign0 != class_sign1)
  {
    // Resulting combination is not convex.
    return false;
  }
  
  // Check if the nonshared vertex is in the first triangle. If this
  // happens we have a partial overlap which is also not good.
  // We can check this when all classifications have same sign.
  if (class_sign0 == class_sign1 && class_sign1 == class_sign2)
  {
    fprintf (stderr, "Warning!!! object '%s' contains a face that partially"
	"overlaps another face!\n", mesh->name);
    //@@@used[trinum]=true; Should I set used in this case?
    return false;
  }

  // combine the triangle with the poly (insert the nonshared triangle point
  // between the 2 shared points in the poly
  for (p=0; p < plen; p++)
  {
    if (poly[p]==sharedvertices[0])
      break;
  }
  plen++;
  p++;
  for (int p2=plen; p2>p; p2--)
    poly[p2]=poly[p2-1];

  poly[p]=nonshared;

  return true;
}

// lighting=flase creates polygon lit with full bright texture colours
void LevelWriter::WriteFaces(iDocumentNode* paramsnode, 
    Lib3dsMesh* mesh, bool lighting, unsigned int numMesh,
    bool writesprite)
{
  if (writesprite)
  {
    for (unsigned int f = 0; f < mesh->faces; f++)
    {
      unsigned short* xyz = mesh->faceL[f].points;
      csRef<iDocumentNode> tnode =
	paramsnode->CreateNodeBefore (CS_NODE_ELEMENT);
      tnode->SetValue ("t");
      tnode->SetAttributeAsFloat ("v1", xyz[0]);
      tnode->SetAttributeAsFloat ("v2", xyz[1]);
      tnode->SetAttributeAsFloat ("v3", xyz[2]);
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
      fprintf (stderr, "Mesh Object '%s' uses an unsupported mapping type."
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

    // if alpha settings are specified for the first face, apply them
    // to all faces.(this could be done per face, but it's understood that
    // each object only has one material anyway )
    Lib3dsMaterial* mat = p3dsFile->materials;
    int alpha = 100;
    if (mesh->faces > 0)
    {
      while( mat && strcmp( mat->name, mesh->faceL[0].material) != 0 )
        mat = mat->next;

      if (mat && mat->transparency > 0 )
        alpha = 100 - ((int) mat->transparency * 100);
    }

    for (unsigned int f = 0; f < mesh->faces; f++)
    {
      if (used[f])
	continue;

      csRef<iDocumentNode> pnode =
	paramsnode->CreateNodeBefore (CS_NODE_ELEMENT);
      pnode->SetValue ("p");
     
      // TODO: a map debug mode could output names for triangles...
      // pnode->SetAttribute ("name", 
      // WriteL ("'x%d_%d'  (", numMesh, f);
      
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
	    // Combining more than 2 triangles is getting too hard to do
	    // it right... so we stop here
	    break;
	  }
    	}
      }

      int mappoints[3]; int np=0;
      for (int i=0; i<plen; i++)
      {
	if (np<3 && poly[i] == newpointmap[ppp[np]])
	  mappoints[np++]=i;

	csRef<iDocumentNode> vnode =
	  pnode->CreateNodeBefore (CS_NODE_ELEMENT);
	vnode->SetValue ("v");
	csRef<iDocumentNode> textnode =
	  vnode->CreateNodeBefore (CS_NODE_TEXT);
	textnode->SetValueAsInt (poly[i]);
      }

      if (alpha < 100)
      {        
	csRef<iDocumentNode> alphanode = 
        pnode->CreateNodeBefore (CS_NODE_ELEMENT);             
	alphanode->SetValue ("alpha");
	csRef<iDocumentNode> textnode =
	    alphanode->CreateNodeBefore (CS_NODE_TEXT);
	textnode->SetValueAsInt (alpha);
      }
      
      if (!lighting)
      {
	csRef<iDocumentNode> lightingnode =
	  pnode->CreateNodeBefore (CS_NODE_ELEMENT);
	lightingnode->SetValue ("lighting");
	csRef<iDocumentNode> textnode =
	  lightingnode->CreateNodeBefore (CS_NODE_TEXT);
	textnode->SetValue ("no");
      }

      if (outputuv)
      {
	csRef<iDocumentNode> texmapnode =
	  pnode->CreateNodeBefore (CS_NODE_ELEMENT);
	texmapnode->SetValue ("texmap");
	
	csRef<iDocumentNode> uvnode;
	
	float *uvcoords = mesh->texelL [ ppp[0] ];
	uvnode = texmapnode->CreateNodeBefore (CS_NODE_ELEMENT);
	uvnode->SetValue ("uv");
	uvnode->SetAttributeAsInt ("idx", mappoints[0]);
	uvnode->SetAttributeAsFloat ("u", uvcoords[0]);
	uvnode->SetAttributeAsFloat ("v", 
	    flags & FLAG_SWAP_V ? (1.f - uvcoords[1]) : uvcoords[1] );

	uvcoords = mesh->texelL [ ppp[1] ];
	uvnode = texmapnode->CreateNodeBefore (CS_NODE_ELEMENT);
	uvnode->SetValue ("uv");
	uvnode->SetAttributeAsInt ("idx", mappoints[1]);
	uvnode->SetAttributeAsFloat ("u", uvcoords[0]);
	uvnode->SetAttributeAsFloat ("v", 
	    flags & FLAG_SWAP_V ? (1.f - uvcoords[1]) : uvcoords[1] );

	uvcoords = mesh->texelL [ ppp[2] ];
	uvnode = texmapnode->CreateNodeBefore (CS_NODE_ELEMENT);
	uvnode->SetValue ("uv");
	uvnode->SetAttributeAsInt ("idx", mappoints[2]);
	uvnode->SetAttributeAsFloat ("u", uvcoords[0]);
	uvnode->SetAttributeAsFloat ("v",                             	
	    flags & FLAG_SWAP_V ? (1.f - uvcoords[1]) : uvcoords[1] );
      }
    }

    delete[] used;
  }
}

csPtr<iDocument> LevelWriter::WriteSprite (const char* spritename)
{
  Lib3dsMesh* mesh = lib3ds_file_mesh_by_name(p3dsFile, spritename);
  if (!mesh)
    return 0;

  csRef<iDocument> document = xml->CreateDocument();
  csRef<iDocumentNode> rootnode = document->CreateRoot ();  
  
  // Generate a sprite file
  csRef<iDocumentNode> meshfactnode =
    rootnode->CreateNodeBefore (CS_NODE_ELEMENT);
  meshfactnode->SetValue ("meshfact");
  meshfactnode->SetAttribute ("name", mesh->name);

  csRef<iDocumentNode> pluginnode =
    meshfactnode->CreateNodeBefore (CS_NODE_ELEMENT);
  pluginnode->SetValue ("plugin");
  csRef<iDocumentNode> textnode =
    pluginnode->CreateNodeBefore (CS_NODE_TEXT);
  textnode->SetValue ("crystalspace.mesh.loader.factory.sprite.3d");

  csRef<iDocumentNode> paramsnode =
    meshfactnode->CreateNodeBefore (CS_NODE_ELEMENT);
  paramsnode->SetValue ("params");
  
  csRef<iDocumentNode> materialnode = 
    paramsnode->CreateNodeBefore (CS_NODE_ELEMENT);
  materialnode->SetValue ("material");
  textnode = 
    materialnode->CreateNodeBefore (CS_NODE_TEXT);
  textnode->SetValue (mesh->faceL->material);

  csRef<iDocumentNode> framenode =
    paramsnode->CreateNodeBefore (CS_NODE_ELEMENT);
  framenode->SetValue ("frame");
  framenode->SetAttribute ("name", "f1");

  WriteVertices (framenode, mesh, true);

  WriteFaces (paramsnode, mesh, false, 0, true);

  csRef<iDocumentNode> actionnode =
    paramsnode->CreateNodeBefore (CS_NODE_ELEMENT);
  actionnode->SetValue ("action");
  actionnode->SetAttribute ("name", "default");
  csRef<iDocumentNode> fnode =
    actionnode->CreateNodeBefore (CS_NODE_ELEMENT);
  fnode->SetValue ("f");
  fnode->SetAttribute ("name", "f1");
  fnode->SetAttributeAsInt ("delay", 1000);

  return csPtr<iDocument> (document);
}

/**
 * Outputs all the objects present in the 3ds file.
 * Based on the object name we create MESHOBJ or PART.
 *
 */
void LevelWriter::WriteObjects (iDocumentNode* sectornode)
{
  Lib3dsMesh *p3dsMesh;

  // iterate on all meshes
  int numMesh = 0;
  bool lighting = true;
  for (p3dsMesh = p3dsFile->meshes; p3dsMesh; 
		    p3dsMesh = p3dsMesh->next, numMesh++)
  {
    // if mesh has no faces, skip it
    if (!p3dsMesh->faceL)
      continue;
      
    csRef<iDocumentNode> meshobjnode = 
      sectornode->CreateNodeBefore (CS_NODE_ELEMENT);
    meshobjnode->SetValue ("meshobj");
    meshobjnode->SetAttribute ("name", p3dsMesh->name);

    csRef<iDocumentNode> pluginnode =
      meshobjnode->CreateNodeBefore (CS_NODE_ELEMENT);
    pluginnode->SetValue ("plugin");
    csRef<iDocumentNode> textnode =
      pluginnode->CreateNodeBefore (CS_NODE_TEXT);
    textnode->SetValue ("thing");

    csRef<iDocumentNode> paramsnode =
      meshobjnode->CreateNodeBefore (CS_NODE_ELEMENT);
    paramsnode->SetValue ("params");

    WriteVertices(paramsnode, p3dsMesh);
    
    csRef<iDocumentNode> materialnode = 
      paramsnode->CreateNodeBefore (CS_NODE_ELEMENT);
    materialnode->SetValue ("material");
    textnode = 
      materialnode->CreateNodeBefore (CS_NODE_TEXT);
    textnode->SetValue (p3dsMesh->faceL->material);

    // if 'l' flag specified, set lighting to false
    lighting = ! GetObjectNameSetting (p3dsMesh->name, 'l');
    // override object level setting if command-line arg was given
    if (flags & FLAG_LIGHTING)
      lighting = false;                                               

    WriteFaces(paramsnode, p3dsMesh, lighting, numMesh);

    csRef<iDocumentNode> zmodenode =
      meshobjnode->CreateNodeBefore (CS_NODE_ELEMENT);
    zmodenode->SetValue ("zuse");

    if (GetObjectNameSetting (p3dsMesh->name, 't') ||
	    strstr(p3dsMesh->name, "_t_")) /* _t_ is old way */
      zmodenode->SetValue ("ztest");
    if ( GetObjectNameSetting (p3dsMesh->name, 'u'))
      zmodenode->SetValue ("zuse");
    if ( GetObjectNameSetting (p3dsMesh->name, 'f'))    
      zmodenode->SetValue ("zfill");
    if ( GetObjectNameSetting (p3dsMesh->name, 'n'))
      zmodenode->SetValue ("znone");

    csRef<iDocumentNode> prioritynode =
      meshobjnode->CreateNodeBefore (CS_NODE_ELEMENT);
    prioritynode->SetValue ("priority");
    csRef<iDocumentNode> prioritytextnode = 
      prioritynode->CreateNodeBefore (CS_NODE_TEXT);
    prioritytextnode->SetValue ("object");

    if (GetObjectNameSetting ( p3dsMesh->name, 'a') || 
         strstr(p3dsMesh->name, "_t_")) // old way
      prioritytextnode->SetValue ("alpha");

    if (GetObjectNameSetting ( p3dsMesh->name, 'w'))
      prioritytextnode->SetValue ("wall");
    if (GetObjectNameSetting ( p3dsMesh->name, 's'))
      prioritytextnode->SetValue ("sky");
    if (GetObjectNameSetting ( p3dsMesh->name, 'o'))
      prioritytextnode->SetValue ("object");                        
  }
}

bool GetObjectNameSetting(const char * objectname, char setting)
{
  if (objectname && strlen(objectname) > 3)
  {
    // if missing either delimiter, return false
    if (objectname[0] != '|') return false;

    const char * fardelim = strchr(&objectname[1], '|');
    if (fardelim == objectname) return false;

    for (int pos = 1; pos < (fardelim - objectname); pos++)
      if (objectname[ pos ] == setting) 
        return true;
  }

  return false;
}
