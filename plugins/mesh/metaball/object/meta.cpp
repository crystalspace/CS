/*
    Metaballs Demo
    Copyright (C) 1999 by Denis Dmitriev

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

#include <stdarg.h>
#include <string.h>
#include "qsqrt.h"
#include "cssysdef.h"
#include "csutil/scf.h"
#include "csutil/cscolor.h"
#include "iengine/rview.h"
#include "iengine/movable.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/material.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "igeom/clip2d.h"
#include "iutil/cfgmgr.h"
#include "isys/system.h"

#include "meta.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csMetaBall)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iMetaBallState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMetaBall::MetaBallState)
  SCF_IMPLEMENTS_INTERFACE (iMetaBallState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

#define MAP_RESOLUTION  256
static float asin_table[2*MAP_RESOLUTION+1];

csMetaBall::csMetaBall (iMeshObjectFactory *fact)
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiMetaBallState);
  th = NULL;
  alpha = frame = 0;
  meta_balls = NULL;
  factory = fact;
  Sys = NULL;
  vis_cb = NULL;
  do_lighting = false;
  initialize = false;
  shape_num = 0;
  cur_camera_num = 0;
  cur_movable_num = 0;
  MixMode = 0;
  rad = csVector3(1,1,1);
//-------------------------------------- Initialize metaball default values
  num_meta_balls = 3;
  max_vertices = 6000;
  env_map_mult = 1.0;
  env_mapping = TRUE_ENV_MAP;
  mp.iso_level = 1.0;
  mp.charge = 3.5;
  mp.rate = 0.03;
  current_lod = 1;
  current_features = ALL_FEATURES;
}

csMetaBall::~csMetaBall ()
{
  if (vis_cb) vis_cb->DecRef ();
  delete [] meta_balls;
  delete [] mesh.triangles;
  delete [] mesh.vertices[0];
  delete [] mesh.texels[0];
  delete [] mesh.vertex_colors[0];
  initialize = false;
}

float csMetaBall::map (float x)
{
  return asin_table[(int)(MAP_RESOLUTION*(1+x))];
}

void csMetaBall::InitTables(void)
{
  for (int i = -MAP_RESOLUTION, j = 0; i <= MAP_RESOLUTION; i++, j++)
  {
    float c = 1.0 * i / MAP_RESOLUTION;
    switch(env_mapping)
    {
      case TRUE_ENV_MAP:
        asin_table[j] = env_map_mult * (0.5+asin(c)/M_PI);
        break;
      case FAKE_ENV_MAP:
        asin_table[j] = 0.5 * env_map_mult * (1 + c);
        break;
    }
  }
}

bool csMetaBall::Initialize ()
{
  if (!initialize)
  {
    initialize = true;
    meta_balls = new MetaBall[num_meta_balls];
    memset(&mesh,0,sizeof(G3DTriangleMesh));
    mesh.num_vertices_pool = 1;
    mesh.triangles = new csTriangle[int(max_vertices/3)];
    mesh.vertices[0] = new csVector3[max_vertices];
    mesh.texels[0] = new csVector2[max_vertices];
    mesh.vertex_colors[0] = new csColor[max_vertices];
    for (int i = 0; i < max_vertices; i++)
    {
      mesh.vertices[0][i].Set(0,0,0);
      mesh.texels[0][i].Set(0,0);
      mesh.vertex_colors[0][i].Set(1,1,1);
    }
    InitTables();
    mesh.do_fog = false;
    mesh.do_mirror = false;
    mesh.do_morph_texels = false;
    mesh.do_morph_colors = false;
    mesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
    NextFrame(0);
  }
  return true;
}

void csMetaBall::SetMetaBallCount (int number)
{
  if (number < 1 || number  == num_meta_balls)
    return;
  num_meta_balls = number;
  delete [] meta_balls;
  meta_balls = new MetaBall [num_meta_balls];

}

void csMetaBall::SetQualityEnvironmentMapping (bool toggle)
{
  env_mapping = toggle ? TRUE_ENV_MAP : FAKE_ENV_MAP;
  InitTables ();
}

void csMetaBall::SetEnvironmentMappingFactor (float env_mult)
{
  env_map_mult = env_mult;
  InitTables ();
}

#if 0
static void LitVertex( const csVector3 &n, csColor &c )
{
  if (n.z > 0)
    c.red = c.green = c.blue = 0;
  else
  {
    float l = n.z*n.z;
    c.red = c.green = c.blue = l;
  }
}
#endif

bool csMetaBall::DrawTest( iRenderView* rview, iMovable* movable)
{
// This is basically a ripped of version of drawtest from the
// ball mesh object.

  iGraphics3D *g3d = rview->GetGraphics3D();
  iCamera *cam = rview->GetCamera();
  csReversibleTransform tr_o2c = cam->GetTransform() *
	  movable->GetFullTransform().GetInverse();
  float fov = cam->GetFOV();
  float shftx = cam->GetShiftX();
  float shfty = cam->GetShiftY();
  csBox2 sbox;
  csBox3 cbox;
  if ( GetScreenBoundingBox( cam->GetCameraNumber(),
	movable->GetUpdateNumber(), fov, shftx,shfty, tr_o2c, sbox, cbox ) < 0)
	return false;
  int clip_portal, clip_plane, clip_z_plane;
  if (rview->ClipBBox( sbox,cbox, clip_portal, clip_plane,
  	clip_z_plane) == false )
    return false;
  g3d->SetObjectToCamera( &tr_o2c );
  mesh.clip_portal = clip_portal;
  mesh.clip_plane = clip_plane;
  mesh.clip_z_plane = clip_z_plane;
  mesh.do_mirror = cam->IsMirrored();
  return true;
}

void csMetaBall::UpdateLighting( iLight **, int, iMovable * )
{
  //TODO: The lighting code is incomplete...
#if 0
      csVector3 n(0, 0, 0);
      for(int k = 0; k < num_meta_balls; k++)
      {
        csVector3 rv(mesh.vertices[0][m].x - meta_balls[k].center.x,
          mesh.vertices[0][m].y - meta_balls[k].center.y,
          mesh.vertices[0][m].z - meta_balls[k].center.z);

        float r = rv.Norm();
        float c = mp.charge / (r*r*r);

        n += rv * c;
      }
      n = n.Unit();
	  LitVertex ( n, mesh.vertex_colors[0][m]);
#endif
}

void csMetaBall::GetObjectBoundingBox(csBox3& bbox, int )
{
  bbox = object_bbox;
}

static void Perspective( const csVector3& v, csVector2& p, float fov,
		float sx, float sy )
{
  float iz = fov / v.z;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}

void csMetaBall::CreateBoundingBox()
{
  float minx, maxx, miny, maxy, minz, maxz;
  minx = maxx = meta_balls[0].center.x;
  miny = maxy = meta_balls[0].center.y;
  minz = maxz = meta_balls[0].center.z;
  csBox3 bb;

  int maxxi = 0, maxyi = 0, maxzi = 0, minxi = 0, minyi = 0, minzi = 0;

  for ( int i = 0; i < mesh.num_vertices; i++ )
  {
	if ( mesh.vertices[0][i].x < minx ) {minx = mesh.vertices[0][i].x; minxi = i; }
	else
	if ( mesh.vertices[0][i].x > maxx ) {maxx = mesh.vertices[0][i].x; maxxi = i; }

	if ( mesh.vertices[0][i].y < miny ) {miny = mesh.vertices[0][i].y; minyi = i; }
	else
	if ( mesh.vertices[0][i].y > maxy ) {maxy = mesh.vertices[0][i].y; maxyi = i; }

	if ( mesh.vertices[0][i].z < minz ) {minz = mesh.vertices[0][i].z; minzi = i; }
	else
	if ( mesh.vertices[0][i].z > maxz ) {maxz = mesh.vertices[0][i].z; maxzi = i; }
  }
  bb.StartBoundingBox( csVector3(minx,miny,minz));
  bb.AddBoundingVertexSmart( csVector3(maxx,maxy,maxz));
  if ( !object_bbox.Contains(bb)) shape_num++;
  object_bbox = bb;
  rad.Set(object_bbox.Max() - object_bbox.GetCenter());

#if 0
  printf("Minimum bounding found at %d,%d,%d\n", minxi, minyi, minzi);
  printf("Maximum bounding found at %d,%d,%d\n", maxxi, maxyi, maxzi);
  printf("Minimum bounding vertex at (%f,%f,%f)\n", minx, miny, minz);
  printf("Maximum bounding vertex at (%f,%f,%f)\n", maxx, maxy, maxz);
  printf("Vertex zero : (%f,%f,%f)\n",mesh.vertices[0][0].x, mesh.vertices[0][0].y, mesh.vertices[0][0].z);
  printf("Num verts : %d\n", mesh.num_vertices);
#endif

}

float csMetaBall::GetScreenBoundingBox( long cam_num, long mov_num,
		float fov, float sx, float sy, const csReversibleTransform& trans,
		csBox2& sbox, csBox3& cbox )
{
  csVector2 one_corner;
  GetTransformedBoundingBox( cam_num, mov_num, trans, cbox );
  if ((cbox.MinZ() < 0) && (cbox.MaxZ() < 0))
	return -1.0;
  if (cbox.MinZ() <= 0)
	sbox.Set(-10000,-10000,10000,10000);
  else
  {
	Perspective( cbox.Max(), one_corner, fov, sx, sy );
	sbox.StartBoundingBox( one_corner );
	csVector3 v ( cbox.MinX(), cbox.MinY(), cbox.MaxZ());
	Perspective( v, one_corner, fov, sx, sy );
	sbox.AddBoundingVertexSmart( one_corner );
	v.Set( cbox.MaxX(), cbox.MaxY(), cbox.MinZ());
	Perspective( v, one_corner, fov, sx, sy );
	sbox.AddBoundingVertexSmart( one_corner );
  }
  return cbox.MaxZ();
}
void csMetaBall::GetTransformedBoundingBox( long cam_num, long move_num,
		const csReversibleTransform& trans, csBox3& cbox)
{
  if ((cur_camera_num == cam_num) && (cur_movable_num == move_num))
  {
	cbox = camera_bbox;
	return;
  }
  cur_camera_num = cam_num;
  cur_movable_num = move_num;
  camera_bbox.StartBoundingBox( trans * object_bbox.Min());
  camera_bbox.AddBoundingVertexSmart( trans * object_bbox.Max());
  cbox = camera_bbox;
}

bool csMetaBall::HitBeamBBox (const csVector3& start, const csVector3& end)
{
  csVector3 isect;
  return HitBeamObject (start, end, isect, NULL);
}

bool csMetaBall::HitBeamOutline (const csVector3& start, const csVector3& end)
{
  csVector3 isect;
  return HitBeamObject (start, end, isect, NULL);
}

bool csMetaBall::HitBeamObject( const csVector3& start, const csVector3& end,
  csVector3& isect, float *pr)
{
  // @@@ We might consider checking to a lower LOD version only.
  // This function is not very fast if the bounding box test succeeds.
  // Plagarism notice: Ripped form Sprite3D.
  csSegment3 seg (start, end);
  if (csIntersect3::BoxSegment (object_bbox, seg, isect, pr) < 0)
    return false;
  int i, max = int(vertices_tesselated/3);
  for (i = 0 ; i < max ; i++)
  {
    if (csIntersect3::IntersectTriangle (mesh.vertices[0][i+2], mesh.vertices[0][i+1],
    	mesh.vertices[0][i], seg, isect))
    {
      if (pr)
      {
        *pr = qsqrt (csSquaredDist::PointPoint (start, isect) /
		csSquaredDist::PointPoint (start, end));
      }
      return true;
    }
  }
  return false;
}

void csMetaBall::NextFrame(csTime)
{
  if ((mesh.num_vertices == 0) || (mp.rate != 0))
  {
  int i,j,m;
  float l = ( do_lighting ) ? 1.0 : 0.0;
  alpha += mp.rate;
  mesh.num_vertices = 0;
  mesh.num_triangles = 0;
//-------------------------------- MetaBall - Set position -------
// Below is the generic position calculation that was supplied with
// metaballs by Denis Dmitriev. In future I will be looking at replacing
// this method with a definable skeletal animation method.

#if 1
  for (i = 0; i < num_meta_balls; i++)
  {
    float m = fmod((i + 1) / 3.0, 1.5) + 0.5;

    csVector3 &c = meta_balls[i].center;
    c.x = 4 * m * sin (m * alpha + i * M_PI / 4);
    c.y = 4 * m * cos (1.4 * m * alpha + m * M_PI / 6);
    c.z = 4 * sin (m * alpha * 1.3214);

//  printf("Ball %d, (%f,%f,%f)\n",i, c.x,c.y,c.z);
  }
#endif

//----------------------------------------------------------------
// Calculate meta balls is the core of the meta ball engine.
// Everything in this module is basically housekeeping routines,
// except for InitTables, which sets up the correct tables for the
// resolution used.

  CalculateMetaBalls();

//----------------------------------------------------------------
// The next routine sets up the triangles and the texturing for
// the mesh...
  int num = 0, trigs = int( vertices_tesselated / 3);
  for (i = 0; i < trigs; i++, num += 3)
  {
	// Fill our mesh buffer with the created triangles
	mesh.triangles[i].a = num + 2;
	mesh.triangles[i].b = num + 1;
	mesh.triangles[i].c = num;

	for ( j = 0; j < 3; j++ )
	{
	  m = num + j;
      csVector3 n(0, 0, 0);
      for(int k = 0; k < num_meta_balls; k++)
      {
      	csVector3 rv(mesh.vertices[0][m].x - meta_balls[k].center.x,
          mesh.vertices[0][m].y - meta_balls[k].center.y,
          mesh.vertices[0][m].z - meta_balls[k].center.z);

        float r = rv.Norm();
        float c = mp.charge / (r*r*r);

        n += rv * c;
      }
      n = n.Unit();
	  mesh.texels[0][m].x = map (n.x);
	  mesh.texels[0][m].y = map (n.y);
	  csColor &c = mesh.vertex_colors[0][m];
	  c.red = c.blue = c.green = l; // Default lighting. Not real perty yet.
	}
  }
  mesh.num_triangles = trigs;
  mesh.num_vertices = vertices_tesselated;
  CreateBoundingBox();
  } // If Rate != 0
}

void csMetaBall::HardTransform( const csReversibleTransform& )
{
  printf("Hard transform\n");
}

bool csMetaBall::Draw( iRenderView* rview, iMovable* /* movable */,
		csZBufMode mode )
{
  if (!th)
  {
	printf("csMetaBall: Draw aborted, no material applied to this object\n");
	return false;
  }
  mesh.mat_handle = th->GetMaterialHandle();
  mesh.use_vertex_color = true;

#if 0
  printf("Dumping Triangles: %d Vertices: %d\n", mesh.num_triangles, mesh.num_vertices );
  for (int i = 0; i < mesh.num_vertices; i++ )
  {
	csVector3 v = mesh.vertices[0][i];
	printf("V(%f,%f,%f) ; %d\n", v.x,v.y,v.z, i);
  }
  for ( int i = 0; i < mesh.num_triangles; i++ )
  {
	csTriangle tr = mesh.triangles[i];
	printf("TRIANGLE(%d,%d,%d) ; %d\n",tr.a,tr.b,tr.c,i);
  }
  printf("Finished dump =============================================\n");
#endif

  if (vis_cb) if (!vis_cb->BeforeDrawing ( this, rview )) return false;
  iGraphics3D* G3D = rview->GetGraphics3D();
  G3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, mode);
  mesh.fxmode = MixMode | CS_FX_GOURAUD;
  G3D->DrawTriangleMesh(mesh);
  return true;
}

SCF_IMPLEMENT_IBASE(csMetaBallFactory)
  SCF_IMPLEMENTS_INTERFACE(iMeshObjectFactory)
SCF_IMPLEMENT_IBASE_END

csMetaBallFactory::csMetaBallFactory( iBase *par )
{
  SCF_CONSTRUCT_IBASE(par);
}
csMetaBallFactory::~csMetaBallFactory()
{
}
iMeshObject* csMetaBallFactory::NewInstance()
{
  csMetaBall* cm = new csMetaBall((iMeshObjectFactory *) this);
  cm->Initialize();
  iMeshObject* im = SCF_QUERY_INTERFACE( cm, iMeshObject );
  im->DecRef();
  return im;
}

SCF_IMPLEMENT_IBASE (csMetaBallType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMetaBallType::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csMetaBallType)

SCF_EXPORT_CLASS_TABLE (metaball)
  SCF_EXPORT_CLASS (csMetaBallType, "crystalspace.mesh.object.metaball",
    "Crystal Space MetaBall mesh object")
SCF_EXPORT_CLASS_TABLE_END

csMetaBallType::csMetaBallType( iBase *par )
{
  SCF_CONSTRUCT_IBASE (par);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
}

csMetaBallType::~csMetaBallType()
{
}

iMeshObjectFactory* csMetaBallType::NewFactory()
{
  csMetaBallFactory* cm = new csMetaBallFactory(this);
  iMeshObjectFactory* ifact = SCF_QUERY_INTERFACE(cm, iMeshObjectFactory);
  ifact->DecRef();
  return ifact;
}
