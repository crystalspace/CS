/*
    Copyright (C) 2005 by Jorrit Tyberghein

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
#include "csqint.h"
#include "csqsqrt.h"

#include "csgeom/box.h"
#include "csgeom/bsptree.h"
#include "csgeom/frustum.h"
#include "csgeom/math.h"
#include "csgeom/math3d.h"
#include "csgeom/sphere.h"
#include "csgeom/trimesh.h"
#include "csgfx/normalmaptools.h"
#include "csgfx/renderbuffer.h"
#include "csutil/csendian.h"
#include "csutil/csmd5.h"
#include "csutil/memfile.h"
#include "csutil/sysfunc.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "iengine/shadows.h"
#include "igeom/clip2d.h"
#include "iutil/cache.h"
#include "iutil/databuff.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/strset.h"
#include "iutil/verbositymanager.h"
#include "iutil/cmdline.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/rendermesh.h"
#include "cstool/vertexcompress.h"

#include "instmesh.h"



CS_LEAKGUARD_IMPLEMENT (csInstmeshMeshObject);
CS_LEAKGUARD_IMPLEMENT (csInstmeshMeshObjectFactory);

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csInstmeshMeshObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iShadowCaster)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iShadowReceiver)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iInstancingMeshState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iLightingInfo)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  {
    static scfInterfaceID iPolygonMesh_scfID = (scfInterfaceID)-1;
    if (iPolygonMesh_scfID == (scfInterfaceID)-1)
      iPolygonMesh_scfID = iSCF::SCF->GetInterfaceID ("iPolygonMesh");
    if (iInterfaceID == iPolygonMesh_scfID &&
      scfCompatibleVersion(iVersion, scfInterfaceTraits<iPolygonMesh>::GetVersion()))
    {
#ifdef CS_DEBUG
      csPrintf ("Deprecated feature use: iPolygonMesh queried from Genmesh "
    "object; use iMeshObject->GetObjectModel()->"
    "GetPolygonMeshColldet() instead.\n");
#endif
      (&scfiPolygonMesh)->IncRef ();
      return CS_STATIC_CAST(iPolygonMesh*, &scfiPolygonMesh);
    }
  }
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csInstmeshMeshObject::ShadowCaster)
  SCF_IMPLEMENTS_INTERFACE (iShadowCaster)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csInstmeshMeshObject::ShadowReceiver)
  SCF_IMPLEMENTS_INTERFACE (iShadowReceiver)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csInstmeshMeshObject::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csInstmeshMeshObject::PolyMesh)
  SCF_IMPLEMENTS_INTERFACE (iPolygonMesh)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csInstmeshMeshObject::InstancingMeshState)
  SCF_IMPLEMENTS_INTERFACE (iInstancingMeshState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csInstmeshMeshObject::LightingInfo)
  SCF_IMPLEMENTS_INTERFACE (iLightingInfo)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csInstmeshMeshObject::eiRenderBufferAccessor)
  SCF_IMPLEMENTS_INTERFACE (iRenderBufferAccessor)
SCF_IMPLEMENT_IBASE_END

csInstmeshMeshObject::csInstmeshMeshObject (csInstmeshMeshObjectFactory* factory) :
	pseudoDynInfo (29, 32),
	affecting_lights (29, 32)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiInstancingMeshState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiShadowCaster);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiShadowReceiver);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLightingInfo);

  scfiRenderBufferAccessor = new eiRenderBufferAccessor (this);
  csInstmeshMeshObject::factory = factory;
  vc = factory->vc;
  logparent = 0;
  initialized = false;
  cur_movablenr = -1;
  material = 0;
  MixMode = 0;
  vis_cb = 0;
  lit_fact_colors = 0;
  num_lit_fact_colors = 0;
  static_fact_colors = 0;
  do_lighting = true;
  do_manual_colors = false;
  base_color.red = 0;
  base_color.green = 0;
  base_color.blue = 0;
  current_lod = 1;
  current_features = 0;
  do_shadows = true;
  do_shadow_rec = false;
  lighting_dirty = true;
  shadow_caps = false;

  dynamic_ambient_version = 0;

  bufferHolder.AttachNew (new csRenderBufferHolder);

  g3d = CS_QUERY_REGISTRY (factory->object_reg, iGraphics3D);
  mesh_vertices_dirty_flag = true;
  mesh_texels_dirty_flag = true;
  mesh_normals_dirty_flag = true;
  mesh_colors_dirty_flag = true;
  mesh_triangle_dirty_flag = true;
  mesh_tangents_dirty_flag = true;

  object_bbox_valid = false;
}

csInstmeshMeshObject::~csInstmeshMeshObject ()
{
  if (vis_cb) vis_cb->DecRef ();
  delete[] lit_fact_colors;
  delete[] static_fact_colors;

  ClearPseudoDynLights ();

  scfiRenderBufferAccessor->DecRef ();
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiInstancingMeshState);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiShadowCaster);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiShadowReceiver);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiLightingInfo);
  SCF_DESTRUCT_IBASE ();
}

size_t csInstmeshMeshObject::max_instance_id = 0;

void csInstmeshMeshObject::CalculateInstanceArrays ()
{
  mesh_vertices_dirty_flag = true;
  mesh_texels_dirty_flag = true;
  mesh_normals_dirty_flag = true;
  mesh_colors_dirty_flag = true;
  mesh_triangle_dirty_flag = true;
  mesh_tangents_dirty_flag = true;

  vertex_buffer = 0;
  texel_buffer = 0;
  normal_buffer = 0;
  color_buffer = 0;
  index_buffer = 0;
  binormal_buffer = 0;
  tangent_buffer = 0;

  object_bbox_valid = false; // @@@ Think again? Do while managing instances!

  size_t fact_vt_len = factory->fact_vertices.Length ();
  size_t vt_len = fact_vt_len * instances.Length ();
  csVector3* fact_vertices = factory->fact_vertices.GetArray ();
  mesh_vertices.SetMinimalCapacity (vt_len);
  mesh_vertices.SetLength (0);
  csVector2* fact_texels = factory->fact_texels.GetArray ();
  mesh_texels.SetMinimalCapacity (vt_len);
  mesh_texels.SetLength (0);
  csVector3* fact_normals = factory->fact_normals.GetArray ();
  mesh_normals.SetMinimalCapacity (vt_len);
  mesh_normals.SetLength (0);
  csColor4* fact_colors = factory->fact_colors.GetArray ();
  mesh_colors.SetMinimalCapacity (vt_len);
  mesh_colors.SetLength (0);

  size_t fact_tri_len = factory->fact_triangles.Length ();
  size_t tri_len = fact_tri_len * instances.Length ();
  csTriangle* fact_triangles = factory->fact_triangles.GetArray ();
  mesh_triangles.SetMinimalCapacity (tri_len);
  mesh_triangles.SetLength (0);

  size_t i, idx;
  for (i = 0 ; i < instances.Length () ; i++)
  {
    // @@@ Do more optimal with array copy for texels and colors?
    const csReversibleTransform& tr = instances[i].transform;
    for (idx = 0 ; idx < fact_vt_len ; idx++)
    {
      mesh_vertices.Push (tr.Other2This (fact_vertices[idx]));
      mesh_texels.Push (fact_texels[idx]);
      mesh_normals.Push (tr.Other2ThisRelative (fact_normals[idx]));
      mesh_colors.Push (fact_colors[idx]);
    }
    size_t mult = i * fact_vt_len;
    for (idx = 0 ; idx < fact_tri_len ; idx++)
    {
      csTriangle tri = fact_triangles[idx];
      tri.a += mult;
      tri.b += mult;
      tri.c += mult;
      mesh_triangles.Push (tri);
    }
  }
}

size_t csInstmeshMeshObject::AddInstance (const csReversibleTransform& trans)
{
  csInstance inst;
  inst.transform = trans;
  ++max_instance_id;
  inst.id = max_instance_id;
  instances.Push (inst);
  initialized = false;
  return max_instance_id;
}

void csInstmeshMeshObject::RemoveInstance (size_t id)
{
  size_t i;
  for (i = 0 ; i < instances.Length () ; i++)
    if (instances[i].id == id)
    {
      instances.DeleteIndexFast (i);
      initialized = false;
      return;
    }
}

void csInstmeshMeshObject::RemoveAllInstances ()
{
  instances.Empty ();
  initialized = false;
}

void csInstmeshMeshObject::MoveInstance (size_t id,
    const csReversibleTransform& trans)
{
  // @@@ Not fast? Avoid loop somehow?
  size_t i;
  for (i = 0 ; i < instances.Length () ; i++)
    if (instances[i].id == id)
    {
      instances[i].transform = trans;
      // @@@ Do in a more optimal way! Don't set everything dirty!
      initialized = false;
      return;
    }
}

const csReversibleTransform& csInstmeshMeshObject::GetInstanceTransform (
    size_t id)
{
  // @@@ Not fast? Avoid loop somehow?
  size_t i;
  for (i = 0 ; i < instances.Length () ; i++)
    if (instances[i].id == id)
      return instances[i].transform;
  static csReversibleTransform dummy;
  return dummy;
}

void csInstmeshMeshObject::ClearPseudoDynLights ()
{
  csHash<csShadowArray*, csPtrKey<iLight> >::GlobalIterator it (
    pseudoDynInfo.GetIterator ());
  while (it.HasNext ())
  {
    csShadowArray* arr = it.Next ();
    delete arr;
  }
}

void csInstmeshMeshObject::CheckLitColors ()
{
  if (do_manual_colors) return;
  int numcol = factory->GetVertexCount () * instances.Length ();
  if (numcol != num_lit_fact_colors)
  {
    ClearPseudoDynLights ();

    num_lit_fact_colors = numcol;
    delete[] lit_fact_colors;
    lit_fact_colors = new csColor4 [num_lit_fact_colors];
    delete[] static_fact_colors;
    static_fact_colors = new csColor4 [num_lit_fact_colors];
  }
}

void csInstmeshMeshObject::InitializeDefault (bool clear)
{
  SetupObject ();

  if (!do_shadow_rec) return;
  if (do_manual_colors) return;

  // Set all colors to ambient light.
  int i;
  CheckLitColors ();
  if (clear)
  {
    //csColor amb;
    //factory->engine->GetAmbientLight (amb);
    for (i = 0 ; i < num_lit_fact_colors ; i++)
    {
      lit_fact_colors[i].Set (0, 0, 0);
      static_fact_colors[i].Set (0, 0, 0);
    }
  }
  lighting_dirty = true;
}

void csInstmeshMeshObject::CalculateBBoxRadius ()
{
  object_bbox_valid = true;
  if (mesh_vertices.Length () == 0)
  {
    object_bbox.Set (0, 0, 0, 0, 0, 0);
    radius.Set (0, 0, 0);
    return;
  }
  csVector3& v0 = mesh_vertices[0];
  object_bbox.StartBoundingBox (v0);
  csVector3 max_sq_radius (v0.x*v0.x + v0.x*v0.x,
        v0.y*v0.y + v0.y*v0.y, v0.z*v0.z + v0.z*v0.z);
  size_t i;
  for (i = 1 ; i < mesh_vertices.Length () ; i++)
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

const csVector3& csInstmeshMeshObject::GetRadius ()
{
  SetupObject ();
  if (!object_bbox_valid) CalculateBBoxRadius ();
  return radius;
}

const csBox3& csInstmeshMeshObject::GetObjectBoundingBox ()
{
  SetupObject ();
  if (!object_bbox_valid) CalculateBBoxRadius ();
  return object_bbox;
}

void csInstmeshMeshObject::SetObjectBoundingBox (const csBox3& bbox)
{
  SetupObject ();
  object_bbox_valid = true;
  object_bbox = bbox;
}

char* csInstmeshMeshObject::GenerateCacheName ()
{
  csMemFile mf;
  mf.Write ("instmesh", 8);
  uint32 l;
  l = csConvertEndian ((uint32)factory->GetVertexCount ());
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((uint32)factory->GetTriangleCount ());
  mf.Write ((char*)&l, 4);

  if (logparent)
  {
    if (logparent->QueryObject ()->GetName ())
      mf.Write (logparent->QueryObject ()->GetName (),
        strlen (logparent->QueryObject ()->GetName ()));
    iMovable* movable = logparent->GetMovable ();
    iSector* sect = movable->GetSectors ()->Get (0);
    if (sect && sect->QueryObject ()->GetName ())
      mf.Write (sect->QueryObject ()->GetName (),
        strlen (sect->QueryObject ()->GetName ()));
  }

  csMD5::Digest digest = csMD5::Encode (mf.GetData (), mf.GetSize ());
  csString hex(digest.HexString());
  return hex.Detach();
}

const char CachedLightingMagic[] = "GmL1";
const int CachedLightingMagicSize = sizeof (CachedLightingMagic);

bool csInstmeshMeshObject::ReadFromCache (iCacheManager* cache_mgr)
{
  if (!do_shadow_rec) return true;
  SetupObject ();
  lighting_dirty = true;
  char* cachename = GenerateCacheName ();
  cache_mgr->SetCurrentScope (cachename);
  delete[] cachename;

  bool rc = false;
  csRef<iDataBuffer> db = cache_mgr->ReadCache ("genmesh_lm", 0, (uint32)~0);
  if (db)
  {
    csMemFile mf ((const char*)(db->GetData ()), db->GetSize ());
    char magic[CachedLightingMagicSize];
    if (mf.Read (magic, CachedLightingMagicSize - 1) != 4) goto stop;
    magic[CachedLightingMagicSize - 1] = 0;
    if (strcmp (magic, CachedLightingMagic) == 0)
    {
      int v;
      for (v = 0; v < num_lit_fact_colors; v++)
      {
	csColor4& c = static_fact_colors[v];
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
	float* intensities = new float[num_lit_fact_colors];
	shadowArr->shadowmap = intensities;
	for (int n = 0; n < num_lit_fact_colors; n++)
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

bool csInstmeshMeshObject::WriteToCache (iCacheManager* cache_mgr)
{
  if (!do_shadow_rec) return true;
  char* cachename = GenerateCacheName ();
  cache_mgr->SetCurrentScope (cachename);
  delete[] cachename;

  bool rc = false;
  csMemFile mf;
  mf.Write (CachedLightingMagic, CachedLightingMagicSize - 1);
  for (int v = 0; v < num_lit_fact_colors; v++)
  {
    const csColor4& c = static_fact_colors[v];
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

  csHash<csShadowArray*, csPtrKey<iLight> >::GlobalIterator pdlIt (
    pseudoDynInfo.GetIterator ());
  while (pdlIt.HasNext ())
  {
    mf.Write ((char*)&c, sizeof (c));

    csPtrKey<iLight> l;
    csShadowArray* shadowArr = pdlIt.Next (l);
    const char* lid = l->GetLightID ();
    mf.Write ((char*)lid, 16);

    float* intensities = shadowArr->shadowmap;
    for (int n = 0; n < num_lit_fact_colors; n++)
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
    "genmesh_lm", 0, (uint32)~0);
  cache_mgr->SetCurrentScope (0);
  return rc;
}

void csInstmeshMeshObject::PrepareLighting ()
{
}

void csInstmeshMeshObject::LightChanged (iLight*)
{
  lighting_dirty = true;
}

void csInstmeshMeshObject::LightDisconnect (iLight* light)
{
  affecting_lights.Delete (light);
  lighting_dirty = true;
}

#define SHADOW_CAST_BACKFACE

void csInstmeshMeshObject::AppendShadows (iMovable* movable,
    iShadowBlockList* shadows, const csVector3& origin)
{
  if (!do_shadows) return;
  int tri_num = factory->GetTriangleCount ();
  const csVector3* vt = factory->GetVertices ();
  int vt_num = factory->GetVertexCount ();
  const csVector3* vt_world, * vt_array_to_delete;
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
//@@@ FIXME
//    for (i = 0 ; i < vt_num ; i++)
//      vt_world[i] = movtrans.This2Other (vt[i]);
  }

  iShadowBlock *list = shadows->NewShadowBlock (tri_num);
  csFrustum *frust;
  bool cw = true;                   //@@@ Use mirroring parameter here!
  const csTriangle* tri = factory->GetTriangles ();
  for (i = 0 ; i < tri_num ; i++, tri++)
  {
    csPlane3 pl (vt_world[tri->c], vt_world[tri->b], vt_world[tri->a]);
    //if (pl.VisibleFromPoint (origin) != cw) continue;
    float clas = pl.Classify (origin);
    if (ABS (clas) < EPSILON) continue;
#ifdef SHADOW_CAST_BACKFACE
    if ((clas < 0) == cw) continue;
#else
    if ((clas <= 0) != cw) continue;
#endif

    // Let the casted shadow appear with a tiny tiny offset...
    const csVector3 offs = csVector3 (pl.norm) * csVector3 (EPSILON);
    pl.DD += (origin + offs) * pl.norm;
#ifndef SHADOW_CAST_BACKFACE
    pl.Invert ();
#endif
    frust = list->AddShadow (origin, 0, 3, pl);
#ifdef SHADOW_CAST_BACKFACE
    frust->GetVertex (0).Set (vt_world[tri->c] - origin);
    frust->GetVertex (1).Set (vt_world[tri->b] - origin);
    frust->GetVertex (2).Set (vt_world[tri->a] - origin);
#else
    frust->GetVertex (0).Set (vt_world[tri->a] - origin);
    frust->GetVertex (1).Set (vt_world[tri->b] - origin);
    frust->GetVertex (2).Set (vt_world[tri->c] - origin);
#endif
  }

  delete[] vt_array_to_delete;
}

bool csInstmeshMeshObject::SetMaterialWrapper (iMaterialWrapper* mat)
{
  material = mat;
  iMaterialWrapper* mater = material;
  if (!mater) mater = factory->GetMaterialWrapper ();
  material_needs_visit = mater->IsVisitRequired ();
  return true;
}

void csInstmeshMeshObject::SetupShaderVariableContext ()
{
  uint bufferMask = (uint)CS_BUFFER_ALL_MASK;
  bufferHolder->SetAccessor (scfiRenderBufferAccessor, bufferMask);
}
  
void csInstmeshMeshObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;
    CalculateInstanceArrays ();
    delete[] lit_fact_colors;
    lit_fact_colors = 0;
    if (!do_manual_colors)
    {
      num_lit_fact_colors = factory->fact_vertices.Length ()
	* instances.Length ();
      lit_fact_colors = new csColor4 [num_lit_fact_colors];
      int i;
      for (i = 0 ; i <  num_lit_fact_colors; i++)
        lit_fact_colors[i].Set (0, 0, 0);
      static_fact_colors = new csColor4 [num_lit_fact_colors];
      for (i = 0 ; i <  num_lit_fact_colors; i++)
        //static_fact_colors[i] = base_color;	// Initialize to base color.
        static_fact_colors[i].Set (0, 0, 0);
    }
    iMaterialWrapper* mater = material;
    if (!mater) mater = factory->GetMaterialWrapper ();
    CS_ASSERT (mater != 0);
    material_needs_visit = mater->IsVisitRequired ();

    SetupShaderVariableContext ();
  }
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
void csInstmeshMeshObject::CastShadows (iMovable* movable, iFrustumView* fview)
{
  SetupObject ();

  if (do_manual_colors) return;
  if (!do_lighting) return;

  iBase* b = (iBase *)fview->GetUserdata ();
  csRef<iLightingProcessInfo> lpi = SCF_QUERY_INTERFACE(b,iLightingProcessInfo);
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

  const csVector3* normals = factory->GetNormals ();
  const csVector3* vertices = factory->GetVertices ();
  csColor4* colors = static_fact_colors;
  // Compute light position in object coordinates
  csVector3 wor_light_pos = li->GetMovable ()->GetFullPosition ();
  csVector3 obj_light_pos = o2w.Other2This (wor_light_pos);

  bool pseudoDyn = li->GetDynamicType () == CS_LIGHT_DYNAMICTYPE_PSEUDO;
  csShadowArray* shadowArr = 0;
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
    const csVector3& normal = normals[i];
#ifdef SHADOW_CAST_BACKFACE
    csVector3 v = o2w.This2Other (vertices[i]) - wor_light_pos;
#else
    /*
      A small fraction of the normal is added to prevent unwanted
      self-shadowing (due small inaccuracies, the tri(s) this vertex
      lies on may shadow it.)
     */
    csVector3 v = o2w.This2Other (vertices[i] + (normal * VERTEX_OFFSET)) -
      wor_light_pos;
    /*csVector3 vN (v); vN.Normalize();
    v -= (vN * 0.1f);*/
#endif

    if (!light_frustum->Contains (v))
    {
      continue;
    }
    
    float vrt_sq_dist = csSquaredDist::PointPoint (obj_light_pos,
      vertices[i]);
    if (vrt_sq_dist >= csSquare (li->GetCutoffDistance ())) continue;
    
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

void csInstmeshMeshObject::UpdateLightingOne (
  const csReversibleTransform& trans, iLight* li)
{
  const csVector3* normals = mesh_normals.GetArray ();
  csColor4* colors = lit_fact_colors;
  // Compute light position in object coordinates
  csVector3 wor_light_pos = li->GetMovable ()->GetFullPosition ();
  csVector3 obj_light_pos = trans.Other2This (wor_light_pos);
  float obj_sq_dist = csSquaredDist::PointPoint (obj_light_pos, 0);
  if (obj_sq_dist >= csSquare (li->GetCutoffDistance ())) return;
  float in_obj_dist =
    (obj_sq_dist >= SMALL_EPSILON) ? csQisqrt (obj_sq_dist) : 1.0f;

  csColor light_color = li->GetColor () * (256. / CS_NORMAL_LIGHT_LEVEL)
      * li->GetBrightnessAtDistance (csQsqrt (obj_sq_dist));
  if (light_color.red < EPSILON && light_color.green < EPSILON
  	&& light_color.blue < EPSILON)
    return;

  csColor col;
  int i;
  int numcol = factory->GetVertexCount () * instances.Length ();
  if (obj_sq_dist < SMALL_EPSILON)
  {
    for (i = 0 ; i < numcol ; i++)
    {
      colors[i] += light_color;
    }
  }
  else
  {
    obj_light_pos *= in_obj_dist;
    for (i = 0 ; i < numcol ; i++)
    {
      float cosinus = obj_light_pos * normals[i];
      // because the vector from the object center to the light center
      // in object space is equal to the position of the light

      if (cosinus > 0)
      {
        col = light_color;
        if (cosinus < 1) col *= cosinus;
        colors[i] += col;
      }
    }
  }
}

/*
Rules for color calculation:
  EAmb = Static Engine Ambient
  SAmb = Dynamic Sector Ambient
  BC   = Base Color (base_color)
  FC   = Color Array from factory
  SC   = Static Color Array (static_fact_colors)
  LC   = Colors calculated from all relevant lights
  LDC  = Colors calculated from dynamic lights only
  C    = Final Color Array (lit_fact_colors)

  sr   = do_shadow_rec flag
  l    = lighting flag
  mc   = manual colors flag

  sr   mc   l    formula
  ----------------------
  *    1    *    C[i] = FC[i]
  *    0    0    C[i] = BC+FC[i]
  1    0    1    C[i] = BC+SC[i]+EAmb+SAmb+FC[i]+LDC[i]
  0    0    1    C[i] = BC+LC[i]+EAmb+SAmb+FC[i]
*/

void csInstmeshMeshObject::UpdateLighting (
    const csArray<iLightSectorInfluence*>& lights,
    iMovable* movable)
{
  int i;
  if (cur_movablenr != movable->GetUpdateNumber ())
  {
    lighting_dirty = true;
    cur_movablenr = movable->GetUpdateNumber ();
  }

  if (factory->DoFullBright ())
  {
    int numcol = factory->GetVertexCount () * instances.Length ();
    lighting_dirty = false;
    for (i = 0 ; i < numcol ; i++)
    {
      lit_fact_colors[i].Set (1, 1, 1);
    }
    return;
  }

  if (do_manual_colors) return;

  const csColor4* colors_ptr = mesh_colors.GetArray ();

  if (do_lighting)
  {
    if (!lighting_dirty)
    {
      iSector* sect = movable->GetSectors ()->Get (0);
      if (dynamic_ambient_version == sect->GetDynamicAmbientVersion ())
        return;
      dynamic_ambient_version = sect->GetDynamicAmbientVersion ();
    }
    lighting_dirty = false;
    mesh_colors_dirty_flag = true;

    csColor col;
    if (factory->engine)
    {
      factory->engine->GetAmbientLight (col);
      col += base_color;
      iSector* sect = movable->GetSectors ()->Get (0);
      if (sect)
        col += sect->GetDynamicAmbientLight ();
    }
    else
    {
      col = base_color;
    }
    int numcol = factory->GetVertexCount () * instances.Length ();
    for (i = 0 ; i < numcol ; i++)
    {
      lit_fact_colors[i] = col + static_fact_colors[i] + colors_ptr[i];
    }
    if (do_shadow_rec)
    {
      csReversibleTransform trans = movable->GetFullTransform ();
      csSet<csPtrKey<iLight> >::GlobalIterator it = affecting_lights.
      	GetIterator ();
      while (it.HasNext ())
      {
        iLight* l = (iLight*)it.Next ();
        UpdateLightingOne (trans, l);
      }
      csHash<csShadowArray*, csPtrKey<iLight> >::GlobalIterator pdlIt =
        pseudoDynInfo.GetIterator ();
      while (pdlIt.HasNext ())
      {
        csPtrKey<iLight> l;
        csShadowArray* shadowArr = pdlIt.Next (l);
        csColor c = l->GetColor ();
        if (c.red > EPSILON || c.green > EPSILON || c.blue > EPSILON)
        {
          c = c * (256. / CS_NORMAL_LIGHT_LEVEL);
          float* intensities = shadowArr->shadowmap;
          for (int i = 0; i < num_lit_fact_colors; i++)
          {
            lit_fact_colors[i] += c * intensities[i];
          }
        }
      }
    }
    else
    {
      // Do the lighting.
      csReversibleTransform trans = movable->GetFullTransform ();
      // the object center in world coordinates. "0" because the object
      // center in object space is obviously at (0,0,0).
      int num_lights = (int)lights.Length ();
      for (int l = 0 ; l < num_lights ; l++)
      {
        iLight* li = lights[l]->GetLight ();
        li->AddAffectedLightingInfo (&scfiLightingInfo);
        affecting_lights.Add (li);
        UpdateLightingOne (trans, li);
      }
    }
    // @@@ Try to avoid this loop!
    // Clamp all vertex colors to 2.
    for (i = 0 ; i < numcol ; i++)
      lit_fact_colors[i].Clamp (2., 2., 2.);
  }
  else
  {
    if (!lighting_dirty)
      return;
    lighting_dirty = false;
    mesh_colors_dirty_flag = true;

    int numcol = factory->GetVertexCount () * instances.Length ();
    for (i = 0 ; i < numcol ; i++)
    {
      lit_fact_colors[i] = base_color + colors_ptr[i];
      lit_fact_colors[i].Clamp (2., 2., 2.);
    }
  }
}

csRenderMesh** csInstmeshMeshObject::GetRenderMeshes (
	int& n, iRenderView* rview, 
	iMovable* movable, uint32 frustum_mask)
{
  CheckLitColors ();
  SetupObject ();

  n = 0;

  iCamera* camera = rview->GetCamera ();

  int clip_portal, clip_plane, clip_z_plane;
  rview->CalculateClipSettings (frustum_mask, clip_portal, clip_plane,
      clip_z_plane);

  lighting_movable = movable;

  if (!do_manual_colors && !do_shadow_rec && factory->light_mgr)
  {
    // Remember relevant lights for later.
    relevant_lights = factory->light_mgr->GetRelevantLights (
    	logparent, -1, false);
  }

  const csReversibleTransform o2wt = movable->GetFullTransform ();
  const csVector3& wo = o2wt.GetOrigin ();

  // Array still needed?@@@
  {
    renderMeshes.SetLength (1);

    iMaterialWrapper* mater = material;
    if (!mater) mater = factory->GetMaterialWrapper ();
    if (!mater)
    {
      csPrintf ("INTERNAL ERROR: mesh used without material!\n");
      return 0;
    }

    if (mater->IsVisitRequired ()) mater->Visit ();

    bool rmCreated;
    csRenderMesh*& meshPtr = rmHolder.GetUnusedMesh (rmCreated,
      rview->GetCurrentFrameNumber ());

    meshPtr->mixmode = MixMode;
    meshPtr->clip_portal = clip_portal;
    meshPtr->clip_plane = clip_plane;
    meshPtr->clip_z_plane = clip_z_plane;
    meshPtr->do_mirror = camera->IsMirrored ();
    meshPtr->meshtype = CS_MESHTYPE_TRIANGLES;
    meshPtr->indexstart = 0;
    meshPtr->indexend = mesh_triangles.Length () * 3;
    meshPtr->material = mater;
    CS_ASSERT (mater != 0);
    meshPtr->worldspace_origin = wo;
    meshPtr->buffers = bufferHolder;
    meshPtr->geometryInstance = (void*)factory;
    meshPtr->object2world = o2wt;

    renderMeshes[0] = meshPtr;
  }

  n = (int)renderMeshes.Length ();
  return renderMeshes.GetArray ();
}

void csInstmeshMeshObject::GetRadius (csVector3& rad, csVector3& cent)
{
  rad = GetRadius ();
  cent.Set (0.0f);
}

bool csInstmeshMeshObject::HitBeamOutline (const csVector3& start,
  const csVector3& end, csVector3& isect, float* pr)
{
  // This is now closer to an outline hitting method. It will
  // return as soon as it touches any triangle in the mesh, and
  // will be a bit faster than its more accurate cousin (below).

  csSegment3 seg (start, end);
  int i, max = factory->GetTriangleCount();
  const csTriangle *tr = factory->GetTriangles();
  const csVector3 *vrt = factory->GetVertices ();
  for (i = 0 ; i < max ; i++)
  {
    if (csIntersect3::SegmentTriangle (seg, vrt[tr[i].a], vrt[tr[i].b],
        vrt[tr[i].c], isect))
    {
      if (pr) *pr = csQsqrt (csSquaredDist::PointPoint (start, isect) /
        csSquaredDist::PointPoint (start, end));

      return true;
    }
  }
  return false;
}

bool csInstmeshMeshObject::HitBeamObject (const csVector3& start,
  const csVector3& end, csVector3& isect, float *pr, int* polygon_idx,
  iMaterialWrapper** material)
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
  const csVector3 *vrt = factory->GetVertices ();
  csVector3 tmp;
  const csTriangle *tr = factory->GetTriangles();
  for (i = 0 ; i < max ; i++)
  {
    if (csIntersect3::SegmentTriangle (seg, vrt[tr[i].a], vrt[tr[i].b],
        vrt[tr[i].c], tmp))
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

  if (material)
  {
    // @@@ Submeshes not yet supported!
    //const csPDelArray<csInstmeshSubMesh>& sm = subMeshes.Length () == 0
    	//? factory->GetSubMeshes ()
	//: subMeshes;
    //if (sm.Length () == 0)
    //{
      *material = csInstmeshMeshObject::material;
      if (!*material) *material = factory->GetMaterialWrapper ();
    //}
  }

  return true;
}

int csInstmeshMeshObject::PolyMesh::GetVertexCount ()
{
  return scfParent->factory->GetVertexCount ();
}

csVector3* csInstmeshMeshObject::PolyMesh::GetVertices ()
{
  //@@@FIXME: data must come from mesh itself. Not factory
  return 0;
  //return scfParent->factory->GetVertices ();
}

int csInstmeshMeshObject::PolyMesh::GetPolygonCount ()
{
  //@@@FIXME: data from mesh instead of factory
  return 0;
  //return scfParent->factory->GetTriangleCount ();
}

csMeshedPolygon* csInstmeshMeshObject::PolyMesh::GetPolygons ()
{
  //@@@FIXME: data from mesh instead of factory
  return 0;
  //return scfParent->factory->GetPolygons ();
}

int csInstmeshMeshObject::PolyMesh::GetTriangleCount ()
{
  //@@@FIXME: data from mesh instead of factory
  return 0;
  //return scfParent->factory->GetTriangleCount ();
}

csTriangle* csInstmeshMeshObject::PolyMesh::GetTriangles ()
{
  //@@@FIXME: data from mesh instead of factory
  return 0;
  //return scfParent->factory->GetTriangles ();
}

void csInstmeshMeshObject::PreGetBuffer (csRenderBufferHolder* holder, 
					csRenderBufferName buffer)
{
  if (!holder) return;

  if (buffer == CS_BUFFER_COLOR)
  {
    if (!do_manual_colors)
    {
      UpdateLighting (relevant_lights, lighting_movable);
    }
    if (mesh_colors_dirty_flag)
    {
      if (!do_manual_colors)
      {
        if (!color_buffer ||
          (color_buffer->GetSize() != (sizeof (csColor4) * 
          num_lit_fact_colors)))
        {
          // Recreate the render buffer only if the new data cannot fit inside
          //  the existing buffer.
          color_buffer = csRenderBuffer::CreateRenderBuffer (
            num_lit_fact_colors, 
            do_lighting ? CS_BUF_DYNAMIC : CS_BUF_STATIC,
            CS_BUFCOMP_FLOAT, 4, false);
        }
        mesh_colors_dirty_flag = false;
        const csColor4* fact_colors = 0;
        fact_colors = lit_fact_colors;
        color_buffer->CopyInto (fact_colors, num_lit_fact_colors);
      }
      else
      {
	int numcol = factory->fact_vertices.Length () * instances.Length ();
        if (!color_buffer || 
          (color_buffer->GetSize() != (sizeof (csColor4) * numcol)))
        {
          // Recreate the render buffer only if the new data cannot fit inside
          //  the existing buffer.
          color_buffer = csRenderBuffer::CreateRenderBuffer (
            numcol, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 4, false);
        }
        mesh_colors_dirty_flag = false;
        const csColor4* fact_colors = 0;
        fact_colors = factory->GetColors ();
        color_buffer->CopyInto (fact_colors, numcol);
      }
    }
    holder->SetRenderBuffer (buffer, color_buffer);
    return;
  }
  else if (buffer == CS_BUFFER_POSITION)
  {
    if (mesh_vertices_dirty_flag)
    {
      if (!vertex_buffer)
        vertex_buffer = csRenderBuffer::CreateRenderBuffer (
          mesh_vertices.Length (), CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3, false);
      mesh_vertices_dirty_flag = false;
      vertex_buffer->CopyInto ((void*)mesh_vertices.GetArray (),
	  mesh_vertices.Length ());
    }
    holder->SetRenderBuffer (buffer, vertex_buffer);
    return;
  }
  else if (buffer == CS_BUFFER_TEXCOORD0) 
  {
    if (mesh_texels_dirty_flag)
    {
      if (!texel_buffer)
        texel_buffer = csRenderBuffer::CreateRenderBuffer (
          mesh_texels.Length (), CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 2, false);
      mesh_texels_dirty_flag = false;
      texel_buffer->CopyInto ((void*)mesh_texels.GetArray (),
	  mesh_texels.Length ());
    }
    holder->SetRenderBuffer (buffer, texel_buffer);
    return;
  }
  else if (buffer == CS_BUFFER_NORMAL)
  {
    if (mesh_normals_dirty_flag)
    {
      if (!normal_buffer)
        normal_buffer = csRenderBuffer::CreateRenderBuffer (
          mesh_normals.Length (), CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3, false);
      mesh_normals_dirty_flag = false;
      normal_buffer->CopyInto ((void*)mesh_normals.GetArray (),
	  mesh_normals.Length ());
    }
    holder->SetRenderBuffer (buffer, normal_buffer);
    return;
  }
  else if (buffer == CS_BUFFER_TANGENT || buffer == CS_BUFFER_BINORMAL) 
  {
    if (mesh_tangents_dirty_flag)
    {
      if (!tangent_buffer)
        tangent_buffer = csRenderBuffer::CreateRenderBuffer (
          mesh_vertices.Length (), CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3);
      if (!binormal_buffer)
        binormal_buffer = csRenderBuffer::CreateRenderBuffer (
          mesh_vertices.Length (), CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3);
      mesh_tangents_dirty_flag = false;

      csVector3* tangentData = new csVector3[mesh_vertices.Length () * 2];
      csVector3* bitangentData = tangentData + mesh_vertices.Length ();
      csNormalMappingTools::CalculateTangents (mesh_triangles.Length (), 
        mesh_triangles.GetArray (), mesh_vertices.Length (),
	mesh_vertices.GetArray (), mesh_normals.GetArray (), 
        mesh_texels.GetArray (), tangentData, bitangentData);

      tangent_buffer->CopyInto (tangentData, mesh_vertices.Length ());
      binormal_buffer->CopyInto (bitangentData, mesh_vertices.Length ());

      delete[] tangentData;
    }
    holder->SetRenderBuffer (buffer, (buffer == CS_BUFFER_TANGENT) ?
      tangent_buffer : binormal_buffer);
    return;
  }
  else if (buffer == CS_BUFFER_INDEX)
  {
    if (mesh_triangle_dirty_flag)
    {
      if (!index_buffer)
        index_buffer = csRenderBuffer::CreateIndexRenderBuffer (
          mesh_triangles.Length ()*3, CS_BUF_STATIC,
          CS_BUFCOMP_UNSIGNED_INT, 0, mesh_vertices.Length () - 1);
      mesh_triangle_dirty_flag = false;
      index_buffer->CopyInto ((void*)mesh_triangles.GetArray (),
	  mesh_triangles.Length ()*3);
    }
    holder->SetRenderBuffer (buffer, index_buffer);
    return;
  }
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csInstmeshMeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iInstancingFactoryState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csInstmeshMeshObjectFactory::InstancingFactoryState)
  SCF_IMPLEMENTS_INTERFACE (iInstancingFactoryState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csInstmeshMeshObjectFactory::csInstmeshMeshObjectFactory (
  iMeshObjectType *pParent, iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiInstancingFactoryState);

  csInstmeshMeshObjectFactory::object_reg = object_reg;

  logparent = 0;
  instmesh_type = pParent;

  material = 0;
  light_mgr = CS_QUERY_REGISTRY (object_reg, iLightManager);

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg,
    "crystalspace.shared.stringset", iStringSet);

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

  csRef<iCommandLineParser> cmdline = CS_QUERY_REGISTRY (
  	object_reg, iCommandLineParser);
  do_fullbright = cmdline->GetOption ("fullbright");
}

csInstmeshMeshObjectFactory::~csInstmeshMeshObjectFactory ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiInstancingFactoryState);
  SCF_DESTRUCT_IBASE ();
}

void csInstmeshMeshObjectFactory::AddVertex (const csVector3& v,
      const csVector2& uv, const csVector3& normal,
      const csColor4& color)
{
  if (fact_vertices.Length () == 0)
  {
    factory_bbox.StartBoundingBox (v);
    factory_radius = csQsqrt (csSquaredDist::PointPoint (v,
	  csVector3 (0)));
  }
  else
  {
    factory_bbox.AddBoundingVertexSmart (v);
    float rad = csQsqrt (csSquaredDist::PointPoint (v,
	  csVector3 (0)));
    if (rad > factory_radius) factory_radius = rad;
  }
  fact_vertices.Push (v);
  fact_texels.Push (uv);
  fact_normals.Push (normal);
  fact_colors.Push (color);
}

void csInstmeshMeshObjectFactory::Compress ()
{
  size_t old_num = fact_vertices.Length ();
  csCompressVertexInfo* vt = csVertexCompressor::Compress (
    	fact_vertices, fact_texels, fact_normals, fact_colors);
  if (vt)
  {
    printf ("From %d to %d\n", int (old_num), int (fact_vertices.Length ()));
    fflush (stdout);

    // Now we can remap the vertices in all triangles.
    size_t i;
    for (i = 0 ; i < fact_triangles.Length () ; i++)
    {
      fact_triangles[i].a = (int)vt[fact_triangles[i].a].new_idx;
      fact_triangles[i].b = (int)vt[fact_triangles[i].b].new_idx;
      fact_triangles[i].c = (int)vt[fact_triangles[i].c].new_idx;
    }
    delete[] vt;
  }
}

struct CompressVertex
{
  size_t orig_idx;
  float x, y, z;
  size_t new_idx;
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

bool csInstmeshMeshObjectFactory::CompressVertices (
    csVector3* orig_verts, size_t orig_num_vts,
    csVector3*& new_verts, size_t& new_num_vts,
    csTriangle* orig_tris, size_t num_tris,
    csTriangle*& new_tris,
    size_t*& mapping)
{
  new_num_vts = orig_num_vts;
  new_tris = orig_tris;
  new_verts = orig_verts;
  mapping = 0;
  if (orig_num_vts <= 0) return false;

  // Copy all the vertices.
  CompressVertex *vt = new CompressVertex[orig_num_vts];
  size_t i, j;
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
  size_t last_unique = 0;
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
    new_tris[i].a = (int)vt[orig_tris[i].a].new_idx;
    new_tris[i].b = (int)vt[orig_tris[i].b].new_idx;
    new_tris[i].c = (int)vt[orig_tris[i].c].new_idx;
  }
  mapping = new size_t[orig_num_vts];
  for (i = 0 ; i < orig_num_vts ; i++)
    mapping[i] = vt[i].new_idx;

  delete[] vt;
  return true;
}

void csInstmeshMeshObjectFactory::CalculateNormals ()
{
  size_t i;
  size_t j;

  size_t num_triangles = fact_triangles.Length ();
  csTriangle* tris;
  csVector3* new_verts;
  size_t new_num_verts;
  size_t* mapping;

  bool compressed = CompressVertices (fact_vertices.GetArray (),
      fact_vertices.Length (),
      new_verts, new_num_verts,
      fact_triangles.GetArray (), num_triangles, tris,
      mapping);

  csTriangleMesh* tri_mesh = new csTriangleMesh ();
  tri_mesh->SetTriangles (tris, (int)num_triangles);
  csTriangleVertices* tri_verts = new csTriangleVertices (tri_mesh,
    new_verts, (int)new_num_verts);

  csVector3* mesh_tri_normals = new csVector3[num_triangles];
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

  csVector3* new_normals = fact_normals.GetArray ();
  if (compressed)
    new_normals = new csVector3[new_num_verts];

  // Calculate vertex normals, by averaging connected triangle normals.
  for (i = 0 ; i < new_num_verts ; i++)
  {
    csTriangleVertex &vt = tri_verts->GetVertex ((int)i);
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
    for (j = 0 ; j < fact_vertices.Length () ; j++)
    {
      fact_normals[j] = new_normals[mapping[j]];
    }

    delete[] new_normals;
    delete[] new_verts;
    delete[] tris;
    delete[] mapping;
  }

  delete[] mesh_tri_normals;
}

void csInstmeshMeshObjectFactory::GenerateSphere (const csSphere& sphere,
    int num)
{
  //@@@ FIXME
#if 0
  int num_vertices = 0;
  int num_triangles = 0;
  csDirtyAccessArray<csVector3> vertices;
  csDirtyAccessArray<csVector2> uvverts;
  csDirtyAccessArray<csTriangle> triangles;
  float radius = 1.0f;

  csArray<int> prev_verticesT;
  csArray<int> prev_verticesB;
  float u, v;
  int i, j;

  // Number of degrees between layers.
  float radius_step = 180.0f / num;
  float vert_radius = radius;

  int num2 = num;

  // Generate the first series of vertices (the outer circle).
  // Calculate u,v for them.
  for (j = 0; j < num2; j++)
  {
    float new_radius = radius;
    float new_height = 0.0f;
    float angle = j * 2.0f * radius_step * TWO_PI / 360.0f;
    prev_verticesT.GetExtend (j) = num_vertices;
    prev_verticesB.GetExtend (j) = num_vertices;
    vertices.GetExtend (num_vertices).Set (new_radius * (float) cos (angle),
      new_height, new_radius * (float) sin (angle));

    u = (float) cos (angle) * 0.5f + 0.5f;
    v = (float) sin (angle) * 0.5f + 0.5f;

    uvverts.GetExtend (num_vertices).Set (u, v);
    num_vertices++;
  }

  // Array with new vertex indices.
  csArray<int> new_verticesT;
  csArray<int> new_verticesB;

  // First create the layered triangle strips.
  for (i = 1; i < (num / 2); i++)
  {
    //-----
    // First create a new series of vertices.
    //-----
    // Angle from the center to the new circle of vertices.
    float new_angle = i * radius_step * TWO_PI / 360.0f;
    // Radius of the new circle of vertices.
    float new_radius = radius * (float) cos (new_angle);
    // Height of the new circle of vertices.
    float new_height = vert_radius * (float) sin (new_angle);
    // UV radius.
    float uv_radius = (1.0f - 2.0f * (float) i / (float)num) * 0.5f;
    for (j = 0; j < num2; j++)
    {
      float angle = j * 2.0f * radius_step * TWO_PI / 360.0f;

      u = uv_radius * (float) cos (angle) + 0.5f;
      v = uv_radius * (float) sin (angle) + 0.5f;

      new_verticesT.GetExtend (j) = num_vertices;
      vertices.GetExtend (num_vertices).Set (new_radius * (float) cos (angle),
        new_height, new_radius * (float) sin (angle));
      uvverts.GetExtend (num_vertices).Set (u, v);
      num_vertices++;


      new_verticesB.GetExtend (j) = num_vertices;
      vertices.GetExtend (num_vertices).Set (new_radius * (float) cos (angle),
        -new_height, new_radius * (float) sin (angle));

      uvverts.GetExtend (num_vertices).Set (u, v);
      num_vertices++;
    }

    //-----
    // Now make the triangle strips.
    //-----
    for (j = 0; j < num; j++)
    {
      int j1num = (j+1)%num;
      csTriangle& tri1 = triangles.GetExtend (num_triangles);
      tri1.c = prev_verticesT[j];
      tri1.b = new_verticesT[j1num];
      tri1.a = new_verticesT[j];
      num_triangles++;
      csTriangle& tri2 = triangles.GetExtend (num_triangles);
      tri2.c = prev_verticesT[j];
      tri2.b = prev_verticesT[j1num];
      tri2.a = new_verticesT[j1num];
      num_triangles++;

      csTriangle& tri3 = triangles.GetExtend (num_triangles);
      tri3.a = prev_verticesB[j];
      tri3.b = new_verticesB[j1num];
      tri3.c = new_verticesB[j];
      num_triangles++;
      csTriangle& tri4 = triangles.GetExtend (num_triangles);
      tri4.a = prev_verticesB[j];
      tri4.b = prev_verticesB[j1num];
      tri4.c = new_verticesB[j1num];
      num_triangles++;

    }

    //-----
    // Copy the new vertex array to prev_vertices.
    //-----
    for (j = 0 ; j < num2 ; j++)
    {
      prev_verticesT.GetExtend (j) = new_verticesT[j];
      prev_verticesB.GetExtend (j) = new_verticesB[j];
    }
  }

  // Create the top and bottom vertices.
  int top_vertex = num_vertices;
  vertices.GetExtend (num_vertices).Set (0.0f, vert_radius, 0.0f);
  uvverts.GetExtend (num_vertices).Set (0.5f, 0.5f);
  num_vertices++;
  int bottom_vertex = 0;

  bottom_vertex = num_vertices;
  vertices.GetExtend (num_vertices).Set (0.0f, -vert_radius, 0.0f);
  uvverts.GetExtend (num_vertices).Set (0.5f, 0.5f);
  num_vertices++;


  //-----
  // Make the top triangle fan.
  //-----
  for (j = 0 ; j < num ; j++)
  {
    int j1num = (j+1)%num;
    csTriangle& tri = triangles.GetExtend (num_triangles);
    tri.c = top_vertex;
    tri.b = prev_verticesT[j];
    tri.a = prev_verticesT[j1num];
    num_triangles++;
  }

  //-----
  // Make the bottom triangle fan.
  //-----

  for (j = 0 ; j < num ; j++)
  {
    int j1num = (j+1)%num;
    csTriangle& tri = triangles.GetExtend (num_triangles);
    tri.a = bottom_vertex;
    tri.b = prev_verticesB[j];
    tri.c = prev_verticesB[j1num];
    num_triangles++;
  }

  float sphere_radius = sphere.GetRadius ();
  for (i = 0 ; i < num_vertices ; i++)
  {
    vertices[i].x *= sphere_radius;
    vertices[i].y *= sphere_radius;
    vertices[i].z *= sphere_radius;
    vertices[i] += sphere.GetCenter ();
  }

  SetVertexCount (num_vertices);
  csVector3* genfact_vertices = GetVertices();
  memcpy (genfact_vertices, vertices.GetArray (),
      sizeof(csVector3)*num_vertices);

  csVector2* genfact_texels = GetTexels();
  memcpy (genfact_texels, uvverts.GetArray (),
      sizeof(csVector2)*num_vertices);

  SetTriangleCount (num_triangles);
  csTriangle* ball_triangles = GetTriangles();
  memcpy (ball_triangles, triangles.GetArray (),
      sizeof(csTriangle)*num_triangles);

  csVector3* normals = GetNormals();
  for (i = 0; i < num_vertices; i++)
  {
    normals[i] = genfact_vertices[i];
    normals[i].Normalize ();
  }

  Invalidate();
#endif
}

void csInstmeshMeshObjectFactory::GenerateBox (const csBox3& box)
{
  csVector3 v;
  v.Set(box.MinX(),box.MaxY(),box.MinZ()); v.Normalize();
  AddVertex (csVector3 (box.MinX(), box.MaxY(), box.MinZ()),
  	csVector2 (0, 0), v, csColor4 (0, 0, 0));
  v.Set(box.MinX(),box.MaxY(),box.MinZ()); v.Normalize();
  AddVertex (csVector3 (box.MinX(), box.MaxY(), box.MinZ()),
	csVector2 (0, 1), v, csColor4 (0, 0, 0));
  v.Set(box.MinX(),box.MaxY(),box.MinZ()); v.Normalize();
  AddVertex (csVector3 (box.MinX(), box.MaxY(), box.MinZ()),
	csVector2 (1, 0), v, csColor4 (0, 0, 0));

  v.Set(box.MinX(),box.MaxY(),box.MaxZ()); v.Normalize();
  AddVertex (csVector3 (box.MinX(), box.MaxY(), box.MaxZ()),
  	csVector2 (0, 0), v, csColor4 (0, 0, 0));
  v.Set(box.MinX(),box.MaxY(),box.MaxZ()); v.Normalize();
  AddVertex (csVector3 (box.MinX(), box.MaxY(), box.MaxZ()),
	csVector2 (0, 0), v, csColor4 (0, 0, 0));
  v.Set(box.MinX(),box.MaxY(),box.MaxZ()); v.Normalize();
  AddVertex (csVector3 (box.MinX(), box.MaxY(), box.MaxZ()),
	csVector2 (1, 0), v, csColor4 (0, 0, 0));

  v.Set(box.MaxX(),box.MaxY(),box.MaxZ()); v.Normalize();
  AddVertex (csVector3 (box.MaxX(), box.MaxY(), box.MaxZ()),
  	csVector2 (1, 0), v, csColor4 (0, 0, 0));
  v.Set(box.MaxX(),box.MaxY(),box.MaxZ()); v.Normalize();
  AddVertex (csVector3 (box.MaxX(), box.MaxY(), box.MaxZ()),
	csVector2 (0, 0), v, csColor4 (0, 0, 0));
  v.Set(box.MaxX(),box.MaxY(),box.MaxZ()); v.Normalize();
  AddVertex (csVector3 (box.MaxX(), box.MaxY(), box.MaxZ()),
	csVector2 (1, 0), v, csColor4 (0, 0, 0));

  v.Set(box.MaxX(),box.MaxY(),box.MinZ()); v.Normalize();
  AddVertex (csVector3 (box.MaxX(), box.MaxY(), box.MinZ()),
  	csVector2 (1, 0), v, csColor4 (0, 0, 0));
  v.Set(box.MaxX(),box.MaxY(),box.MinZ()); v.Normalize();
  AddVertex (csVector3 (box.MaxX(), box.MaxY(), box.MinZ()),
	csVector2 (1, 1), v, csColor4 (0, 0, 0));
  v.Set(box.MaxX(),box.MaxY(),box.MinZ()); v.Normalize();
  AddVertex (csVector3 (box.MaxX(), box.MaxY(), box.MinZ()),
	csVector2 (0, 0), v, csColor4 (0, 0, 0));

  v.Set(box.MinX(),box.MinY(),box.MaxZ()); v.Normalize();
  AddVertex (csVector3 (box.MinX(), box.MinY(), box.MaxZ()),
  	csVector2 (0, 1), v, csColor4 (0, 0, 0));
  v.Set(box.MinX(),box.MinY(),box.MaxZ()); v.Normalize();
  AddVertex (csVector3 (box.MinX(), box.MinY(), box.MaxZ()),
	csVector2 (1, 1), v, csColor4 (0, 0, 0));
  v.Set(box.MinX(),box.MinY(),box.MaxZ()); v.Normalize();
  AddVertex (csVector3 (box.MinX(), box.MinY(), box.MaxZ()),
	csVector2 (1, 1), v, csColor4 (0, 0, 0));

  v.Set(box.MaxX(),box.MinY(),box.MaxZ()); v.Normalize();
  AddVertex (csVector3 (box.MaxX(), box.MinY(), box.MaxZ()),
  	csVector2 (0, 1), v, csColor4 (0, 0, 0));
  v.Set(box.MaxX(),box.MinY(),box.MaxZ()); v.Normalize();
  AddVertex (csVector3 (box.MaxX(), box.MinY(), box.MaxZ()),
	csVector2 (1, 1), v, csColor4 (0, 0, 0));
  v.Set(box.MaxX(),box.MinY(),box.MaxZ()); v.Normalize();
  AddVertex (csVector3 (box.MaxX(), box.MinY(), box.MaxZ()),
	csVector2 (1, 0), v, csColor4 (0, 0, 0));

  v.Set(box.MaxX(),box.MinY(),box.MinZ()); v.Normalize();
  AddVertex (csVector3 (box.MaxX(), box.MinY(), box.MinZ()),
  	csVector2 (1, 1), v, csColor4 (0, 0, 0));
  v.Set(box.MaxX(),box.MinY(),box.MinZ()); v.Normalize();
  AddVertex (csVector3 (box.MaxX(), box.MinY(), box.MinZ()),
	csVector2 (0, 1), v, csColor4 (0, 0, 0));
  v.Set(box.MaxX(),box.MinY(),box.MinZ()); v.Normalize();
  AddVertex (csVector3 (box.MaxX(), box.MinY(), box.MinZ()),
	csVector2 (0, 0), v, csColor4 (0, 0, 0));

  v.Set(box.MinX(),box.MinY(),box.MinZ()); v.Normalize();
  AddVertex (csVector3 (box.MinX(), box.MinY(), box.MinZ()),
  	csVector2 (0, 1), v, csColor4 (0, 0, 0));
  v.Set(box.MinX(),box.MinY(),box.MinZ()); v.Normalize();
  AddVertex (csVector3 (box.MinX(), box.MinY(), box.MinZ()),
	csVector2 (1, 1), v, csColor4 (0, 0, 0));
  v.Set(box.MinX(),box.MinY(),box.MinZ()); v.Normalize();
  AddVertex (csVector3 (box.MinX(), box.MinY(), box.MinZ()),
	csVector2 (0, 1), v, csColor4 (0, 0, 0));

  AddTriangle (csTriangle (0, 9, 18));
  AddTriangle (csTriangle (0, 18, 21));

  AddTriangle (csTriangle (3, 6, 10));
  AddTriangle (csTriangle (3, 10, 1));

  AddTriangle (csTriangle (4, 2, 22));
  AddTriangle (csTriangle (4, 22, 12));

  AddTriangle (csTriangle (7, 5, 13));
  AddTriangle (csTriangle (7, 13, 15));

  AddTriangle (csTriangle (11, 8, 16));
  AddTriangle (csTriangle (11, 16, 19));

  AddTriangle (csTriangle (23, 20, 17));
  AddTriangle (csTriangle (23, 17, 14));
}

void csInstmeshMeshObjectFactory::HardTransform (
    const csReversibleTransform& t)
{
  size_t i;
  for (i = 0 ; i < fact_vertices.Length () ; i++)
  {
    fact_vertices[i] = t.This2Other (fact_vertices[i]);
    fact_normals[i] = t.This2OtherRelative (fact_normals[i]);
  }
}

csPtr<iMeshObject> csInstmeshMeshObjectFactory::NewInstance ()
{
  csInstmeshMeshObject* cm = new csInstmeshMeshObject (this);
  cm->SetMixMode (default_mixmode);
  cm->SetLighting (default_lighting);
  cm->SetColor (default_color);
  cm->SetManualColors (default_manualcolors);
  cm->SetShadowCasting (default_shadowcasting);
  cm->SetShadowReceiving (default_shadowreceiving);

  csRef<iMeshObject> im (SCF_QUERY_INTERFACE (cm, iMeshObject));
  cm->DecRef ();
  return csPtr<iMeshObject> (im);
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csInstmeshMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csInstmeshMeshObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csInstmeshMeshObjectType)


csInstmeshMeshObjectType::csInstmeshMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

  do_verbose = false;
}

csInstmeshMeshObjectType::~csInstmeshMeshObjectType ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObjectFactory> csInstmeshMeshObjectType::NewFactory ()
{
  csInstmeshMeshObjectFactory* cm = new csInstmeshMeshObjectFactory (this,
    object_reg);
  csRef<iMeshObjectFactory> ifact (
    SCF_QUERY_INTERFACE (cm, iMeshObjectFactory));
  cm->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}

bool csInstmeshMeshObjectType::Initialize (iObjectRegistry* object_reg)
{
  csInstmeshMeshObjectType::object_reg = object_reg;

  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (object_reg, iVerbosityManager));
  if (verbosemgr) 
    do_verbose = verbosemgr->Enabled ("genmesh");

  return true;
}
