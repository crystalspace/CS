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
#include "csgeom/trimesh.h"
#include "cssys/csendian.h"
#include "csutil/csmd5.h"
#include "csutil/memfile.h"
#include "genmesh.h"
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
#include "iengine/statlght.h"
#include "iengine/dynlight.h"
#include "iutil/objreg.h"
#include "iutil/cache.h"
#include "iutil/object.h"
#include "iutil/cmdline.h"
#include "iutil/strset.h"

#include "qsqrt.h"

#include "ivideo/rendermesh.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csGenmeshMeshObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
#ifdef CS_USE_NEW_RENDERER
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iRenderBufferSource)
#endif
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iShadowCaster)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iShadowReceiver)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iGeneralMeshState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iLightingInfo)
  {
    static scfInterfaceID iPolygonMesh_scfID = (scfInterfaceID)-1;		
    if (iPolygonMesh_scfID == (scfInterfaceID)-1)				
      iPolygonMesh_scfID = iSCF::SCF->GetInterfaceID ("iPolygonMesh");		
    if (iInterfaceID == iPolygonMesh_scfID &&				
      scfCompatibleVersion (iVersion, iPolygonMesh_VERSION))		
    {
#ifdef CS_DEBUG
      printf ("Deprecated feature use: iPolygonMesh queried from Genmesh "
	"object; use iMeshObject->GetObjectModel()->"
	"GetPolygonMeshColldet() instead.\n");
#endif
      (&scfiPolygonMesh)->IncRef ();						
      return STATIC_CAST(iPolygonMesh*, &scfiPolygonMesh);				
    }
  }
SCF_IMPLEMENT_IBASE_END

#ifdef CS_USE_NEW_RENDERER
SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshMeshObject::BufferSource)
  SCF_IMPLEMENTS_INTERFACE (iRenderBufferSource) 
SCF_IMPLEMENT_EMBEDDED_IBASE_END
#endif

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
  SCF_CONSTRUCT_IBASE (0);
#ifdef CS_USE_NEW_RENDERER
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiRenderBufferSource);
#endif
  //SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiGeneralMeshState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiShadowCaster);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiShadowReceiver);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLightingInfo);
  csGenmeshMeshObject::factory = factory;
  logparent = 0;
  initialized = false;
  cur_cameranr = -1;
  cur_movablenr = -1;
  material = 0;
  MixMode = 0;
  vis_cb = 0;
  lit_mesh_colors = 0;
  num_lit_mesh_colors = 0;
  do_lighting = true;
  do_manual_colors = false;
  color.red = 0;
  color.green = 0;
  color.blue = 0;
  current_lod = 1;
  current_features = 0;
  do_shadows = true;
  do_shadow_rec = false;
  lighting_dirty = true;
  shadow_caps = false;

  dynamic_ambient.Set (0,0,0);
  ambient_version = 0;

#ifdef CS_USE_NEW_RENDERER
  g3d = CS_QUERY_REGISTRY (factory->object_reg, iGraphics3D);
#endif
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

void csGenmeshMeshObject::InitializeDefault (bool clear)
{
  SetupObject ();

  if (!do_shadow_rec) return;
  if (do_manual_colors) return;

  // Set all colors to ambient light (@@@ NEED TO GET AMBIENT!)
  int i;
  CheckLitColors ();
  if (clear)
    for (i = 0 ; i < num_lit_mesh_colors ; i++)
      lit_mesh_colors[i].Set (0, 0, 0);
  lighting_dirty = true;
}

char* csGenmeshMeshObject::GenerateCacheName ()
{
  const csBox3& b = factory->GetObjectBoundingBox ();

  csMemFile mf;
  mf.Write ("genmesh", 7);
  long l;
  l = convert_endian (factory->GetVertexCount ());
  mf.Write ((char*)&l, 4);
  l = convert_endian (factory->GetTriangleCount ());
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
      l = convert_endian ((int)QInt ((pos.x * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int)QInt ((pos.y * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int)QInt ((pos.z * 1000)+.5));
      mf.Write ((char*)&l, 4);
      csReversibleTransform tr = movable->GetFullTransform ();
      const csMatrix3& o2t = tr.GetO2T ();
      l = convert_endian ((int)QInt ((o2t.m11 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int)QInt ((o2t.m12 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int)QInt ((o2t.m13 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int)QInt ((o2t.m21 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int)QInt ((o2t.m22 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int)QInt ((o2t.m23 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int)QInt ((o2t.m31 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int)QInt ((o2t.m32 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int)QInt ((o2t.m33 * 1000)+.5));
      mf.Write ((char*)&l, 4);
    }
  }

  l = convert_endian ((int)QInt ((b.MinX () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int)QInt ((b.MinY () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int)QInt ((b.MinZ () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int)QInt ((b.MaxX () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int)QInt ((b.MaxY () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int)QInt ((b.MaxZ () * 1000)+.5));
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

#define LMMAGIC	    "LM04" // must be 4 chars!

bool csGenmeshMeshObject::ReadFromCache (iCacheManager* cache_mgr)
{
  if (!do_shadow_rec) return true;
  SetupObject ();
  lighting_dirty = true;
  char* cachename = GenerateCacheName ();
  cache_mgr->SetCurrentScope (cachename);
  delete[] cachename;

  bool rc = false;
  csRef<iDataBuffer> db = cache_mgr->ReadCache ("genmesh_lm", 0, ~0);
  if (db)
  {
    csMemFile mf ((const char*)(db->GetData ()), db->GetSize ());
    char magic[5];
    if (mf.Read (magic, 4) != 4) goto stop;
    magic[4] = 0;
    if (strcmp (magic, LMMAGIC)) goto stop;

    char cont;
    if (mf.Read ((char*)&cont, 1) != 1) goto stop;
    while (cont)
    {
      char lid[16];
      if (mf.Read (lid, 16) != 16) goto stop;
      iStatLight *il = factory->engine->FindLightID (lid);
      if (!il) goto stop;
      iLight* l = il->QueryLight ();
      affecting_lights.Add (l);
      il->AddAffectedLightingInfo (&scfiLightingInfo);
      if (mf.Read ((char*)&cont, 1) != 1) goto stop;
    }
    rc = true;
  }

stop:
  cache_mgr->SetCurrentScope (0);
  return rc;
}

bool csGenmeshMeshObject::WriteToCache (iCacheManager* cache_mgr)
{
  if (!do_shadow_rec) return true;
  char* cachename = GenerateCacheName ();
  cache_mgr->SetCurrentScope (cachename);
  delete[] cachename;

  bool rc = false;
  csMemFile mf;
  mf.Write (LMMAGIC, 4);
  csGlobalHashIterator it (affecting_lights.GetHashMap ());
  while (it.HasNext ())
  {
    iLight* l = (iLight*)it.Next ();
    csRef<iStatLight> sl = SCF_QUERY_INTERFACE (l, iStatLight);
    if (sl)
    {
      char cont = 1;
      mf.Write ((char*)&cont, 1);
      const char* lid = l->GetLightID ();
      mf.Write ((char*)lid, 16);
    }
  }
  char cont = 0;
  mf.Write ((char*)&cont, 1);
  if (!cache_mgr->CacheData ((void*)(mf.GetData ()), mf.GetSize (),
    	"genmesh_lm", 0, ~0))
    goto stop;

  rc = true;

stop:
  cache_mgr->SetCurrentScope (0);
  return rc;
}

void csGenmeshMeshObject::PrepareLighting ()
{
}

void csGenmeshMeshObject::DynamicLightChanged (iDynLight* dynlight)
{
  (void)dynlight;
  lighting_dirty = true;
}

void csGenmeshMeshObject::DynamicLightDisconnect (iDynLight* dynlight)
{
  affecting_lights.Delete (dynlight->QueryLight ());
  lighting_dirty = true;
}

void csGenmeshMeshObject::StaticLightChanged (iStatLight* statlight)
{
  (void)statlight;
  lighting_dirty = true;
}

void csGenmeshMeshObject::StaticLightDisconnect (iStatLight* statlight)
{
  affecting_lights.Delete (statlight->QueryLight ());
  lighting_dirty = true;
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
    vt_array_to_delete = 0;
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
    frust = list->AddShadow (origin, 0, 3, pl);
    frust->GetVertex (0).Set (vt_world[tri->a] - origin);
    frust->GetVertex (1).Set (vt_world[tri->b] - origin);
    frust->GetVertex (2).Set (vt_world[tri->c] - origin);
  }

  delete[] vt_array_to_delete;
}

bool csGenmeshMeshObject::SetMaterialWrapper (iMaterialWrapper* mat)
{
  material = mat;
  iMaterialWrapper* mater = material;
  if (!mater) mater = factory->GetMaterialWrapper ();
  material_needs_visit = mater->IsVisitRequired ();
  return true;
}

void csGenmeshMeshObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;
    delete[] lit_mesh_colors;
    lit_mesh_colors = 0;
    if (!do_manual_colors)
    {
      num_lit_mesh_colors = factory->GetVertexCount ();
      lit_mesh_colors = new csColor [num_lit_mesh_colors];
      for (int i = 0 ; i <  num_lit_mesh_colors; i++)
          lit_mesh_colors[i].Set (.2, .2, .2);
    }
    iMaterialWrapper* mater = material;
    if (!mater) mater = factory->GetMaterialWrapper ();
    material_needs_visit = mater->IsVisitRequired ();
  }
}

bool csGenmeshMeshObject::DrawTest (iRenderView* rview, iMovable* movable)
{
#ifndef CS_USE_NEW_RENDERER
  SetupObject ();
  CheckLitColors ();
#endif

  iGraphics3D* g3d = rview->GetGraphics3D ();
//#endif
  iCamera* camera = rview->GetCamera ();

  // First create the transformation from object to camera space directly:
  //   W = Mow * O - Vow;
  //   C = Mwc * (W - Vwc)
  // ->
  //   C = Mwc * (Mow * O - Vow - Vwc)
  //   C = Mwc * Mow * O - Mwc * (Vow + Vwc)
#ifndef CS_USE_NEW_RENDERER
  csReversibleTransform tr_o2c;
#endif
  // Shouldn't this be done in the renderer?
  tr_o2c = camera->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movable->GetFullTransform ();

  int clip_portal, clip_plane, clip_z_plane;
  csVector3 radius;
  csSphere sphere;
  GetRadius (radius, sphere.GetCenter ());
  float max_radius = radius.x;
  if (max_radius < radius.y) max_radius = radius.y;
  if (max_radius < radius.z) max_radius = radius.z;
  sphere.SetRadius (max_radius);
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
  mesh.object2camera = tr_o2c;
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

  //csFrustumContext* ctxt = fview->GetFrustumContext ();
  iBase* b = (iBase *)fview->GetUserdata ();
  iLightingProcessInfo* lpi = (iLightingProcessInfo*)b;
  CS_ASSERT (lpi != 0);

  iLight* li = lpi->GetLight ();
  bool dyn = lpi->IsDynamic ();

  if (!dyn)
  {
    csRef<iStatLight> sl = SCF_QUERY_INTERFACE (li, iStatLight);
    sl->AddAffectedLightingInfo (&scfiLightingInfo);
  }
  else
  {
    csRef<iDynLight> dl = SCF_QUERY_INTERFACE (li, iDynLight);
    dl->AddAffectedLightingInfo (&scfiLightingInfo);
  }

  affecting_lights.Add (li);
}

void csGenmeshMeshObject::UpdateLightingOne (const csReversibleTransform& trans,
	iLight* li)
{
  csVector3* normals = factory->GetNormals ();
  csColor* colors = lit_mesh_colors;
  // Compute light position in object coordinates
  csVector3 wor_light_pos = li->GetCenter ();
  csVector3 obj_light_pos = trans.Other2This (wor_light_pos);
  float obj_sq_dist = csSquaredDist::PointPoint (obj_light_pos, 0);
  if (obj_sq_dist >= li->GetInfluenceRadiusSq ()) return;
  float in_obj_dist = (obj_sq_dist >= SMALL_EPSILON)?qisqrt (obj_sq_dist):1.0f;

  csColor light_color = li->GetColor () * (256. / CS_NORMAL_LIGHT_LEVEL)
      * li->GetBrightnessAtDistance (qsqrt (obj_sq_dist));

  csColor col;
  int i;
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

void csGenmeshMeshObject::UpdateLighting2 (iMovable* movable)
{
  SetupObject ();
  CheckLitColors ();

  if (do_manual_colors) return;
  if (!do_shadow_rec) return;

  if (!lighting_dirty) return;
  lighting_dirty = false;

  int i;
  csColor* colors = lit_mesh_colors;

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

  csReversibleTransform trans = movable->GetFullTransform ();
  csGlobalHashIterator it (affecting_lights.GetHashMap ());
  while (it.HasNext ())
  {
    iLight* l = (iLight*)it.Next ();
    UpdateLightingOne (trans, l);
  }

  // @@@ Try to avoid this loop!
  // Clamp all vertex colors to 2.
  for (i = 0 ; i < factory->GetVertexCount () ; i++)
    colors[i].Clamp (2., 2., 2.);
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
    UpdateLightingOne (trans, li);
  }

  // @@@ Try to avoid this loop!
  // Clamp all vertex colors to 2.
  for (i = 0 ; i < factory->GetVertexCount () ; i++)
    colors[i].Clamp (2., 2., 2.);
}

bool csGenmeshMeshObject::Draw (iRenderView* rview, iMovable* movable,
	csZBufMode mode)
{
  UpdateLighting2 (movable);
  iMaterialWrapper* mater = material;
  if (!mater) mater = factory->GetMaterialWrapper ();
  if (!mater)
  {
    printf ("INTERNAL ERROR: mesh used without material!\n");
    return false;
  }
  if (material_needs_visit) mater->Visit ();
  iMaterialHandle* mat = mater->GetMaterialHandle ();
  if (!mat)
  {
    printf ("INTERNAL ERROR: mesh used without valid material handle!\n");
    return false;
  }

  if (vis_cb) if (!vis_cb->BeforeDrawing (this, rview)) return false;

  iGraphics3D* g3d = rview->GetGraphics3D ();

  // Prepare for rendering.
#ifndef CS_USE_NEW_RENDERER
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, mode);

  G3DTriangleMesh& m = factory->GetMesh ();
  iVertexBuffer* vbuf = factory->GetVertexBuffer ();
  iVertexBufferManager* vbufmgr = factory->GetVertexBufferManager ();
  m.mat_handle = mat;
  m.use_vertex_color = true;
  m.mixmode = MixMode | CS_FX_GOURAUD;
  CS_ASSERT (!vbuf->IsLocked ());
  const csBox3& b = factory->GetObjectBoundingBox ();
  vbufmgr->LockBuffer (vbuf,
  	factory->GetVertices (),
	factory->GetTexels (),
	do_manual_colors ? factory->GetColors () : lit_mesh_colors,
	factory->GetVertexCount (), 0,
	b);
  rview->CalculateFogMesh (g3d->GetObjectToCamera (), m);
  g3d->DrawTriangleMesh (m);
  vbufmgr->UnlockBuffer (vbuf);
#endif

  return true;
}

#ifdef CS_USE_NEW_RENDERER
iRenderBuffer *csGenmeshMeshObject::GetRenderBuffer (csStringID name)
{
  return factory->GetRenderBuffer (name);
}
#endif

csRenderMesh** csGenmeshMeshObject::GetRenderMeshes (int& n)
{
#ifdef CS_USE_NEW_RENDERER
  SetupObject ();
  //if (vis_cb) if (!vis_cb->BeforeDrawing (this, rview)) return false;

  // iGraphics3D* g3d = rview->GetGraphics3D ();

//  iCamera* camera = rview->GetCamera ();
/*  tr_o2c = camera->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movable->GetFullTransform ();
*/

  iMaterialWrapper* mater = material;
  if (!mater) mater = factory->GetMaterialWrapper ();
  if (!mater)
  {
    printf ("INTERNAL ERROR: mesh used without material!\n");
    return false;
  }

  // iGraphics3D* g3d = rview->GetGraphics3D ();

  if (material_needs_visit) mater->Visit ();
  //mesh.transform = &tr_o2c;
  //mesh.object2world = movable->GetFullTransform ();

  // Prepare for rendering.
  mesh.z_buf_mode = CS_ZBUF_TEST;// mode;
  mesh.mixmode = CS_FX_COPY; // MixMode;

  mesh.indexstart = 0;
  mesh.indexend = factory->GetTriangleCount () * 3;
  //mesh.mathandle = mater->GetMaterialHandle();
  mesh.material = mater;
  csRef<iRenderBufferSource> source = SCF_QUERY_INTERFACE (factory, iRenderBufferSource);
  mesh.buffersource = source;
  mesh.meshtype = CS_MESHTYPE_TRIANGLES;
  meshPtr = &mesh;
  n = 1;
  return &meshPtr;
#else
  n = 0;
  return 0;
#endif
}

void csGenmeshMeshObject::GetObjectBoundingBox (csBox3& bbox, int /*type*/)
{
  bbox = factory->GetObjectBoundingBox ();
}

void csGenmeshMeshObject::GetRadius (csVector3& rad, csVector3& cent)
{
  rad = factory->GetRadius ();
  cent.Set (0);
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
  const csVector3& end, csVector3& isect, float *pr, int* polygon_idx)
{
  if (polygon_idx) *polygon_idx = -1;
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
      temp = csSquaredDist::PointPoint (start, tmp);
      if (temp < dist)
      {
        isect = tmp;
	dist = temp;
      }
    }
  }
  if (pr) *pr = qsqrt (dist * itot_dist);
  if (dist >= tot_dist)
    return false;
  return true;
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

int csGenmeshMeshObject::PolyMesh::GetTriangleCount ()
{
  return scfParent->factory->GetTriangleCount ();
}

csTriangle* csGenmeshMeshObject::PolyMesh::GetTriangles ()
{
  return scfParent->factory->GetTriangles ();
}

iObjectModel* csGenmeshMeshObject::GetObjectModel ()
{
  return factory->GetObjectModel ();
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csGenmeshMeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  {
    static scfInterfaceID iPolygonMesh_scfID = (scfInterfaceID)-1;		
    if (iPolygonMesh_scfID == (scfInterfaceID)-1)				
      iPolygonMesh_scfID = iSCF::SCF->GetInterfaceID ("iPolygonMesh");		
    if (iInterfaceID == iPolygonMesh_scfID &&				
      scfCompatibleVersion (iVersion, iPolygonMesh_VERSION))		
    {
      printf ("Deprecated feature use: iPolygonMesh queried from GenMesh "
	"factory; use iObjectModel->GetPolygonMeshColldet() instead.\n");
      iPolygonMesh* Object = scfiObjectModel.GetPolygonMeshColldet();
      (Object)->IncRef ();						
      return STATIC_CAST(iPolygonMesh*, Object);				
    }
  }
#ifdef CS_USE_NEW_RENDERER
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iRenderBufferSource)
#endif
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iGeneralFactoryState)
#ifndef CS_USE_NEW_RENDERER
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iVertexBufferManagerClient)
#endif
SCF_IMPLEMENT_IBASE_END

#ifdef CS_USE_NEW_RENDERER
SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshMeshObjectFactory::BufferSource)
  SCF_IMPLEMENTS_INTERFACE (iRenderBufferSource) 
SCF_IMPLEMENT_EMBEDDED_IBASE_END
#endif

SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshMeshObjectFactory::GeneralFactoryState)
  SCF_IMPLEMENTS_INTERFACE (iGeneralFactoryState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshMeshObjectFactory::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

#ifndef CS_USE_NEW_RENDERER
SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshMeshObjectFactory::
	eiVertexBufferManagerClient)
  SCF_IMPLEMENTS_INTERFACE (iVertexBufferManagerClient)
SCF_IMPLEMENT_EMBEDDED_IBASE_END
#endif

csGenmeshMeshObjectFactory::csGenmeshMeshObjectFactory (iBase *pParent,
      iObjectRegistry* object_reg)
#ifdef CS_USE_NEW_RENDERER 
     : anon_buffers(object_reg)
#endif
{
  SCF_CONSTRUCT_IBASE (pParent);
#ifdef CS_USE_NEW_RENDERER
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiRenderBufferSource);
#endif
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiGeneralFactoryState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
#ifndef CS_USE_NEW_RENDERER 
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiVertexBufferManagerClient);
#endif
  csGenmeshMeshObjectFactory::object_reg = object_reg;

  scfiPolygonMesh.SetFactory (this);
  scfiObjectModel.SetPolygonMeshBase (&scfiPolygonMesh);
  scfiObjectModel.SetPolygonMeshColldet (&scfiPolygonMesh);
  scfiObjectModel.SetPolygonMeshViscull (0);
  scfiObjectModel.SetPolygonMeshShadows (&scfiPolygonMesh);

  logparent = 0;
  initialized = false;
  object_bbox_valid = false;
  mesh_tri_normals = 0;
#ifndef CS_USE_NEW_RENDERER
  top_mesh.num_triangles = 0;
  top_mesh.triangles = 0;
  top_mesh.vertex_fog = 0;
#else
  num_mesh_triangles = 0;
  mesh_triangles = 0;
#endif
  num_mesh_vertices = 0;
  mesh_vertices = 0;
  mesh_texels = 0;
  mesh_colors = 0;
  mesh_normals = 0;
#ifndef CS_USE_NEW_RENDERER
  vbufmgr = 0;
#endif
  material = 0;
  polygons = 0;
#ifdef CS_USE_NEW_RENDERER
  vertex_buffer = 0;
  normal_buffer = 0;
  texel_buffer = 0;
  color_buffer = 0;
  index_buffer = 0;

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);

  csRef<iStringSet> strings = 
    CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
    "crystalspace.renderer.stringset", iStringSet);
  vertex_name = strings->Request ("vertices");
  texel_name = strings->Request ("texture coordinates");
  normal_name = strings->Request ("normals");
  color_name = strings->Request ("colors");
  index_name = strings->Request ("indices");

  autonormals = false;
  mesh_vertices_dirty_flag = false;
  mesh_texels_dirty_flag = false;
  mesh_normals_dirty_flag = false;
  mesh_colors_dirty_flag = false;
  mesh_triangle_dirty_flag = false;
#endif

  csRef<iEngine> eng = CS_QUERY_REGISTRY (object_reg, iEngine);
  engine = eng;	// We don't want a circular reference!
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
  delete[] mesh_tri_normals;

#ifdef CS_USE_NEW_RENDERER
  delete [] mesh_triangles;
#endif
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
  csVector3& v0 = mesh_vertices[0];
  object_bbox.StartBoundingBox (v0);
  csVector3 max_sq_radius (v0.x*v0.x + v0.x*v0.x,
    	v0.y*v0.y + v0.y*v0.y, v0.z*v0.z + v0.z*v0.z);
  int i;
  for (i = 1 ; i < num_mesh_vertices ; i++)
  {
    csVector3& v = mesh_vertices[i];
    object_bbox.AddBoundingVertexSmart (v);
    csVector3 sq_radius (v.x*v.x + v.x*v.x,
    	v.y*v.y + v.y*v.y, v.z*v.z + v.z*v.z);
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

iRenderBuffer *csGenmeshMeshObjectFactory::GetRenderBuffer (csStringID name)
{
  if (name == vertex_name)
  {
    if (mesh_vertices_dirty_flag)
    {
      vertex_buffer = g3d->CreateRenderBuffer (
        sizeof (csVector3)*num_mesh_vertices, CS_BUF_STATIC, 
        CS_BUFCOMP_FLOAT, 3, false);
      mesh_vertices_dirty_flag = false;
      csVector3* vbuf = (csVector3*)vertex_buffer->Lock(CS_BUF_LOCK_NORMAL);
      memcpy (vbuf, mesh_vertices, sizeof(csVector3)*num_mesh_vertices);
      vertex_buffer->Release ();
    }
    if (vertex_buffer)
    {
      return vertex_buffer;
    }
    return 0;
  }
  if (name == texel_name)
  {
    if (mesh_texels_dirty_flag)
    {
      texel_buffer = g3d->CreateRenderBuffer (
        sizeof (csVector2)*num_mesh_vertices, CS_BUF_STATIC, 
        CS_BUFCOMP_FLOAT, 2, false);
      mesh_texels_dirty_flag = false;
      csVector2* tbuf = (csVector2*)texel_buffer->Lock (CS_BUF_LOCK_NORMAL);
      memcpy (tbuf, mesh_texels, sizeof (csVector2) * num_mesh_vertices);
      texel_buffer->Release ();
    }
    if (texel_buffer)
    {
      return texel_buffer;
    }
    return 0;
  }
  if (name == normal_name)
  {
    if (mesh_normals_dirty_flag)
    {
      normal_buffer = g3d->CreateRenderBuffer (
        sizeof (csVector3)*num_mesh_vertices, CS_BUF_STATIC,
        CS_BUFCOMP_FLOAT, 3, false);
      mesh_normals_dirty_flag = false;
      csVector3 *nbuf = (csVector3*)normal_buffer->Lock(CS_BUF_LOCK_NORMAL);
      memcpy (nbuf, mesh_normals, sizeof (csVector3)*num_mesh_vertices);
      normal_buffer->Release();
    }
    if(normal_buffer)
    {
      return normal_buffer;
    }
    return 0;
  }
  if (name == color_name)
  {
    if (mesh_colors_dirty_flag)
    {
      color_buffer = g3d->CreateRenderBuffer (
        sizeof (csColor)*num_mesh_vertices, CS_BUF_STATIC,
        CS_BUFCOMP_FLOAT, 3, false);
      mesh_colors_dirty_flag = false;
      csColor *cbuf = (csColor*)color_buffer->Lock(CS_BUF_LOCK_NORMAL);
      memcpy (cbuf, mesh_colors, sizeof (csColor) * num_mesh_vertices);
      color_buffer->Release();
    }
    if (color_buffer)
    {
      return color_buffer;
    }
    return 0;
  }
  if (name == index_name)
  {
    if (mesh_triangle_dirty_flag)
    {
      index_buffer = g3d->CreateRenderBuffer (
        sizeof (unsigned int)*num_mesh_triangles*3, CS_BUF_STATIC,
        CS_BUFCOMP_UNSIGNED_INT, 1, true);
      mesh_triangle_dirty_flag = false;
      unsigned int *ibuf = (unsigned int *)index_buffer->Lock(
        CS_BUF_LOCK_NORMAL);
      int i;
      for (i = 0; i < num_mesh_triangles; i ++) 
      {
        ibuf[i * 3 + 0] = mesh_triangles[i].a;
        ibuf[i * 3 + 1] = mesh_triangles[i].b;
        ibuf[i * 3 + 2] = mesh_triangles[i].c;
      }
      index_buffer->Release ();
    }
    if (index_buffer)
    {
      return index_buffer;
    }
    return 0;
  }
  return anon_buffers.GetRenderBuffer(name);
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
  memset (mesh_normals, 0, sizeof (csVector3)*num_mesh_vertices);
  mesh_vertices = new csVector3 [num_mesh_vertices];
  mesh_colors = new csColor [num_mesh_vertices];
  mesh_texels = new csVector2 [num_mesh_vertices];
#ifndef CS_USE_NEW_RENDERER
  top_mesh.vertex_fog = new G3DFogInfo [num_mesh_vertices];
#endif

  scfiObjectModel.ShapeChanged ();
}

void csGenmeshMeshObjectFactory::SetTriangleCount (int n)
{
  csTriangle* new_triangles = new csTriangle [n];
#ifndef CS_USE_NEW_RENDERER
  if (top_mesh.triangles)
  {
    memcpy (new_triangles, top_mesh.triangles, sizeof (csTriangle)*
    	MIN (n, top_mesh.num_triangles));
  }
  top_mesh.num_triangles = n;
  delete[] top_mesh.triangles;
  top_mesh.triangles = new_triangles;
#else
  if (mesh_triangles)
  {
    memcpy (new_triangles, mesh_triangles, sizeof (csTriangle)*
    	MIN (n, num_mesh_triangles));
  }
  num_mesh_triangles = n;
  delete [] mesh_triangles;
  mesh_triangles = new_triangles;
#endif

  initialized = false;
  scfiObjectModel.ShapeChanged ();
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
  mapping = 0;
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
  csTriangleVertices* tri_verts = new csTriangleVertices (tri_mesh,
  	new_verts, new_num_verts);

  delete[] mesh_tri_normals;
  mesh_tri_normals = new csVector3[num_triangles];
#ifdef CS_USE_NEW_RENDERER
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
    if (vt.con_triangles.Length ())
    {
      csVector3 &n = new_normals[i];
      n.Set (0,0,0);
      for (j = 0; j < vt.con_triangles.Length () ; j++)
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

bool csGenmeshMeshObjectFactory::AddRenderBuffer (const char *name, 
  csRenderBufferComponentType component_type, int component_size)
{
#ifdef CS_USE_NEW_RENDERER
  return anon_buffers.AddRenderBuffer (name, component_type,
    component_size, num_mesh_vertices);
#else
  return false;
#endif
}

bool csGenmeshMeshObjectFactory::SetRenderBufferComponent (const char *name,
	int index, int component, float value)
{
#ifdef CS_USE_NEW_RENDERER
  return anon_buffers.SetRenderBufferComponent(name, index, component, value);
#else
  return false;
#endif
}

bool csGenmeshMeshObjectFactory::SetRenderBufferComponent (const char *name,
	int index, int component, int value)
{
#ifdef CS_USE_NEW_RENDERER
  return anon_buffers.SetRenderBufferComponent(name, index, component, value);
#else
  return false;
#endif
}

bool csGenmeshMeshObjectFactory::SetRenderBuffer (const char *name,
	float *value)
{
#ifdef CS_USE_NEW_RENDERER
  return anon_buffers.SetRenderBuffer (name, value, num_mesh_vertices);
#else
  return false;
#endif
}

bool csGenmeshMeshObjectFactory::SetRenderBuffer (const char *name, int *value)
{
#ifdef CS_USE_NEW_RENDERER
  return anon_buffers.SetRenderBuffer(name, value, num_mesh_vertices);
#else
  return false;
#endif
}

void csGenmeshMeshObjectFactory::Invalidate ()
{
  object_bbox_valid = false;
  delete[] polygons;
  polygons = 0;

#ifdef CS_USE_NEW_RENDERER
  mesh_vertices_dirty_flag = true;
  mesh_texels_dirty_flag = true;
  mesh_normals_dirty_flag = true;
  mesh_colors_dirty_flag = true;
  mesh_triangle_dirty_flag = true;
#endif
}

SCF_IMPLEMENT_IBASE (csGenmeshMeshObjectFactory::PolyMesh)
  SCF_IMPLEMENTS_INTERFACE (iPolygonMesh)
SCF_IMPLEMENT_IBASE_END

int csGenmeshMeshObjectFactory::PolyMesh::GetVertexCount ()
{
  return factory->GetVertexCount ();
}

csVector3* csGenmeshMeshObjectFactory::PolyMesh::GetVertices ()
{
  return factory->GetVertices ();
}

int csGenmeshMeshObjectFactory::PolyMesh::GetPolygonCount ()
{
  return factory->GetTriangleCount ();
}

csMeshedPolygon* csGenmeshMeshObjectFactory::PolyMesh::GetPolygons ()
{
  return factory->GetPolygons ();
}

int csGenmeshMeshObjectFactory::PolyMesh::GetTriangleCount ()
{
  return factory->GetTriangleCount ();
}

csTriangle* csGenmeshMeshObjectFactory::PolyMesh::GetTriangles ()
{
  return factory->GetTriangles ();
}

void csGenmeshMeshObjectFactory::HardTransform (
    const csReversibleTransform& t)
{
  int i;
  for (i = 0 ; i < num_mesh_vertices ; i++)
    mesh_vertices[i] = t.This2Other (mesh_vertices[i]);
#ifdef CS_USE_NEW_RENDERER
  mesh_vertices_dirty_flag = true;
#endif  
  initialized = false;
  scfiObjectModel.ShapeChanged ();
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
    scfParent->vbuf = 0;
    scfParent->vbufmgr = 0;
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

bool csGenmeshMeshObjectType::Initialize (iObjectRegistry* object_reg)
{
  csGenmeshMeshObjectType::object_reg = object_reg;

  csRef<iCommandLineParser> cmdline = CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser);
  if (cmdline)
  {
    do_verbose = cmdline->GetOption ("verbose") != 0;
  }

  return true;
}


