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
#include "csgeom/bsptree.h"
#include "csgfx/normalmaptools.h"
#include "csutil/csendian.h"
#include "csutil/csmd5.h"
#include "csutil/memfile.h"
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
#include "iutil/cache.h"
#include "iutil/object.h"
#include "iutil/cmdline.h"
#include "iutil/strset.h"
#include "genmesh.h"

#include "csqsqrt.h"

#include "ivideo/rendermesh.h"

CS_LEAKGUARD_IMPLEMENT (csGenmeshMeshObject);
CS_LEAKGUARD_IMPLEMENT (csGenmeshMeshObjectFactory);
#ifdef CS_USE_NEW_RENDERER
CS_LEAKGUARD_IMPLEMENT (csGenmeshMeshObject::eiShaderVariableAccessor);
#endif

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csGenmeshMeshObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iShadowCaster)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iShadowReceiver)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iGeneralMeshState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iLightingInfo)
  {
    static scfInterfaceID iPolygonMesh_scfID = (scfInterfaceID)-1;
    if (iPolygonMesh_scfID == (scfInterfaceID)-1)
      iPolygonMesh_scfID = iSCF::SCF->GetInterfaceID ("iPolygonMesh");
    if (iInterfaceID == iPolygonMesh_scfID &&
      scfCompatibleVersion(iVersion, scfInterface<iPolygonMesh>::GetVersion()))
    {
#ifdef CS_DEBUG
      printf ("Deprecated feature use: iPolygonMesh queried from Genmesh "
    "object; use iMeshObject->GetObjectModel()->"
    "GetPolygonMeshColldet() instead.\n");
#endif
      (&scfiPolygonMesh)->IncRef ();
      return CS_STATIC_CAST(iPolygonMesh*, &scfiPolygonMesh);
    }
  }
SCF_IMPLEMENT_IBASE_END

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

#ifdef CS_USE_NEW_RENDERER
SCF_IMPLEMENT_IBASE (csGenmeshMeshObject::eiShaderVariableAccessor)
  SCF_IMPLEMENTS_INTERFACE (iShaderVariableAccessor)
SCF_IMPLEMENT_IBASE_END
#endif

csGenmeshMeshObject::csGenmeshMeshObject (csGenmeshMeshObjectFactory* factory) :
	pseudoDynInfo (29, 32),
	affecting_lights (29, 32)
{
  SCF_CONSTRUCT_IBASE (0);
  //SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiGeneralMeshState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiShadowCaster);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiShadowReceiver);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLightingInfo);
#ifdef CS_USE_NEW_RENDERER
  scfiShaderVariableAccessor = new eiShaderVariableAccessor (this);
#endif
  csGenmeshMeshObject::factory = factory;
  vc = factory->vc;
  logparent = 0;
  initialized = false;
  cur_cameranr = -1;
  cur_movablenr = -1;
  material = 0;
  MixMode = 0;
  vis_cb = 0;
  lit_mesh_colors = 0;
  num_lit_mesh_colors = 0;
  static_mesh_colors = 0;
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

  anim_ctrl_verts = false;
  anim_ctrl_texels = false;
  anim_ctrl_normals = false;
  anim_ctrl_colors = false;

#ifdef CS_USE_NEW_RENDERER
  num_sorted_mesh_triangles = 0;
  sorted_mesh_triangles = 0;

  svcontext = 0;

  g3d = CS_QUERY_REGISTRY (factory->object_reg, iGraphics3D);
  buffers_version = (uint)-1;
  mesh_colors_dirty_flag = true;
#endif
}

csGenmeshMeshObject::~csGenmeshMeshObject ()
{
  if (vis_cb) vis_cb->DecRef ();
  delete[] lit_mesh_colors;
  delete[] static_mesh_colors;
#ifdef CS_USE_NEW_RENDERER
  delete[] sorted_mesh_triangles;

  if (svcontext) svcontext->DecRef ();
#endif

  ClearPseudoDynLights ();

#ifdef CS_USE_NEW_RENDERER
  scfiShaderVariableAccessor->DecRef ();
#endif
  //SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiGeneralMeshState);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiShadowCaster);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiShadowReceiver);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiLightingInfo);
  SCF_DESTRUCT_IBASE ();
}

const csVector3* csGenmeshMeshObject::AnimControlGetVertices ()
{
  return anim_ctrl->UpdateVertices (vc->GetCurrentTicks (),
  	factory->GetVertices (),
	factory->GetVertexCount (),
	factory->scfiObjectModel.GetShapeNumber ());
}

const csVector2* csGenmeshMeshObject::AnimControlGetTexels ()
{
  return anim_ctrl->UpdateTexels (vc->GetCurrentTicks (),
  	factory->GetTexels (),
	factory->GetVertexCount (),
	factory->scfiObjectModel.GetShapeNumber ());
}

const csVector3* csGenmeshMeshObject::AnimControlGetNormals ()
{
  return anim_ctrl->UpdateNormals (vc->GetCurrentTicks (),
  	factory->GetNormals (),
	factory->GetVertexCount (),
	factory->scfiObjectModel.GetShapeNumber ());
}

const csColor* csGenmeshMeshObject::AnimControlGetColors (csColor* source)
{
  return anim_ctrl->UpdateColors (vc->GetCurrentTicks (),
  	source,
	factory->GetVertexCount (),
	factory->scfiObjectModel.GetShapeNumber ());
}

void csGenmeshMeshObject::SetAnimationControl (
	iGenMeshAnimationControl* ac)
{
  anim_ctrl = ac;
  if (ac)
  {
    anim_ctrl_verts = ac->AnimatesVertices ();
    anim_ctrl_texels = ac->AnimatesTexels ();
    anim_ctrl_normals = ac->AnimatesNormals ();
    anim_ctrl_colors = ac->AnimatesColors ();
  }
  else
  {
    anim_ctrl_verts = false;
    anim_ctrl_texels = false;
    anim_ctrl_normals = false;
    anim_ctrl_colors = false;
  }
  SetupShaderVariableContext ();
}

void csGenmeshMeshObject::ClearPseudoDynLights ()
{
  csHash<csShadowArray*, iLight*>::GlobalIterator it (
    pseudoDynInfo.GetIterator ());
  while (it.HasNext ())
  {
    csShadowArray* arr = it.Next ();
    delete arr;
  }
}

void csGenmeshMeshObject::CheckLitColors ()
{
  if (do_manual_colors) return;
  if (factory->GetVertexCount () != num_lit_mesh_colors)
  {
    ClearPseudoDynLights ();

    num_lit_mesh_colors = factory->GetVertexCount ();
    delete[] lit_mesh_colors;
    lit_mesh_colors = new csColor [num_lit_mesh_colors];
    delete[] static_mesh_colors;
    static_mesh_colors = new csColor [num_lit_mesh_colors];
  }
}

void csGenmeshMeshObject::InitializeDefault (bool clear)
{
  SetupObject ();

  if (!do_shadow_rec) return;
  if (do_manual_colors) return;

  // Set all colors to ambient light.
  int i;
  CheckLitColors ();
  if (clear)
  {
    csColor amb;
    factory->engine->GetAmbientLight (amb);
    for (i = 0 ; i < num_lit_mesh_colors ; i++)
    {
      lit_mesh_colors[i].Set (0, 0, 0);
      static_mesh_colors[i] = amb;
    }
  }
  lighting_dirty = true;
}

char* csGenmeshMeshObject::GenerateCacheName ()
{
  const csBox3& b = factory->GetObjectBoundingBox ();

  csMemFile mf;
  mf.Write ("genmesh", 7);
  uint32 l;
  l = convert_endian ((uint32)factory->GetVertexCount ());
  mf.Write ((char*)&l, 4);
  l = convert_endian ((uint32)factory->GetTriangleCount ());
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
      l = convert_endian ((int32)csQint ((pos.x * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int32)csQint ((pos.y * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int32)csQint ((pos.z * 1000)+.5));
      mf.Write ((char*)&l, 4);
      csReversibleTransform tr = movable->GetFullTransform ();
      const csMatrix3& o2t = tr.GetO2T ();
      l = convert_endian ((int32)csQint ((o2t.m11 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int32)csQint ((o2t.m12 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int32)csQint ((o2t.m13 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int32)csQint ((o2t.m21 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int32)csQint ((o2t.m22 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int32)csQint ((o2t.m23 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int32)csQint ((o2t.m31 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int32)csQint ((o2t.m32 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = convert_endian ((int32)csQint ((o2t.m33 * 1000)+.5));
      mf.Write ((char*)&l, 4);
    }
  }

  l = convert_endian ((int32)csQint ((b.MinX () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int32)csQint ((b.MinY () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int32)csQint ((b.MinZ () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int32)csQint ((b.MaxX () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int32)csQint ((b.MaxY () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int32)csQint ((b.MaxZ () * 1000)+.5));
  mf.Write ((char*)&l, 4);

  csMD5::Digest digest = csMD5::Encode (mf.GetData (), mf.GetSize ());
  csString hex(digest.HexString());
  return hex.Detach();
}

const char CachedLightingMagic[] = "GmL1";
const int CachedLightingMagicSize = sizeof (CachedLightingMagic);

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
    char magic[CachedLightingMagicSize];
    if (mf.Read (magic, CachedLightingMagicSize - 1) != 4) goto stop;
    magic[CachedLightingMagicSize - 1] = 0;
    if (strcmp (magic, CachedLightingMagic) == 0)
    {
      int v;
      for (v = 0; v < num_lit_mesh_colors; v++)
      {
	csColor& c = static_mesh_colors[v];
	uint8 b;
	if (mf.Read ((char*)&b, sizeof (b)) != sizeof (b)) goto stop;
	c.red = (float)b / (float)CS_NORMAL_LIGHT_LEVEL;
	if (mf.Read ((char*)&b, sizeof (b)) != sizeof (b)) goto stop;
	c.green = (float)b / (float)CS_NORMAL_LIGHT_LEVEL;
	if (mf.Read ((char*)&b, sizeof (b)) != sizeof (b)) goto stop;
	c.blue = (float)b / (float)CS_NORMAL_LIGHT_LEVEL;
      }

      uint8 c;
      if (mf.Read ((char*)&c, sizeof (c)) != sizeof (c)) goto stop;
      while (c != 0)
      {
	char lid[16];
	if (mf.Read (lid, 16) != 16) goto stop;
	iLight *l = factory->engine->FindLightID (lid);
	if (!l) goto stop;
	l->AddAffectedLightingInfo (&scfiLightingInfo);

	csShadowArray* shadowArr = new csShadowArray();
	float* intensities = new float[num_lit_mesh_colors];
	shadowArr->shadowmap = intensities;
	for (int n = 0; n < num_lit_mesh_colors; n++)
	{
          uint8 b;
          if (mf.Read ((char*)&b, sizeof (b)) != sizeof (b))
          {
            delete shadowArr;
            goto stop;
          }
          intensities[n] = (float)b / (float)CS_NORMAL_LIGHT_LEVEL;
	}
	pseudoDynInfo.Put (l, shadowArr);

        if (mf.Read ((char*)&c, sizeof (c)) != sizeof (c)) goto stop;
      }
      rc = true;
    }
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
  mf.Write (CachedLightingMagic, CachedLightingMagicSize - 1);
  for (int v = 0; v < num_lit_mesh_colors; v++)
  {
    const csColor& c = static_mesh_colors[v];
    int i; uint8 b;

    i = csQint (c.red * (float)CS_NORMAL_LIGHT_LEVEL);
    if (i < 0) i = 0; if (i > 255) i = 255; b = i;
    mf.Write ((char*)&b, sizeof (b));

    i = csQint (c.green * (float)CS_NORMAL_LIGHT_LEVEL);
    if (i < 0) i = 0; if (i > 255) i = 255; b = i;
    mf.Write ((char*)&b, sizeof (b));

    i = csQint (c.blue * (float)CS_NORMAL_LIGHT_LEVEL);
    if (i < 0) i = 0; if (i > 255) i = 255; b = i;
    mf.Write ((char*)&b, sizeof (b));
  }
  uint8 c = 1;

  csHash<csShadowArray*, iLight*>::GlobalIterator pdlIt (
    pseudoDynInfo.GetIterator ());
  while (pdlIt.HasNext ())
  {
    mf.Write ((char*)&c, sizeof (c));

    iLight* l;
    csShadowArray* shadowArr = pdlIt.Next (l);
    const char* lid = l->GetLightID ();
    mf.Write ((char*)lid, 16);

    float* intensities = shadowArr->shadowmap;
    for (int n = 0; n < num_lit_mesh_colors; n++)
    {
      int i; uint8 b;
      i = csQint (intensities[n] * (float)CS_NORMAL_LIGHT_LEVEL);
      if (i < 0) i = 0; if (i > 255) i = 255; b = i;
      mf.Write ((char*)&b, sizeof (b));
    }
  }
  c = 0;
  mf.Write ((char*)&c, sizeof (c));


  rc = cache_mgr->CacheData ((void*)(mf.GetData ()), mf.GetSize (),
    "genmesh_lm", 0, ~0);
  cache_mgr->SetCurrentScope (0);
  return rc;
}

void csGenmeshMeshObject::PrepareLighting ()
{
}

void csGenmeshMeshObject::LightChanged (iLight*)
{
  lighting_dirty = true;
}

void csGenmeshMeshObject::LightDisconnect (iLight* light)
{
  affecting_lights.Delete (light);
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

void csGenmeshMeshObject::SetupShaderVariableContext ()
{
#ifdef CS_USE_NEW_RENDERER
  if (svcontext == 0)
    svcontext = new csShaderVariableContext ();
  csShaderVariable* sv;
  if (!factory->back2front)
  {
    sv = svcontext->GetVariableAdd (csGenmeshMeshObjectFactory::index_name);
    sv->SetAccessor (factory->scfiShaderVariableAccessor);
  }
  
  bool ac_verts = false;
  bool ac_texels = false;
  bool ac_normals = false;
  if (anim_ctrl)
  {
    ac_verts = anim_ctrl->AnimatesVertices ();
    ac_texels = anim_ctrl->AnimatesTexels ();
    ac_normals = anim_ctrl->AnimatesNormals ();
  }

  sv = svcontext->GetVariableAdd (csGenmeshMeshObjectFactory::vertex_name);
  if (ac_verts)
    sv->SetAccessor (scfiShaderVariableAccessor);
  else
    sv->SetAccessor (factory->scfiShaderVariableAccessor);

  sv = svcontext->GetVariableAdd (csGenmeshMeshObjectFactory::texel_name);
  if (ac_texels)
    sv->SetAccessor (scfiShaderVariableAccessor);
  else
    sv->SetAccessor (factory->scfiShaderVariableAccessor);

  sv = svcontext->GetVariableAdd (csGenmeshMeshObjectFactory::normal_name);
  if (ac_normals)
    sv->SetAccessor (scfiShaderVariableAccessor);
  else
    sv->SetAccessor (factory->scfiShaderVariableAccessor);

  sv = svcontext->GetVariableAdd (csGenmeshMeshObjectFactory::tangent_name);
  sv->SetAccessor (factory->scfiShaderVariableAccessor);
  sv = svcontext->GetVariableAdd (csGenmeshMeshObjectFactory::binormal_name);
  sv->SetAccessor (factory->scfiShaderVariableAccessor);

  /*sv = dynDomain->GetVariableAdd (csGenmeshMeshObjectFactory::color_name);
  sv->SetAccessor (&factory->shaderVarAccessor);*/
  sv = svcontext->GetVariableAdd (csGenmeshMeshObjectFactory::color_name);
  sv->SetAccessor (scfiShaderVariableAccessor);

  for (size_t i=0;i<factory->GetAnonymousNames().Length();i++)
  {
    sv = svcontext->GetVariableAdd (factory->GetAnonymousNames().Get(i));
    sv->SetAccessor (factory->scfiShaderVariableAccessor);
  }
#endif
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
      int i;
      for (i = 0 ; i <  num_lit_mesh_colors; i++)
        lit_mesh_colors[i].Set (0.2f, 0.2f, 0.2f);  // @@@ ???
      static_mesh_colors = new csColor [num_lit_mesh_colors];
      for (i = 0 ; i <  num_lit_mesh_colors; i++)
        static_mesh_colors[i] = color;//.Set (0, 0, 0);
    }
    iMaterialWrapper* mater = material;
    if (!mater) mater = factory->GetMaterialWrapper ();
    CS_ASSERT (mater != 0);
    material_needs_visit = mater->IsVisitRequired ();

    SetupShaderVariableContext ();
  }
}

bool csGenmeshMeshObject::DrawTest (iRenderView* rview, iMovable* movable,
	uint32 frustum_mask)
{
  SetupObject ();
  CheckLitColors ();

  iCamera* camera = rview->GetCamera ();

  // First create the transformation from object to camera space directly:
  //   W = Mow * O - Vow;
  //   C = Mwc * (W - Vwc)
  // ->
  //   C = Mwc * (Mow * O - Vow - Vwc)
  //   C = Mwc * Mow * O - Mwc * (Vow + Vwc)
  csReversibleTransform tr_o2c;
  // Shouldn't this be done in the renderer?
  tr_o2c = camera->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movable->GetFullTransform ();

  int clip_portal, clip_plane, clip_z_plane;
  rview->CalculateClipSettings (frustum_mask, clip_portal, clip_plane,
  	clip_z_plane);

#ifndef CS_USE_NEW_RENDERER
  iGraphics3D* g3d = rview->GetGraphics3D ();
  g3d->SetObjectToCamera (&tr_o2c);
  G3DTriangleMesh& m = factory->GetMesh ();
  m.clip_portal = clip_portal;
  m.clip_plane = clip_plane;
  m.clip_z_plane = clip_z_plane;
  m.do_mirror = camera->IsMirrored ();
#endif

  if (factory->light_mgr)
  {
    const csArray<iLight*>& relevant_lights = factory->light_mgr
    	->GetRelevantLights (logparent, -1, false);
    UpdateLighting (relevant_lights, movable);
  }

  return true;
}

#define VERTEX_OFFSET       (10.0f * SMALL_EPSILON)

/*
  Lighting w/o local shadows:
  - Contribution from all affecting lights is calculated and summed up
    at runtime.
  Lighting with local shadows:
  - Contribution from static lights is calculated, summed and stored.
  - For every static pseudo-dynamic lights, the intensity of contribution
    is stored.
  - At runtime, the static lighting colors are copied to the actual used
    colors, the intensities of the pseudo-dynamic lights are multiplied
    with the actual colors of that lights and added as well, and finally,
    dynamic lighst are calculated.
 */
void csGenmeshMeshObject::CastShadows (iMovable* movable, iFrustumView* fview)
{
  SetupObject ();

  if (do_manual_colors) return;
  if (!do_lighting) return;

  iBase* b = (iBase *)fview->GetUserdata ();
  iLightingProcessInfo* lpi = (iLightingProcessInfo*)b;
  CS_ASSERT (lpi != 0);

  iLight* li = lpi->GetLight ();
  bool dyn = lpi->IsDynamic ();

  if (!dyn)
  {
    if (!do_shadow_rec || li->GetDynamicType () == CS_LIGHT_DYNAMICTYPE_PSEUDO)
    {
      li->AddAffectedLightingInfo (&scfiLightingInfo);
      if (li->GetDynamicType () != CS_LIGHT_DYNAMICTYPE_PSEUDO)
        affecting_lights.Add (li);
    }
  }
  else
  {
    if (!affecting_lights.In (li))
    {
      li->AddAffectedLightingInfo (&scfiLightingInfo);
      affecting_lights.Add (li);
    }
    if (do_shadow_rec) return;
  }

  if (!do_shadow_rec) return;

  csReversibleTransform o2w (movable->GetFullTransform ());

  csFrustum *light_frustum = fview->GetFrustumContext ()->GetLightFrustum ();
  iShadowBlockList* shadows = fview->GetFrustumContext ()->GetShadows ();
  iShadowIterator* shadowIt = shadows->GetShadowIterator ();

  csVector3* normals = factory->GetNormals ();
  csVector3* vertices = factory->GetVertices ();
  csColor* colors = static_mesh_colors;
  // Compute light position in object coordinates
  csVector3 wor_light_pos = li->GetCenter ();
  csVector3 obj_light_pos = o2w.Other2This (wor_light_pos);

  bool pseudoDyn = li->GetDynamicType () == CS_LIGHT_DYNAMICTYPE_PSEUDO;
  csShadowArray* shadowArr;
  if (pseudoDyn)
  {
    shadowArr = new csShadowArray ();
    pseudoDynInfo.Put (li, shadowArr);
    shadowArr->shadowmap = new float[factory->GetVertexCount ()];
    memset(shadowArr->shadowmap, 0, factory->GetVertexCount() * sizeof(float));
  }

  csColor light_color = li->GetColor () * (256. / CS_NORMAL_LIGHT_LEVEL);

  csColor col;
  int i;
  for (i = 0 ; i < factory->GetVertexCount () ; i++)
  {
    csVector3 normal = normals[i];
    /*
      A small fraction of the normal is added to prevent unwanted
      self-shadowing (due small inaccuracies, the tri(s) this vertex
      lies on may shadow it.)
     */
    csVector3 v = o2w.This2Other (vertices[i] + (normal * VERTEX_OFFSET)) -
      wor_light_pos;

    if (!light_frustum->Contains (v))
    {
      continue;
    }
    
    float vrt_sq_dist = csSquaredDist::PointPoint (obj_light_pos,
      vertices[i]);
    if (vrt_sq_dist >= li->GetInfluenceRadiusSq ()) continue;
    
    bool inShadow = false;
    shadowIt->Reset ();
    while (shadowIt->HasNext ())
    {
      csFrustum* shadowFrust = shadowIt->Next ();
      if (shadowFrust->Contains (v))
      {
	inShadow = true;
	break;
      }
    }
    if (inShadow) continue;

    float in_vrt_dist =
      (vrt_sq_dist >= SMALL_EPSILON) ? csQisqrt (vrt_sq_dist) : 1.0f;

    float cosinus;
    if (vrt_sq_dist < SMALL_EPSILON) cosinus = 1;
    else cosinus = (obj_light_pos - vertices[i]) * normal;
    // because the vector from the object center to the light center
    // in object space is equal to the position of the light

    if (cosinus > 0)
    {
      if (vrt_sq_dist >= SMALL_EPSILON) cosinus *= in_vrt_dist;
      float bright = li->GetBrightnessAtDistance (csQsqrt (vrt_sq_dist));
      if (cosinus < 1) bright *= cosinus;
      if (pseudoDyn)
      {
	// Pseudo-dynamic
	if (bright > 2.0f) bright = 2.0f; // @@@ clamp here?
	shadowArr->shadowmap[i] = bright;
      }
      else
      {
	col = light_color * bright;
	colors[i] += col;
      }
    }
  }
}

void csGenmeshMeshObject::FinalizeLighting (iMovable* movable, iLight* light,
                        const csArray<bool>& influences)
{
}

void csGenmeshMeshObject::UpdateLightingOne (
  const csReversibleTransform& trans, iLight* li)
{
  csVector3* normals = factory->GetNormals ();
  csColor* colors = lit_mesh_colors;
  // Compute light position in object coordinates
  csVector3 wor_light_pos = li->GetCenter ();
  csVector3 obj_light_pos = trans.Other2This (wor_light_pos);
  float obj_sq_dist = csSquaredDist::PointPoint (obj_light_pos, 0);
  if (obj_sq_dist >= li->GetInfluenceRadiusSq ()) return;
  float in_obj_dist =
    (obj_sq_dist >= SMALL_EPSILON) ? csQisqrt (obj_sq_dist) : 1.0f;

  csColor light_color = li->GetColor () * (256. / CS_NORMAL_LIGHT_LEVEL)
      * li->GetBrightnessAtDistance (csQsqrt (obj_sq_dist));
  if (light_color.red < EPSILON && light_color.green < EPSILON
  	&& light_color.blue < EPSILON)
    return;

  csColor col;
  int i;
  for (i = 0 ; i < factory->GetVertexCount () ; i++)
  {
    csVector3& normal = normals[i];
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

  if (!lighting_dirty) return;
  lighting_dirty = false;

  int i;
  csColor* colors = lit_mesh_colors;

  if (do_shadow_rec)
  {
    memcpy (colors, static_mesh_colors,
      num_lit_mesh_colors * sizeof (csColor));

    iSector* sect = movable->GetSectors ()->Get (0);
    if (sect)
    {
      csColor col;
      col = sect->GetDynamicAmbientLight ();
      if (col.red > EPSILON || col.green > EPSILON || col.blue > EPSILON)
        for (i = 0 ; i < factory->GetVertexCount () ; i++)
	  colors[i] += col;
    }
  }
  else
  {
    csColor col;
    // Set all colors to ambient light.
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
  }

  csReversibleTransform trans = movable->GetFullTransform ();
  csSet<iLight*>::GlobalIterator it = affecting_lights.GetIterator ();
  while (it.HasNext ())
  {
    iLight* l = (iLight*)it.Next ();
    UpdateLightingOne (trans, l);
  }
  csHash<csShadowArray*, iLight*>::GlobalIterator pdlIt =
    pseudoDynInfo.GetIterator ();
  while (pdlIt.HasNext ())
  {
    iLight* l;
    csShadowArray* shadowArr = pdlIt.Next (l);
    csColor c = l->GetColor ();
    if (c.red > EPSILON || c.green > EPSILON || c.blue > EPSILON)
    {
      c = c * (256. / CS_NORMAL_LIGHT_LEVEL);
      float* intensities = shadowArr->shadowmap;
      for (int i = 0; i < num_lit_mesh_colors; i++)
      {
        colors[i] += c * intensities[i];
      }
    }
  }

  // @@@ Try to avoid this loop!
  // Clamp all vertex colors to 2.
  for (i = 0 ; i < factory->GetVertexCount () ; i++)
    colors[i].Clamp (2., 2., 2.);

#ifdef CS_USE_NEW_RENDERER
  mesh_colors_dirty_flag = true;
#endif
}

void csGenmeshMeshObject::UpdateLighting (const csArray<iLight*>& lights,
    iMovable* movable)
{
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
  int num_lights = lights.Length ();
  for (l = 0 ; l < num_lights ; l++)
  {
    iLight* li = lights[l];
    li->AddAffectedLightingInfo (&scfiLightingInfo);
    affecting_lights.Add (li);
    UpdateLightingOne (trans, li);
  }

  // @@@ Try to avoid this loop!
  // Clamp all vertex colors to 2.
  for (i = 0 ; i < factory->GetVertexCount () ; i++)
    colors[i].Clamp (2., 2., 2.);

#ifdef CS_USE_NEW_RENDERER
  mesh_colors_dirty_flag = true;
#endif
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

#ifndef CS_USE_NEW_RENDERER
  iGraphics3D* g3d = rview->GetGraphics3D ();

  // Prepare for rendering.
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, mode);

  G3DTriangleMesh& m = factory->GetMesh ();
  iVertexBuffer* vbuf = factory->GetVertexBuffer ();
  iVertexBufferManager* vbufmgr = factory->GetVertexBufferManager ();
  m.mat_handle = mat;
  m.use_vertex_color = true;
  m.mixmode = MixMode;
  CS_ASSERT (!vbuf->IsLocked ());
  const csBox3& b = factory->GetObjectBoundingBox ();
  csColor* c = do_manual_colors ? factory->GetColors() : lit_mesh_colors;
  // @@@ FIXME: const_cast<> == BAD!
  vbufmgr->LockBuffer (vbuf,
    anim_ctrl_verts  ? CS_CONST_CAST(csVector3*,AnimControlGetVertices()) :
		       factory->GetVertices (),
    anim_ctrl_texels ? CS_CONST_CAST(csVector2*,AnimControlGetTexels()) :
		       factory->GetTexels (),
    anim_ctrl_colors ? CS_CONST_CAST(csColor*,AnimControlGetColors(c)) : c,
    factory->GetVertexCount (), 0, b);
  rview->CalculateFogMesh (g3d->GetObjectToCamera (), m);
  g3d->DrawTriangleMesh (m);
  vbufmgr->UnlockBuffer (vbuf);
#endif

  return true;
}

#ifdef CS_USE_NEW_RENDERER
/*iRenderBuffer *csGenmeshMeshObject::GetRenderBuffer (csStringID name)
{
  return factory->GetRenderBuffer (name);
}*/
#endif

csRenderMesh** csGenmeshMeshObject::GetRenderMeshes (
	int& n, iRenderView* rview, 
	iMovable* movable, uint32 frustum_mask)
{
#ifdef CS_USE_NEW_RENDERER
  SetupObject ();
  CheckLitColors ();

  n = 0;

  iCamera* camera = rview->GetCamera ();

  // First create the transformation from object to camera space directly:
  //   W = Mow * O - Vow;
  //   C = Mwc * (W - Vwc)
  // ->
  //   C = Mwc * (Mow * O - Vow - Vwc)
  //   C = Mwc * Mow * O - Mwc * (Vow + Vwc)
  csReversibleTransform tr_o2c;
  // Shouldn't this be done in the renderer?
  tr_o2c = camera->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movable->GetFullTransform ();

  int clip_portal, clip_plane, clip_z_plane;
  rview->CalculateClipSettings (frustum_mask, clip_portal, clip_plane,
      clip_z_plane);
  csVector3 camera_origin = tr_o2c.GetT2OTranslation ();

  lighting_movable = movable;

  if (!do_manual_colors && !do_shadow_rec && factory->light_mgr)
  {
    // Remember relevant lights for later.
    relevant_lights = factory->light_mgr->GetRelevantLights (
    	logparent, -1, false);
  }

  iMaterialWrapper* mater = material;
  if (!mater) mater = factory->GetMaterialWrapper ();
  if (!mater)
  {
    printf ("INTERNAL ERROR: mesh used without material!\n");
    return 0;
  }

  if (material_needs_visit) mater->Visit ();

  bool rmCreated;
  csRenderMesh*& meshPtr = rmHolder.GetUnusedMesh (rmCreated,
    rview->GetCurrentFrameNumber ());

  if (factory->back2front)
  {
    if (!sorted_index_buffer)
    {
      sorted_index_buffer = g3d->CreateIndexRenderBuffer (
      	sizeof (unsigned int)*factory->GetTriangleCount()*3,
	CS_BUF_DYNAMIC, CS_BUFCOMP_UNSIGNED_INT, 0, factory->GetVertexCount() - 1);
    }
    if (num_sorted_mesh_triangles != factory->GetTriangleCount ())
    {
      delete[] sorted_mesh_triangles;
      num_sorted_mesh_triangles = factory->GetTriangleCount ();
      sorted_mesh_triangles = new csTriangle [num_sorted_mesh_triangles];
    }

    csBSPTree* back2front_tree = factory->back2front_tree;
    if (!back2front_tree)
    {
      factory->BuildBack2FrontTree ();
      back2front_tree = factory->back2front_tree;
    }
    const csDirtyAccessArray<int>& triidx = back2front_tree->Back2Front (
    	tr_o2c.GetOrigin ());
    CS_ASSERT (triidx.Length () == (size_t)num_sorted_mesh_triangles);

    csTriangle* factory_triangles = factory->GetTriangles ();
    int i;
    for (i = 0 ; i < num_sorted_mesh_triangles ; i++)
      sorted_mesh_triangles[i] = factory_triangles[triidx[i]];
    sorted_index_buffer->CopyToBuffer (sorted_mesh_triangles,
    	sizeof (unsigned int)*num_sorted_mesh_triangles*3);

    csShaderVariable* sv = svcontext->GetVariableAdd(
    	csGenmeshMeshObjectFactory::index_name);
    sv->SetAccessor (0);
    sv->SetValue (sorted_index_buffer);
  }

  meshPtr->mixmode = MixMode;
  meshPtr->clip_portal = clip_portal;
  meshPtr->clip_plane = clip_plane;
  meshPtr->clip_z_plane = clip_z_plane;
  meshPtr->do_mirror = camera->IsMirrored ();
  meshPtr->meshtype = CS_MESHTYPE_TRIANGLES;
  meshPtr->indexstart = 0;
  meshPtr->indexend = factory->GetTriangleCount () * 3;
  meshPtr->material = mater;
  CS_ASSERT (mater != 0);
  meshPtr->object2camera = tr_o2c;
  meshPtr->camera_origin = camera_origin;
  meshPtr->camera_transform = &camera->GetTransform();
  if (rmCreated)
    meshPtr->variablecontext = svcontext;
  meshPtr->geometryInstance = (void*)factory;
 
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
  cent.Set (0.0f);
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
      if (pr) *pr = csQsqrt (csSquaredDist::PointPoint (start, isect) /
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
	if (polygon_idx) *polygon_idx = i;
      }
    }
  }
  if (pr) *pr = csQsqrt (dist * itot_dist);
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

#ifdef CS_USE_NEW_RENDERER
void csGenmeshMeshObject::PreGetShaderVariableValue (csShaderVariable* var)
{
  if (anim_ctrl)
  {
    // If we have an animation control then we must get the vertex data
    // here.
    int num_mesh_vertices = factory->GetVertexCount ();
    if (var->Name == csGenmeshMeshObjectFactory::vertex_name)
    {
      if (!vertex_buffer)
        vertex_buffer = g3d->CreateRenderBuffer (
            sizeof (csVector3)*num_mesh_vertices, CS_BUF_STATIC,
            CS_BUFCOMP_FLOAT, 3, false);
      const csVector3* mesh_vertices = AnimControlGetVertices ();
      if (!mesh_vertices) mesh_vertices = factory->GetVertices ();
      vertex_buffer->CopyToBuffer (
          mesh_vertices, sizeof(csVector3)*num_mesh_vertices);
      var->SetValue (vertex_buffer);
      return;
    }
    if (var->Name == csGenmeshMeshObjectFactory::texel_name)
    {
      if (!texel_buffer)
        texel_buffer = g3d->CreateRenderBuffer (
            sizeof (csVector2)*num_mesh_vertices, CS_BUF_STATIC,
            CS_BUFCOMP_FLOAT, 2, false);
      const csVector2* mesh_texels = AnimControlGetTexels ();
      if (!mesh_texels) mesh_texels = factory->GetTexels ();
      texel_buffer->CopyToBuffer (
          mesh_texels, sizeof (csVector2)*num_mesh_vertices);
      var->SetValue (texel_buffer);
      return;
    }
    if (var->Name == csGenmeshMeshObjectFactory::normal_name)
    {
      if (!normal_buffer)
        normal_buffer = g3d->CreateRenderBuffer (
            sizeof (csVector3)*num_mesh_vertices, CS_BUF_STATIC,
            CS_BUFCOMP_FLOAT, 3, false);
      const csVector3* mesh_normals = AnimControlGetNormals ();
      if (!mesh_normals) mesh_normals = factory->GetNormals ();
      normal_buffer->CopyToBuffer (
          mesh_normals, sizeof (csVector3)*num_mesh_vertices);
      var->SetValue (normal_buffer);
      return;
    }
  }

  if (var->Name == csGenmeshMeshObjectFactory::color_name)
  {
    if (!do_manual_colors && !do_shadow_rec && factory->light_mgr)
    {
      UpdateLighting (relevant_lights, lighting_movable);
    }
    else
    {
      UpdateLighting2 (lighting_movable);
    }
    if (mesh_colors_dirty_flag || anim_ctrl_colors)
    {
      if (!do_manual_colors)
      {
        if (!color_buffer ||
            (color_buffer->GetSize() != (sizeof (csColor) * 
	    num_lit_mesh_colors)))
        {
          // Recreate the render buffer only if the new data cannot fit inside
          //  the existing buffer.
          color_buffer = g3d->CreateRenderBuffer (
              sizeof (csColor) * num_lit_mesh_colors, 
	      do_lighting ? CS_BUF_DYNAMIC : CS_BUF_STATIC,
              CS_BUFCOMP_FLOAT, 3, false);
        }
        mesh_colors_dirty_flag = false;
        const csColor* mesh_colors = 0;
	if (anim_ctrl_colors)
	  mesh_colors = AnimControlGetColors (
		do_lighting ? lit_mesh_colors : static_mesh_colors);
        else
	  mesh_colors = do_lighting ? lit_mesh_colors : static_mesh_colors;
        color_buffer->CopyToBuffer (mesh_colors,
	    sizeof (csColor) * num_lit_mesh_colors);
      }
      else
      {
        if (!color_buffer || 
            (color_buffer->GetSize() != (sizeof (csColor) * 
	    factory->GetVertexCount())))
        {
          // Recreate the render buffer only if the new data cannot fit inside
          //  the existing buffer.
          color_buffer = g3d->CreateRenderBuffer (
              sizeof (csColor) * factory->GetVertexCount(), CS_BUF_STATIC,
              CS_BUFCOMP_FLOAT, 3, false);
        }
        mesh_colors_dirty_flag = false;
        const csColor* mesh_colors = 0;
	if (anim_ctrl_colors)
	  mesh_colors = AnimControlGetColors (factory->GetColors ());
	else
          mesh_colors = factory->GetColors ();
        color_buffer->CopyToBuffer (mesh_colors,
          sizeof (csColor) * factory->GetVertexCount());        
      }
    }
    var->SetValue(color_buffer);
    return;
  }
}
#endif

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csGenmeshMeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  {
    static scfInterfaceID iPolygonMesh_scfID = (scfInterfaceID)-1;
    if (iPolygonMesh_scfID == (scfInterfaceID)-1)
      iPolygonMesh_scfID = iSCF::SCF->GetInterfaceID ("iPolygonMesh");
    if (iInterfaceID == iPolygonMesh_scfID &&
      scfCompatibleVersion(iVersion, scfInterface<iPolygonMesh>::GetVersion()))
    {
      printf ("Deprecated feature use: iPolygonMesh queried from GenMesh "
    "factory; use iObjectModel->GetPolygonMeshColldet() instead.\n");
      iPolygonMesh* Object = scfiObjectModel.GetPolygonMeshColldet();
      (Object)->IncRef ();
      return CS_STATIC_CAST(iPolygonMesh*, Object);
    }
  }
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iGeneralFactoryState)
#ifndef CS_USE_NEW_RENDERER
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iVertexBufferManagerClient)
#endif
SCF_IMPLEMENT_IBASE_END

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

#ifdef CS_USE_NEW_RENDERER
SCF_IMPLEMENT_IBASE (csGenmeshMeshObjectFactory::eiShaderVariableAccessor)
  SCF_IMPLEMENTS_INTERFACE (iShaderVariableAccessor)
SCF_IMPLEMENT_IBASE_END
#endif

csStringID csGenmeshMeshObjectFactory::vertex_name = csInvalidStringID;
csStringID csGenmeshMeshObjectFactory::texel_name = csInvalidStringID;
csStringID csGenmeshMeshObjectFactory::normal_name = csInvalidStringID;
csStringID csGenmeshMeshObjectFactory::color_name = csInvalidStringID;
csStringID csGenmeshMeshObjectFactory::index_name = csInvalidStringID;
csStringID csGenmeshMeshObjectFactory::tangent_name = csInvalidStringID;
csStringID csGenmeshMeshObjectFactory::binormal_name = csInvalidStringID;

csGenmeshMeshObjectFactory::csGenmeshMeshObjectFactory (iMeshObjectType *pParent,
      iObjectRegistry* object_reg)
#ifdef CS_USE_NEW_RENDERER
     : anon_buffers(object_reg)
#endif
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiGeneralFactoryState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
#ifndef CS_USE_NEW_RENDERER
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiVertexBufferManagerClient);
#endif
#ifdef CS_USE_NEW_RENDERER
  scfiShaderVariableAccessor = new eiShaderVariableAccessor (this);
#endif
  csGenmeshMeshObjectFactory::object_reg = object_reg;

  scfiPolygonMesh.SetFactory (this);
  scfiObjectModel.SetPolygonMeshBase (&scfiPolygonMesh);
  scfiObjectModel.SetPolygonMeshColldet (&scfiPolygonMesh);
  scfiObjectModel.SetPolygonMeshViscull (0);
  scfiObjectModel.SetPolygonMeshShadows (&scfiPolygonMesh);

  logparent = 0;
  genmesh_type = pParent;
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
  light_mgr = CS_QUERY_REGISTRY (object_reg, iLightManager);
  back2front = false;
  back2front_tree = 0;

#ifdef CS_USE_NEW_RENDERER
  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg,
    "crystalspace.shared.stringset", iStringSet);

  if ((vertex_name == csInvalidStringID) ||
    (texel_name == csInvalidStringID) ||
    (normal_name == csInvalidStringID) ||
    (color_name == csInvalidStringID) ||
    (index_name == csInvalidStringID) ||
    (tangent_name == csInvalidStringID) ||
    (binormal_name == csInvalidStringID))
  {
    vertex_name = strings->Request ("vertices");
    texel_name = strings->Request ("texture coordinates");
    normal_name = strings->Request ("normals");
    color_name = strings->Request ("colors");
    index_name = strings->Request ("indices");
    tangent_name = strings->Request ("tangents");
    binormal_name = strings->Request ("binormals");
  }

  mesh_vertices_dirty_flag = false;
  mesh_texels_dirty_flag = false;
  mesh_normals_dirty_flag = false;
  mesh_colors_dirty_flag = false;
  mesh_triangle_dirty_flag = false;
  mesh_tangents_dirty_flag = false;

  buffers_version = 0;
#endif

  autonormals = false;

  default_mixmode = 0;
  default_lighting = true;
  default_color.Set (0, 0, 0);
  default_manualcolors = false;
  default_shadowcasting = true;
  default_shadowreceiving = false;

  csRef<iEngine> eng = CS_QUERY_REGISTRY (object_reg, iEngine);
  engine = eng; // We don't want a circular reference!

  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
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

  delete back2front_tree;

#ifdef CS_USE_NEW_RENDERER
  scfiShaderVariableAccessor->DecRef ();
#endif
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiGeneralFactoryState);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel);
#ifndef CS_USE_NEW_RENDERER
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiVertexBufferManagerClient);
#endif
  SCF_DESTRUCT_IBASE ();
}

void csGenmeshMeshObjectFactory::SetAnimationControlFactory (
	iGenMeshAnimationControlFactory* ac)
{
  anim_ctrl_fact = ac;
}

void csGenmeshMeshObjectFactory::SetBack2Front (bool b2f)
{
  delete back2front_tree;
  back2front_tree = 0;
  back2front = b2f;
}

void csGenmeshMeshObjectFactory::BuildBack2FrontTree ()
{
  if (back2front_tree) return;
  back2front_tree = new csBSPTree ();
  back2front_tree->Build (GetTriangles (), GetTriangleCount (),
  	GetVertices ());
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
  radius.Set (csQsqrt (max_sq_radius.x),
    csQsqrt (max_sq_radius.y), csQsqrt (max_sq_radius.z));
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

/*
bool csGenmeshMeshObjectFactory::UpdateRenderBuffers ()
{
  bool changed = false;
  if (mesh_vertices_dirty_flag)
  {
    vertex_buffer = g3d->CreateRenderBuffer (
      sizeof (csVector3)*num_mesh_vertices, CS_BUF_STATIC,
      CS_BUFCOMP_FLOAT, 3, false);
    mesh_vertices_dirty_flag = false;
    csVector3* vbuf = (csVector3*)vertex_buffer->Lock(CS_BUF_LOCK_NORMAL);
    memcpy (vbuf, mesh_vertices, sizeof(csVector3)*num_mesh_vertices);
    vertex_buffer->Release ();
    changed = true;
  }
  if (mesh_texels_dirty_flag)
  {
    texel_buffer = g3d->CreateRenderBuffer (
      sizeof (csVector2)*num_mesh_vertices, CS_BUF_STATIC,
      CS_BUFCOMP_FLOAT, 2, false);
    mesh_texels_dirty_flag = false;
    csVector2* tbuf = (csVector2*)texel_buffer->Lock (CS_BUF_LOCK_NORMAL);
    memcpy (tbuf, mesh_texels, sizeof (csVector2) * num_mesh_vertices);
    texel_buffer->Release ();
    changed = true;
  }
  if (mesh_normals_dirty_flag)
  {
    normal_buffer = g3d->CreateRenderBuffer (
      sizeof (csVector3)*num_mesh_vertices, CS_BUF_STATIC,
      CS_BUFCOMP_FLOAT, 3, false);
    mesh_normals_dirty_flag = false;
    csVector3 *nbuf = (csVector3*)normal_buffer->Lock(CS_BUF_LOCK_NORMAL);
    memcpy (nbuf, mesh_normals, sizeof (csVector3)*num_mesh_vertices);
    normal_buffer->Release();
    changed = true;
  }
  if (mesh_colors_dirty_flag)
  {
    color_buffer = g3d->CreateRenderBuffer (
      sizeof (csColor)*num_mesh_vertices, CS_BUF_STATIC,
      CS_BUFCOMP_FLOAT, 3, false);
    mesh_colors_dirty_flag = false;
    csColor *cbuf = (csColor*)color_buffer->Lock(CS_BUF_LOCK_NORMAL);
    memcpy (cbuf, mesh_colors, sizeof (csColor) * num_mesh_vertices);
    color_buffer->Release();
    changed = true;
  }
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
    changed = true;
  }
  if (changed) buffers_version++;
  return changed;
}

iRenderBuffer *csGenmeshMeshObjectFactory::GetRenderBuffer (csStringID name)
{
  UpdateRenderBuffers ();
  if (name == vertex_name)
  {
    return vertex_buffer;
  }
  if (name == texel_name)
  {
    return texel_buffer;
  }
  if (name == normal_name)
  {
    return normal_buffer;
  }
  if (name == color_name)
  {
    return color_buffer;
  }
  if (name == index_name)
  {
    return index_buffer;
  }
  return anon_buffers.GetRenderBuffer(name);
}
*/

#ifdef CS_USE_NEW_RENDERER
void csGenmeshMeshObjectFactory::PreGetShaderVariableValue (
  csShaderVariable* var)
{
  if (var->Name == vertex_name)
  {
    if (mesh_vertices_dirty_flag)
    {
      if (!vertex_buffer)
        vertex_buffer = g3d->CreateRenderBuffer (
          sizeof (csVector3)*num_mesh_vertices, CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3, false);
      mesh_vertices_dirty_flag = false;
      vertex_buffer->CopyToBuffer (
        mesh_vertices, sizeof(csVector3)*num_mesh_vertices);
    }
    var->SetValue(vertex_buffer);
    return;
  }
  if (var->Name == texel_name)
  {
    if (mesh_texels_dirty_flag)
    {
      if (!texel_buffer)
        texel_buffer = g3d->CreateRenderBuffer (
          sizeof (csVector2)*num_mesh_vertices, CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 2, false);
      mesh_texels_dirty_flag = false;
      texel_buffer->CopyToBuffer (
        mesh_texels, sizeof (csVector2)*num_mesh_vertices);
    }
    var->SetValue(texel_buffer);
    return;
  }
  if (var->Name == normal_name)
  {
    if (mesh_normals_dirty_flag)
    {
      if (!normal_buffer)
        normal_buffer = g3d->CreateRenderBuffer (
          sizeof (csVector3)*num_mesh_vertices, CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3, false);
      mesh_normals_dirty_flag = false;
      normal_buffer->CopyToBuffer (
        mesh_normals, sizeof (csVector3)*num_mesh_vertices);
    }
    var->SetValue(normal_buffer);
    return;
  }
  if ((var->Name == tangent_name) || (var->Name == binormal_name))
  {
    if (mesh_tangents_dirty_flag)
    {
      if (!tangent_buffer)
        tangent_buffer = g3d->CreateRenderBuffer (
          sizeof (csVector3)*num_mesh_vertices, CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3);
      if (!binormal_buffer)
        binormal_buffer = g3d->CreateRenderBuffer (
          sizeof (csVector3)*num_mesh_vertices, CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3);
      mesh_tangents_dirty_flag = false;

      csVector3* tangentData = new csVector3[num_mesh_vertices * 2];
      csVector3* bitangentData = tangentData + num_mesh_vertices;
      csNormalMappingTools::CalculateTangents (num_mesh_triangles, 
	mesh_triangles, num_mesh_vertices, mesh_vertices, mesh_normals, 
	mesh_texels, tangentData, bitangentData);

      tangent_buffer->CopyToBuffer (tangentData, 
	sizeof (csVector3) * num_mesh_vertices);
      binormal_buffer->CopyToBuffer (bitangentData, 
	sizeof (csVector3) * num_mesh_vertices);

      delete[] tangentData;
    }
    var->SetValue((var->Name == tangent_name) ? tangent_buffer : 
      binormal_buffer);
    return;
  }
  /*if (var->Name == color_name)
  {
    if (mesh_colors_dirty_flag)
    {
      if (!color_buffer)
        color_buffer = g3d->CreateRenderBuffer (
          sizeof (csColor)*num_mesh_vertices, CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3, false);
      mesh_colors_dirty_flag = false;
      color_buffer->CopyToBuffer (
        mesh_colors, sizeof (csColor) * num_mesh_vertices);
    }
    var->SetValue(color_buffer);
    return;
  }*/
  if (var->Name == index_name)
  {
    if (mesh_triangle_dirty_flag)
    {
      if (!index_buffer)
	index_buffer = g3d->CreateIndexRenderBuffer (
          sizeof (unsigned int)*num_mesh_triangles*3, CS_BUF_STATIC,
          CS_BUFCOMP_UNSIGNED_INT, 0, num_mesh_vertices - 1);
      mesh_triangle_dirty_flag = false;
      index_buffer->CopyToBuffer (
        //ibuf, sizeof (unsigned int)*num_mesh_triangles*3);
        mesh_triangles, sizeof (unsigned int)*num_mesh_triangles*3);
    }
    var->SetValue(index_buffer);
    return;
  }
  iRenderBuffer *a = anon_buffers.GetRenderBuffer (var->Name);
  if (a!=0)
  {
    var->SetValue(a);
    return;
  }
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
#ifdef CS_USE_NEW_RENDERER
  vertex_buffer = 0;
  normal_buffer = 0;
  texel_buffer = 0;
  color_buffer = 0;
  mesh_vertices_dirty_flag = true;
  mesh_texels_dirty_flag = true;
  mesh_normals_dirty_flag = true;
  mesh_colors_dirty_flag = true;
  mesh_tangents_dirty_flag = true;
#else
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
    memcpy (new_triangles, top_mesh.triangles,
      sizeof (csTriangle) * MIN (n, top_mesh.num_triangles));
  }
  top_mesh.num_triangles = n;
  delete[] top_mesh.triangles;
  top_mesh.triangles = new_triangles;
#else
  if (mesh_triangles)
  {
    memcpy (new_triangles, mesh_triangles,
      sizeof (csTriangle) * MIN (n, num_mesh_triangles));
  }
  num_mesh_triangles = n;
  delete [] mesh_triangles;
  mesh_triangles = new_triangles;

  index_buffer = 0;
  mesh_triangle_dirty_flag = true;
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
  int i;
  size_t j;

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
  autonormals = true;

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
    SetVertexCount(24);
    csVector3* vertices = GetVertices();
    vertices[0].Set(box.MinX(), box.MaxY(), box.MinZ());
    vertices[1].Set(box.MinX(), box.MaxY(), box.MinZ());
    vertices[2].Set(box.MinX(), box.MaxY(), box.MinZ());

    vertices[3].Set(box.MinX(), box.MaxY(), box.MaxZ());
    vertices[4].Set(box.MinX(), box.MaxY(), box.MaxZ());
    vertices[5].Set(box.MinX(), box.MaxY(), box.MaxZ());

    vertices[6].Set(box.MaxX(), box.MaxY(), box.MaxZ());
    vertices[7].Set(box.MaxX(), box.MaxY(), box.MaxZ());
    vertices[8].Set(box.MaxX(), box.MaxY(), box.MaxZ());

    vertices[9].Set(box.MaxX(), box.MaxY(), box.MinZ());
    vertices[10].Set(box.MaxX(), box.MaxY(), box.MinZ());
    vertices[11].Set(box.MaxX(), box.MaxY(), box.MinZ());

    vertices[12].Set(box.MinX(), box.MinY(), box.MaxZ());
    vertices[13].Set(box.MinX(), box.MinY(), box.MaxZ());
    vertices[14].Set(box.MinX(), box.MinY(), box.MaxZ());

    vertices[15].Set(box.MaxX(), box.MinY(), box.MaxZ());
    vertices[16].Set(box.MaxX(), box.MinY(), box.MaxZ());
    vertices[17].Set(box.MaxX(), box.MinY(), box.MaxZ());

    vertices[18].Set(box.MaxX(), box.MinY(), box.MinZ());
    vertices[19].Set(box.MaxX(), box.MinY(), box.MinZ());
    vertices[20].Set(box.MaxX(), box.MinY(), box.MinZ());

    vertices[21].Set(box.MinX(), box.MinY(), box.MinZ());
    vertices[22].Set(box.MinX(), box.MinY(), box.MinZ());
    vertices[23].Set(box.MinX(), box.MinY(), box.MinZ());

    csVector2* texels = GetTexels();
    // the comments indicate which face
    // (numbered 1-6) the texel applies to
    texels[0].Set(0, 0); // 1
    texels[1].Set(0, 1); // 2
    texels[2].Set(1, 0); // 3

    texels[3].Set(0, 0); // 2
    texels[4].Set(0, 0); // 3
    texels[5].Set(1, 0); // 4

    texels[6].Set(1, 0); // 2
    texels[7].Set(0, 0); // 4
    texels[8].Set(1, 0); // 5

    texels[9].Set(1, 0); // 1
    texels[10].Set(1, 1); // 2
    texels[11].Set(0, 0); // 5

    texels[12].Set(0, 1); // 3
    texels[13].Set(1, 1); // 4
    texels[14].Set(1, 1); // 6

    texels[15].Set(0, 1); // 4
    texels[16].Set(1, 1); // 5
    texels[17].Set(1, 0); // 6

    texels[18].Set(1, 1); // 1
    texels[19].Set(0, 1); // 5
    texels[20].Set(0, 0); // 6

    texels[21].Set(0, 1); // 1
    texels[22].Set(1, 1); // 3
    texels[23].Set(0, 1); // 6

    SetTriangleCount(12);
    csTriangle* triangles = GetTriangles();
    triangles[0].a = 0; triangles[0].b = 9; triangles[0].c = 18;
    triangles[1].a = 0; triangles[1].b = 18; triangles[1].c = 21;

    triangles[2].a = 3; triangles[2].b = 6; triangles[2].c = 10;
    triangles[3].a = 3; triangles[3].b = 10; triangles[3].c = 1;

    triangles[4].a = 4; triangles[4].b = 2; triangles[4].c = 22;
    triangles[5].a = 4; triangles[5].b = 22; triangles[5].c = 12;

    triangles[6].a = 7; triangles[6].b = 5; triangles[6].c = 13;
    triangles[7].a = 7; triangles[7].b = 13; triangles[7].c = 15;

    triangles[8].a = 11; triangles[8].b = 8; triangles[8].c = 16;
    triangles[9].a = 11; triangles[9].b = 16; triangles[9].c = 19;

    triangles[10].a = 23; triangles[10].b = 20; triangles[10].c = 17;
    triangles[11].a = 23; triangles[11].b = 17; triangles[11].c = 14;

    csVector3* normals = GetNormals();
    normals[0].Set(box.MinX(),box.MaxY(),box.MinZ()); normals[0].Normalize();
    normals[1].Set(box.MinX(),box.MaxY(),box.MinZ()); normals[1].Normalize();
    normals[2].Set(box.MinX(),box.MaxY(),box.MinZ()); normals[2].Normalize();

    normals[3].Set(box.MinX(),box.MaxY(),box.MaxZ()); normals[3].Normalize();
    normals[4].Set(box.MinX(),box.MaxY(),box.MaxZ()); normals[4].Normalize();
    normals[5].Set(box.MinX(),box.MaxY(),box.MaxZ()); normals[5].Normalize();

    normals[6].Set(box.MaxX(),box.MaxY(),box.MaxZ()); normals[6].Normalize();
    normals[7].Set(box.MaxX(),box.MaxY(),box.MaxZ()); normals[7].Normalize();
    normals[8].Set(box.MaxX(),box.MaxY(),box.MaxZ()); normals[8].Normalize();

    normals[9].Set(box.MaxX(),box.MaxY(),box.MinZ()); normals[9].Normalize();
    normals[10].Set(box.MaxX(),box.MaxY(),box.MinZ()); normals[10].Normalize();
    normals[11].Set(box.MaxX(),box.MaxY(),box.MinZ()); normals[11].Normalize();

    normals[12].Set(box.MinX(),box.MinY(),box.MaxZ()); normals[12].Normalize();
    normals[13].Set(box.MinX(),box.MinY(),box.MaxZ()); normals[13].Normalize();
    normals[14].Set(box.MinX(),box.MinY(),box.MaxZ()); normals[14].Normalize();

    normals[15].Set(box.MaxX(),box.MinY(),box.MaxZ()); normals[15].Normalize();
    normals[16].Set(box.MaxX(),box.MinY(),box.MaxZ()); normals[16].Normalize();
    normals[17].Set(box.MaxX(),box.MinY(),box.MaxZ()); normals[17].Normalize();

    normals[18].Set(box.MaxX(),box.MinY(),box.MinZ()); normals[18].Normalize();
    normals[19].Set(box.MaxX(),box.MinY(),box.MinZ()); normals[19].Normalize();
    normals[20].Set(box.MaxX(),box.MinY(),box.MinZ()); normals[20].Normalize();

    normals[21].Set(box.MinX(),box.MinY(),box.MinZ()); normals[21].Normalize();
    normals[22].Set(box.MinX(),box.MinY(),box.MinZ()); normals[22].Normalize();
    normals[23].Set(box.MinX(),box.MinY(),box.MinZ()); normals[23].Normalize();

    Invalidate();

#if 0
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
#endif
}

bool csGenmeshMeshObjectFactory::AddRenderBuffer (const char *name,
  csRenderBufferComponentType component_type, int component_size)
{
#ifdef CS_USE_NEW_RENDERER
  anon_buffer_names.Push (strings->Request(name));
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
  mesh_tangents_dirty_flag = true;
#endif

  scfiObjectModel.ShapeChanged ();
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
  //CalculateNormals ();
}

csPtr<iMeshObject> csGenmeshMeshObjectFactory::NewInstance ()
{
  csGenmeshMeshObject* cm = new csGenmeshMeshObject (this);
  cm->SetMixMode (default_mixmode);
  cm->SetLighting (default_lighting);
  cm->SetColor (default_color);
  cm->SetManualColors (default_manualcolors);
  cm->SetShadowCasting (default_shadowcasting);
  cm->SetShadowReceiving (default_shadowreceiving);

  if (anim_ctrl_fact)
  {
    csRef<iGenMeshAnimationControl> anim_ctrl = anim_ctrl_fact
    	->CreateAnimationControl ();
    cm->SetAnimationControl (anim_ctrl);
  }

  csRef<iMeshObject> im (SCF_QUERY_INTERFACE (cm, iMeshObject));
  cm->DecRef ();
  return csPtr<iMeshObject> (im);
}

#ifndef CS_USE_NEW_RENDERER
void csGenmeshMeshObjectFactory::eiVertexBufferManagerClient::ManagerClosing ()
{
  scfParent->vbuf = 0;
  scfParent->vbufmgr = 0;
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
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
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
