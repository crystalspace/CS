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
#include "csgeom/box.h"
#include "csgeom/frustum.h"
#include "cssys/csendian.h"
#include "csutil/csmd5.h"
#include "csutil/memfile.h"
#include "genmesh.h"
#include "gmtri.h"
#include "iengine/shadows.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/material.h"
#include "ivideo/vbufmgr.h"
#include "iengine/material.h"
#include "iengine/sector.h"
#include "iengine/camera.h"
#include "iengine/mesh.h"
#include "igeom/clip2d.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iutil/objreg.h"
#include "iutil/object.h"
#include "qsqrt.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csGenmeshMeshObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iShadowCaster)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iShadowReceiver)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPolygonMesh)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iGeneralMeshState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iLightingInfo)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshMeshObject::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshMeshObject::ShadowCaster)
  SCF_IMPLEMENTS_INTERFACE (iShadowCaster)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshMeshObject::ShadowReceiver)
  SCF_IMPLEMENTS_INTERFACE (iShadowReceiver)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshMeshObject::PolyMesh)
  SCF_IMPLEMENTS_INTERFACE (iPolygonMesh)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshMeshObject::GeneralMeshState)
  SCF_IMPLEMENTS_INTERFACE (iGeneralMeshState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshMeshObject::LightingInfo)
  SCF_IMPLEMENTS_INTERFACE (iLightingInfo)
SCF_IMPLEMENT_EMBEDDED_IBASE_END



csGenmeshMeshObject::csGenmeshMeshObject (csGenmeshMeshObjectFactory* factory)
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiGeneralMeshState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiShadowCaster);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiShadowReceiver);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLightingInfo);
  csGenmeshMeshObject::factory = factory;
  logparent = NULL;
  initialized = false;
  cur_cameranr = -1;
  cur_movablenr = -1;
  material = NULL;
  MixMode = 0;
  vis_cb = NULL;
  lit_mesh_colors = NULL;
  num_lit_mesh_colors = 0;
  shapenr = 0;
  do_lighting = true;
  do_manual_colors = false;
  color.red = 0;
  color.green = 0;
  color.blue = 0;
  current_lod = 1;
  current_features = 0;
  do_shadows = true;
  do_shadow_rec = false;

  dynamic_ambient.Set (0,0,0);
  ambient_version = 0;
}

csGenmeshMeshObject::~csGenmeshMeshObject ()
{
  if (vis_cb) vis_cb->DecRef ();
  delete[] lit_mesh_colors;
}

void csGenmeshMeshObject::CheckLitColors ()
{
  if (do_manual_colors) return;
  if (factory->GetVertexCount () != num_lit_mesh_colors)
  {
    num_lit_mesh_colors = factory->GetVertexCount ();
    delete[] lit_mesh_colors;
    lit_mesh_colors = new csColor [num_lit_mesh_colors];
  }
}

void csGenmeshMeshObject::InitializeDefault ()
{
  SetupObject ();

  if (!do_shadow_rec) return;
  if (do_manual_colors) return;

  // Set all colors to ambient light (@@@ NEED TO GET AMBIENT!)
  int i;
  CheckLitColors ();
  for (i = 0 ; i < num_lit_mesh_colors ; i++)
    lit_mesh_colors[i].Set (0, 0, 0);
}

char* csGenmeshMeshObject::GenerateCacheName ()
{
  const csBox3& b = factory->GetObjectBoundingBox ();

  csMemFile mf;
  mf.Write ("genmesh", 7);
  long l;
  l = convert_endian ((long)factory->GetVertexCount ());
  mf.Write ((char*)&l, 4);
  l = convert_endian ((long)factory->GetTriangleCount ());
  mf.Write ((char*)&l, 4);

  if (logparent)
  {
    csRef<iMeshWrapper> mw (SCF_QUERY_INTERFACE (logparent, iMeshWrapper));
    if (mw)
    {
      if (mw->QueryObject ()->GetName ())
        mf.Write (mw->QueryObject ()->GetName (),
		strlen (mw->QueryObject ()->GetName ()));
      iMovable* movable = mw->GetMovable ();
      iSector* sect = movable->GetSectors ()->Get (0);
      if (sect && sect->QueryObject ()->GetName ())
        mf.Write (sect->QueryObject ()->GetName (),
		strlen (sect->QueryObject ()->GetName ()));
      csVector3 pos = movable->GetFullPosition ();
      l = convert_endian ((long)QInt ((pos.x * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((long)QInt ((pos.y * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((long)QInt ((pos.z * 1000)+.5));
      mf.Write ((char*)&l, 4);
      csReversibleTransform tr = movable->GetFullTransform ();
      const csMatrix3& o2t = tr.GetO2T ();
      l = convert_endian ((long)QInt ((o2t.m11 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((long)QInt ((o2t.m12 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((long)QInt ((o2t.m13 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((long)QInt ((o2t.m21 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((long)QInt ((o2t.m22 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((long)QInt ((o2t.m23 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((long)QInt ((o2t.m31 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((long)QInt ((o2t.m32 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((long)QInt ((o2t.m33 * 1000)+.5));
      mf.Write ((char*)&l, 4);
    }
  }

  l = convert_endian ((long)QInt ((b.MinX () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((long)QInt ((b.MinY () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((long)QInt ((b.MinZ () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((long)QInt ((b.MaxX () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((long)QInt ((b.MaxY () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((long)QInt ((b.MaxZ () * 1000)+.5));
  mf.Write ((char*)&l, 4);

  csMD5::Digest digest = csMD5::Encode (mf.GetData (), mf.GetSize ());

  char* cachename = new char[33];
  sprintf (cachename,
  	"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
  	digest.data[0], digest.data[1], digest.data[2], digest.data[3],
  	digest.data[4], digest.data[5], digest.data[6], digest.data[7],
  	digest.data[8], digest.data[9], digest.data[10], digest.data[11],
  	digest.data[12], digest.data[13], digest.data[14], digest.data[15]);
  return cachename;
}

bool csGenmeshMeshObject::ReadFromCache (iCacheManager* cache_mgr)
{
  return true;
}

bool csGenmeshMeshObject::WriteToCache (iCacheManager* cache_mgr)
{
  return true;
}

void csGenmeshMeshObject::PrepareLighting ()
{
}

void csGenmeshMeshObject::DynamicLightChanged (iDynLight* dynlight)
{
  (void)dynlight;
}

void csGenmeshMeshObject::DynamicLightDisconnect (iDynLight* dynlight)
{
  (void)dynlight;
}

void csGenmeshMeshObject::StaticLightChanged (iStatLight* statlight)
{
  (void)statlight;
}

void csGenmeshMeshObject::AppendShadows (iMovable* movable,
	iShadowBlockList* shadows, const csVector3& origin)
{
  if (!do_shadows) return;
  int tri_num = factory->GetTriangleCount ();
  csVector3* vt = factory->GetVertices ();
  int vt_num = factory->GetVertexCount ();
  csVector3* vt_world, * vt_array_to_delete;
  int i;
  if (movable->IsFullTransformIdentity ())
  {
    vt_array_to_delete = NULL;
    vt_world = vt;
  }
  else
  {
    vt_array_to_delete = new csVector3 [vt_num];
    vt_world = vt_array_to_delete;
    csReversibleTransform movtrans = movable->GetFullTransform ();
    for (i = 0 ; i < vt_num ; i++)
      vt_world[i] = movtrans.This2Other (vt[i]);
  }

  iShadowBlock *list = shadows->NewShadowBlock (tri_num);
  csFrustum *frust;
  bool cw = true;                   //@@@ Use mirroring parameter here!
  csTriangle* tri = factory->GetTriangles ();
  for (i = 0 ; i < tri_num ; i++, tri++)
  {
    csPlane3 pl (vt_world[tri->c], vt_world[tri->b], vt_world[tri->a]);
    //if (pl.VisibleFromPoint (origin) != cw) continue;
    float clas = pl.Classify (origin);
    if (ABS (clas) < EPSILON) continue;
    if ((clas <= 0) != cw) continue;

    pl.DD += origin * pl.norm;
    pl.Invert ();
    frust = list->AddShadow (origin, NULL, 3, pl);
    frust->GetVertex (0).Set (vt_world[tri->a] - origin);
    frust->GetVertex (1).Set (vt_world[tri->b] - origin);
    frust->GetVertex (2).Set (vt_world[tri->c] - origin);
  }

  delete[] vt_array_to_delete;
}

void csGenmeshMeshObject::GetTransformedBoundingBox (long cameranr,
	long movablenr, const csReversibleTransform& trans, csBox3& cbox)
{
  if (cur_cameranr == cameranr && cur_movablenr == movablenr)
  {
    cbox = camera_bbox;
    return;
  }
  cur_cameranr = cameranr;
  cur_movablenr = movablenr;
  const csBox3& b = factory->GetObjectBoundingBox ();

  camera_bbox.StartBoundingBox (
    trans * b.GetCorner (CS_BOX_CORNER_xyz));
  camera_bbox.AddBoundingVertexSmart (
    trans * b.GetCorner (CS_BOX_CORNER_Xyz));
  camera_bbox.AddBoundingVertexSmart (
    trans * b.GetCorner (CS_BOX_CORNER_xYz));
  camera_bbox.AddBoundingVertexSmart (
    trans * b.GetCorner (CS_BOX_CORNER_XYz));
  camera_bbox.AddBoundingVertexSmart (
    trans * b.GetCorner (CS_BOX_CORNER_xyZ));
  camera_bbox.AddBoundingVertexSmart (
    trans * b.GetCorner (CS_BOX_CORNER_XyZ));
  camera_bbox.AddBoundingVertexSmart (
    trans * b.GetCorner (CS_BOX_CORNER_xYZ));
  camera_bbox.AddBoundingVertexSmart (
    trans * b.GetCorner (CS_BOX_CORNER_XYZ));

  cbox = camera_bbox;
}

static void Perspective (const csVector3& v, csVector2& p, float fov,
    	float sx, float sy)
{
  float iz = fov / v.z;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}

float csGenmeshMeshObject::GetScreenBoundingBox (long cameranr,
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

void csGenmeshMeshObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;
    delete[] lit_mesh_colors;
    lit_mesh_colors = NULL;
    if (!do_manual_colors)
    {
      num_lit_mesh_colors = factory->GetVertexCount ();
      lit_mesh_colors = new csColor [num_lit_mesh_colors];
    }
  }
}

void csGenmeshMeshObject::FireListeners ()
{
  int i;
  for (i = 0 ; i < listeners.Length () ; i++)
    listeners[i]->ObjectModelChanged (&scfiObjectModel);
}

void csGenmeshMeshObject::AddListener (iObjectModelListener *listener)
{
  RemoveListener (listener);
  listeners.Push (listener);
}

void csGenmeshMeshObject::RemoveListener (iObjectModelListener *listener)
{
  int idx = listeners.Find (listener);
  if (idx == -1) return ;
  listeners.Delete (idx);
}

bool csGenmeshMeshObject::DrawTest (iRenderView* rview, iMovable* movable)
{
  SetupObject ();
  CheckLitColors ();

#ifndef CS_USE_NEW_RENDERER
  iGraphics3D* g3d = rview->GetGraphics3D ();
#endif
  iCamera* camera = rview->GetCamera ();

  // First create the transformation from object to camera space directly:
  //   W = Mow * O - Vow;
  //   C = Mwc * (W - Vwc)
  // ->
  //   C = Mwc * (Mow * O - Vow - Vwc)
  //   C = Mwc * Mow * O - Mwc * (Vow + Vwc)
  tr_o2c = camera->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movable->GetFullTransform ();

  csVector3 radius;
  csSphere sphere;
  GetRadius (radius, sphere.GetCenter ());
  float max_radius = radius.x;
  if (max_radius < radius.y) max_radius = radius.y;
  if (max_radius < radius.z) max_radius = radius.z;
  sphere.SetRadius (max_radius);
  int clip_portal, clip_plane, clip_z_plane;
  if (rview->ClipBSphere (tr_o2c, sphere, clip_portal, clip_plane,
  	clip_z_plane) == false)
    return false;

#ifndef CS_USE_NEW_RENDERER
  g3d->SetObjectToCamera (&tr_o2c);
  G3DTriangleMesh& m = factory->GetMesh ();
  m.clip_portal = clip_portal;
  m.clip_plane = clip_plane;
  m.clip_z_plane = clip_z_plane;
  m.do_mirror = camera->IsMirrored ();
#else
  mesh.clip_portal = clip_portal;
  mesh.clip_plane = clip_plane;
  mesh.clip_z_plane = clip_z_plane;
  mesh.do_mirror = camera->IsMirrored ();
#endif
  return true;
}

void csGenmeshMeshObject::CastShadows (iMovable* movable, iFrustumView* fview)
{
  SetupObject ();

  if (do_manual_colors) return;
  if (!do_shadow_rec) return;
  if (!do_lighting) return;

  csFrustumContext* ctxt = fview->GetFrustumContext ();
  iBase* b = (iBase *)fview->GetUserdata ();
  csRef<iLightingProcessInfo> lpi = SCF_QUERY_INTERFACE (b,
  	iLightingProcessInfo);
  CS_ASSERT (lpi != NULL);

  iLight* li = lpi->GetLight ();

  csFrustum* light_frust = ctxt->GetLightFrustum ();
  const csVector3& light_pos = light_frust->GetOrigin ();

  int i;
  CheckLitColors ();
  csColor* colors = lit_mesh_colors;
  csVector3* normals = factory->GetNormals ();

  // Do the lighting.
  csReversibleTransform trans = movable->GetFullTransform ();
  // the object center in world coordinates. "0" because the object
  // center in object space is obviously at (0,0,0).

  // Compute light position in object coordinates
  csVector3 wor_light_pos = light_pos;
  csVector3 obj_light_pos = trans.Other2This (wor_light_pos);
  float obj_sq_dist = csSquaredDist::PointPoint (obj_light_pos, 0);
  if (obj_sq_dist >= li->GetSquaredRadius ()) return;
  float in_obj_dist = (obj_sq_dist >= SMALL_EPSILON)
  	? qisqrt (obj_sq_dist) : 1.0f;

  csColor light_color = lpi->GetColor () * (256. / CS_NORMAL_LIGHT_LEVEL)
      * li->GetBrightnessAtDistance (qsqrt (obj_sq_dist));

  for (i = 0 ; i < factory->GetVertexCount () ; i++)
  {
    csVector3 normal = normals[i];
    float cosinus;
    if (obj_sq_dist < SMALL_EPSILON) cosinus = 1;
    else cosinus = obj_light_pos * normal;
    // because the vector from the object center to the light center
    // in object space is equal to the position of the light

    if (cosinus > 0)
    {
      color = light_color;
      if (obj_sq_dist >= SMALL_EPSILON) cosinus *= in_obj_dist;
      if (cosinus < 1) color *= cosinus;
      colors[i] += color;
      colors[i].Clamp (2., 2., 2.);
    }
  }
}

void csGenmeshMeshObject::UpdateLighting (iLight** lights, int num_lights,
    iMovable* movable)
{
  SetupObject ();
  CheckLitColors ();

  if (do_manual_colors) return;
  if (do_shadow_rec) return;

  int i, l;
  csColor* colors = lit_mesh_colors;
  csVector3* normals = factory->GetNormals ();

  // Set all colors to ambient light.
  csColor col;
  if (factory->engine)
  {
    factory->engine->GetAmbientLight (col);
    col += color;
    iSector* sect = movable->GetSectors ()->Get (0);
    if (sect)
      col += sect->GetDynamicAmbientLight ();
  }
  else
  {
    col = color;
  }
  for (i = 0 ; i < factory->GetVertexCount () ; i++)
    colors[i] = col;

  if (!do_lighting) return;
    // @@@ it is not efficient to do this all the time.

  // Do the lighting.
  csReversibleTransform trans = movable->GetFullTransform ();
  // the object center in world coordinates. "0" because the object
  // center in object space is obviously at (0,0,0).
  for (l = 0 ; l < num_lights ; l++)
  {
    iLight* li = lights[l];
    // Compute light position in object coordinates
    csVector3 wor_light_pos = li->GetCenter ();
    csVector3 obj_light_pos = trans.Other2This (wor_light_pos);
    float obj_sq_dist = csSquaredDist::PointPoint (obj_light_pos, 0);
    if (obj_sq_dist >= li->GetSquaredRadius ()) continue;

    float in_obj_dist = (obj_sq_dist >= SMALL_EPSILON)?qisqrt (obj_sq_dist):1.0f;

    csColor light_color = li->GetColor () * (256. / CS_NORMAL_LIGHT_LEVEL)
      * li->GetBrightnessAtDistance (qsqrt (obj_sq_dist));

    for (i = 0 ; i < factory->GetVertexCount () ; i++)
    {
      csVector3 normal = normals[i];
      float cosinus;
      if (obj_sq_dist < SMALL_EPSILON) cosinus = 1;
      else cosinus = obj_light_pos * normal;
      // because the vector from the object center to the light center
      // in object space is equal to the position of the light

      if (cosinus > 0)
      {
        col = light_color;
        if (obj_sq_dist >= SMALL_EPSILON) cosinus *= in_obj_dist;
        if (cosinus < 1) col *= cosinus;
	colors[i] += col;
      }
    }
  }

  // Clamp all vertex colors to 2.
  for (i = 0 ; i < factory->GetVertexCount () ; i++)
    colors[i].Clamp (2., 2., 2.);
}

bool csGenmeshMeshObject::Draw (iRenderView* rview, iMovable* /*movable*/,
	csZBufMode mode)
{
  iMaterialWrapper* mater = material;
  if (!mater) mater = factory->GetMaterialWrapper ();
  if (!mater)
  {
    printf ("INTERNAL ERROR: mesh used without material!\n");
    return false;
  }
  iMaterialHandle* mat = mater->GetMaterialHandle ();
  if (!mat)
  {
    printf ("INTERNAL ERROR: mesh used without valid material handle!\n");
    return false;
  }

  if (vis_cb) if (!vis_cb->BeforeDrawing (this, rview)) return false;

#ifndef CS_USE_NEW_RENDERER
  iGraphics3D* g3d = rview->GetGraphics3D ();
#else
  iRender3D* r3d = rview->GetGraphics3D ();
#endif

  // Prepare for rendering.
#ifndef CS_USE_NEW_RENDERER
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, mode);
#else
  mesh.z_buf_mode = mode;
#endif

  mater->Visit ();
#ifndef CS_USE_NEW_RENDERER
  G3DTriangleMesh& m = factory->GetMesh ();
  iVertexBuffer* vbuf = factory->GetVertexBuffer ();
  iVertexBufferManager* vbufmgr = factory->GetVertexBufferManager ();
  m.mat_handle = mat;
  m.use_vertex_color = true;
  m.mixmode = MixMode | CS_FX_GOURAUD;
  CS_ASSERT (!vbuf->IsLocked ());
  vbufmgr->LockBuffer (vbuf,
  	factory->GetVertices (),
	factory->GetTexels (),
	do_manual_colors ? factory->GetColors () : lit_mesh_colors,
	factory->GetVertexCount (), 0);
  rview->CalculateFogMesh (g3d->GetObjectToCamera (), m);
  g3d->DrawTriangleMesh (m);
  vbufmgr->UnlockBuffer (vbuf);
#else
  mesh.SetIndexRange (0, factory->GetTriangleCount () * 3);
  mesh.SetMaterialHandle (mater->GetMaterialHandle ());
  csRef<iStreamSource> stream = SCF_QUERY_INTERFACE (factory, iStreamSource);
  mesh.SetStreamSource (stream);
  mesh.SetType (csRenderMesh::MESHTYPE_TRIANGLES);
  r3d->DrawMesh (&mesh);
#endif

  return true;
}

#ifdef CS_USE_NEW_RENDERER
bool csGenmeshMeshObject::DrawZ (iRenderView* rview, iMovable* /*movable*/,
	csZBufMode mode)
{
  if (vis_cb) if (!vis_cb->BeforeDrawing (this, rview)) return false;

  iRender3D* r3d = rview->GetGraphics3D ();

  // Prepare for rendering.
  mesh.z_buf_mode = mode;

  r3d->SetObjectToCamera (&tr_o2c);
  mesh.SetIndexRange (0, factory->GetTriangleCount () * 3);
  mesh.SetMaterialHandle (NULL);
  csRef<iStreamSource> stream = SCF_QUERY_INTERFACE (factory, iStreamSource);
  mesh.SetStreamSource (stream);
  mesh.SetType (csRenderMesh::MESHTYPE_TRIANGLES);
  r3d->DrawMesh (&mesh);

  return true;
}

bool csGenmeshMeshObject::DrawShadow (iRenderView* rview, iMovable* /*movable*/,
	csZBufMode mode)
{
  csRef<iMaterialWrapper> mater = factory->shadowmat;

  iRender3D* r3d = rview->GetGraphics3D ();

  mater->Visit ();

  // Prepare for rendering.
  mesh.z_buf_mode = CS_ZBUF_TEST;
  mesh.mixmode = CS_FX_COPY;

  mesh.SetIndexRange (0, factory->GetTriangleCount () * 12);
  mesh.SetMaterialHandle (mater->GetMaterialHandle ());
  csRef<iStreamSource> stream = SCF_QUERY_INTERFACE (factory, iStreamSource);
  mesh.SetStreamSource (stream);
  mesh.SetType (csRenderMesh::MESHTYPE_TRIANGLES);

  r3d->SetObjectToCamera (&tr_o2c);
  r3d->SetShadowState (CS_SHADOW_VOLUME_PASS1);
  r3d->DrawMesh (&mesh);
  r3d->SetShadowState (CS_SHADOW_VOLUME_PASS2);
  r3d->DrawMesh (&mesh);

  return true;
}

bool csGenmeshMeshObject::DrawLight (iRenderView* rview, iMovable* /*movable*/,
	csZBufMode mode)
{
  iMaterialWrapper* mater = material;
  if (!mater) mater = factory->GetMaterialWrapper ();
  if (!mater)
  {
    printf ("INTERNAL ERROR: mesh used without material!\n");
    return false;
  }
  iMaterialHandle* mat = mater->GetMaterialHandle ();
  if (!mat)
  {
    printf ("INTERNAL ERROR: mesh used without valid material handle!\n");
    return false;
  }

  iRender3D* r3d = rview->GetGraphics3D ();

  mater->Visit ();

  // Prepare for rendering.
  mesh.z_buf_mode = CS_ZBUF_TEST;
  mesh.mixmode = CS_FX_ADD;

  r3d->SetObjectToCamera (&tr_o2c);
  mesh.SetIndexRange (0, factory->GetTriangleCount () * 3);
  mesh.SetMaterialHandle (mater->GetMaterialHandle ());
  csRef<iStreamSource> stream = SCF_QUERY_INTERFACE (factory, iStreamSource);
  mesh.SetStreamSource (stream);
  mesh.SetType (csRenderMesh::MESHTYPE_TRIANGLES);
  r3d->DrawMesh (&mesh);

  return true;
}
#endif // CS_USE_NEW_RENDERER

void csGenmeshMeshObject::GetObjectBoundingBox (csBox3& bbox, int /*type*/)
{
  bbox = factory->GetObjectBoundingBox ();
}

bool csGenmeshMeshObject::HitBeamOutline (const csVector3& start,
  const csVector3& end, csVector3& isect, float* pr)
{
  // This is now closer to an outline hitting method. It will
  // return as soon as it touches any triangle in the mesh, and
  // will be a bit faster than its more accurate cousin (below).

  csSegment3 seg (start, end);
  int i, max = factory->GetTriangleCount();
  csTriangle *tr = factory->GetTriangles();
  csVector3 *vrt = factory->GetVertices ();
  for (i = 0 ; i < max ; i++)
  {
    if (csIntersect3::IntersectTriangle (vrt[tr[i].a], vrt[tr[i].b],
    	vrt[tr[i].c], seg, isect))
    {
      if (pr) *pr = qsqrt (csSquaredDist::PointPoint (start, isect) /
		csSquaredDist::PointPoint (start, end));

      return true;
    }
  }
  return false;
}

bool csGenmeshMeshObject::HitBeamObject (const csVector3& start,
  const csVector3& end, csVector3& isect, float *pr)
{
  // This is the slow version. Use for an accurate hit on the object.
  // It will cycle through every triangle in the mesh serching for the
  // closest intersection. Slower, but returns the closest hit.
  // Usage is optional.

  csSegment3 seg (start, end);
  int i, max = factory->GetTriangleCount();
  float tot_dist = csSquaredDist::PointPoint (start, end);
  float dist, temp;
  float itot_dist = 1 / tot_dist;
  dist = temp = tot_dist;
  csVector3 *vrt = factory->GetVertices (), tmp;
  csTriangle *tr = factory->GetTriangles();
  for (i = 0 ; i < max ; i++)
  {
    if (csIntersect3::IntersectTriangle (vrt[tr[i].a], vrt[tr[i].b],
    	vrt[tr[i].c], seg, tmp))
    {
      if ( dist > (temp = csSquaredDist::PointPoint (start, tmp)))
      {
        isect = tmp;
	dist = temp;
        if (pr) *pr = qsqrt( dist * itot_dist);
      }
    }
  }
  if (dist == tot_dist)
    return false;
  return true;
}

void csGenmeshMeshObject::GetRadius (csVector3& rad, csVector3& cent)
{
  rad = factory->GetRadius ();
  cent.Set (0);
}

int csGenmeshMeshObject::PolyMesh::GetVertexCount ()
{
  return scfParent->factory->GetVertexCount ();
}

csVector3* csGenmeshMeshObject::PolyMesh::GetVertices ()
{
  return scfParent->factory->GetVertices ();
}

int csGenmeshMeshObject::PolyMesh::GetPolygonCount ()
{
  return scfParent->factory->GetTriangleCount ();
}

csMeshedPolygon* csGenmeshMeshObject::PolyMesh::GetPolygons ()
{
  return scfParent->factory->GetPolygons ();
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csGenmeshMeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
#ifdef CS_USE_NEW_RENDERER
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iStreamSource)
#endif
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iGeneralFactoryState)
#ifndef CS_USE_NEW_RENDERER
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iVertexBufferManagerClient)
#endif
SCF_IMPLEMENT_IBASE_END

#ifdef CS_USE_NEW_RENDERER
SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshMeshObjectFactory::StreamSource)
  SCF_IMPLEMENTS_INTERFACE (iStreamSource) 
SCF_IMPLEMENT_EMBEDDED_IBASE_END
#endif

SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshMeshObjectFactory::GeneralFactoryState)
  SCF_IMPLEMENTS_INTERFACE (iGeneralFactoryState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

#ifndef CS_USE_NEW_RENDERER
SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshMeshObjectFactory::
	eiVertexBufferManagerClient)
  SCF_IMPLEMENTS_INTERFACE (iVertexBufferManagerClient)
SCF_IMPLEMENT_EMBEDDED_IBASE_END
#endif

csGenmeshMeshObjectFactory::csGenmeshMeshObjectFactory (iBase *pParent,
	iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE (pParent);
#ifdef CS_USE_NEW_RENDERER
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiStreamSource);
#endif
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiGeneralFactoryState);
#ifndef CS_USE_NEW_RENDERER 
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiVertexBufferManagerClient);
#endif
  csGenmeshMeshObjectFactory::object_reg = object_reg;
  logparent = NULL;
  initialized = false;
  object_bbox_valid = false;
  mesh_tri_normals = NULL;
#ifndef CS_USE_NEW_RENDERER
  top_mesh.num_triangles = 0;
  top_mesh.triangles = NULL;
  top_mesh.vertex_fog = NULL;
#else
  num_mesh_triangles = 0;
  mesh_triangles = NULL;
#endif
  num_mesh_vertices = 0;
  mesh_vertices = NULL;
  mesh_texels = NULL;
  mesh_colors = NULL;
  mesh_normals = NULL;
#ifndef CS_USE_NEW_RENDERER
  vbufmgr = NULL;
#endif
  material = NULL;
  polygons = NULL;
#ifdef CS_USE_NEW_RENDERER
  vertex_buffer = NULL;
  normal_buffer = NULL;
  trinormal_buffer = NULL;
  texel_buffer = NULL;
  color_buffer = NULL;
  index_buffer = NULL;

  r3d = CS_QUERY_REGISTRY (object_reg, iRender3D);

  vertex_name = r3d->GetStringContainer ()->Request ("vertices");
  texel_name = r3d->GetStringContainer ()->Request ("texture coordinates");
  normal_name = r3d->GetStringContainer ()->Request ("normals");
  trinormal_name = r3d->GetStringContainer ()->Request ("shadow normals");
  color_name = r3d->GetStringContainer ()->Request ("colors");
  index_name = r3d->GetStringContainer ()->Request ("indices");

  autonormals = false;
  mesh_vertices_dirty_flag = false;
  mesh_texels_dirty_flag = false;
  mesh_normals_dirty_flag = false;
  mesh_tri_normals_dirty_flag = false;
  mesh_colors_dirty_flag = false;
  mesh_triangle_dirty_flag = false;
#endif

  csRef<iEngine> eng = CS_QUERY_REGISTRY (object_reg, iEngine);
  engine = eng;	// We don't want a circular reference!

  //@@@ Crashes istest and isomap - NEW RENDERER ???
#ifdef CS_USE_NEW_RENDERER
  shadowmat = engine->FindMaterial ("shadow extruder");
#endif
}

csGenmeshMeshObjectFactory::~csGenmeshMeshObjectFactory ()
{
#ifndef CS_USE_NEW_RENDERER
  if (vbufmgr) vbufmgr->RemoveClient (&scfiVertexBufferManagerClient);
#endif
  delete[] mesh_normals;
  delete[] mesh_vertices;
  delete[] mesh_colors;
  delete[] mesh_texels;
#ifndef CS_USE_NEW_RENDERER
  delete[] top_mesh.triangles;
  delete[] top_mesh.vertex_fog;
#endif
  delete[] polygons;
  if (mesh_tri_normals)
    delete [] mesh_tri_normals;
}

void csGenmeshMeshObjectFactory::CalculateBBoxRadius ()
{
  object_bbox_valid = true;
  if (num_mesh_vertices == 0)
  {
    object_bbox.StartBoundingBox ();
    radius.Set (0, 0, 0);
    return;
  }
  object_bbox.StartBoundingBox (mesh_vertices[0]);
  csVector3 max_sq_radius (0);
  int i;
  for (i = 1 ; i < num_mesh_vertices ; i++)
  {
    csVector3& v = mesh_vertices[i];
    object_bbox.AddBoundingVertexSmart (v);
    csVector3 sq_radius (v.x*v.x, v.y*v.y, v.z*v.z);
    if (sq_radius.x > max_sq_radius.x) max_sq_radius.x = sq_radius.x;
    if (sq_radius.y > max_sq_radius.y) max_sq_radius.y = sq_radius.y;
    if (sq_radius.z > max_sq_radius.z) max_sq_radius.z = sq_radius.z;
  }
  radius.Set (qsqrt (max_sq_radius.x),
  	qsqrt (max_sq_radius.y), qsqrt (max_sq_radius.z));
}

const csVector3& csGenmeshMeshObjectFactory::GetRadius ()
{
  SetupFactory ();
  if (!object_bbox_valid) CalculateBBoxRadius ();
  return radius;
}

const csBox3& csGenmeshMeshObjectFactory::GetObjectBoundingBox ()
{
  SetupFactory ();
  if (!object_bbox_valid) CalculateBBoxRadius ();
  return object_bbox;
}

#ifndef CS_USE_NEW_RENDERER
void csGenmeshMeshObjectFactory::SetupVertexBuffer ()
{
  if (!vbuf)
  {
    if (!vbufmgr)
    {
      csRef<iGraphics3D> g3d (CS_QUERY_REGISTRY (object_reg, iGraphics3D));
      // @@@ priority should be a parameter.
      vbufmgr = g3d->GetVertexBufferManager ();
      vbufmgr->AddClient (&scfiVertexBufferManagerClient);
    }
    vbuf = vbufmgr->CreateBuffer (0);
    top_mesh.buffers[0] = vbuf;
  }
}
#endif

void csGenmeshMeshObjectFactory::SetupFactory ()
{
  if (!initialized)
  {
    initialized = true;
    object_bbox_valid = false;
#ifndef CS_USE_NEW_RENDERER
    SetupVertexBuffer ();

    top_mesh.morph_factor = 0;
    top_mesh.num_vertices_pool = 1;
    top_mesh.do_morph_texels = false;
    top_mesh.do_morph_colors = false;
    top_mesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
#endif
  }
}

#ifdef CS_USE_NEW_RENDERER

iRenderBuffer *csGenmeshMeshObjectFactory::GetBuffer (csStringID name)
{
  csRef<iRender3D> r3d (CS_QUERY_REGISTRY (object_reg, iRender3D));
  if (name == vertex_name)
  {
    if (mesh_vertices_dirty_flag)
    {
#ifndef CS_USE_SHADOW_VOLUMES
      vertex_buffer = r3d->GetBufferManager ()->CreateBuffer (
        sizeof (csVector3)*num_mesh_vertices, CS_BUF_STATIC);
      csVector3* vbuf = (csVector3*)vertex_buffer->Lock (
      	iRenderBuffer::CS_BUF_LOCK_NORMAL);
      memcpy (vbuf, mesh_vertices, sizeof (csVector3)*num_mesh_vertices);
#else
      vertex_buffer = r3d->GetBufferManager ()->CreateBuffer (
        sizeof (csVector3)*(num_mesh_triangles*3+1), CS_BUF_STATIC);
      csVector3* vbuf = (csVector3*)vertex_buffer->Lock(iRenderBuffer::CS_BUF_LOCK_NORMAL);
      int vbuf_index = 0;
      for (int i = 0; i < num_mesh_triangles; i ++) {
        vbuf[vbuf_index++] = mesh_vertices[mesh_triangles[i].a];
        vbuf[vbuf_index++] = mesh_vertices[mesh_triangles[i].b];
        vbuf[vbuf_index++] = mesh_vertices[mesh_triangles[i].c];
      }
	  vbuf[vbuf_index] = csVector3 (0,0,0);
#endif
      vertex_buffer->Release ();
      mesh_vertices_dirty_flag = false;
    }
    return vertex_buffer;
  }
  if (name == texel_name)
  {
    if (mesh_texels_dirty_flag)
    {
#ifndef CS_USE_SHADOW_VOLUMES
      texel_buffer = r3d->GetBufferManager ()->CreateBuffer (
        sizeof (csVector2)*num_mesh_vertices, CS_BUF_STATIC);
      csVector2* tbuf = (csVector2*)texel_buffer->Lock (
      	iRenderBuffer::CS_BUF_LOCK_NORMAL);
      memcpy (tbuf, mesh_texels, sizeof (csVector2)*num_mesh_vertices);
#else
      texel_buffer = r3d->GetBufferManager ()->CreateBuffer (
        sizeof (csVector2)*(num_mesh_triangles*3+1), CS_BUF_STATIC);
      csVector2* tbuf = (csVector2*)texel_buffer->Lock (iRenderBuffer::CS_BUF_LOCK_NORMAL);
      int tbuf_index = 0;
      for (int i = 0; i < num_mesh_triangles; i ++) {
        tbuf[tbuf_index++] = mesh_texels[mesh_triangles[i].a];
        tbuf[tbuf_index++] = mesh_texels[mesh_triangles[i].b];
        tbuf[tbuf_index++] = mesh_texels[mesh_triangles[i].c];
      }
	  tbuf[tbuf_index] = csVector2 (0, 0);
#endif
      texel_buffer->Release ();
      mesh_texels_dirty_flag = false;
    }
    return texel_buffer;
  }
  if (name == normal_name)
  {
    if (mesh_normals_dirty_flag)
    {
#ifndef CS_USE_SHADOW_VOLUMES
      normal_buffer = r3d->GetBufferManager ()->CreateBuffer (
        sizeof (csVector3)*num_mesh_vertices, CS_BUF_STATIC);
      csVector3 *nbuf = (csVector3*)normal_buffer->Lock (
      	iRenderBuffer::CS_BUF_LOCK_NORMAL);
      memcpy (nbuf, mesh_normals, sizeof (csVector3)*num_mesh_vertices);
#else
      normal_buffer = r3d->GetBufferManager ()->CreateBuffer (
        sizeof (csVector3)*num_mesh_triangles*3, CS_BUF_STATIC);
      csVector3 *nbuf = (csVector3*)normal_buffer->Lock(iRenderBuffer::CS_BUF_LOCK_NORMAL);
      int nbuf_index = 0;
      for (int i = 0; i < num_mesh_triangles; i ++) {
        nbuf[nbuf_index++] = mesh_normals[mesh_triangles[i].a];
        nbuf[nbuf_index++] = mesh_normals[mesh_triangles[i].b];
        nbuf[nbuf_index++] = mesh_normals[mesh_triangles[i].c];
      }
#endif
      normal_buffer->Release ();
      mesh_normals_dirty_flag = false;
    }
    return normal_buffer;
  }
  if (name == trinormal_name) 
  {
    if (mesh_tri_normals_dirty_flag)
    {
       if (autonormals) /* autonormalized */
      {
        trinormal_buffer = r3d->GetBufferManager ()->CreateBuffer (
          sizeof (csVector3)*(num_mesh_triangles*3+1), CS_BUF_STATIC);
        csVector3 *tbuf = (csVector3*)trinormal_buffer->Lock(iRenderBuffer::CS_BUF_LOCK_NORMAL);
        int tbuf_index = 0;
        for (int i = 0; i < num_mesh_triangles; i ++) {
          tbuf[tbuf_index++] = mesh_tri_normals[i];
          tbuf[tbuf_index++] = mesh_tri_normals[i];
          tbuf[tbuf_index++] = mesh_tri_normals[i];
        }
        tbuf[tbuf_index] = csVector3 (0,0,0);
        trinormal_buffer->Release ();
        mesh_tri_normals_dirty_flag = false;
      } else {/* computed without autonormal */
        trinormal_buffer = r3d->GetBufferManager ()->CreateBuffer (
          sizeof (csVector3)*(num_mesh_triangles*3+1), CS_BUF_STATIC);
        csVector3 *tbuf = (csVector3*)trinormal_buffer->Lock(iRenderBuffer::CS_BUF_LOCK_NORMAL);
        int tbuf_index = 0;
        for (int i = 0; i < num_mesh_triangles; i ++) {
          csVector3 ab = mesh_vertices [mesh_triangles[i].b] - mesh_vertices [mesh_triangles[i].a];
          csVector3 bc = mesh_vertices [mesh_triangles[i].c] - mesh_vertices [mesh_triangles[i].b];
          csVector3 face_normal = ab % bc;
          tbuf[tbuf_index++] = face_normal;
          tbuf[tbuf_index++] = face_normal;
          tbuf[tbuf_index++] = face_normal;
        }
        tbuf[tbuf_index] = csVector3 (0,0,0);
        trinormal_buffer->Release ();
        mesh_tri_normals_dirty_flag = false;
      }
    }
    return trinormal_buffer;
  }
  if (name == color_name)
  {
    if (mesh_colors_dirty_flag)
    {
#ifndef CS_USE_SHADOW_VOLUMES
      color_buffer = r3d->GetBufferManager ()->CreateBuffer (
        sizeof (csColor)*num_mesh_vertices, CS_BUF_STATIC);
      csColor *cbuf = (csColor*)color_buffer->Lock (
      	iRenderBuffer::CS_BUF_LOCK_NORMAL);
      memcpy (cbuf, mesh_colors, sizeof (csColor)*num_mesh_vertices);
#else
      color_buffer = r3d->GetBufferManager ()->CreateBuffer (
        sizeof (csColor)*num_mesh_triangles*3, CS_BUF_STATIC);
      csColor *cbuf = (csColor*)color_buffer->Lock(iRenderBuffer::CS_BUF_LOCK_NORMAL);
      int cbuf_index = 0;
      for (int i = 0; i < num_mesh_triangles; i ++) {
        cbuf[cbuf_index ++] = mesh_colors[mesh_triangles[i].a];
        cbuf[cbuf_index ++] = mesh_colors[mesh_triangles[i].b];
        cbuf[cbuf_index ++] = mesh_colors[mesh_triangles[i].c];
      }
#endif
      color_buffer->Release();
      mesh_colors_dirty_flag = false;
    }
    return color_buffer;
  }
  if (name == index_name)
  {
    if (mesh_triangle_dirty_flag)
    {
#ifndef CS_USE_SHADOW_VOLUMES
      index_buffer = r3d->GetBufferManager ()->CreateBuffer (
        sizeof (unsigned int)*num_mesh_triangles*3, CS_BUF_INDEX);
#else
      index_buffer = r3d->GetBufferManager ()->CreateBuffer (
        sizeof (unsigned int)*num_mesh_triangles*12, CS_BUF_INDEX);
#endif
      unsigned int *ibuf = (unsigned int *)index_buffer->Lock(iRenderBuffer::CS_BUF_LOCK_NORMAL);

#ifdef CS_USE_SHADOW_VOLUMES
      struct Edge {
		csVector3 a, b;
        int ind_a, ind_b;
      };
      csBasicVector EdgeStack;
      int QuadsIndex = num_mesh_triangles * 3;
	  int TriIndex = 0;
#endif
	  int i;
      for (i = 0; i < num_mesh_triangles; i ++) 
      {
#ifndef CS_USE_SHADOW_VOLUMES
        ibuf[i * 3 + 0] = mesh_triangles[i].a;
        ibuf[i * 3 + 1] = mesh_triangles[i].b;
        ibuf[i * 3 + 2] = mesh_triangles[i].c;
#else 
        ibuf[i * 3 + 0] = TriIndex ++;
        ibuf[i * 3 + 1] = TriIndex ++;
        ibuf[i * 3 + 2] = TriIndex ++;

        bool found_a = false, found_b = false, found_c = false;
        for (int j = EdgeStack.Length()-1; j >= 0; j --) {
          Edge *e = (Edge *)EdgeStack[j];
          if (!found_a) {
            if (e->a == mesh_vertices[mesh_triangles[i].a] && 
                e->b == mesh_vertices[mesh_triangles[i].b]) {
              ibuf[QuadsIndex ++] = e->ind_a;
              ibuf[QuadsIndex ++] = TriIndex - 3;
              ibuf[QuadsIndex ++] = TriIndex - 2;
              ibuf[QuadsIndex ++] = TriIndex - 2;
              ibuf[QuadsIndex ++] = e->ind_b;
              ibuf[QuadsIndex ++] = e->ind_a;
              EdgeStack.Delete (j);
			  delete e;
              found_a = true;
              continue;
            }
            if (e->a == mesh_vertices[mesh_triangles[i].b] &&
                e->b == mesh_vertices[mesh_triangles[i].a]) {
              ibuf[QuadsIndex ++] = e->ind_a, 
              ibuf[QuadsIndex ++] = TriIndex - 2;
              ibuf[QuadsIndex ++] = TriIndex - 3;
              ibuf[QuadsIndex ++] = TriIndex - 3;
              ibuf[QuadsIndex ++] = e->ind_b;
              ibuf[QuadsIndex ++] = e->ind_a;
              EdgeStack.Delete (j);
			  delete e;
              found_a = true;
              continue;
            }
          }
          if (!found_b) {
            if (e->a == mesh_vertices[mesh_triangles[i].b] && 
                e->b == mesh_vertices[mesh_triangles[i].c]) {
              ibuf[QuadsIndex ++] = e->ind_a;
              ibuf[QuadsIndex ++] = TriIndex - 2;
              ibuf[QuadsIndex ++] = TriIndex - 1;
              ibuf[QuadsIndex ++] = TriIndex - 1; 
              ibuf[QuadsIndex ++] = e->ind_a; 
              ibuf[QuadsIndex ++] = e->ind_b;
              EdgeStack.Delete (j);
			  delete e;
              found_b = true;
              continue;
            }
            if (e->a == mesh_vertices[mesh_triangles[i].c] &&
                e->b == mesh_vertices[mesh_triangles[i].b]) {
              ibuf[QuadsIndex ++] = e->ind_a; 
              ibuf[QuadsIndex ++] = TriIndex - 1; 
              ibuf[QuadsIndex ++] = TriIndex - 2;
              ibuf[QuadsIndex ++] = TriIndex - 2;
              ibuf[QuadsIndex ++] = e->ind_b; 
              ibuf[QuadsIndex ++] = e->ind_a;
              EdgeStack.Delete (j);
			  delete e;
              found_b = true;
              continue;
            }
          }
          if (!found_c) {
            if (e->a == mesh_vertices[mesh_triangles[i].c] && 
                e->b == mesh_vertices[mesh_triangles[i].a]) {
              ibuf[QuadsIndex ++] = e->ind_a; 
              ibuf[QuadsIndex ++] = TriIndex - 1; 
              ibuf[QuadsIndex ++] = TriIndex - 3;
              ibuf[QuadsIndex ++] = TriIndex - 3; 
              ibuf[QuadsIndex ++] = e->ind_b; 
              ibuf[QuadsIndex ++] = e->ind_a;
              EdgeStack.Delete (j);
			  delete e;
              found_c = true;
              continue;
            }
            if (e->a == mesh_vertices[mesh_triangles[i].a] &&
                e->b == mesh_vertices[mesh_triangles[i].c]) {
              ibuf[QuadsIndex ++] = e->ind_a; 
              ibuf[QuadsIndex ++] = TriIndex - 3;
              ibuf[QuadsIndex ++] = TriIndex - 1;
              ibuf[QuadsIndex ++] = TriIndex - 1; 
              ibuf[QuadsIndex ++] = e->ind_b; 
              ibuf[QuadsIndex ++] = e->ind_a;
              EdgeStack.Delete (j);
			  delete e;
              found_c = true;
              continue;
            }
          }
        }
        if (!found_a) {
          Edge *a = new Edge;
          a->a = mesh_vertices[mesh_triangles[i].a];
          a->b = mesh_vertices[mesh_triangles[i].b];
          a->ind_a = TriIndex - 3;
          a->ind_b = TriIndex - 2;
          EdgeStack.Push (a);
        }
        if (!found_b) {
          Edge *b = new Edge;
          b->a = mesh_vertices[mesh_triangles[i].b];
          b->b = mesh_vertices[mesh_triangles[i].c];
          b->ind_a = TriIndex - 2;
          b->ind_b = TriIndex - 1;
          EdgeStack.Push (b);
        }
        if (!found_c) {
          Edge *c = new Edge;
          c->a = mesh_vertices[mesh_triangles[i].c];
          c->b = mesh_vertices[mesh_triangles[i].a];
          c->ind_a = TriIndex - 1;
          c->ind_b = TriIndex - 3;
          EdgeStack.Push (c);
        }
#endif
      }
#ifdef CS_USE_SHADOW_VOLUMES
      for (i = 0; i < EdgeStack.Length (); i ++) 
      {
        Edge *e = (Edge *)EdgeStack.Get (i);
        ibuf[QuadsIndex ++] = e->ind_a;
        ibuf[QuadsIndex ++] = e->ind_a;
        ibuf[QuadsIndex ++] = e->ind_b;
      }
#endif
      index_buffer->Release ();
      mesh_triangle_dirty_flag = false;
    }
    return index_buffer;
  }
  return NULL;
}





int csGenmeshMeshObjectFactory::GetComponentCount (csStringID name)
{
  if (name == vertex_name) return 3;
  if (name == texel_name) return 2;
  if (name == normal_name) return 3;
  if (name == trinormal_name) return 3;
  if (name == color_name) return 4;
  if (name == index_name) return 1;
  return 0;
}
#endif

void csGenmeshMeshObjectFactory::SetVertexCount (int n)
{
  num_mesh_vertices = n;
  initialized = false;
  delete[] mesh_normals;
  delete[] mesh_vertices;
  delete[] mesh_colors;
  delete[] mesh_texels;
#ifndef CS_USE_NEW_RENDERER
  delete[] top_mesh.vertex_fog;
#endif
  mesh_normals = new csVector3 [num_mesh_vertices];
  mesh_vertices = new csVector3 [num_mesh_vertices];
  mesh_colors = new csColor [num_mesh_vertices];
  mesh_texels = new csVector2 [num_mesh_vertices];
#ifndef CS_USE_NEW_RENDERER
  top_mesh.vertex_fog = new G3DFogInfo [num_mesh_vertices];
#endif
}

void csGenmeshMeshObjectFactory::SetTriangleCount (int n)
{
#ifndef CS_USE_NEW_RENDERER
  top_mesh.num_triangles = n;
  initialized = false;
  delete[] top_mesh.triangles;
  top_mesh.triangles = new csTriangle [top_mesh.num_triangles];
#else
  num_mesh_triangles = n;
  if (mesh_triangles) delete [] mesh_triangles;
  mesh_triangles = new csTriangle [num_mesh_triangles];
#endif
}

struct CompressVertex
{
  int orig_idx;
  float x, y, z;
  int new_idx;
};

static int compare_vt (const void *p1, const void *p2)
{
  CompressVertex *sp1 = (CompressVertex *)p1;
  CompressVertex *sp2 = (CompressVertex *)p2;
  if (sp1->x < sp2->x)
    return -1;
  else if (sp1->x > sp2->x)
    return 1;
  if (sp1->y < sp2->y)
    return -1;
  else if (sp1->y > sp2->y)
    return 1;
  if (sp1->z < sp2->z)
    return -1;
  else if (sp1->z > sp2->z)
    return 1;
  return 0;
}

static int compare_vt_orig (const void *p1, const void *p2)
{
  CompressVertex *sp1 = (CompressVertex *)p1;
  CompressVertex *sp2 = (CompressVertex *)p2;
  if (sp1->orig_idx < sp2->orig_idx)
    return -1;
  else if (sp1->orig_idx > sp2->orig_idx)
    return 1;
  return 0;
}

bool csGenmeshMeshObjectFactory::CompressVertices (
	csVector3* orig_verts, int orig_num_vts,
	csVector3*& new_verts, int& new_num_vts,
	csTriangle* orig_tris, int num_tris,
	csTriangle*& new_tris,
	int*& mapping)
{
  new_num_vts = orig_num_vts;
  new_tris = orig_tris;
  new_verts = orig_verts;
  mapping = NULL;
  if (orig_num_vts <= 0) return false;

  // Copy all the vertices.
  CompressVertex *vt = new CompressVertex[orig_num_vts];
  int i, j;
  for (i = 0; i < orig_num_vts; i++)
  {
    vt[i].orig_idx = i;
    vt[i].x = (float)ceil (orig_verts[i].x * 1000000);
    vt[i].y = (float)ceil (orig_verts[i].y * 1000000);
    vt[i].z = (float)ceil (orig_verts[i].z * 1000000);
  }

  // First sort so that all (nearly) equal vertices are together.
  qsort (vt, orig_num_vts, sizeof (CompressVertex), compare_vt);

  // Count unique values and tag all doubles with the index of the unique one.
  // new_idx in the vt table will be the index inside vt to the unique vector.
  new_num_vts = 1;
  int last_unique = 0;
  vt[0].new_idx = last_unique;
  for (i = 1 ; i < orig_num_vts ; i++)
  {
    if (
      vt[i].x != vt[last_unique].x ||
      vt[i].y != vt[last_unique].y ||
      vt[i].z != vt[last_unique].z)
    {
      last_unique = i;
      new_num_vts++;
    }

    vt[i].new_idx = last_unique;
  }

  // If count_unique == num_vertices then there is nothing to do.
  if (new_num_vts == orig_num_vts)
  {
    delete[] vt;
    return false;
  }

  // Now allocate and fill new vertex tables.
  // After this new_idx in the vt table will be the new index
  // of the vector.
  new_verts = new csVector3[new_num_vts];
  new_verts[0] = orig_verts[vt[0].orig_idx];

  vt[0].new_idx = 0;
  j = 1;
  for (i = 1 ; i < orig_num_vts ; i++)
  {
    if (vt[i].new_idx == i)
    {
      new_verts[j] = orig_verts[vt[i].orig_idx];
      vt[i].new_idx = j;
      j++;
    }
    else
      vt[i].new_idx = j - 1;
  }

  // Now we sort the table back on orig_idx so that we have
  // a mapping from the original indices to the new one (new_idx).
  qsort (vt, orig_num_vts, sizeof (CompressVertex), compare_vt_orig);

  // Now we can remap the vertices in all triangles.
  new_tris = new csTriangle[num_tris];
  for (i = 0 ; i < num_tris ; i++)
  {
    new_tris[i].a = vt[orig_tris[i].a].new_idx;
    new_tris[i].b = vt[orig_tris[i].b].new_idx;
    new_tris[i].c = vt[orig_tris[i].c].new_idx;
  }
  mapping = new int[orig_num_vts];
  for (i = 0 ; i < orig_num_vts ; i++)
    mapping[i] = vt[i].new_idx;

  delete[] vt;
  return true;
}

void csGenmeshMeshObjectFactory::CalculateNormals ()
{
  int i, j;

#ifndef CS_USE_NEW_RENDERER
  int num_triangles = top_mesh.num_triangles;
#else
  int num_triangles = num_mesh_triangles;
#endif
  csTriangle* tris;
  csVector3* new_verts;
  int new_num_verts;
  int* mapping;

#ifndef CS_USE_NEW_RENDERER
  bool compressed = CompressVertices (mesh_vertices, num_mesh_vertices,
      new_verts, new_num_verts,
      top_mesh.triangles, num_triangles, tris,
      mapping);
#else
  bool compressed = CompressVertices (mesh_vertices, num_mesh_vertices,
      new_verts, new_num_verts,
      mesh_triangles, num_triangles, tris,
      mapping);
#endif

  csTriangleMesh* tri_mesh = new csTriangleMesh ();
  tri_mesh->SetTriangles (tris, num_triangles);
  csGenTriangleVertices* tri_verts = new csGenTriangleVertices (tri_mesh,
  	new_verts, new_num_verts);

  if (mesh_tri_normals)
    delete [] mesh_tri_normals;
  mesh_tri_normals = new csVector3[num_triangles];
#ifdef CS_USE_NEW_RENDERER
  mesh_tri_normals_dirty_flag = true;
  autonormals = true;
#endif

  // Calculate triangle normals.
  // Get the cross-product of 2 edges of the triangle and normalize it.
  for (i = 0; i < num_triangles; i++)
  {
    csVector3 ab = new_verts [tris[i].b] - new_verts [tris[i].a];
    csVector3 bc = new_verts [tris[i].c] - new_verts [tris[i].b];
    mesh_tri_normals [i] = ab % bc;
    float norm = mesh_tri_normals[i].Norm ();
    if (norm)
      mesh_tri_normals[i] /= norm;
  }

  csVector3* new_normals = mesh_normals;
  if (compressed)
    new_normals = new csVector3[new_num_verts];

  // Calculate vertex normals, by averaging connected triangle normals.
  for (i = 0 ; i < new_num_verts ; i++)
  {
    csTriangleVertex &vt = tri_verts->GetVertex (i);
    if (vt.num_con_triangles)
    {
      csVector3 &n = new_normals[i];
      n.Set (0,0,0);
      for (j = 0; j < vt.num_con_triangles; j++)
        n += mesh_tri_normals [vt.con_triangles[j]];
      float norm = n.Norm ();
      if (norm)
        n /= norm;
    }
    else
    {
      // If there are no connecting triangles then we just
      // initialize the normal to a default value.
      new_normals[i].Set (1, 0, 0);
    }
  }

  delete tri_verts;
  delete tri_mesh;

  if (compressed)
  {
    // Translate the mapped normal table back to the original table.
    for (i = 0 ; i < num_mesh_vertices ; i++)
    {
      mesh_normals[i] = new_normals[mapping[i]];
    }

    delete[] new_normals;
    delete[] new_verts;
    delete[] tris;
    delete[] mapping;
  }
}

void csGenmeshMeshObjectFactory::GenerateBox (const csBox3& box)
{
  SetVertexCount (8);
  SetTriangleCount (12);
  int i;
  csVector3* v = GetVertices ();
  csVector2* uv = GetTexels ();
  for (i = 0 ; i < 8 ; i++)
  {
    v[i].Set (box.GetCorner (7-i));
  }
  uv[0].Set (0, 0);
  uv[1].Set (1, 0);
  uv[2].Set (0, 1);
  uv[3].Set (1, 1);
  uv[4].Set (1, 1);
  uv[5].Set (0, 0);
  uv[6].Set (0, 0);
  uv[7].Set (0, 1);
  csTriangle* triangles = GetTriangles ();
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
}

void csGenmeshMeshObjectFactory::Invalidate ()
{
  object_bbox_valid = false;
  delete[] polygons;
  polygons = NULL;

#ifdef CS_USE_NEW_RENDERER
  mesh_vertices_dirty_flag = true;
  mesh_texels_dirty_flag = true;
  mesh_normals_dirty_flag = true;
  mesh_colors_dirty_flag = true;
  mesh_triangle_dirty_flag = true;
  mesh_tri_normals_dirty_flag = true;
#endif
}

void csGenmeshMeshObjectFactory::HardTransform (
    const csReversibleTransform& t)
{
  int i;
  for (i = 0 ; i < num_mesh_vertices ; i++)
    mesh_vertices[i] = t.This2Other (mesh_vertices[i]);
  initialized = false;
#ifdef CS_USE_NEW_RENDERER
  mesh_vertices_dirty_flag = true;
#endif  
}

csPtr<iMeshObject> csGenmeshMeshObjectFactory::NewInstance ()
{
  csGenmeshMeshObject* cm = new csGenmeshMeshObject (this);
  csRef<iMeshObject> im (SCF_QUERY_INTERFACE (cm, iMeshObject));
  cm->DecRef ();
  return csPtr<iMeshObject> (im);
}

#ifndef CS_USE_NEW_RENDERER
void csGenmeshMeshObjectFactory::eiVertexBufferManagerClient::ManagerClosing ()
{
  if (scfParent->vbuf)
  {
    scfParent->vbuf = NULL;
    scfParent->vbufmgr = NULL;
  }
}
#endif

csMeshedPolygon* csGenmeshMeshObjectFactory::GetPolygons ()
{
  if (!polygons)
  {
#ifndef CS_USE_NEW_RENDERER
    csTriangle* triangles = top_mesh.triangles;
    polygons = new csMeshedPolygon [top_mesh.num_triangles];
#else
    csTriangle* triangles = mesh_triangles;
    polygons = new csMeshedPolygon [num_mesh_triangles];
#endif
    int i;
#ifndef CS_USE_NEW_RENDERER
    for (i = 0 ; i < top_mesh.num_triangles ; i++)
#else
    for (i = 0 ; i < num_mesh_triangles ; i++)
#endif
    {
      polygons[i].num_vertices = 3;
      polygons[i].vertices = &triangles[i].a;
    }
  }
  return polygons;
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csGenmeshMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshMeshObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csGenmeshMeshObjectType)

SCF_EXPORT_CLASS_TABLE (genmesh)
  SCF_EXPORT_CLASS (csGenmeshMeshObjectType, "crystalspace.mesh.object.genmesh",
    "Crystal Space General Mesh Type")
SCF_EXPORT_CLASS_TABLE_END

csGenmeshMeshObjectType::csGenmeshMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csGenmeshMeshObjectType::~csGenmeshMeshObjectType ()
{
}

csPtr<iMeshObjectFactory> csGenmeshMeshObjectType::NewFactory ()
{
  csGenmeshMeshObjectFactory* cm = new csGenmeshMeshObjectFactory (this,
  	object_reg);
  csRef<iMeshObjectFactory> ifact (
  	SCF_QUERY_INTERFACE (cm, iMeshObjectFactory));
  cm->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}

