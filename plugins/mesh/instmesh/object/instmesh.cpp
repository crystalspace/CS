/*
Copyright (C) 2005 by Jorrit Tyberghein
Copyright (C) 2006 by Piotr Obrzut

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
#include "csgeom/vector3.h"
#include "csgeom/vector4.h"
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
#include "cstool/normalcalc.h"
#include "cstool/primitives.h"
#include "ivideo/rendermesh.h"

#include "instmesh.h"


CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(InstMesh)
{

static csShaderVariable dummy_variable;

csInstmeshMeshObject::csInstmeshMeshObject (csInstmeshMeshObjectFactory* factory) :
        scfImplementationType (this),
	pseudoDynInfo (29, 32),
	affecting_lights (29, 32)
{
  csInstmeshMeshObject::factory = factory;
  vc = factory->vc;
  logparent = 0;
  initialized = false;
  cur_movablenr = -1;
  material = 0;
  MixMode = 0;
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
  autobb = true;
  radius = 0;

  instance_template = (csPtr<csShaderVariable> (
      new csShaderVariable (factory->strings->Request ("instance_template"))));
  instance_template->SetType (csShaderVariable::ARRAY);
  instance_template->SetArraySize (0);

  if (factory->material)
    SetMaterialWrapper (factory->material);

  dynamic_ambient_version = 0;

  bufferHolder.AttachNew (new csRenderBufferHolder);
  instances = csHash<csInstance*> (2003);

  g3d = CS_QUERY_REGISTRY (factory->object_reg, iGraphics3D);
  object_bbox_valid = false;
}

csInstmeshMeshObject::~csInstmeshMeshObject ()
{
  delete[] lit_fact_colors;
  delete[] static_fact_colors;

  ClearPseudoDynLights ();
}
csArray<csVector4> csInstmeshMeshObject::Variable2Vectors (csShaderVariable& parameter)
{
  csArray<csVector4> vectors;
  switch (parameter.GetType ())
  {
  case csShaderVariable::COLOR:
  case csShaderVariable::VECTOR2:
  case csShaderVariable::VECTOR3:
  case csShaderVariable::VECTOR4:
    {
      csVector4 vector;
      parameter.GetValue (vector);
      vectors.Push (vector);
    }
    break;
  case csShaderVariable::MATRIX:
  case csShaderVariable::TRANSFORM:
    break;
  default:
    break;
  }
  return vectors;
}
void csInstmeshMeshObject::SetBoundingBox (const csBox3& box)
{
  autobb = false;
  object_bbox = box;
  radius = csQsqrt (csSquaredDist::PointPoint (box.Min (), box.Max ()));
  object_bbox_valid = false;
}

size_t csInstmeshMeshObject::max_instance_id = 0;

size_t csInstmeshMeshObject::AddInstancesVariable (
  const csShaderVariable& parameter)
{
  csRef<csShaderVariable> var (csPtr<csShaderVariable> (
      new csShaderVariable (parameter)));
  instance_template->AddVariableToArray (var);
  
  dummy_variable = parameter;
  csArray<csVector4> vectors = Variable2Vectors (dummy_variable);
  
  size_t s = 0;
  if (var_vect_indices.GetSize () == 0)
    var_vect_indices.Push (0);
  else
  {
    size_t s = var_vect_indices.GetSize ();
    var_vect_indices.Push (var_vect_indices[s - 1] + vectors.GetSize ());
  }

  csHash<csInstance*>::GlobalIterator it = instances.GetIterator ();
  while (it.HasNext ())
  {
    csRef<csShaderVariable> var (csPtr<csShaderVariable> (
    new csShaderVariable (parameter)));
    it.Next ()->values = vectors;
  }
  return var_vect_indices[s];
};
void csInstmeshMeshObject::SetInstanceVariable (size_t instance_id, csShaderVariable variable)
{
  csArray<csVector4> vectors = Variable2Vectors (variable);

  int var_id = -1;
  //search for variable index
  for (size_t i = 0; i < instance_template->GetArraySize (); i++)
  {
    csShaderVariable* el = instance_template->GetArrayElement (i);
    if (el->GetName () == variable.GetName ())
    {
      var_id = (int)var_vect_indices[i];
    }
  }
  if (var_id != -1)
  {
    csArray<csInstance*> inst =
      instances.GetAll<csArrayElementHandler<csInstance*>,
      CS::Memory::AllocatorMalloc > (instance_id);

    for (size_t i = 0; i < inst.GetSize (); i++)
      if (inst[i]->id == instance_id) 
      {
        for (size_t j = 0; j < vectors.GetSize (); j++)
        {
          inst[i]->values[var_id + j] = vectors[j];
        }
      }
  }
}
void csInstmeshMeshObject::SetInstanceVariable (size_t instance_id, size_t variable_id, 
    const csVector3 &variable)
{
  csArray<csInstance*> inst =
    instances.GetAll<csArrayElementHandler<csInstance*>,
    CS::Memory::AllocatorMalloc> (instance_id);

  for (size_t i = 0; i < inst.GetSize (); i++)
    if (inst[i]->id == instance_id) 
    {
      inst[i]->values[variable_id] = variable;
    }
}
const csShaderVariable& csInstmeshMeshObject::GetInstanceVariable (
  size_t instance_id, size_t variable_id)
{
  csArray<csInstance*> inst =
    instances.GetAll<csArrayElementHandler<csInstance*>,
    CS::Memory::AllocatorMalloc > (instance_id);

  for (size_t i = 0; i < inst.GetSize (); i++)
    if (inst[i]->id == instance_id) 
    {
      //vectors 2 shader variable
      csArray<csVector4> variables = inst[i]->values;
      csShaderVariable* variable = instance_template->GetArrayElement (variable_id);
      switch (variable->GetType ())
      {
      case csShaderVariable::COLOR:
      case csShaderVariable::VECTOR2:
      case csShaderVariable::VECTOR3:
        dummy_variable.SetValue (csVector3 (variables[0].x, variables[0].y, variables[0].z));
        break;
      case csShaderVariable::VECTOR4:
      case csShaderVariable::MATRIX:
      case csShaderVariable::TRANSFORM:
        break;
      default:
        break;
      }
      
      return dummy_variable;
    }

  dummy_variable = csShaderVariable ();
  return dummy_variable;
}

size_t csInstmeshMeshObject::AddInstance ()
{
  csInstance* inst = new csInstance ();
  for (size_t i = 0; i < instance_template->GetArraySize (); i++)
  {
    inst->values = Variable2Vectors (*instance_template->GetArrayElement (i));
  }
  object_bbox_valid = false;
  inst->id = max_instance_id++;
  instances.PutUnique (inst->id, inst); 
  return inst->id;
}

void csInstmeshMeshObject::RemoveInstance (size_t id)
{
  csArray<csInstance*> values =
    instances.GetAll<csArrayElementHandler<csInstance*>,
    CS::Memory::AllocatorMalloc> (id);
  for (size_t i = 0; i < values.GetSize (); i++)
    if (values[i]->id == id) 
    {
      instances.Delete (id, values[i]);
      delete values[i];
    }
   object_bbox_valid = false;
}

void csInstmeshMeshObject::RemoveAllInstances ()
{
  csHash<csInstance*>::GlobalIterator it = instances.GetIterator ();
  while (it.HasNext ())
  {
    delete it.Next ();
  }
  instances.Empty ();
  object_bbox_valid = false;
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
  //if (do_manual_colors) return;
  //size_t numcol = factory->GetVertexCount () * instances.Length ();
  //if (numcol != num_lit_fact_colors)
  //{
  //  ClearPseudoDynLights ();

  //  num_lit_fact_colors = numcol;
  //  delete[] lit_fact_colors;
  //  lit_fact_colors = new csColor4 [num_lit_fact_colors];
  //  delete[] static_fact_colors;
  //  static_fact_colors = new csColor4 [num_lit_fact_colors];
  //}
}

void csInstmeshMeshObject::InitializeDefault (bool clear)
{
  SetupObject ();

  if (!do_shadow_rec) return;
  if (do_manual_colors) return;

  // Set all colors to ambient light.
  size_t i;
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

  if (instance_template->GetArraySize () > 0)
    if (instance_template->GetArrayElement (0)->GetType () ==
      csShaderVariable::TRANSFORM)
    {
      object_bbox.StartBoundingBox ();
      
      csHash<csInstance*>::GlobalIterator it = instances.GetIterator ();
      while (it.HasNext ())
      {
        csInstance* inst = it.Next ();
        csMatrix3 mat (inst->values[0].x, inst->values[0].y, inst->values[0].z,
          inst->values[1].x, inst->values[1].y, inst->values[1].z,
          inst->values[2].x, inst->values[2].y, inst->values[2].z);
        csVector3 vect (inst->values[3].x, inst->values[3].y, inst->values[3].z);
        csReversibleTransform trans (mat, vect);

        object_bbox.AddBoundingVertex (trans.Other2ThisRelative (factory->factory_bbox.Min ()));
        object_bbox.AddBoundingVertex (trans.Other2ThisRelative (factory->factory_bbox.Max ()));
      }
      
      float max_sqradius = 0.0f;
      const csVector3& center = object_bbox.GetCenter ();
      it = instances.GetIterator ();
      while (it.HasNext ())
      {
        csInstance* inst = it.Next ();
        csMatrix3 mat (inst->values[0].x, inst->values[0].y, inst->values[0].z,
          inst->values[1].x, inst->values[1].y, inst->values[1].z,
          inst->values[2].x, inst->values[2].y, inst->values[2].z);
        csVector3 vect (inst->values[3].x, inst->values[3].y, inst->values[3].z);
        csReversibleTransform trans (mat, vect);

        float sqradius = csSquaredDist::PointPoint (center, factory->factory_bbox.Min ());
        if (sqradius > max_sqradius) max_sqradius = sqradius;
        sqradius = csSquaredDist::PointPoint (center, factory->factory_bbox.Max ());
        if (sqradius > max_sqradius) max_sqradius = sqradius;
      }
      radius = csQsqrt (max_sqradius);
    }
}

float csInstmeshMeshObject::GetRadius ()
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
      size_t v;
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
        l->AddAffectedLightingInfo ((iLightingInfo*)this);

        csShadowArray* shadowArr = new csShadowArray();
        float* intensities = new float[num_lit_fact_colors];
        shadowArr->shadowmap = intensities;
        for (size_t n = 0; n < num_lit_fact_colors; n++)
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
  for (size_t v = 0; v < num_lit_fact_colors; v++)
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
    for (size_t n = 0; n < num_lit_fact_colors; n++)
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

void csInstmeshMeshObject::DisconnectAllLights ()
{
  csSet<csPtrKey<iLight> >::GlobalIterator it = affecting_lights.
    GetIterator ();
  while (it.HasNext ())
  {
    iLight* l = (iLight*)it.Next ();
    l->RemoveAffectedLightingInfo ((iLightingInfo*)this);
  }
  affecting_lights.Empty ();
  lighting_dirty = true;
}

#define SHADOW_CAST_BACKFACE

void csInstmeshMeshObject::AppendShadows (iMovable* movable,
                                           iShadowBlockList* shadows, const csVector3& origin)
{
  if (!do_shadows) return;
  size_t tri_num = factory->GetTriangleCount ();
  const csVector3* vt = factory->GetVertices ();
  size_t vt_num = factory->GetVertexCount ();
  const csVector3* vt_world, * vt_array_to_delete;
  size_t i;
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

  iShadowBlock *list = shadows->NewShadowBlock ((int)tri_num);
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
void csInstmeshMeshObject::SetupShaderParams ()
{
  csHash<csRef<iShader>, csStringID>::GlobalIterator it = 
    material->GetMaterial ()->GetShaders ().GetIterator ();
  while (it.HasNext ())
  {
    csRef<iShader> shader = it.Next ();
    shader->AddVariable (instance_template);
  }
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
  bufferHolder->SetAccessor (factory->renderBufferAccessor, bufferMask);
}

void csInstmeshMeshObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;
    delete[] lit_fact_colors;
    lit_fact_colors = 0;
    //if (!do_manual_colors)
    //{
    //  num_lit_fact_colors = factory->fact_vertices.Length ()
    //    * instances.Length ();
    //  lit_fact_colors = new csColor4 [num_lit_fact_colors];
    //  size_t i;
    //  for (i = 0 ; i <  num_lit_fact_colors; i++)
    //    lit_fact_colors[i].Set (0, 0, 0);
    //  lighting_dirty = true;
    //  static_fact_colors = new csColor4 [num_lit_fact_colors];
    //  for (i = 0 ; i <  num_lit_fact_colors; i++)
    //    //static_fact_colors[i] = base_color;	// Initialize to base color.
    //    static_fact_colors[i].Set (0, 0, 0);
    //}
    iMaterialWrapper* mater = material;
    if (!mater) mater = factory->GetMaterialWrapper ();
    CS_ASSERT (mater != 0);
    material_needs_visit = mater->IsVisitRequired ();

    SetupShaderParams ();
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
      li->AddAffectedLightingInfo ((iLightingInfo*)this);
      if (li->GetDynamicType () != CS_LIGHT_DYNAMICTYPE_PSEUDO)
        affecting_lights.Add (li);
    }
  }
  else
  {
    if (!affecting_lights.In (li))
    {
      li->AddAffectedLightingInfo ((iLightingInfo*)this);
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
  for (size_t i = 0 ; i < factory->GetVertexCount () ; i++)
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
  //const csVector3* normals = fact_normals.GetArray ();
  //csColor4* colors = lit_fact_colors;
  //// Compute light position in object coordinates
  //csVector3 wor_light_pos = li->GetMovable ()->GetFullPosition ();
  //csVector3 obj_light_pos = trans.Other2This (wor_light_pos);
  //float obj_sq_dist = csSquaredDist::PointPoint (obj_light_pos, 0);
  //if (obj_sq_dist >= csSquare (li->GetCutoffDistance ())) return;
  //float in_obj_dist =
  //  (obj_sq_dist >= SMALL_EPSILON) ? csQisqrt (obj_sq_dist) : 1.0f;

  //csColor light_color = li->GetColor () * (256. / CS_NORMAL_LIGHT_LEVEL)
  //  * li->GetBrightnessAtDistance (csQsqrt (obj_sq_dist));
  //if (light_color.red < EPSILON && light_color.green < EPSILON
  //  && light_color.blue < EPSILON)
  //  return;

  //csColor col;
  //size_t i;
  //size_t numcol = factory->GetVertexCount () * instances.Length ();
  //if (obj_sq_dist < SMALL_EPSILON)
  //{
  //  for (i = 0 ; i < numcol ; i++)
  //  {
  //    colors[i] += light_color;
  //  }
  //}
  //else
  //{
  //  obj_light_pos *= in_obj_dist;
  //  for (i = 0 ; i < numcol ; i++)
  //  {
  //    float cosinus = obj_light_pos * normals[i];
  //    // because the vector from the object center to the light center
  //    // in object space is equal to the position of the light

  //    if (cosinus > 0)
  //    {
  //      col = light_color;
  //      if (cosinus < 1) col *= cosinus;
  //      colors[i] += col;
  //    }
  //  }
  //}
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
  //size_t i;
  //if (cur_movablenr != movable->GetUpdateNumber ())
  //{
  //  lighting_dirty = true;
  //  cur_movablenr = movable->GetUpdateNumber ();
  //}

  //if (factory->DoFullBright ())
  //{
  //  size_t numcol = factory->GetVertexCount () * instances.Length ();
  //  lighting_dirty = false;
  //  for (i = 0 ; i < numcol ; i++)
  //  {
  //    lit_fact_colors[i].Set (1, 1, 1);
  //  }
  //  return;
  //}

  //if (do_manual_colors) return;

  //const csColor4* colors_ptr = mesh_colors.GetArray ();

  //if (do_lighting)
  //{
  //  if (!lighting_dirty)
  //  {
  //    iSector* sect = movable->GetSectors ()->Get (0);
  //    if (dynamic_ambient_version == sect->GetDynamicAmbientVersion ())
  //      return;
  //    dynamic_ambient_version = sect->GetDynamicAmbientVersion ();
  //  }
  //  lighting_dirty = false;
  //  mesh_colors_dirty_flag = true;

  //  csColor col;
  //  if (factory->engine)
  //  {
  //    factory->engine->GetAmbientLight (col);
  //    col += base_color;
  //    iSector* sect = movable->GetSectors ()->Get (0);
  //    if (sect)
  //      col += sect->GetDynamicAmbientLight ();
  //  }
  //  else
  //  {
  //    col = base_color;
  //  }
  //  size_t numcol = factory->GetVertexCount () * instances.Length ();
  //  for (i = 0 ; i < numcol ; i++)
  //  {
  //    lit_fact_colors[i] = col + static_fact_colors[i] + colors_ptr[i];
  //  }
  //  if (do_shadow_rec)
  //  {
  //    csReversibleTransform trans = movable->GetFullTransform ();
  //    csSet<csPtrKey<iLight> >::GlobalIterator it = affecting_lights.
  //      GetIterator ();
  //    while (it.HasNext ())
  //    {
  //      iLight* l = (iLight*)it.Next ();
  //      UpdateLightingOne (trans, l);
  //    }
  //    csHash<csShadowArray*, csPtrKey<iLight> >::GlobalIterator pdlIt =
  //      pseudoDynInfo.GetIterator ();
  //    while (pdlIt.HasNext ())
  //    {
  //      csPtrKey<iLight> l;
  //      csShadowArray* shadowArr = pdlIt.Next (l);
  //      csColor c = l->GetColor ();
  //      if (c.red > EPSILON || c.green > EPSILON || c.blue > EPSILON)
  //      {
  //        c = c * (256. / CS_NORMAL_LIGHT_LEVEL);
  //        float* intensities = shadowArr->shadowmap;
  //        for (size_t i = 0; i < num_lit_fact_colors; i++)
  //        {
  //          lit_fact_colors[i] += c * intensities[i];
  //        }
  //      }
  //    }
  //  }
  //  else
  //  {
  //    // Do the lighting.
  //    csReversibleTransform trans = movable->GetFullTransform ();
  //    // the object center in world coordinates. "0" because the object
  //    // center in object space is obviously at (0,0,0).
  //    size_t num_lights = lights.Length ();
  //    for (size_t l = 0 ; l < num_lights ; l++)
  //    {
  //      iLight* li = lights[l]->GetLight ();
  //      li->AddAffectedLightingInfo ((iLightingInfo*)this);
  //      affecting_lights.Add (li);
  //      UpdateLightingOne (trans, li);
  //    }
  //  }
  //  // @@@ Try to avoid this loop!
  //  // Clamp all vertex colors to 2.
  //  for (i = 0 ; i < numcol ; i++)
  //    lit_fact_colors[i].Clamp (2., 2., 2.);
  //}
  //else
  //{
  //  if (!lighting_dirty)
  //    return;
  //  lighting_dirty = false;
  //  mesh_colors_dirty_flag = true;

  //  size_t numcol = factory->GetVertexCount () * instances.Length ();
  //  for (i = 0 ; i < numcol ; i++)
  //  {
  //    lit_fact_colors[i] = base_color + colors_ptr[i];
  //    lit_fact_colors[i].Clamp (2., 2., 2.);
  //  }
  //}
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

  renderMeshes.SetLength (1);

  iMaterialWrapper* mater = material;
  if (!mater) mater = factory->GetMaterialWrapper ();
  if (!mater)
  {
    csPrintf ("INTERNAL ERROR: mesh used without material!\n");
    return 0;
  }

  if (mater->IsVisitRequired ())
  {
    mater->Visit ();
  }

  bool rmCreated;
  csRenderMesh*& meshPtr = rmHolder.GetUnusedMesh (rmCreated,
    rview->GetCurrentFrameNumber ());

  meshPtr->mixmode = CS_MIXMODE_ALPHATEST_ENABLE;
  meshPtr->clip_portal = clip_portal;
  meshPtr->clip_plane = clip_plane;
  meshPtr->clip_z_plane = clip_z_plane;
  meshPtr->do_mirror = camera->IsMirrored ();
  meshPtr->meshtype = CS_MESHTYPE_TRIANGLES;
  meshPtr->indexstart = 0;
  meshPtr->indexend = (unsigned int)(factory->fact_triangles.Length () * 3);
  meshPtr->material = mater;
  CS_ASSERT (mater != 0);
  meshPtr->worldspace_origin = wo;
  meshPtr->buffers = bufferHolder;
  meshPtr->geometryInstance = (void*)factory;
  meshPtr->object2world = o2wt;
  meshPtr->instances = instances;

  renderMeshes[0] = meshPtr;

  n = 1;
  return &meshPtr;

}

void csInstmeshMeshObject::GetRadius (float& rad, csVector3& cent)
{
  rad = GetRadius ();
  cent = object_bbox.GetCenter ();
}

bool csInstmeshMeshObject::HitBeamOutline (const csVector3& start,
                                            const csVector3& end, csVector3& isect, float* pr)
{
  // This is now closer to an outline hitting method. It will
  // return as soon as it touches any triangle in the mesh, and
  // will be a bit faster than its more accurate cousin (below).

  csSegment3 seg (start, end);
  size_t i, max = factory->GetTriangleCount();
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
  size_t i, max = factory->GetTriangleCount();
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
        if (polygon_idx) *polygon_idx = (int)i;
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
  return (int)parent->factory->GetVertexCount ();
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

//----------------------------------------------------------------------
csInstmeshMeshObjectFactory::csInstmeshMeshObjectFactory (
  iMeshObjectType *pParent, iObjectRegistry* object_reg) : 
  scfImplementationType (this, (iBase*)pParent)
{
  csInstmeshMeshObjectFactory::object_reg = object_reg;

  logparent = 0;
  instmesh_type = pParent;

  material = 0;
  light_mgr = csQueryRegistry<iLightManager> (object_reg);

  g3d = csQueryRegistry<iGraphics3D> (object_reg);
  strings = csQueryRegistryTagInterface<iStringSet> (object_reg,
    "crystalspace.shared.stringset");

  autonormals = false;
  autonormals_compress = true;

  default_mixmode = 0;
  default_lighting = true;
  default_color.Set (0, 0, 0);
  default_manualcolors = false;
  default_shadowcasting = true;
  default_shadowreceiving = false;

  vertices_dirty_flag = true;
  texels_dirty_flag = true;
  normals_dirty_flag = true;
  triangle_dirty_flag = true;
  tangents_dirty_flag = true;

  csRef<iEngine> eng = csQueryRegistry<iEngine> (object_reg);
  engine = eng; // We don't want a circular reference!

  vc = csQueryRegistry<iVirtualClock> (object_reg);

  csRef<iCommandLineParser> cmdline = csQueryRegistry<iCommandLineParser> (
  	object_reg);
  do_fullbright = (cmdline->GetOption ("fullbright") != 0);

  renderBufferAccessor.AttachNew (new RenderBufferAccessor (this));
}

csInstmeshMeshObjectFactory::~csInstmeshMeshObjectFactory ()
{
}

void csInstmeshMeshObjectFactory::CalculateBoundingVolumes ()
{
  size_t i;
  factory_bbox.StartBoundingBox (fact_vertices[0]);
  factory_radius = csQsqrt (csSquaredDist::PointPoint (
    fact_vertices[0], csVector3 (0)));
  for (i = 0 ; i < fact_vertices.Length () ; i++)
  {
    const csVector3& v = fact_vertices[i];
    factory_bbox.AddBoundingVertexSmart (v);
    float rad = csQsqrt (csSquaredDist::PointPoint (v,
      csVector3 (0)));
    if (rad > factory_radius) factory_radius = rad;
  }
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

void csInstmeshMeshObjectFactory::CalculateNormals (bool compress)
{
  csNormalCalculator::CalculateNormals (
    fact_vertices, fact_triangles, fact_normals, compress);
  autonormals = true;
  autonormals_compress = compress;
  CalculateBoundingVolumes ();
}

void csInstmeshMeshObjectFactory::GenerateSphere (const csEllipsoid& sphere,
                                                   int num, bool cyl_mapping, bool toponly, bool reversed)
{
  csPrimitives::GenerateSphere (sphere, num, fact_vertices, fact_texels,
    fact_normals, fact_triangles, cyl_mapping, toponly, reversed);
  fact_colors.SetLength (fact_vertices.Length ());
  memset (fact_colors.GetArray (), 0, sizeof (csColor4)*fact_vertices.Length ());
  CalculateBoundingVolumes ();
}
void csInstmeshMeshObjectFactory::GenerateQuad (const csVector3& v1, const csVector3& v2, 
                                                 const csVector3& v3, const csVector3& v4)
{
  csPrimitives::GenerateQuad (v1, v2, v3, v4, fact_vertices, fact_texels,
    fact_normals, fact_triangles);
  fact_colors.SetLength (fact_vertices.Length ());
  memset (fact_colors.GetArray (), 0, sizeof (csColor4)*fact_vertices.Length ());
}
void csInstmeshMeshObjectFactory::GenerateBox (const csBox3& box)
{
  csPrimitives::GenerateBox (box, fact_vertices, fact_texels,
    fact_normals, fact_triangles);
  fact_colors.SetLength (fact_vertices.Length ());
  memset (fact_colors.GetArray (), 0, sizeof (csColor4)*fact_vertices.Length ());
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

void csInstmeshMeshObjectFactory::PreGetBuffer (csRenderBufferHolder* holder, 
                                                 csRenderBufferName buffer)
{
  if (!holder) return;

  if (buffer == CS_BUFFER_COLOR)
  {
    //if (!do_manual_colors)
    //{
    //  UpdateLighting (relevant_lights, lighting_movable);
    //}
    //if (mesh_colors_dirty_flag)
    //{
    //if (!do_manual_colors)
    //{
    //  if (!color_buffer ||
    //    (color_buffer->GetSize() != (sizeof (csColor4) * 
    //    num_lit_fact_colors)))
    //  {
    //    // Recreate the render buffer only if the new data cannot fit inside
    //    //  the existing buffer.
    //    color_buffer = csRenderBuffer::CreateRenderBuffer (
    //      num_lit_fact_colors, 
    //      do_lighting ? CS_BUF_DYNAMIC : CS_BUF_STATIC,
    //      CS_BUFCOMP_FLOAT, 4, false);
    //  }
    //  mesh_colors_dirty_flag = false;
    //  const csColor4* fact_colors = 0;
    //  fact_colors = lit_fact_colors;

    //  color_buffer->CopyInto (fact_colors, num_lit_fact_colors);
    //}
    //else
    //{
    //  size_t numcol = factory->fact_vertices.Length () * instances.Length ();
    //  if (!color_buffer || 
    //    (color_buffer->GetSize() != (sizeof (csColor4) * numcol)))
    //  {
    //    // Recreate the render buffer only if the new data cannot fit inside
    //    //  the existing buffer.
    //    color_buffer = csRenderBuffer::CreateRenderBuffer (
    //      numcol, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 4, false);
    //  }
    //  mesh_colors_dirty_flag = false;
    //  const csColor4* fact_colors = 0;
    //  fact_colors = factory->GetColors ();
    //  color_buffer->CopyInto (fact_colors, numcol);
    //}
    //}
    //    holder->SetRenderBuffer (buffer, color_buffer);
    return;
  }
  else if (buffer == CS_BUFFER_POSITION)
  {
    if (vertices_dirty_flag)
    {
      if (!vertex_buffer)
        vertex_buffer = csRenderBuffer::CreateRenderBuffer (
        fact_vertices.Length (), CS_BUF_STATIC,
        CS_BUFCOMP_FLOAT, 3, false);
      vertices_dirty_flag = false;
      vertex_buffer->CopyInto ((void*)fact_vertices.GetArray (),
        fact_vertices.Length ());
    }
    holder->SetRenderBuffer (buffer, vertex_buffer);
    return;
  }
  else if (buffer == CS_BUFFER_TEXCOORD0) 
  {
    if (texels_dirty_flag)
    {
      if (!texel_buffer)
        texel_buffer = csRenderBuffer::CreateRenderBuffer (
        fact_texels.Length (), CS_BUF_STATIC,
        CS_BUFCOMP_FLOAT, 2, false);
      texels_dirty_flag = false;
      texel_buffer->CopyInto ((void*)fact_texels.GetArray (),
        fact_texels.Length ());
    }
    holder->SetRenderBuffer (buffer, texel_buffer);
    return;
  }
  else if (buffer == CS_BUFFER_NORMAL)
  {
    if (normals_dirty_flag)
    {
      if (!normal_buffer)
        normal_buffer = csRenderBuffer::CreateRenderBuffer (
        fact_normals.Length (), CS_BUF_STATIC,
        CS_BUFCOMP_FLOAT, 3, false);
      normals_dirty_flag = false;
      normal_buffer->CopyInto ((void*)fact_normals.GetArray (),
        fact_normals.Length ());
    }
    holder->SetRenderBuffer (buffer, normal_buffer);
    return;
  }
  else if (buffer == CS_BUFFER_TANGENT || buffer == CS_BUFFER_BINORMAL) 
  {
    if (tangents_dirty_flag)
    {
      if (!tangent_buffer)
        tangent_buffer = csRenderBuffer::CreateRenderBuffer (
        fact_vertices.Length (), CS_BUF_STATIC,
        CS_BUFCOMP_FLOAT, 3);
      if (!binormal_buffer)
        binormal_buffer = csRenderBuffer::CreateRenderBuffer (
        fact_vertices.Length (), CS_BUF_STATIC,
        CS_BUFCOMP_FLOAT, 3);
      tangents_dirty_flag = false;

      csVector3* tangentData = new csVector3[fact_vertices.Length () * 2];
      csVector3* bitangentData = tangentData + fact_vertices.Length ();
      csNormalMappingTools::CalculateTangents (fact_triangles.Length (), 
        fact_triangles.GetArray (), fact_vertices.Length (),
        fact_vertices.GetArray (), fact_normals.GetArray (), 
        fact_texels.GetArray (), tangentData, bitangentData);

      tangent_buffer->CopyInto (tangentData, fact_vertices.Length ());
      binormal_buffer->CopyInto (bitangentData, fact_vertices.Length ());

      delete[] tangentData;
    }
    holder->SetRenderBuffer (buffer, (buffer == CS_BUFFER_TANGENT) ?
tangent_buffer : binormal_buffer);
    return;
  }
  else if (buffer == CS_BUFFER_INDEX)
  {
    if (triangle_dirty_flag)
    {
      if (!index_buffer)
        index_buffer = csRenderBuffer::CreateIndexRenderBuffer (
        fact_triangles.Length ()*3, CS_BUF_STATIC,
        CS_BUFCOMP_UNSIGNED_INT, 0, fact_vertices.Length () - 1);
      triangle_dirty_flag = false;
      index_buffer->CopyInto ((void*)fact_triangles.GetArray (),
        fact_triangles.Length ()*3);
    }
    holder->SetRenderBuffer (buffer, index_buffer);
    return;
  }
}

//----------------------------------------------------------------------
SCF_IMPLEMENT_FACTORY (csInstmeshMeshObjectType)


csInstmeshMeshObjectType::csInstmeshMeshObjectType (iBase* pParent) :
  scfImplementationType (this, pParent)
{
  do_verbose = false;
}

csInstmeshMeshObjectType::~csInstmeshMeshObjectType ()
{
}

csPtr<iMeshObjectFactory> csInstmeshMeshObjectType::NewFactory ()
{
  csRef<csInstmeshMeshObjectFactory> cm;
  cm.AttachNew (new csInstmeshMeshObjectFactory (this,
    object_reg));
  csRef<iMeshObjectFactory> ifact (
    scfQueryInterface<iMeshObjectFactory> (cm));
  return csPtr<iMeshObjectFactory> (ifact);
}

bool csInstmeshMeshObjectType::Initialize (iObjectRegistry* object_reg)
{
  csInstmeshMeshObjectType::object_reg = object_reg;

  csRef<iVerbosityManager> verbosemgr (
    csQueryRegistry<iVerbosityManager> (object_reg));
  if (verbosemgr) 
    do_verbose = verbosemgr->Enabled ("instmesh");

  return true;
}

}
CS_PLUGIN_NAMESPACE_END(InstMesh)
