/*
    Copyright (C) 1999-2001 by Jorrit Tyberghein
  
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
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "csgeom/vector3.h"
#include "csutil/garray.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "iengine/rview.h"
#include "isys/system.h"
#include "ivideo/txtmgr.h"
#include "isys/vfs.h"
#include "terrfunc.h"

IMPLEMENT_IBASE (csTerrFuncObject)
  IMPLEMENTS_INTERFACE (iTerrainObject)
  IMPLEMENTS_EMBEDDED_INTERFACE (iTerrFuncState)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csTerrFuncObject::TerrFuncState)
  IMPLEMENTS_INTERFACE (iTerrFuncState)
IMPLEMENT_EMBEDDED_IBASE_END

static float TerrFunc (void*, float x, float y)
{
  return 8. * (sin (x*40.)+cos (y*40.));
}

csTerrFuncObject::csTerrFuncObject (iSystem* pSys,
	iTerrainObjectFactory *pFactory)
{
  CONSTRUCT_IBASE (NULL)
  CONSTRUCT_EMBEDDED_IBASE (scfiTerrFuncState);
  pSystem = pSys;
  csTerrFuncObject::pFactory = pFactory;
  initialized = false;
  blockx = 4;
  blocky = 4;
  gridx[0] = 4; gridy[0] = 4;
  gridx[1] = 3; gridy[1] = 3;
  gridx[2] = 2; gridy[2] = 2;
  gridx[3] = 1; gridy[3] = 1;
  topleft.Set (0, 0, 0);
  scale.Set (1, 1, 1);
  normals[0] = NULL;
  normals[1] = NULL;
  normals[2] = NULL;
  normals[3] = NULL;
  dirlight_numbers[0] = NULL;
  dirlight_numbers[1] = NULL;
  dirlight_numbers[2] = NULL;
  dirlight_numbers[3] = NULL;
  do_dirlight = false;
  dirlight_number = 0;
  materials = NULL;
  base_color.red = 0;
  base_color.green = 0;
  base_color.blue = 0;
  height_func = TerrFunc;
  normal_func = NULL;
}

csTerrFuncObject::~csTerrFuncObject ()
{
  int i;
  for (i = 0 ; i < 4 ; i++)
    delete[] trimesh[i]; // @@@ Memory leak: Cleanup vertex arrays too!
  delete[] normals[0];
  delete[] normals[1];
  delete[] normals[2];
  delete[] normals[3];
  delete[] dirlight_numbers[0];
  delete[] dirlight_numbers[1];
  delete[] dirlight_numbers[2];
  delete[] dirlight_numbers[3];
  delete[] materials;
}

void csTerrFuncObject::LoadMaterialGroup (iEngine* engine, const char *pName,
	int iStart, int iEnd)
{
  if (!materials)
    materials = new iMaterialWrapper* [blockx * blocky];

  int i;
  char pMatName[256];

  for (i = iStart ; i <= iEnd ; i++)
  {
    sprintf (pMatName, pName, i);
    iMaterialWrapper* mat = engine->FindMaterial (pMatName);
    int bx = i % blockx;
    int by = i / blockx;
    int newi = bx*blockx + by;
    materials[newi] = mat;
  }
}

void csTerrFuncObject::ComputeNormals ()
{
  int lod;
  int i1, i, x, y;
  float dx, dy;
  float inv_totx = 1. / float (1+blockx*gridx[0]);
  float inv_toty = 1. / float (1+blocky*gridy[0]);
  csVector3 n, v[8];
  for (lod = 0 ; lod < 4 ; lod++)
  {
    int totx = 1+blockx*gridx[lod];
    int toty = 1+blocky*gridy[lod];
    float ftotx = float (totx);
    float ftoty = float (toty);
    for (y = 0 ; y < toty ; y++)
    {
      dy = float (y) / ftoty;
      for (x = 0 ; x < totx ; x++)
      {
	dx = float (x) / ftotx;
	if (normal_func)
	  n = normal_func (normal_func_data, dx, dy);
	else
	{
	  v[0].Set (-.1, height_func (height_func_data, dx-inv_totx, dy-inv_toty), -.1);
	  v[1].Set (  0, height_func (height_func_data, dx, dy-inv_toty), -.1);
	  v[2].Set ( .1, height_func (height_func_data, dx+inv_totx, dy-inv_toty), -.1);
	  v[3].Set ( .1, height_func (height_func_data, dx+inv_totx, dy), 0);
	  v[4].Set ( .1, height_func (height_func_data, dx+inv_totx, dy+inv_toty),  .1);
	  v[5].Set (  0, height_func (height_func_data, dx, dy+inv_toty),  .1);
	  v[6].Set (-.1, height_func (height_func_data, dx-inv_totx, dy+inv_toty),  .1);
	  v[7].Set (-.1, height_func (height_func_data, dx-inv_totx, dy), 0);
	  n.Set (0, 0, 0);
	  i1 = 7;
	  for (i = 0 ; i < 8 ; i++)
	  {
	    n += (v[i1] % v[i]).Unit ();
	    i1 = i;
	  }
	  n.Normalize ();
	}
	normals[lod][y*totx+x] = n;
      }
    }
  }
}

void csTerrFuncObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;
    if (!materials) materials = new iMaterialWrapper* [blockx * blocky];
    int lod;
    for (lod = 0 ; lod < 4 ; lod++)
    {
      delete[] trimesh[lod];
      delete[] normals[lod];
      delete[] dirlight_numbers[lod];
      G3DTriangleMesh* meshes = new G3DTriangleMesh [blockx * blocky];
      trimesh[lod] = meshes;
      normals[lod] = new csVector3 [(1+blockx*gridx[lod])
      	* (1+blocky*gridy[lod])];
      dirlight_numbers[lod] = new long [blockx * blocky];
    }

    ComputeNormals ();

    for (lod = 0 ; lod < 4 ; lod++)
    {
      G3DTriangleMesh* meshes = trimesh[lod];
      int bx, by, gx, gy;
      for (by = 0 ; by < blocky ; by++)
        for (bx = 0 ; bx < blockx ; bx++)
	{
	  dirlight_numbers[lod][by*blockx+bx] = -1;
	  G3DTriangleMesh* m = &meshes[by*blockx + bx];
	  m->vertex_colors[0] = NULL;
    	  m->morph_factor = 0;
    	  m->num_vertices_pool = 1;
    	  m->num_materials = 1;
    	  m->use_vertex_color = true;
    	  m->do_morph_texels = false;
    	  m->do_morph_colors = false;
    	  m->do_fog = false;
    	  m->vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
    	  m->fxmode = CS_FX_GOURAUD;
	  m->num_vertices = (gridx[lod]+1) * (gridy[lod]+1);
	  m->vertices[0] = new csVector3[m->num_vertices];
	  m->texels[0][0] = new csVector2[m->num_vertices];
	  m->vertex_fog = new G3DFogInfo [m->num_vertices];
	  m->vertex_colors[0] = new csColor[m->num_vertices];
	  csVector3 tl = topleft;
	  tl.x += bx*scale.x;
	  tl.y += 0;
	  tl.z += by*scale.z;
	  csVector2 tluv (0, 0);
#if 0
	  tluv.x += float (bx) / float (blockx);
	  tluv.y += float (by) / float (blocky);
#endif
	  float dx, dy;
	  for (gy = 0 ; gy <= gridy[lod] ; gy++)
	  {
	    dy = float (by*gridy[lod]+gy) / float (1. + blocky * gridy[lod]);
	    for (gx = 0 ; gx <= gridx[lod] ; gx++)
	    {
	      dx = float (bx*gridx[lod]+gx) / float (1. + blockx * gridx[lod]);
	      int vtidx = gy*(gridx[lod]+1)+gx;
	      csVector3 v = tl;
	      v.x += gx*scale.x / float (gridx[lod]);
	      v.y += height_func (height_func_data, dx, dy) * scale.y;
	      v.z += gy*scale.z / float (gridy[lod]);
	      m->vertices[0][vtidx] = v;
	      csVector2 uv = tluv;
#if 0
	      uv.x += float (gx) / float (blockx*gridx[lod]);
	      uv.y += float (gy) / float (blocky*gridy[lod]);
#else
	      uv.x += float (gx) / float (gridx[lod]);
	      uv.y += float (gy) / float (gridy[lod]);
	      //uv.x = uv.x * .999;// + .0005;
	      //uv.y = uv.y * .999;// + .0005;
#endif
	      m->texels[0][0][vtidx] = uv;
	      m->vertex_colors[0][vtidx].Set (1, 1, 1);
	    }
	  }
	  m->num_triangles = 2*gridx[lod]*gridy[lod];
	  m->triangles = new csTriangle [m->num_triangles];
	  for (gy = 0 ; gy < gridy[lod] ; gy++)
	    for (gx = 0 ; gx < gridx[lod] ; gx++)
	    {
	      int tridx = (gy*gridx[lod]+gx) * 2;
	      int vtidx = gy*(gridx[lod]+1)+gx;	// Top left vertex of triangles
	      m->triangles[tridx].c = vtidx;
	      m->triangles[tridx].b = vtidx+1;
	      m->triangles[tridx].a = vtidx+gridx[lod]+1;
	      m->triangles[tridx+1].c = vtidx+1;
	      m->triangles[tridx+1].b = vtidx+gridx[lod]+1+1;
	      m->triangles[tridx+1].a = vtidx+gridx[lod]+1;
	    }
        }
    }
  }
}

void csTerrFuncObject::RecomputeLighting (int lod, int bx, int by)
{
  if (!do_dirlight) return;
  int blidx = by*blockx + bx;
  if (dirlight_number != dirlight_numbers[lod][blidx])
  {
    dirlight_numbers[lod][blidx] = dirlight_number;
    G3DTriangleMesh* meshes = trimesh[lod];
    G3DTriangleMesh* m = &meshes[blidx];
    int gx, gy;
    csColor* vtcols = m->vertex_colors[0];
    int normidx_dim = 1+blockx*gridx[lod];
    int normidx = by*gridy[lod]*normidx_dim + bx*gridx[lod];
    csVector3* norms_y = &normals[lod][normidx];
    for (gy = 0 ; gy <= gridy[lod] ; gy++, norms_y += normidx_dim)
    {
      csVector3* norms = norms_y;
      for (gx = 0 ; gx <= gridx[lod] ; gx++)
      {
	float l = dirlight * *(norms++);
	(*(vtcols++)).Set (
		base_color.red+dirlight_color.red * l,
		base_color.green+dirlight_color.green * l,
		base_color.blue+dirlight_color.blue * l);
      }
    }
  }
}

void csTerrFuncObject::Draw (iRenderView* rview, bool use_z_buf)
{
  SetupObject ();
  iGraphics3D* pG3D = rview->GetGraphics3D ();
  iCamera* pCamera = rview->GetCamera ();

  csReversibleTransform& camtrans = pCamera->GetTransform ();
  iClipper2D* pClipper = rview->GetClipper ();
  pG3D->SetObjectToCamera (&camtrans);
  pG3D->SetClipper (pClipper->GetClipPoly (), pClipper->GetNumVertices ());
  pG3D->SetPerspectiveAspect (pCamera->GetFOV ());
  pG3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE,
      use_z_buf ? CS_ZBUF_USE : CS_ZBUF_FILL);

  int bx, by;
  int blidx = 0;
  for (by = 0 ; by < blocky ; by++)
  {
    for (bx = 0 ; bx < blockx ; bx++, blidx++)
    {
      RecomputeLighting (0, bx, by);
      G3DTriangleMesh* meshes = trimesh[0];
      G3DTriangleMesh* m = &meshes[blidx];
      m->mat_handle[0] = materials[blidx]->GetMaterialHandle ();
      if (!m->mat_handle[0])
        m->mat_handle[0] = materials[0]->GetMaterialHandle ();
      m->do_mirror = pCamera->IsMirrored ();
      m->do_clip = true;
      pG3D->DrawTriangleMesh (*m);
    }
  }
}

int csTerrFuncObject::CollisionDetect (csTransform* transform)
{
  // Translate us into terrain coordinate space.
  csVector3 p = transform->GetOrigin () - topleft;
  p.x /= scale.x * float (blockx);
  p.z /= scale.z * float (blocky);
  // If our location is outside the terrain then we cannot hit it.
  if (p.x < 0 || p.z < 0 || p.x > 1 || p.z > 1) return 0;

  // Return height of terrain at this location in Y coord.
  float h = height_func (height_func_data, p.x, p.z)*scale.y+2;
  if (h < p.y) return 0;
  p.y = h;
  // Translate us back.
  p.x *= scale.x * float (blockx);
  p.z *= scale.z * float (blocky);
  p = p + topleft;
  transform->SetOrigin (p);
  return 1;
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csTerrFuncObjectFactory)
  IMPLEMENTS_INTERFACE (iTerrainObjectFactory)
IMPLEMENT_IBASE_END

csTerrFuncObjectFactory::csTerrFuncObjectFactory (iSystem* pSys)
{
  CONSTRUCT_IBASE (NULL);
  pSystem = pSys;
}

csTerrFuncObjectFactory::~csTerrFuncObjectFactory ()
{
}

iTerrainObject* csTerrFuncObjectFactory::NewInstance ()
{
  csTerrFuncObject* pTerrObj = new csTerrFuncObject (pSystem, this);
  return (iTerrainObject*)pTerrObj;
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csTerrFuncObjectType)
  IMPLEMENTS_INTERFACE (iTerrainObjectType)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csTerrFuncObjectType)

EXPORT_CLASS_TABLE (terrfunc)
  EXPORT_CLASS (csTerrFuncObjectType, "crystalspace.terrain.object.terrfunc",
    "Crystal Space Function Terrain Type")
EXPORT_CLASS_TABLE_END

csTerrFuncObjectType::csTerrFuncObjectType (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csTerrFuncObjectType::~csTerrFuncObjectType ()
{
}

bool csTerrFuncObjectType::Initialize (iSystem *pSys)
{
  pSystem = pSys;
  return true;
}

iTerrainObjectFactory* csTerrFuncObjectType::NewFactory()
{
  csTerrFuncObjectFactory *pFactory = new csTerrFuncObjectFactory (pSystem);
  return (iTerrainObjectFactory*)pFactory;
}
