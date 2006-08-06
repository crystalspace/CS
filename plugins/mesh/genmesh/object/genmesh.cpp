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
#include "csutil/scfstr.h"
#include "csutil/sysfunc.h"
#include "cstool/rbuflock.h"

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
#include "ivaria/reporter.h"
#include "cstool/vertexcompress.h"
#include "cstool/normalcalc.h"
#include "cstool/primitives.h"

#include "genmesh.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(Genmesh)
{

CS_LEAKGUARD_IMPLEMENT (csGenmeshMeshObject);
CS_LEAKGUARD_IMPLEMENT (csGenmeshMeshObjectFactory);

csGenmeshMeshObject::csGenmeshMeshObject (csGenmeshMeshObjectFactory* factory) :
        scfImplementationType (this), factorySubMeshesChangeNum (~0),
	pseudoDynInfo (29, 32),
	affecting_lights (29, 32)
{
  csGenmeshMeshObject::factory = factory;
  vc = factory->vc;
  logparent = 0;
  initialized = false;
  cur_movablenr = -1;
  material = 0;
  MixMode = 0;
  lit_mesh_colors = 0;
  num_lit_mesh_colors = 0;
  static_mesh_colors = 0;
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

  anim_ctrl_verts = false;
  anim_ctrl_texels = false;
  anim_ctrl_normals = false;
  anim_ctrl_colors = false;

  num_sorted_mesh_triangles = 0;
  sorted_mesh_triangles = 0;

  svcontext.AttachNew (new csShaderVariableContext);
  bufferHolder.AttachNew (new csRenderBufferHolder);

  g3d = csQueryRegistry<iGraphics3D> (factory->object_reg);
  buffers_version = (uint)-1;
  mesh_colors_dirty_flag = true;
}

csGenmeshMeshObject::~csGenmeshMeshObject ()
{
  delete[] lit_mesh_colors;
  delete[] static_mesh_colors;
  delete[] sorted_mesh_triangles;

  ClearPseudoDynLights ();
}

const csVector3* csGenmeshMeshObject::AnimControlGetVertices ()
{
  return anim_ctrl->UpdateVertices (vc->GetCurrentTicks (),
  	factory->GetVertices (),
	factory->GetVertexCount (),
	factory->GetShapeNumber ());
}

const csVector2* csGenmeshMeshObject::AnimControlGetTexels ()
{
  return anim_ctrl->UpdateTexels (vc->GetCurrentTicks (),
  	factory->GetTexels (),
	factory->GetVertexCount (),
	factory->GetShapeNumber ());
}

const csVector3* csGenmeshMeshObject::AnimControlGetNormals ()
{
  return anim_ctrl->UpdateNormals (vc->GetCurrentTicks (),
  	factory->GetNormals (),
	factory->GetVertexCount (),
	factory->GetShapeNumber ());
}

const csColor4* csGenmeshMeshObject::AnimControlGetColors (csColor4* source)
{
  return anim_ctrl->UpdateColors (vc->GetCurrentTicks (),
  	source,
	factory->GetVertexCount (),
	factory->GetShapeNumber ());
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
    
    //small hack to force animation initiallizing
    AnimControlGetVertices ();
    AnimControlGetTexels ();
    AnimControlGetNormals ();
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

void csGenmeshMeshObject::UpdateSubMeshProxies () const
{
  const SubMeshesContainer& sm = factory->GetSubMeshes();
  if (factorySubMeshesChangeNum != sm.GetChangeNum())
  {
    if (sm.GetSize() == 0)
      subMeshes.Empty();
    else
    {
      SubMeshProxiesContainer newSubMeshes;
      for (size_t i = 0; i < sm.GetSize(); i++)
      {
        const char* name = sm[i]->GetName();
        csRef<SubMeshProxy> proxy = subMeshes.FindSubMesh (name);
        if (!proxy.IsValid())
          proxy.AttachNew (new SubMeshProxy);
        proxy->parentSubMesh = sm[i];
        newSubMeshes.AddSubMesh (proxy);
      }
      subMeshes = newSubMeshes;
    }
    factorySubMeshesChangeNum = sm.GetChangeNum();
  }
}

void csGenmeshMeshObject::ClearPseudoDynLights ()
{
  csHash<csShadowArray*, csPtrKey<iLight> >::GlobalIterator it (
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
    lit_mesh_colors = new csColor4 [num_lit_mesh_colors];
    delete[] static_mesh_colors;
    static_mesh_colors = new csColor4 [num_lit_mesh_colors];
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
    //csColor amb;
    //factory->engine->GetAmbientLight (amb);
    for (i = 0 ; i < num_lit_mesh_colors ; i++)
    {
      lit_mesh_colors[i].Set (0, 0, 0);
      static_mesh_colors[i].Set (0, 0, 0);
    }
  }
  lighting_dirty = true;
}

char* csGenmeshMeshObject::GenerateCacheName ()
{
  csMemFile mf;
  mf.Write ("genmesh", 7);
  uint32 l;
  l = csLittleEndian::Convert ((uint32)factory->GetVertexCount ());
  mf.Write ((char*)&l, 4);
  l = csLittleEndian::Convert ((uint32)factory->GetTriangleCount ());
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

bool csGenmeshMeshObject::ReadFromCache (iCacheManager* cache_mgr)
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
      for (v = 0; v < num_lit_mesh_colors; v++)
      {
	csColor4& c = static_mesh_colors[v];
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
	l->AddAffectedLightingInfo (this);

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
    const csColor4& c = static_mesh_colors[v];
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
    "genmesh_lm", 0, (uint32)~0);
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

void csGenmeshMeshObject::DisconnectAllLights ()
{
  csSet<csPtrKey<iLight> >::GlobalIterator it = affecting_lights.
      	GetIterator ();
  while (it.HasNext ())
  {
    iLight* l = (iLight*)it.Next ();
    l->RemoveAffectedLightingInfo (this);
  }
  affecting_lights.Empty ();
  lighting_dirty = true;
}

#define SHADOW_CAST_BACKFACE

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
  csShaderVariable* sv;
  
  bool ac_verts = false;
  bool ac_texels = false;
  bool ac_normals = false;
  if (anim_ctrl)
  {
    ac_verts = anim_ctrl->AnimatesVertices ();
    ac_texels = anim_ctrl->AnimatesTexels ();
    ac_normals = anim_ctrl->AnimatesNormals ();
  }

  uint bufferMask = (uint)CS_BUFFER_ALL_MASK;

  size_t i;
  iStringSet* strings = factory->GetStrings();
  const csArray<csStringID>& factoryUBNs = factory->GetUserBufferNames();
  // Set up factorys user buffers...
  for (i = 0; i < factoryUBNs.Length(); i++)
  {
    const csStringID userBuf = factoryUBNs.Get(i);
    const char* bufName = strings->Request (userBuf);
    csRenderBufferName userName = 
      csRenderBuffer::GetBufferNameFromDescr (bufName);
    if (userName >= CS_BUFFER_POSITION)
    {
      bufferHolder->SetRenderBuffer (userName, 
	factory->GetUserBuffers().GetRenderBuffer (userBuf));
      bufferMask &= ~CS_BUFFER_MAKE_MASKABLE(userName);
    }
    else
    {
      sv = svcontext->GetVariableAdd (userBuf);
      sv->SetAccessor (factory);
    }
  }
  // Set up meshs user buffers...
  for (i = 0; i < user_buffer_names.Length(); i++)
  {
    const csStringID userBuf = user_buffer_names.Get(i);
    const char* bufName = strings->Request (userBuf);
    csRenderBufferName userName = 
      csRenderBuffer::GetBufferNameFromDescr (bufName);
    if (userName >= CS_BUFFER_POSITION)
    {
      bufferHolder->SetRenderBuffer (userName, 
	userBuffers.GetRenderBuffer (userBuf));
      bufferMask &= ~CS_BUFFER_MAKE_MASKABLE(userName);
    }
    else
    {
      sv = svcontext->GetVariableAdd (userBuf);
      sv->SetAccessor (this);
    }
  }
  bufferHolder->SetAccessor (this, bufferMask);
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
      lit_mesh_colors = new csColor4 [num_lit_mesh_colors];
      int i;
      for (i = 0 ; i <  num_lit_mesh_colors; i++)
        lit_mesh_colors[i].Set (0, 0, 0);
      static_mesh_colors = new csColor4 [num_lit_mesh_colors];
      for (i = 0 ; i <  num_lit_mesh_colors; i++)
        //static_mesh_colors[i] = base_color;	// Initialize to base color.
        static_mesh_colors[i].Set (0, 0, 0);
    }
    iMaterialWrapper* mater = material;
    if (!mater) mater = factory->GetMaterialWrapper ();
    material_needs_visit = mater ? mater->IsVisitRequired () : false;

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
void csGenmeshMeshObject::CastShadows (iMovable* movable, iFrustumView* fview)
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
      li->AddAffectedLightingInfo (this);
      if (li->GetDynamicType () != CS_LIGHT_DYNAMICTYPE_PSEUDO)
        affecting_lights.Add (li);
    }
  }
  else
  {
    if (!affecting_lights.In (li))
    {
      li->AddAffectedLightingInfo (this);
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
  csColor4* colors = static_mesh_colors;
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

void csGenmeshMeshObject::UpdateLightingOne (
  const csReversibleTransform& trans, iLight* li)
{
  csVector3* normals = factory->GetNormals ();
  csColor4* colors = lit_mesh_colors;
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
  if (obj_sq_dist < SMALL_EPSILON)
  {
    for (i = 0 ; i < factory->GetVertexCount () ; i++)
    {
      colors[i] += light_color;
    }
  }
  else
  {
    obj_light_pos *= in_obj_dist;
    for (i = 0 ; i < factory->GetVertexCount () ; i++)
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
  SC   = Static Color Array (static_mesh_colors)
  LC   = Colors calculated from all relevant lights
  LDC  = Colors calculated from dynamic lights only
  C    = Final Color Array (lit_mesh_colors)

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

void csGenmeshMeshObject::UpdateLighting (
    const csArray<iLightSectorInfluence*>& lights,
    iMovable* movable)
{
  int i;
  if (factory->DoFullBright ())
  {
    if (lighting_dirty)
    {
      lighting_dirty = false;
      for (i = 0 ; i < factory->GetVertexCount () ; i++)
      {
        lit_mesh_colors[i].Set (1, 1, 1);
      }
    }
    return;
  }

  if (cur_movablenr != movable->GetUpdateNumber ())
  {
    lighting_dirty = true;
    cur_movablenr = movable->GetUpdateNumber ();
  }

  if (do_manual_colors) return;

  csColor4* factory_colors = factory->GetColors ();

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
    for (i = 0 ; i < factory->GetVertexCount () ; i++)
    {
      lit_mesh_colors[i] = col + static_mesh_colors[i] + factory_colors[i];
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
          for (int i = 0; i < num_lit_mesh_colors; i++)
          {
            lit_mesh_colors[i] += c * intensities[i];
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
        li->AddAffectedLightingInfo (this);
        affecting_lights.Add (li);
        UpdateLightingOne (trans, li);
      }
    }
    // @@@ Try to avoid this loop!
    // Clamp all vertex colors to 2.
    for (i = 0 ; i < factory->GetVertexCount () ; i++)
      lit_mesh_colors[i].Clamp (2., 2., 2.);
  }
  else
  {
    if (!lighting_dirty)
      return;
    lighting_dirty = false;
    mesh_colors_dirty_flag = true;

    for (i = 0 ; i < factory->GetVertexCount () ; i++)
    {
      lit_mesh_colors[i] = base_color + factory_colors[i];
      lit_mesh_colors[i].Clamp (2., 2., 2.);
    }
  }
}

csRenderMesh** csGenmeshMeshObject::GetRenderMeshes (
	int& n, iRenderView* rview, 
	iMovable* movable, uint32 frustum_mask)
{
  SetupObject ();
  CheckLitColors ();

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

  UpdateSubMeshProxies ();
  SubMeshProxiesContainer& sm = subMeshes;

  if (sm.GetSize () == 0)
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

    if (factory->back2front)
    {
      sorted_index_buffer = csRenderBuffer::CreateIndexRenderBuffer (
      	factory->GetTriangleCount() * 3, CS_BUF_DYNAMIC,
	CS_BUFCOMP_UNSIGNED_INT, 0, factory->GetVertexCount() - 1);

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
        camera->GetTransform ().Other2This (wo));
      CS_ASSERT (triidx.Length () == (size_t)num_sorted_mesh_triangles);

      csTriangle* factory_triangles = factory->GetTriangles ();
      int i;
      for (i = 0 ; i < num_sorted_mesh_triangles ; i++)
        sorted_mesh_triangles[i] = factory_triangles[triidx[i]];
      sorted_index_buffer->CopyInto (sorted_mesh_triangles,
        sizeof (unsigned int)*num_sorted_mesh_triangles*3);

      bufferHolder->SetRenderBuffer(CS_BUFFER_INDEX, sorted_index_buffer);
      bufferHolder->SetAccessor (this, 
	bufferHolder->GetAccessorMask() 
	& (~CS_BUFFER_MAKE_MASKABLE (CS_BUFFER_INDEX)));

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
    meshPtr->worldspace_origin = wo;
    meshPtr->variablecontext = svcontext;
    meshPtr->buffers = bufferHolder;
    meshPtr->geometryInstance = (void*)factory;
    meshPtr->object2world = o2wt;

    renderMeshes[0] = meshPtr;
  }
  else
  {
    renderMeshes.SetLength (sm.GetSize ());

    for (size_t i = 0; i<sm.GetSize (); ++i)
    {
      SubMeshProxy& subMesh = *(sm[i]);
      iMaterialWrapper* mater = subMesh.GetMaterial();
      if (!mater) mater = factory->GetMaterialWrapper ();
      if (!mater)
      {
        csPrintf ("INTERNAL ERROR: mesh used without material!\n");
        return 0;
      }

      if (mater->IsVisitRequired ()) mater->Visit ();

      bool rmCreated;
      csRenderMesh*& meshPtr = subMesh.rmHolder.GetUnusedMesh (rmCreated,
        rview->GetCurrentFrameNumber ());

      iRenderBuffer* index_buffer = subMesh.GetIndices();
      csRenderBufferHolder* smBufferHolder = subMesh.GetBufferHolder();

      uint smMixMode = subMesh.GetMixmode();
      meshPtr->mixmode = (smMixMode != (uint)~0) ? smMixMode : MixMode;
      meshPtr->clip_portal = clip_portal;
      meshPtr->clip_plane = clip_plane;
      meshPtr->clip_z_plane = clip_z_plane;
      meshPtr->do_mirror = camera->IsMirrored ();
      meshPtr->meshtype = CS_MESHTYPE_TRIANGLES;
      meshPtr->indexstart = 0;
      meshPtr->indexend = (uint)index_buffer->GetElementCount();
      meshPtr->material = mater;
      CS_ASSERT (mater != 0);
      meshPtr->worldspace_origin = wo;
      csRef<MergedSVContext> mergedSVContext;
      mergedSVContext.AttachNew (
        new (factory->genmesh_type->mergedSVContextPool) MergedSVContext (
        static_cast<iShaderVariableContext*> (&subMesh), svcontext));
      meshPtr->variablecontext = mergedSVContext;
      meshPtr->object2world = o2wt;

      smBufferHolder->SetAccessor (this,
        bufferHolder->GetAccessorMask() 
        & (~CS_BUFFER_MAKE_MASKABLE (CS_BUFFER_INDEX)));

      for (size_t b=0; b<CS_BUFFER_COUNT; ++b)
      {
        if (b != CS_BUFFER_INDEX)
        {
          smBufferHolder->SetRenderBuffer ((csRenderBufferName)b, 
            bufferHolder->GetRenderBuffer ((csRenderBufferName)b));
        }
      }

      meshPtr->buffers = smBufferHolder;
      meshPtr->geometryInstance = (void*)factory;

      renderMeshes[i] = meshPtr;
    }
  }

  n = (int)renderMeshes.Length ();
  return renderMeshes.GetArray ();
}

void csGenmeshMeshObject::GetObjectBoundingBox (csBox3& bbox)
{
  bbox = factory->GetObjectBoundingBox ();
}

void csGenmeshMeshObject::SetObjectBoundingBox (const csBox3& bbox)
{
  factory->SetObjectBoundingBox (bbox);
}

void csGenmeshMeshObject::GetRadius (float& rad, csVector3& cent)
{
  rad = factory->GetRadius ();
  cent = factory->GetObjectBoundingBox ().GetCenter ();
}

bool csGenmeshMeshObject::HitBeamOutline (const csVector3& start,
  const csVector3& end, csVector3& isect, float* pr)
{
  // This is now closer to an outline hitting method. It will
  // return as soon as it touches any triangle in the mesh, and
  // will be a bit faster than its more accurate cousin (below).

  UpdateSubMeshProxies ();
  SubMeshProxiesContainer& sm = subMeshes;

  csSegment3 seg (start, end);
  const csVector3 *vrt = factory->GetVertices ();
  if (sm.GetSize() == 0)
  {
    int i, max = factory->GetTriangleCount();
    csTriangle *tr = factory->GetTriangles();
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
  }
  else
  {
    for (size_t s = 0; s < sm.GetSize(); s++)
    {
      iRenderBuffer* indexBuffer = sm[s]->GetIndices();
      csRenderBufferLock<uint, iRenderBuffer*> indices (indexBuffer);
      size_t n = indexBuffer->GetElementCount();
      while (n > 0)
      {
        if (csIntersect3::SegmentTriangle (seg, 
          vrt[indices.Get (0)], vrt[indices.Get (1)], vrt[indices.Get (2)], 
          isect))
        {
          if (pr) *pr = csQsqrt (csSquaredDist::PointPoint (start, isect) /
            csSquaredDist::PointPoint (start, end));
          return true;
        }
        n -= 3;
        indices += 3;
      }
    }
  }
  return false;
}

bool csGenmeshMeshObject::HitBeamObject (const csVector3& start,
  const csVector3& end, csVector3& isect, float *pr, int* polygon_idx,
  iMaterialWrapper** material)
{
  if (polygon_idx) *polygon_idx = -1;
  // This is the slow version. Use for an accurate hit on the object.
  // It will cycle through every triangle in the mesh serching for the
  // closest intersection. Slower, but returns the closest hit.
  // Usage is optional.

  UpdateSubMeshProxies ();
  SubMeshProxiesContainer& sm = subMeshes;

  csSegment3 seg (start, end);
  float tot_dist = csSquaredDist::PointPoint (start, end);
  float dist, temp;
  float itot_dist = 1 / tot_dist;
  dist = temp = tot_dist;
  const csVector3 *vrt = factory->GetVertices ();
  csVector3 tmp;
  if (sm.GetSize() == 0)
  {
    int i, max = factory->GetTriangleCount();
    csTriangle *tr = factory->GetTriangles();
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
      *material = csGenmeshMeshObject::material;
      if (!*material) *material = factory->GetMaterialWrapper ();
    }
  }
  else
  {
    iMaterialWrapper* mat;
    for (size_t s = 0; s < sm.GetSize(); s++)
    {
      iRenderBuffer* indexBuffer = sm[s]->GetIndices();
      csRenderBufferLock<uint> indices (indexBuffer);
      size_t n = indexBuffer->GetElementCount();
      while (n > 0)
      {
        if (csIntersect3::SegmentTriangle (seg, 
          vrt[indices.Get (0)], vrt[indices.Get (1)], vrt[indices.Get (2)], 
          tmp))
        {
          temp = csSquaredDist::PointPoint (start, tmp);
          if (temp < dist)
          {
            isect = tmp;
	    dist = temp;
	    //if (polygon_idx) *polygon_idx = i; // @@@ Uh, how to handle?
            mat = sm[s]->GetMaterial();
          }
        }
        n -= 3;
        indices += 3;
      }
    }
    if (pr) *pr = csQsqrt (dist * itot_dist);
    if (dist >= tot_dist)
      return false;

    if (material) *material = mat;
  }

  return true;
}

iObjectModel* csGenmeshMeshObject::GetObjectModel ()
{
  return factory->GetObjectModel ();
}

void csGenmeshMeshObject::PreGetValue (csShaderVariable* var)
{
  iRenderBuffer *a = userBuffers.GetRenderBuffer (var->GetName());
  if (a != 0)
  {
    var->SetValue (a);
  }
}

void csGenmeshMeshObject::PreGetBuffer (csRenderBufferHolder* holder, 
					csRenderBufferName buffer)
{
  if (!holder) return;
  if (anim_ctrl)
  {
    // If we have an animation control then we must get the vertex data
    // here.
    int num_mesh_vertices = factory->GetVertexCount ();
    if (buffer == CS_BUFFER_POSITION)
    {
      if (!vertex_buffer)
        vertex_buffer = csRenderBuffer::CreateRenderBuffer (
          num_mesh_vertices, CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3, false);
      const csVector3* mesh_vertices = AnimControlGetVertices ();
      if (!mesh_vertices) mesh_vertices = factory->GetVertices ();
      vertex_buffer->CopyInto (mesh_vertices, num_mesh_vertices);
      holder->SetRenderBuffer (buffer, vertex_buffer);
      return;
    }
    if (buffer == CS_BUFFER_TEXCOORD0)
    {
      if (!texel_buffer)
        texel_buffer = csRenderBuffer::CreateRenderBuffer (
          num_mesh_vertices, CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 2, false);
      const csVector2* mesh_texels = AnimControlGetTexels ();
      if (!mesh_texels) mesh_texels = factory->GetTexels ();
      texel_buffer->CopyInto (mesh_texels, num_mesh_vertices);
      holder->SetRenderBuffer (buffer, texel_buffer);
      return;
    }
    if (buffer == CS_BUFFER_NORMAL)
    {
      if (!normal_buffer)
        normal_buffer = csRenderBuffer::CreateRenderBuffer (
          num_mesh_vertices, CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3, false);
      const csVector3* mesh_normals = AnimControlGetNormals ();
      if (!mesh_normals) mesh_normals = factory->GetNormals ();
      normal_buffer->CopyInto (mesh_normals, num_mesh_vertices);
      holder->SetRenderBuffer (buffer, normal_buffer);
      return;
    }
  }

  if (buffer == CS_BUFFER_COLOR)
  {
    if (!do_manual_colors)
    {
      UpdateLighting (relevant_lights, lighting_movable);
    }
    if (mesh_colors_dirty_flag || anim_ctrl_colors)
    {
      if (!do_manual_colors)
      {
        if (!color_buffer ||
          (color_buffer->GetSize() != (sizeof (csColor4) * 
          num_lit_mesh_colors)))
        {
          // Recreate the render buffer only if the new data cannot fit inside
          //  the existing buffer.
          color_buffer = csRenderBuffer::CreateRenderBuffer (
              num_lit_mesh_colors, 
              do_lighting ? CS_BUF_DYNAMIC : CS_BUF_STATIC,
              CS_BUFCOMP_FLOAT, 4, false);
        }
        mesh_colors_dirty_flag = false;
        const csColor4* mesh_colors = 0;
        if (anim_ctrl_colors)
          mesh_colors = AnimControlGetColors (lit_mesh_colors);
        else
          mesh_colors = lit_mesh_colors;
        color_buffer->CopyInto (mesh_colors, num_lit_mesh_colors);
      }
      else
      {
        if (!color_buffer || 
          (color_buffer->GetSize() != (sizeof (csColor4) * 
          factory->GetVertexCount())))
        {
          // Recreate the render buffer only if the new data cannot fit inside
          //  the existing buffer.
          color_buffer = csRenderBuffer::CreateRenderBuffer (
            factory->GetVertexCount(), CS_BUF_STATIC,
            CS_BUFCOMP_FLOAT, 4, false);
        }
        mesh_colors_dirty_flag = false;
        const csColor4* mesh_colors = 0;
        if (anim_ctrl_colors)
          mesh_colors = AnimControlGetColors (factory->GetColors ());
        else
          mesh_colors = factory->GetColors ();
        color_buffer->CopyInto (mesh_colors, factory->GetVertexCount());        
      }
    }
    holder->SetRenderBuffer (buffer, color_buffer);
    return;
  }

  factory->PreGetBuffer (holder, buffer);
}

iGeneralMeshSubMesh* csGenmeshMeshObject::FindSubMesh (const char* name) const
{
  UpdateSubMeshProxies();
  return static_cast<iGeneralMeshSubMesh*> (subMeshes.FindSubMesh (name));
}

bool csGenmeshMeshObject::AddRenderBuffer (const char *name,
					   iRenderBuffer* buffer)
{
  csStringID bufID = factory->GetStrings()->Request (name);
  if (userBuffers.AddRenderBuffer (bufID, buffer))
  {
    user_buffer_names.Push (bufID);
    return true;
  }
  return false;
}

bool csGenmeshMeshObject::RemoveRenderBuffer (const char *name)
{
  csStringID bufID = factory->GetStrings()->Request (name);
  if (userBuffers.RemoveRenderBuffer (bufID))
  {
    user_buffer_names.DeleteFast (bufID);
    return true;
  }
  return false;
}

csRef<iRenderBuffer> csGenmeshMeshObject::GetRenderBuffer (int index)
{
  csStringID bufID = user_buffer_names[index];
  return userBuffers.GetRenderBuffer (bufID);
}

csRef<iString> csGenmeshMeshObject::GetRenderBufferName (int index) const
{
  csRef<iString> name; 
  name.AttachNew (new scfString (factory->GetStrings ()->Request 
    (user_buffer_names[index])));
  return name;
}

iMeshObjectFactory* csGenmeshMeshObject::GetFactory () const
{
  return (iMeshObjectFactory*)factory;
}

//----------------------------------------------------------------------

csGenmeshMeshObjectFactory::csGenmeshMeshObjectFactory (
  csGenmeshMeshObjectType* pParent, iObjectRegistry* object_reg) : 
  scfImplementationType (this, static_cast<iBase*> (pParent))
{
  csGenmeshMeshObjectFactory::object_reg = object_reg;

  SetPolyMeshStandard ();

  logparent = 0;
  genmesh_type = pParent;
  initialized = false;
  object_bbox_valid = false;

  material = 0;
  polygons = 0;
  light_mgr = CS_QUERY_REGISTRY (object_reg, iLightManager);
  back2front = false;
  back2front_tree = 0;

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg,
    "crystalspace.shared.stringset", iStringSet);

  mesh_vertices_dirty_flag = false;
  mesh_texels_dirty_flag = false;
  mesh_normals_dirty_flag = false;
  mesh_colors_dirty_flag = false;
  mesh_triangle_dirty_flag = false;
  mesh_tangents_dirty_flag = false;

  buffers_version = 0;

  autonormals = false;
  autonormals_compress = true;

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
  do_fullbright = (cmdline->GetOption ("fullbright") != 0);
}

csGenmeshMeshObjectFactory::~csGenmeshMeshObjectFactory ()
{
  ClearSubMeshes ();

  delete[] polygons;

  delete back2front_tree;
}

void csGenmeshMeshObjectFactory::ClearSubMeshes ()
{
  subMeshes.ClearSubMeshes ();
  SetPolyMeshStandard();
}

void csGenmeshMeshObjectFactory::AddSubMesh (unsigned int *triangles,
                                             int tricount,
                                             iMaterialWrapper *material,
				             uint mixmode)
{
  csRef<iRenderBuffer> index_buffer = 
    csRenderBuffer::CreateIndexRenderBuffer (tricount*3,
    CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT, 0, GetVertexCount () - 1);
  csTriangle *triangleData =
    (csTriangle*)index_buffer->Lock(CS_BUF_LOCK_NORMAL);

  for (int i=0; i<tricount; ++i)
  {
    triangleData[i] = GetTriangles ()[triangles[i]];
  }
  index_buffer->Release ();
  subMeshes.AddSubMesh (index_buffer, material, 0, mixmode);
  if (polyMeshType != Submeshes) SetPolyMeshSubmeshes();
}

iGeneralMeshSubMesh* csGenmeshMeshObjectFactory::AddSubMesh (
  iRenderBuffer* indices, iMaterialWrapper *material, const char* name, 
  uint mixmode)
{
  if (polyMeshType != Submeshes) SetPolyMeshSubmeshes();
  return subMeshes.AddSubMesh (indices, material, 
    genmesh_type->submeshNamePool.Register (name), 
    mixmode);
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
  if (mesh_vertices.Length () == 0)
  {
    object_bbox.Set (0, 0, 0, 0, 0, 0);
    radius = 0.0f;
    return;
  }
  csVector3& v0 = mesh_vertices[0];
  object_bbox.StartBoundingBox (v0);
  size_t i;
  for (i = 1 ; i < mesh_vertices.Length () ; i++)
  {
    csVector3& v = mesh_vertices[i];
    object_bbox.AddBoundingVertexSmart (v);
  }

  const csVector3& center = object_bbox.GetCenter ();
  float max_sqradius = 0.0f;
  for (i = 0 ; i < mesh_vertices.Length () ; i++)
  {
    csVector3& v = mesh_vertices[i];
    float sqradius = csSquaredDist::PointPoint (center, v);
    if (sqradius > max_sqradius) max_sqradius = sqradius;
  }

  radius = csQsqrt (max_sqradius);
}

float csGenmeshMeshObjectFactory::GetRadius ()
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

void csGenmeshMeshObjectFactory::SetObjectBoundingBox (const csBox3& bbox)
{
  SetupFactory ();
  object_bbox_valid = true;
  object_bbox = bbox;
}


void csGenmeshMeshObjectFactory::SetupFactory ()
{
  if (!initialized)
  {
    initialized = true;
    object_bbox_valid = false;
  }
}

void csGenmeshMeshObjectFactory::Compress ()
{
  if (subMeshes.GetSize () > 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
    	"crystalspace.genmesh.compress",
	"WARNING! Compress ignored because there are subMeshes!");
    return;
  }

  size_t old_num = mesh_vertices.Length ();
  csCompressVertexInfo* vt = csVertexCompressor::Compress (
    	mesh_vertices, mesh_texels, mesh_normals, mesh_colors);
  if (vt)
  {
    printf ("From %d to %d\n", int (old_num), int (mesh_vertices.Length ()));
    fflush (stdout);

    // Now we can remap the vertices in all triangles.
    size_t i;
    for (i = 0 ; i < mesh_triangles.Length () ; i++)
    {
      mesh_triangles[i].a = (int)vt[mesh_triangles[i].a].new_idx;
      mesh_triangles[i].b = (int)vt[mesh_triangles[i].b].new_idx;
      mesh_triangles[i].c = (int)vt[mesh_triangles[i].c].new_idx;
    }
    delete[] vt;
  }
}

void csGenmeshMeshObjectFactory::SetPolyMeshStandard ()
{
  PolyMesh* polyMesh = new PolyMesh;
  polyMesh->SetFactory (this);
  polygonMesh.AttachNew (polyMesh);
  SetPolygonMeshBase (polygonMesh);
  SetPolygonMeshColldet (polygonMesh);
  SetPolygonMeshViscull (polygonMesh);
  SetPolygonMeshShadows (polygonMesh);
  polyMeshType = Standard;
}

void csGenmeshMeshObjectFactory::SetPolyMeshSubmeshes ()
{
  polygonMesh.AttachNew (new SubMeshesPolyMesh (this, subMeshes));
  SetPolygonMeshBase (polygonMesh);
  SetPolygonMeshColldet (polygonMesh);
  SetPolygonMeshViscull (polygonMesh);
  SetPolygonMeshShadows (polygonMesh);
  polyMeshType = Submeshes;
}

void csGenmeshMeshObjectFactory::PreGetValue (
  csShaderVariable* var)
{
  iRenderBuffer *a = userBuffers.GetRenderBuffer (var->GetName());
  if (a != 0)
  {
    var->SetValue (a);
  }
}

void csGenmeshMeshObjectFactory::PreGetBuffer (csRenderBufferHolder* holder, 
					       csRenderBufferName buffer)
{
  if (!holder) return;
  if (buffer == CS_BUFFER_POSITION)
  {
    if (mesh_vertices_dirty_flag)
    {
      if (!vertex_buffer)
        vertex_buffer = csRenderBuffer::CreateRenderBuffer (
          mesh_vertices.Length (), CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3, false);
      mesh_vertices_dirty_flag = false;
      vertex_buffer->CopyInto (mesh_vertices.GetArray (),
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
      texel_buffer->CopyInto (mesh_texels.GetArray (), mesh_texels.Length ());
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
      normal_buffer->CopyInto (mesh_normals.GetArray (),
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
  if (buffer == CS_BUFFER_INDEX && !back2front)
  {
    if (mesh_triangle_dirty_flag)
    {
      if (!index_buffer)
        index_buffer = csRenderBuffer::CreateIndexRenderBuffer (
          mesh_triangles.Length ()*3, CS_BUF_STATIC,
          CS_BUFCOMP_UNSIGNED_INT, 0, mesh_vertices.Length () - 1);
      mesh_triangle_dirty_flag = false;
      index_buffer->CopyInto (mesh_triangles.GetArray (),
	  mesh_triangles.Length ()*3);
    }
    holder->SetRenderBuffer (buffer, index_buffer);
    return;
  }
}

void csGenmeshMeshObjectFactory::AddVertex (const csVector3& v,
      const csVector2& uv, const csVector3& normal,
      const csColor4& color)
{
  mesh_vertices.Push (v);
  mesh_texels.Push (uv);
  mesh_normals.Push (normal);
  mesh_colors.Push (color);
  Invalidate ();
}

void csGenmeshMeshObjectFactory::AddTriangle (const csTriangle& tri)
{
  mesh_triangles.Push (tri);
  Invalidate ();
}

void csGenmeshMeshObjectFactory::SetVertexCount (int n)
{
  mesh_vertices.SetLength (n);
  mesh_texels.SetLength (n);
  mesh_colors.SetLength (n);
  mesh_normals.SetLength (n);
  initialized = false;

  memset (mesh_normals.GetArray (), 0, sizeof (csVector3)*n);
  memset (mesh_colors.GetArray (), 0, sizeof (csColor4)*n);

  vertex_buffer = 0;
  normal_buffer = 0;
  texel_buffer = 0;
  color_buffer = 0;
  tangent_buffer = 0;
  binormal_buffer = 0;
  mesh_vertices_dirty_flag = true;
  mesh_texels_dirty_flag = true;
  mesh_normals_dirty_flag = true;
  mesh_colors_dirty_flag = true;
  mesh_tangents_dirty_flag = true;

  ShapeChanged ();
}

void csGenmeshMeshObjectFactory::SetTriangleCount (int n)
{
  mesh_triangles.SetLength (n);

  index_buffer = 0;
  mesh_triangle_dirty_flag = true;

  initialized = false;
  ShapeChanged ();
}

void csGenmeshMeshObjectFactory::CalculateNormals (bool compress)
{
  csNormalCalculator::CalculateNormals (
      mesh_vertices, mesh_triangles, mesh_normals, compress);
  autonormals = true;
  autonormals_compress = compress;
}

void csGenmeshMeshObjectFactory::GenerateSphere (const csEllipsoid& ellips,
    int num, bool cyl_mapping, bool toponly, bool reversed)
{
  csPrimitives::GenerateSphere (ellips, num, mesh_vertices, mesh_texels,
      mesh_normals, mesh_triangles, cyl_mapping, toponly, reversed);
  mesh_colors.SetLength (mesh_vertices.Length ());
  memset (mesh_colors.GetArray (), 0,
      sizeof (csColor4)*mesh_vertices.Length ());
  Invalidate();
}

//void csGenmeshMeshObjectFactory::GeneratePlane (const csPlane3& plane)
//{
//
//}
void csGenmeshMeshObjectFactory::GenerateBox (const csBox3& box)
{
  csPrimitives::GenerateBox (box, mesh_vertices, mesh_texels,
      mesh_normals, mesh_triangles);
  mesh_colors.SetLength (mesh_vertices.Length ());
  memset (mesh_colors.GetArray (), 0, sizeof (csColor4)*mesh_vertices.Length ());
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
						  iRenderBuffer* buffer)
{
  csStringID bufID = strings->Request (name);
  if (userBuffers.AddRenderBuffer (bufID, buffer))
  {
    user_buffer_names.Push (bufID);
    return true;
  }
  return false;
}

bool csGenmeshMeshObjectFactory::RemoveRenderBuffer (const char *name)
{
  csStringID bufID = strings->Request (name);
  if (userBuffers.RemoveRenderBuffer (bufID))
  {
    user_buffer_names.DeleteFast (bufID);
    return true;
  }
  return false;
}

csRef<iRenderBuffer> csGenmeshMeshObjectFactory::GetRenderBuffer (int index)
{
  csStringID bufID = user_buffer_names[index];
  return userBuffers.GetRenderBuffer (bufID);
}

csRef<iString> csGenmeshMeshObjectFactory::GetRenderBufferName (int index) const
{
  csRef<iString> name; 
  name.AttachNew (new scfString (strings->Request (user_buffer_names[index])));
  return name;
}

void csGenmeshMeshObjectFactory::Invalidate ()
{
  object_bbox_valid = false;
  delete[] polygons;
  polygons = 0;

  mesh_vertices_dirty_flag = true;
  mesh_texels_dirty_flag = true;
  mesh_normals_dirty_flag = true;
  mesh_colors_dirty_flag = true;
  mesh_triangle_dirty_flag = true;
  mesh_tangents_dirty_flag = true;

  ShapeChanged ();
}

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
  size_t i;
  for (i = 0 ; i < mesh_vertices.Length () ; i++)
  {
    mesh_vertices[i] = t.This2Other (mesh_vertices[i]);
    mesh_normals[i] = t.This2OtherRelative (mesh_normals[i]);
  }

  mesh_vertices_dirty_flag = true;

  initialized = false;
  ShapeChanged ();
}

iMeshObjectType* csGenmeshMeshObjectFactory::GetMeshObjectType () const
{
  return static_cast<iMeshObjectType*> (genmesh_type);
}

csPtr<iMeshObject> csGenmeshMeshObjectFactory::NewInstance ()
{
  csRef<csGenmeshMeshObject> cm;
  cm.AttachNew (new csGenmeshMeshObject (this));
  cm->SetMixMode (default_mixmode);
  cm->SetLighting (default_lighting);
  cm->SetColor (default_color);
  cm->SetManualColors (default_manualcolors);
  cm->SetShadowCasting (default_shadowcasting);
  cm->SetShadowReceiving (default_shadowreceiving);

  if (anim_ctrl_fact)
  {
    csRef<iGenMeshAnimationControl> anim_ctrl = anim_ctrl_fact
    	->CreateAnimationControl (cm);
    cm->SetAnimationControl (anim_ctrl);
  }

  csRef<iMeshObject> im (scfQueryInterface<iMeshObject> (cm));
  return csPtr<iMeshObject> (im);
}

csMeshedPolygon* csGenmeshMeshObjectFactory::GetPolygons ()
{
  if (!polygons)
  {

    csTriangle* triangles = mesh_triangles.GetArray ();
    polygons = new csMeshedPolygon [mesh_triangles.Length ()];
    size_t i;
    for (i = 0 ; i < mesh_triangles.Length () ; i++)
    {
      polygons[i].num_vertices = 3;
      polygons[i].vertices = &triangles[i].a;
    }
  }
  return polygons;
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY (csGenmeshMeshObjectType)

csGenmeshMeshObjectType::csGenmeshMeshObjectType (iBase* pParent) :
  scfImplementationType (this, pParent), do_verbose (false)
{
}

csGenmeshMeshObjectType::~csGenmeshMeshObjectType ()
{
}

csPtr<iMeshObjectFactory> csGenmeshMeshObjectType::NewFactory ()
{
  csRef<csGenmeshMeshObjectFactory> cm;
  cm.AttachNew (new csGenmeshMeshObjectFactory (this,
    object_reg));
  csRef<iMeshObjectFactory> ifact (
    scfQueryInterface<iMeshObjectFactory> (cm));
  return csPtr<iMeshObjectFactory> (ifact);
}

bool csGenmeshMeshObjectType::Initialize (iObjectRegistry* object_reg)
{
  csGenmeshMeshObjectType::object_reg = object_reg;

  csRef<iVerbosityManager> verbosemgr (
    csQueryRegistry<iVerbosityManager> (object_reg));
  if (verbosemgr) 
    do_verbose = verbosemgr->Enabled ("genmesh");

  return true;
}

}
CS_PLUGIN_NAMESPACE_END(Genmesh)
