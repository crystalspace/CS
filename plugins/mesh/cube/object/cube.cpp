/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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
#include "csgeom/math3d.h"
#include "cube.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "iengine/camera.h"
#include "igeom/clip2d.h"
#include "iengine/light.h"
#include "qsqrt.h"

CS_IMPLEMENT_PLUGIN

IMPLEMENT_IBASE (csCubeMeshObject)
  IMPLEMENTS_INTERFACE (iMeshObject)
IMPLEMENT_IBASE_END

csCubeMeshObject::csCubeMeshObject (csCubeMeshObjectFactory* factory)
{
  CONSTRUCT_IBASE (NULL);
  csCubeMeshObject::factory = factory;
  ifactory = QUERY_INTERFACE (factory, iMeshObjectFactory);
  initialized = false;
  cur_cameranr = -1;
  cur_movablenr = -1;
  vis_cb = NULL;
  sizex = factory->GetSizeX ();
  sizey = factory->GetSizeY ();
  sizez = factory->GetSizeZ ();
  shift = factory->GetShift ();
  // Calculate the maximum radius.
  float max_size = sizex;
  if (sizey > max_size) max_size = sizey;
  if (sizez > max_size) max_size = sizez;
  float a = max_size/2.;
  float r = qsqrt (a*a + a*a);
  radius.Set (r, r, r);
  shapenr = 0;
  current_lod = 1;
  current_features = ALL_FEATURES;
}

csCubeMeshObject::~csCubeMeshObject ()
{
  if (vis_cb) vis_cb->DecRef ();
  if (ifactory) ifactory->DecRef ();
}

void csCubeMeshObject::GetTransformedBoundingBox (long cameranr,
	long movablenr, const csReversibleTransform& trans, csBox3& cbox)
{
  if (cur_cameranr == cameranr && cur_movablenr == movablenr)
  {
    cbox = camera_bbox;
    return;
  }
  cur_cameranr = cameranr;
  cur_movablenr = movablenr;

  camera_bbox.StartBoundingBox (trans * vertices[0]);
  camera_bbox.AddBoundingVertexSmart (trans * vertices[1]);
  camera_bbox.AddBoundingVertexSmart (trans * vertices[2]);
  camera_bbox.AddBoundingVertexSmart (trans * vertices[3]);
  camera_bbox.AddBoundingVertexSmart (trans * vertices[4]);
  camera_bbox.AddBoundingVertexSmart (trans * vertices[5]);
  camera_bbox.AddBoundingVertexSmart (trans * vertices[6]);
  camera_bbox.AddBoundingVertexSmart (trans * vertices[7]);

  cbox = camera_bbox;
}

static void Perspective (const csVector3& v, csVector2& p, float fov,
    	float sx, float sy)
{
  float iz = fov / v.z;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}

float csCubeMeshObject::GetScreenBoundingBox (long cameranr,
	long movablenr, float fov, float sx, float sy,
	const csReversibleTransform& trans, csBox2& sbox, csBox3& cbox)
{
  csVector2 oneCorner;

  GetTransformedBoundingBox (cameranr, movablenr, trans, cbox);

  // if the entire bounding box is behind the camera, we're done
  if ((cbox.MinZ () < 0) && (cbox.MaxZ () < 0))
  {
    return -1;
  }

  // Transform from camera to screen space.
  if (cbox.MinZ () <= 0)
  {
    // Sprite is very close to camera.
    // Just return a maximum bounding box.
    sbox.Set (-10000, -10000, 10000, 10000);
  }
  else
  {
    Perspective (cbox.Max (), oneCorner, fov, sx, sy);
    sbox.StartBoundingBox (oneCorner);
    csVector3 v (cbox.MinX (), cbox.MinY (), cbox.MaxZ ());
    Perspective (v, oneCorner, fov, sx, sy);
    sbox.AddBoundingVertexSmart (oneCorner);
    Perspective (cbox.Min (), oneCorner, fov, sx, sy);
    sbox.AddBoundingVertexSmart (oneCorner);
    v.Set (cbox.MaxX (), cbox.MaxY (), cbox.MinZ ());
    Perspective (v, oneCorner, fov, sx, sy);
    sbox.AddBoundingVertexSmart (oneCorner);
  }

  return cbox.MaxZ ();
}

void csCubeMeshObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;
    float sx = sizex/2;
    float sy = sizey/2;
    float sz = sizez/2;
    vertices[0].Set (-sx, -sy, -sz);
    vertices[1].Set ( sx, -sy, -sz);
    vertices[2].Set (-sx,  sy, -sz);
    vertices[3].Set ( sx,  sy, -sz);
    vertices[4].Set (-sx, -sy,  sz);
    vertices[5].Set ( sx, -sy,  sz);
    vertices[6].Set (-sx,  sy,  sz);
    vertices[7].Set ( sx,  sy,  sz);
    int i;
    object_bbox.StartBoundingBox ();
    for (i = 0 ; i < 8 ; i++)
    {
      normals[i] = vertices[i]; normals[i].Normalize ();
      vertices[i] += shift;
      object_bbox.AddBoundingVertex (vertices[i]);
    }
    uv[0].Set (0, 0);
    uv[1].Set (1, 0);
    uv[2].Set (0, 1);
    uv[3].Set (1, 1);
    uv[4].Set (1, 1);
    uv[5].Set (0, 0);
    uv[6].Set (0, 0);
    uv[7].Set (0, 1);
    colors[0].Set (0, 0, 0);
    colors[1].Set (1, 0, 0);
    colors[2].Set (0, 1, 0);
    colors[3].Set (0, 0, 1);
    colors[4].Set (1, 1, 0);
    colors[5].Set (1, 0, 1);
    colors[6].Set (0, 1, 1);
    colors[7].Set (1, 1, 1);
    triangles[0].a = 0; triangles[0].b = 2; triangles[0].c = 3;
    triangles[1].a = 0; triangles[1].b = 3; triangles[1].c = 1;
    triangles[2].a = 1; triangles[2].b = 3; triangles[2].c = 7;
    triangles[3].a = 1; triangles[3].b = 7; triangles[3].c = 5;
    triangles[4].a = 7; triangles[4].b = 4; triangles[4].c = 5;
    triangles[5].a = 7; triangles[5].b = 6; triangles[5].c = 4;
    triangles[6].a = 6; triangles[6].b = 0; triangles[6].c = 4;
    triangles[7].a = 6; triangles[7].b = 2; triangles[7].c = 0;
    triangles[8].a = 6; triangles[8].b = 7; triangles[8].c = 3;
    triangles[9].a = 6; triangles[9].b = 3; triangles[9].c = 2;
    triangles[10].a = 0; triangles[10].b = 1; triangles[10].c = 4;
    triangles[11].a = 1; triangles[11].b = 5; triangles[11].c = 4;
    mesh.num_vertices = 8;
    mesh.vertices[0] = vertices;
    mesh.texels[0] = uv;
    mesh.vertex_colors[0] = colors;
    mesh.morph_factor = 0;
    mesh.num_vertices_pool = 1;
    mesh.num_triangles = 12;
    mesh.triangles = triangles;
    mesh.do_morph_texels = false;
    mesh.do_morph_colors = false;
    mesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
    mesh.vertex_fog = fog;
  }
}

bool csCubeMeshObject::DrawTest (iRenderView* rview, iMovable* movable)
{
  SetupObject ();
  iGraphics3D* g3d = rview->GetGraphics3D ();
  iCamera* camera = rview->GetCamera ();

  // First create the transformation from object to camera space directly:
  //   W = Mow * O - Vow;
  //   C = Mwc * (W - Vwc)
  // ->
  //   C = Mwc * (Mow * O - Vow - Vwc)
  //   C = Mwc * Mow * O - Mwc * (Vow + Vwc)
  csReversibleTransform tr_o2c = camera->GetTransform ()
    	* movable->GetFullTransform ().GetInverse ();
  float fov = camera->GetFOV ();
  float shiftx = camera->GetShiftX ();
  float shifty = camera->GetShiftY ();

  // Test visibility of entire cube by clipping bounding box against clipper.
  // There are three possibilities:
  //	1. box is not visible -> cube is not visible.
  //	2. box is entirely visible -> cube is visible and need not be clipped.
  //	3. box is partially visible -> cube is visible and needs to be clipped
  //	   if rview has do_clip_plane set to true.
  csBox2 sbox;
  csBox3 cbox;
  if (GetScreenBoundingBox (camera->GetCameraNumber (),
  	movable->GetUpdateNumber (), fov, shiftx, shifty,
  	tr_o2c, sbox, cbox) < 0)
    return false;
  int clip_portal, clip_plane, clip_z_plane;
  if (rview->ClipBBox (sbox, cbox, clip_portal, clip_plane,
  	clip_z_plane) == false)
    return false;

  iClipper2D* clipper; clipper = rview->GetClipper ();
  g3d->SetObjectToCamera (&tr_o2c);
  mesh.clip_portal = clip_portal;
  mesh.clip_plane = clip_plane;
  mesh.clip_z_plane = clip_z_plane;
  mesh.do_mirror = camera->IsMirrored ();
  return true;
}

void csCubeMeshObject::UpdateLighting (iLight** lights, int num_lights,
    iMovable* movable)
{
  SetupObject ();

  int i, l;

  // Set all colors to ambient light (@@@ NEED TO GET AMBIENT!)
  for (i = 0 ; i < 8 ; i++)
    colors[i].Set (0, 0, 0);

  // Do the lighting.
  csVector3 obj_center (0);
  csReversibleTransform trans = movable->GetFullTransform ();
  csVector3 wor_center = trans.This2Other (obj_center);
  csColor color;
  for (l = 0 ; l < num_lights ; l++)
  {
    iLight* li = lights[l];
    // Compute light position in object coordinates
    csVector3 wor_light_pos = li->GetCenter ();
    float wor_sq_dist = csSquaredDist::PointPoint (wor_light_pos, wor_center);
    if (wor_sq_dist >= li->GetSquaredRadius ()) continue;

    csVector3 obj_light_pos = trans.Other2This (wor_light_pos);
    float obj_sq_dist = csSquaredDist::PointPoint (obj_light_pos, obj_center);
    float in_obj_dist = qisqrt (obj_sq_dist);

    csVector3 obj_light_dir = (obj_light_pos - obj_center);

    csColor light_color = li->GetColor () * (256. / NORMAL_LIGHT_LEVEL)
      * li->GetBrightnessAtDistance (qsqrt (wor_sq_dist));

    for (i = 0 ; i < 8 ; i++)
    {
      //csVector3 normal = tpl->GetNormal (tf_idx, i);
      csVector3 normal = normals[i];
      float cosinus;
      if (obj_sq_dist < SMALL_EPSILON) cosinus = 1;
      else cosinus = obj_light_dir * normal;

      if (cosinus > 0)
      {
        color = light_color;
        if (obj_sq_dist >= SMALL_EPSILON) cosinus *= in_obj_dist;
        if (cosinus < 1) color *= cosinus;
	colors[i] += color;
      }
    }
  }

  // Clamp all vertex colors to 2.
  for (i = 0 ; i < 8 ; i++)
    colors[i].Clamp (2., 2., 2.);
}

bool csCubeMeshObject::Draw (iRenderView* rview, iMovable* /*movable*/,
	csZBufMode mode)
{
// @@@ TODO:
//     - Z fill vs Z use
  if (!factory->GetMaterialWrapper ())
  {
    printf ("INTERNAL ERROR: cube used without material!\n");
    return false;
  }
  iMaterialHandle* mat = factory->GetMaterialWrapper ()->GetMaterialHandle ();
  if (!mat)
  {
    printf ("INTERNAL ERROR: cube used without valid material handle!\n");
    return false;
  }

  if (vis_cb) if (!vis_cb->BeforeDrawing (this, rview)) return false;

  iGraphics3D* g3d = rview->GetGraphics3D ();

  // Prepare for rendering.
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, mode);

  factory->GetMaterialWrapper ()->Visit ();
  mesh.mat_handle = mat;
  mesh.use_vertex_color = true;
  mesh.fxmode = factory->GetMixMode () | CS_FX_GOURAUD;
  rview->CalculateFogMesh (g3d->GetObjectToCamera (), mesh);
  g3d->DrawTriangleMesh (mesh);

  return true;
}

void csCubeMeshObject::GetObjectBoundingBox (csBox3& bbox, int /*type*/)
{
  SetupObject ();
  bbox = object_bbox;
}

void csCubeMeshObject::HardTransform (const csReversibleTransform& t)
{
  shift = t.This2Other (shift);
  initialized = false;
  shapenr++;
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csCubeMeshObjectFactory)
  IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  IMPLEMENTS_EMBEDDED_INTERFACE (iCubeFactoryState)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csCubeMeshObjectFactory::CubeFactoryState)
  IMPLEMENTS_INTERFACE (iCubeFactoryState)
IMPLEMENT_EMBEDDED_IBASE_END

csCubeMeshObjectFactory::csCubeMeshObjectFactory (iBase *pParent)
{
  CONSTRUCT_IBASE (pParent);
  CONSTRUCT_EMBEDDED_IBASE (scfiCubeFactoryState);
  sizex = 1;
  sizey = 1;
  sizez = 1;
  shift.Set (0, 0, 0);
  material = NULL;
  MixMode = 0;
}

csCubeMeshObjectFactory::~csCubeMeshObjectFactory ()
{
}

iMeshObject* csCubeMeshObjectFactory::NewInstance ()
{
  csCubeMeshObject* cm = new csCubeMeshObject (this);
  iMeshObject* im = QUERY_INTERFACE (cm, iMeshObject);
  im->DecRef ();
  return im;
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csCubeMeshObjectType)
  IMPLEMENTS_INTERFACE (iMeshObjectType)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_EMBEDDED_INTERFACE (iConfig)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csCubeMeshObjectType::csCubeConfig)
  IMPLEMENTS_INTERFACE (iConfig)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_FACTORY (csCubeMeshObjectType)

EXPORT_CLASS_TABLE (cube)
  EXPORT_CLASS (csCubeMeshObjectType, "crystalspace.mesh.object.cube",
    "Crystal Space Cube Mesh Type")
EXPORT_CLASS_TABLE_END

csCubeMeshObjectType::csCubeMeshObjectType (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
  CONSTRUCT_EMBEDDED_IBASE (scfiConfig);
  default_sizex = 1;
  default_sizey = 1;
  default_sizez = 1;
  default_shift.Set (0, 0, 0);
  default_MixMode = 0;
}

csCubeMeshObjectType::~csCubeMeshObjectType ()
{
}

bool csCubeMeshObjectType::Initialize (iSystem*)
{
  return true;
}

iMeshObjectFactory* csCubeMeshObjectType::NewFactory ()
{
  csCubeMeshObjectFactory* cm = new csCubeMeshObjectFactory (this);
  iCubeFactoryState* cubeLook = QUERY_INTERFACE (cm, iCubeFactoryState);
  cubeLook->SetSize (default_sizex, default_sizey, default_sizez);
  cubeLook->SetShift (default_shift.x, default_shift.y, default_shift.z);
  cubeLook->DecRef ();
  iMeshObjectFactory* ifact = QUERY_INTERFACE (cm, iMeshObjectFactory);
  ifact->DecRef ();
  return ifact;
}

#define NUM_OPTIONS 6

static const csOptionDescription config_options [NUM_OPTIONS] =
{
  { 0, "sizex", "X Size", CSVAR_FLOAT },
  { 1, "sizey", "Y Size", CSVAR_FLOAT },
  { 2, "sizez", "Z Size", CSVAR_FLOAT },
  { 3, "shiftx", "X Shift", CSVAR_FLOAT },
  { 4, "shifty", "Y Shift", CSVAR_FLOAT },
  { 5, "shiftz", "Z Shift", CSVAR_FLOAT },
};

bool csCubeMeshObjectType::csCubeConfig::SetOption (int id, csVariant* value)
{
  if (value->type != config_options[id].type)
    return false;
  switch (id)
  {
    case 0: scfParent->default_sizex = value->v.f; break;
    case 1: scfParent->default_sizey = value->v.f; break;
    case 2: scfParent->default_sizez = value->v.f; break;
    case 3: scfParent->default_shift.x = value->v.f; break;
    case 4: scfParent->default_shift.y = value->v.f; break;
    case 5: scfParent->default_shift.z = value->v.f; break;
    default: return false;
  }
  return true;
}

bool csCubeMeshObjectType::csCubeConfig::GetOption (int id, csVariant* value)
{
  value->type = config_options[id].type;
  switch (id)
  {
    case 0: value->v.f = scfParent->default_sizex; break;
    case 1: value->v.f = scfParent->default_sizey; break;
    case 2: value->v.f = scfParent->default_sizez; break;
    case 3: value->v.f = scfParent->default_shift.x; break;
    case 4: value->v.f = scfParent->default_shift.y; break;
    case 5: value->v.f = scfParent->default_shift.z; break;
    default: return false;
  }
  return true;
}

bool csCubeMeshObjectType::csCubeConfig::GetOptionDescription
  (int idx, csOptionDescription* option)
{
  if (idx < 0 || idx >= NUM_OPTIONS)
    return false;
  *option = config_options[idx];
  return true;
}

