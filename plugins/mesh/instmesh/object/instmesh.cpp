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
#include "cstool/rviewclipper.h"
#include "csutil/csendian.h"
#include "csutil/csmd5.h"
#include "csutil/memfile.h"
#include "csutil/sysfunc.h"
#include "csutil/scfarray.h"
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

#include "instmesh.h"





CS_PLUGIN_NAMESPACE_BEGIN(InstMesh)
{

CS_LEAKGUARD_IMPLEMENT (csInstmeshMeshObject);
CS_LEAKGUARD_IMPLEMENT (csInstmeshMeshObject::RenderBufferAccessor);
CS_LEAKGUARD_IMPLEMENT (csInstmeshMeshObjectFactory);

csInstmeshMeshObject::csInstmeshMeshObject (csInstmeshMeshObjectFactory* factory) :
        scfImplementationType (this),
	pseudoDynInfo (29, 32)
{
  renderBufferAccessor.AttachNew (new RenderBufferAccessor (this));
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
  changenr = 0;
  do_shadows = true;
  do_shadow_rec = false;
  lighting_dirty = true;
  lighting_full_dirty = true;
  shadow_caps = false;

  dynamic_ambient_version = 0;

  bufferHolder.AttachNew (new csRenderBufferHolder);

  g3d = csQueryRegistry<iGraphics3D> (factory->object_reg);
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
  delete[] lit_fact_colors;
  delete[] static_fact_colors;

  ClearPseudoDynLights ();
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

  size_t fact_vt_len = factory->fact_vertices.GetSize ();
  size_t vt_len = fact_vt_len * instances.GetSize ();
  csVector3* fact_vertices = factory->fact_vertices.GetArray ();
  mesh_vertices.SetMinimalCapacity (vt_len);
  mesh_vertices.SetSize (0);
  csVector2* fact_texels = factory->fact_texels.GetArray ();
  mesh_texels.SetMinimalCapacity (vt_len);
  mesh_texels.SetSize (0);
  csVector3* fact_normals = factory->fact_normals.GetArray ();
  mesh_normals.SetMinimalCapacity (vt_len);
  mesh_normals.SetSize (0);
  csColor4* fact_colors = factory->fact_colors.GetArray ();
  mesh_colors.SetMinimalCapacity (vt_len);
  mesh_colors.SetSize (0);

  size_t fact_tri_len = factory->fact_triangles.GetSize ();
  size_t tri_len = fact_tri_len * instances.GetSize ();
  csTriangle* fact_triangles = factory->fact_triangles.GetArray ();
  mesh_triangles.SetMinimalCapacity (tri_len);
  mesh_triangles.SetSize (0);

  size_t i, idx;
  for (i = 0 ; i < instances.GetSize () ; i++)
  {
    // @@@ Do more optimal with array copy for texels and colors?
    const csReversibleTransform& tr = instances[i].transform;
    for (idx = 0 ; idx < fact_vt_len ; idx++)
    {
      mesh_vertices.Push (tr.This2Other (fact_vertices[idx]));
      mesh_texels.Push (fact_texels[idx]);
      mesh_normals.Push (tr.This2OtherRelative (fact_normals[idx]));
      mesh_colors.Push (fact_colors[idx]);
    }
    int mult = (int)(i * fact_vt_len);
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

iMeshObjectFactory* csInstmeshMeshObject::GetFactory () const
{
  return (iMeshObjectFactory*)factory;
}

size_t csInstmeshMeshObject::AddInstance (const csReversibleTransform& trans)
{
  csInstance inst;
  inst.transform = trans;
  ++max_instance_id;
  inst.id = max_instance_id;
  size_t idx = instances.Push (inst);
  instances_hash.Put (max_instance_id, idx);
  initialized = false;
  changenr++;
  ShapeChanged ();
  return max_instance_id;
}

void csInstmeshMeshObject::UpdateInstancesHash ()
{
  instances_hash.Empty ();
  size_t i;
  for (i = 0 ; i < instances.GetSize () ; i++)
    instances_hash.Put (instances[i].id, i);
}

void csInstmeshMeshObject::RemoveInstance (size_t id)
{
  size_t idx = instances_hash.Get (id, csArrayItemNotFound);
  if (idx == csArrayItemNotFound) return;
  instances.DeleteIndexFast (idx);
  UpdateInstancesHash ();
  initialized = false;
  changenr++;
  ShapeChanged ();
}

void csInstmeshMeshObject::RemoveAllInstances ()
{
  instances.Empty ();
  instances_hash.Empty ();
  initialized = false;
  changenr++;
  ShapeChanged ();
}

void csInstmeshMeshObject::UpdateInstanceGeometry (size_t instance_idx)
{
  if (initialized)
  {
    csVector3* fact_vertices = factory->fact_vertices.GetArray ();
    csVector3* fact_normals = factory->fact_normals.GetArray ();
    size_t fact_vt_len = factory->fact_vertices.GetSize ();
    size_t v0_id = instance_idx * fact_vt_len;

    for (size_t i = 0; i <  fact_vt_len; i++)
    {
      mesh_vertices[v0_id + i] = 
        instances[instance_idx].transform.This2Other (fact_vertices[i]);
      mesh_normals[v0_id + i] = 
        instances[instance_idx].transform.This2OtherRelative (
	    fact_normals[i]);;
    }
  }
  mesh_vertices_dirty_flag = true;
  mesh_normals_dirty_flag = true;
  changenr++;
}

void csInstmeshMeshObject::MoveInstance (size_t id,
    const csReversibleTransform& trans)
{
  size_t idx = instances_hash.Get (id, csArrayItemNotFound);
  if (idx == csArrayItemNotFound) return;
  instances[idx].transform = trans;
  UpdateInstanceGeometry (idx);
  lighting_dirty = true;
  // Don't set lighting_full_dirty to true since we only moved a single
  // instance.
  instances[idx].lighting_dirty = true;

  if (object_bbox_valid)
  {
    // Check if we have to update the bounding box.
    csBox3 fact_box = factory->GetFactoryBox ();
    fact_box.SetCenter (trans.GetOrigin ());
    // @@@ We ignore the transform here. Assuming that in general
    // the instances will be small so there is little chance for
    // error here.
    if (!object_bbox.Contains (fact_box))
    {
      object_bbox += fact_box;
      ShapeChanged ();
    }
  }
}

const csReversibleTransform& csInstmeshMeshObject::GetInstanceTransform (
    size_t id)
{
  size_t idx = instances_hash.Get (id, csArrayItemNotFound);
  if (idx != csArrayItemNotFound)
    return instances[idx].transform;
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
  size_t numcol = factory->GetVertexCount () * instances.GetSize ();
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

void csInstmeshMeshObject::CalculateBBoxRadius ()
{
  object_bbox_valid = true;
  if (instances.GetSize () == 0)
  {
    object_bbox.Set (0, 0, 0, 0, 0, 0);
    radius = 0.0f;
    return;
  }
  const csBox3& fact_box = factory->GetFactoryBox ();
  object_bbox = fact_box;
  csVector3 pos = instances[0].transform.GetOrigin ();
  object_bbox.SetCenter (pos);
  float max_sqradius = pos * pos;
  size_t i;
  for (i = 1 ; i < instances.GetSize () ; i++)
  {
    csBox3 transformed_box = fact_box;
    pos = instances[i].transform.GetOrigin ();
    transformed_box.SetCenter (pos);
    object_bbox += transformed_box;
    float sqradius = pos * pos;
    if (sqradius > max_sqradius) max_sqradius = sqradius;
  }

  radius = csQsqrt (max_sqradius);
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
  bufferHolder->SetAccessor (renderBufferAccessor, bufferMask);
}
  
void csInstmeshMeshObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;
    CalculateInstanceArrays ();
    delete[] lit_fact_colors;
    lit_fact_colors = 0;
    delete[] static_fact_colors;
    static_fact_colors = 0;
    if (!do_manual_colors)
    {
      num_lit_fact_colors = factory->fact_vertices.GetSize ()
	* instances.GetSize ();
      lit_fact_colors = new csColor4 [num_lit_fact_colors];
      size_t i;
      for (i = 0 ; i <  num_lit_fact_colors; i++)
        lit_fact_colors[i].Set (0, 0, 0);
      lighting_dirty = true;
      lighting_full_dirty = true;
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

csRenderMesh** csInstmeshMeshObject::GetRenderMeshes (
	int& n, iRenderView* rview, 
	iMovable* movable, uint32 frustum_mask)
{
  CheckLitColors ();
  SetupObject ();


  n = 0;

  if (mesh_triangles.GetSize () > 0)
  {
    iCamera* camera = rview->GetCamera ();

    int clip_portal, clip_plane, clip_z_plane;
    CS::RenderViewClipper::CalculateClipSettings (rview->GetRenderContext (),
	frustum_mask, clip_portal, clip_plane, clip_z_plane);

    lighting_movable = movable;

    if (!do_manual_colors && !do_shadow_rec && factory->light_mgr)
    {
      // Remember relevant lights for later.
      scfArrayWrap<iLightInfluenceArray, csSafeCopyArray<csLightInfluence> > 
        relevantLightsWrap (relevant_lights); //Yes, know, its on the stack...

      factory->light_mgr->GetRelevantLights (logparent, &relevantLightsWrap, -1);
    }

    const csReversibleTransform o2wt = movable->GetFullTransform ();
    const csVector3& wo = o2wt.GetOrigin ();

    // Array still needed?@@@
    {
      renderMeshes.SetSize (1);

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

      meshPtr->mixmode = CS_MIXMODE_ALPHATEST_ENABLE;
      meshPtr->clip_portal = clip_portal;
      meshPtr->clip_plane = clip_plane;
      meshPtr->clip_z_plane = clip_z_plane;
      meshPtr->do_mirror = camera->IsMirrored ();
      meshPtr->meshtype = CS_MESHTYPE_TRIANGLES;
      meshPtr->indexstart = 0;
      meshPtr->indexend = (unsigned int)(mesh_triangles.GetSize () * 3);
      meshPtr->material = mater;
      CS_ASSERT (mater != 0);
      meshPtr->worldspace_origin = wo;
      meshPtr->buffers = bufferHolder;
      meshPtr->geometryInstance = (void*)factory;
      meshPtr->object2world = o2wt;

      renderMeshes[0] = meshPtr;
    }

    n = (int)renderMeshes.GetSize ();
    return renderMeshes.GetArray ();
  }
  else 
    return 0;
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
    //const csPDelArray<csInstmeshSubMesh>& sm = subMeshes.GetSize () == 0
    	//? factory->GetSubMeshes ()
	//: subMeshes;
    //if (sm.GetSize () == 0)
    //{
      *material = csInstmeshMeshObject::material;
      if (!*material) *material = factory->GetMaterialWrapper ();
    //}
  }

  return true;
}

size_t csInstmeshMeshObject::TriMesh::GetVertexCount ()
{
  return parent->factory->GetVertexCount ();
}

csVector3* csInstmeshMeshObject::TriMesh::GetVertices ()
{
  //@@@FIXME: data must come from mesh itself. Not factory
  return 0;
  //return scfParent->factory->GetVertices ();
}

size_t csInstmeshMeshObject::TriMesh::GetTriangleCount ()
{
  //@@@FIXME: data from mesh instead of factory
  return 0;
  //return scfParent->factory->GetTriangleCount ();
}

csTriangle* csInstmeshMeshObject::TriMesh::GetTriangles ()
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
      //UpdateLighting (relevant_lights, lighting_movable);
    }
    if (mesh_colors_dirty_flag)
    {
      mesh_colors_dirty_flag = false;
      if (!do_manual_colors)
      {
        if (!color_buffer ||
          (color_buffer->GetSize() != (sizeof (csColor4) * 
          num_lit_fact_colors)))
        {
          // Recreate the render buffer only if the new data cannot fit inside
          // the existing buffer.
          color_buffer = csRenderBuffer::CreateRenderBuffer (
            num_lit_fact_colors, 
            do_lighting ? CS_BUF_DYNAMIC : CS_BUF_STATIC,
            CS_BUFCOMP_FLOAT, 4);
        }
	color_buffer->SetData (lit_fact_colors);
      }
      else
      {
	size_t numcol = factory->fact_vertices.GetSize () * instances.GetSize ();
        if (!color_buffer ||
          (color_buffer->GetSize() != (sizeof (csColor4) * numcol)))
        {
          // Recreate the render buffer only if the new data cannot fit inside
          // the existing buffer.
          color_buffer = csRenderBuffer::CreateRenderBuffer (
            numcol, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 4);
        }
	color_buffer->SetData (factory->GetColors ());
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
          mesh_vertices.GetSize (), CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3);
      mesh_vertices_dirty_flag = false;
      vertex_buffer->SetData ((void*)mesh_vertices.GetArray ());
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
          mesh_texels.GetSize (), CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 2);
      mesh_texels_dirty_flag = false;
      texel_buffer->SetData ((void*)mesh_texels.GetArray ());
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
          mesh_normals.GetSize (), CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3);
      mesh_normals_dirty_flag = false;
      normal_buffer->SetData ((void*)mesh_normals.GetArray ());
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
          mesh_vertices.GetSize (), CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3);
      if (!binormal_buffer)
        binormal_buffer = csRenderBuffer::CreateRenderBuffer (
          mesh_vertices.GetSize (), CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3);
      mesh_tangents_dirty_flag = false;

      csVector3* tangentData = new csVector3[mesh_vertices.GetSize () * 2];
      csVector3* bitangentData = tangentData + mesh_vertices.GetSize ();
      csNormalMappingTools::CalculateTangents (mesh_triangles.GetSize (), 
        mesh_triangles.GetArray (), mesh_vertices.GetSize (),
	mesh_vertices.GetArray (), mesh_normals.GetArray (), 
        mesh_texels.GetArray (), tangentData, bitangentData);

      tangent_buffer->CopyInto (tangentData, mesh_vertices.GetSize ());
      binormal_buffer->CopyInto (bitangentData, mesh_vertices.GetSize ());

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
          mesh_triangles.GetSize ()*3, CS_BUF_STATIC,
          CS_BUFCOMP_UNSIGNED_INT, 0, mesh_vertices.GetSize () - 1);
      mesh_triangle_dirty_flag = false;
      index_buffer->SetData ((void*)mesh_triangles.GetArray ());
    }
    holder->SetRenderBuffer (buffer, index_buffer);
    return;
  }
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

  autonormals = false;
  autonormals_compress = true;
  factory_bbox_valid = false;

  default_mixmode = 0;
  default_lighting = true;
  default_color.Set (0, 0, 0);
  default_manualcolors = false;
  default_shadowcasting = true;
  default_shadowreceiving = false;

  csRef<iEngine> eng = csQueryRegistry<iEngine> (object_reg);
  engine = eng; // We don't want a circular reference!

  vc = csQueryRegistry<iVirtualClock> (object_reg);

  csRef<iCommandLineParser> cmdline = csQueryRegistry<iCommandLineParser> (
  	object_reg);
  do_fullbright = (cmdline->GetOption ("fullbright") != 0);
}

csInstmeshMeshObjectFactory::~csInstmeshMeshObjectFactory ()
{
}

void csInstmeshMeshObjectFactory::CalculateBoundingVolumes ()
{
  if (factory_bbox_valid) return;
  factory_bbox_valid = true;
  size_t i;
  factory_bbox.StartBoundingBox (fact_vertices[0]);
  factory_radius = csQsqrt (fact_vertices[0] * fact_vertices[0]);
  for (i = 0 ; i < fact_vertices.GetSize () ; i++)
  {
    const csVector3& v = fact_vertices[i];
    factory_bbox.AddBoundingVertexSmart (v);
    float rad = csQsqrt (v * v);
    if (rad > factory_radius) factory_radius = rad;
  }
}

void csInstmeshMeshObjectFactory::AddVertex (const csVector3& v,
      const csVector2& uv, const csVector3& normal,
      const csColor4& color)
{
  if (fact_vertices.GetSize () == 0)
  {
    factory_bbox.StartBoundingBox (v);
    factory_radius = csQsqrt (v * v);
  }
  else
  {
    factory_bbox.AddBoundingVertexSmart (v);
    float rad = csQsqrt (v * v);
    if (rad > factory_radius) factory_radius = rad;
  }
  fact_vertices.Push (v);
  fact_texels.Push (uv);
  fact_normals.Push (normal);
  fact_colors.Push (color);
}

void csInstmeshMeshObjectFactory::Compress ()
{
  //size_t old_num = fact_vertices.GetSize ();
  csCompressVertexInfo* vt = csVertexCompressor::Compress (
    	fact_vertices, fact_texels, fact_normals, fact_colors);
  if (vt)
  {
    //printf ("From %d to %d\n", int (old_num), int (fact_vertices.GetSize ()));
    //fflush (stdout);

    // Now we can remap the vertices in all triangles.
    size_t i;
    for (i = 0 ; i < fact_triangles.GetSize () ; i++)
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
}

void csInstmeshMeshObjectFactory::HardTransform (
    const csReversibleTransform& t)
{
  size_t i;
  for (i = 0 ; i < fact_vertices.GetSize () ; i++)
  {
    fact_vertices[i] = t.This2Other (fact_vertices[i]);
    fact_normals[i] = t.This2OtherRelative (fact_normals[i]);
  }
  factory_bbox_valid = false;
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

  csRef<iMeshObject> im (scfQueryInterface<iMeshObject> (cm));
  cm->DecRef ();
  return csPtr<iMeshObject> (im);
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
