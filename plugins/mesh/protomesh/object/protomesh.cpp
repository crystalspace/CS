/*
    Copyright (C) 2004 by Jorrit Tyberghein

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
#include "csgfx/renderbuffer.h"
#include "csgeom/math3d.h"
#include "csgeom/box.h"
#include "csgeom/frustum.h"
#include "csgeom/trimesh.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "iengine/camera.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/material.h"
#include "ivideo/rendermesh.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iutil/objreg.h"
#include "protomesh.h"

#include "csqsqrt.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csProtoMeshObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iProtoMeshState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csProtoMeshObject::ProtoMeshState)
  SCF_IMPLEMENTS_INTERFACE (iProtoMeshState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csProtoMeshObject::RenderBufferAccessor)
  SCF_IMPLEMENTS_INTERFACE (iRenderBufferAccessor)
SCF_IMPLEMENT_IBASE_END

csProtoMeshObject::csProtoMeshObject (csProtoMeshObjectFactory* factory)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiProtoMeshState);

  scfiRenderBufferAccessor = new RenderBufferAccessor (this);

  csProtoMeshObject::factory = factory;
  logparent = 0;
  initialized = false;

  material = 0;
  MixMode = 0;

  color.red = 0;
  color.green = 0;
  color.blue = 0;
  factory_color_nr = (uint)~0;
  mesh_colors_dirty_flag = true;

  current_lod = 1;
  current_features = 0;

  g3d = CS_QUERY_REGISTRY (factory->object_reg, iGraphics3D);

  variableContext.AttachNew (new csShaderVariableContext);
}

csProtoMeshObject::~csProtoMeshObject ()
{
  scfiRenderBufferAccessor->DecRef ();

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiProtoMeshState);
  SCF_DESTRUCT_IBASE ();
}

bool csProtoMeshObject::SetMaterialWrapper (iMaterialWrapper* mat)
{
  material = mat;
  return true;
}

void csProtoMeshObject::SetupBufferHolder ()
{
  if (bufferHolder == 0)
    bufferHolder.AttachNew (new csRenderBufferHolder);

  // Make sure the factory is ok and his its buffers.
  factory->SetupFactory ();

  // When creating buffers we basically have two ways. Either
  // we can create the buffer immediatelly and supply it in the context.
  // Or else we create an accessor for the buffer so that the
  // shader/renderer can fetch the buffer later. The first approach
  // is best in case of buffers that are almost always needed. For
  // example, the vertex positions are certainly needed in all cases.
  // The second approach is best in case of buffers that are only
  // needed in some situations. For example, normals and colors may
  // only be needed by some shaders. So delaying creation avoids
  // the creation of potentially unneeded buffers there.

  // Indices are fetched directly from the factory.
  bufferHolder->SetRenderBuffer (CS_BUFFER_INDEX, factory->index_buffer);
  
  // Vertices are fetched from the factory.
  bufferHolder->SetRenderBuffer (CS_BUFFER_POSITION, factory->vertex_buffer);

  // Texels are fetched from the factory.
  bufferHolder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, factory->texel_buffer);
  
  // Normals are fetched from the factory but we use an accessor
  // for those because they are not always needed.
  // Colors are fetched from the object because we need to add the mesh
  // base color to the static colors in the factory.
  bufferHolder->SetAccessor (scfiRenderBufferAccessor, CS_BUFFER_NORMAL_MASK | CS_BUFFER_COLOR_MASK);
}
  
void csProtoMeshObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;
    SetupBufferHolder ();
  }
}

/*
 * This function actually supplies the meshes to render to the
 * 3D renderer (which will call g3d->DrawMesh()). In this simple
 * case there is only one render mesh but more complex objects
 * can have multiple render meshes.
 */
csRenderMesh** csProtoMeshObject::GetRenderMeshes (
	int& n, iRenderView* rview, 
	iMovable* movable, uint32 frustum_mask)
{
  n = 0;

  if (vis_cb) if (!vis_cb->BeforeDrawing (this, rview)) return false;

  SetupObject ();

  iCamera* camera = rview->GetCamera ();

  int clip_portal, clip_plane, clip_z_plane;
  rview->CalculateClipSettings (frustum_mask, clip_portal, clip_plane,
      clip_z_plane);

  const csReversibleTransform o2wt = movable->GetFullTransform ();
  const csVector3& wo = o2wt.GetOrigin ();

  CS_ASSERT (material != 0);
  material->Visit ();

  factory->PrepareBuffers ();
  if (factory_color_nr != factory->color_nr)
  {
    // The factory colors have changed. Set the
    // colors dirty flag to true to force an update
    // there in the PreGetShaderVariableValue.
    factory_color_nr = factory->color_nr;
    mesh_colors_dirty_flag = true;
  }

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
  meshPtr->indexend = PROTO_TRIS * 3;	// 12 triangles.
  meshPtr->material = material;
  meshPtr->worldspace_origin = wo;
  if (rmCreated)
  {
    meshPtr->buffers = bufferHolder;
    meshPtr->variablecontext = variableContext;
  }
  meshPtr->variablecontext->GetVariableAdd (factory->string_object2world)->
    SetValue (o2wt);

  meshPtr->geometryInstance = (void*)factory;
 
  n = 1;
  return &meshPtr;
}

bool csProtoMeshObject::HitBeamOutline (const csVector3& start,
  const csVector3& end, csVector3& isect, float* pr)
{
  // This is now closer to an outline hitting method. It will
  // return as soon as it touches any triangle in the mesh, and
  // will be a bit faster than its more accurate cousin (below).

  csSegment3 seg (start, end);
  int i, max = PROTO_TRIS;
  csTriangle *tr = factory->GetTriangles();
  csVector3 *vrt = factory->GetVertices ();
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

bool csProtoMeshObject::HitBeamObject (const csVector3& start,
  const csVector3& end, csVector3& isect, float *pr, int* polygon_idx)
{
  if (polygon_idx) *polygon_idx = -1;
  // This is the slow version. Use for an accurate hit on the object.
  // It will cycle through every triangle in the mesh serching for the
  // closest intersection. Slower, but returns the closest hit.
  // Usage is optional.

  csSegment3 seg (start, end);
  int i, max = PROTO_TRIS;
  float tot_dist = csSquaredDist::PointPoint (start, end);
  float dist, temp;
  float itot_dist = 1 / tot_dist;
  dist = temp = tot_dist;
  csVector3 *vrt = factory->GetVertices (), tmp;
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
  return true;
}

iObjectModel* csProtoMeshObject::GetObjectModel ()
{
  return factory->GetObjectModel ();
}

void csProtoMeshObject::PreGetBuffer (csRenderBufferHolder *holder, csRenderBufferName buffer)
{
  if (buffer == CS_BUFFER_COLOR)
  {
    if (mesh_colors_dirty_flag)
    {
      if (!color_buffer)
      {
        // Here we create a render buffer that copies the data
	// since we don't keep a local copy of the color buffer here.
	// (final 'true' parameter).
        color_buffer = csRenderBuffer::CreateRenderBuffer (
              PROTO_VERTS, CS_BUF_STATIC,
              CS_BUFCOMP_FLOAT, 3, true);
      }
      mesh_colors_dirty_flag = false;
      const csColor* factory_colors = factory->colors;
      int i;
      csColor colors[PROTO_VERTS];
      for (i = 0 ; i < PROTO_VERTS ; i++)
        colors[i] = factory_colors[i]+color;
      color_buffer->CopyInto (colors, PROTO_VERTS);
    }
    holder->SetRenderBuffer (CS_BUFFER_COLOR, color_buffer);
  } 
  else 
  {
    factory->PreGetBuffer (holder, buffer);
  }
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csProtoMeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iProtoFactoryState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csProtoMeshObjectFactory::ProtoFactoryState)
  SCF_IMPLEMENTS_INTERFACE (iProtoFactoryState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csProtoMeshObjectFactory::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csStringID csProtoMeshObjectFactory::string_object2world = csInvalidStringID;

csProtoMeshObjectFactory::csProtoMeshObjectFactory (iMeshObjectType *pParent,
      iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiProtoFactoryState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);

  csProtoMeshObjectFactory::object_reg = object_reg;

  scfiPolygonMesh.SetFactory (this);
  scfiObjectModel.SetPolygonMeshBase (&scfiPolygonMesh);
  scfiObjectModel.SetPolygonMeshColldet (&scfiPolygonMesh);
  scfiObjectModel.SetPolygonMeshViscull (&scfiPolygonMesh);
  scfiObjectModel.SetPolygonMeshShadows (&scfiPolygonMesh);

  logparent = 0;
  proto_type = pParent;
  initialized = false;
  object_bbox_valid = false;
  color_nr = 0;

  polygons = 0;

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg,
    "crystalspace.shared.stringset", iStringSet);
  if (string_object2world == csInvalidStringID)
  {
    string_object2world = strings->Request ("object2world transform");
  }

  mesh_vertices_dirty_flag = true;
  mesh_texels_dirty_flag = true;
  mesh_normals_dirty_flag = true;
  mesh_triangle_dirty_flag = true;
}

csProtoMeshObjectFactory::~csProtoMeshObjectFactory ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiProtoFactoryState);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_DESTRUCT_IBASE ();
}

void csProtoMeshObjectFactory::CalculateBBoxRadius ()
{
  object_bbox_valid = true;
  csVector3& v0 = vertices[0];
  object_bbox.StartBoundingBox (v0);
  csVector3 max_sq_radius (v0.x*v0.x + v0.x*v0.x,
        v0.y*v0.y + v0.y*v0.y, v0.z*v0.z + v0.z*v0.z);
  int i;
  for (i = 1 ; i < PROTO_VERTS ; i++)
  {
    csVector3& v = vertices[i];
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

const csVector3& csProtoMeshObjectFactory::GetRadius ()
{
  SetupFactory ();
  if (!object_bbox_valid) CalculateBBoxRadius ();
  return radius;
}

const csBox3& csProtoMeshObjectFactory::GetObjectBoundingBox ()
{
  SetupFactory ();
  if (!object_bbox_valid) CalculateBBoxRadius ();
  return object_bbox;
}

void csProtoMeshObjectFactory::SetObjectBoundingBox (const csBox3& bbox)
{
  object_bbox_valid = true;
  object_bbox = bbox;
}

void csProtoMeshObjectFactory::SetupFactory ()
{
  if (!initialized)
  {
    initialized = true;
    object_bbox_valid = false;
    PrepareBuffers ();
  }
}

void csProtoMeshObjectFactory::PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer)
{
  if (buffer == CS_BUFFER_NORMAL)
  {
    if (mesh_normals_dirty_flag)
    {
      mesh_normals_dirty_flag = false;
      if (!normal_buffer)
      {
        // Create a buffer that doesn't copy the data.
        normal_buffer = csRenderBuffer::CreateRenderBuffer (
          PROTO_VERTS, CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3, false);
      }
      normal_buffer->CopyInto (normals, PROTO_VERTS);
    }
    holder->SetRenderBuffer (CS_BUFFER_NORMAL, normal_buffer);
  }
}

void csProtoMeshObjectFactory::Invalidate ()
{
  object_bbox_valid = false;
  delete[] polygons;
  polygons = 0;

  mesh_vertices_dirty_flag = true;
  mesh_texels_dirty_flag = true;
  mesh_normals_dirty_flag = true;
  mesh_triangle_dirty_flag = true;

  color_nr++;

  scfiObjectModel.ShapeChanged ();
}

void csProtoMeshObjectFactory::PrepareBuffers ()
{
  if (mesh_vertices_dirty_flag)
  {
    mesh_vertices_dirty_flag = false;
    if (!vertex_buffer)
    {
      // Create a buffer that doesn't copy the data.
      vertex_buffer = csRenderBuffer::CreateRenderBuffer (
          PROTO_VERTS, CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
          3);
    }
    vertex_buffer->CopyInto (vertices, PROTO_VERTS);
  }
  if (mesh_texels_dirty_flag)
  {
    mesh_texels_dirty_flag = false;
    if (!texel_buffer)
    {
      // Create a buffer that doesn't copy the data.
      texel_buffer = csRenderBuffer::CreateRenderBuffer (
          PROTO_VERTS, CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
          2);
    }
    texel_buffer->CopyInto (texels, PROTO_VERTS);
  }
  if (mesh_triangle_dirty_flag)
  {
    mesh_triangle_dirty_flag = false;
    if (!index_buffer)
      index_buffer = csRenderBuffer::CreateIndexRenderBuffer (
	PROTO_TRIS*3,
              CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT,
              0, PROTO_VERTS-1);
    index_buffer->CopyInto (triangles, PROTO_TRIS*3);
  }
}

SCF_IMPLEMENT_IBASE (csProtoMeshObjectFactory::PolyMesh)
  SCF_IMPLEMENTS_INTERFACE (iPolygonMesh)
SCF_IMPLEMENT_IBASE_END

csMeshedPolygon* csProtoMeshObjectFactory::PolyMesh::GetPolygons ()
{
  return factory->GetPolygons ();
}

csPtr<iMeshObject> csProtoMeshObjectFactory::NewInstance ()
{
  csProtoMeshObject* cm = new csProtoMeshObject (this);

  csRef<iMeshObject> im = SCF_QUERY_INTERFACE (cm, iMeshObject);
  cm->DecRef ();
  return csPtr<iMeshObject> (im);
}

csMeshedPolygon* csProtoMeshObjectFactory::GetPolygons ()
{
  if (!polygons)
  {
    polygons = new csMeshedPolygon [PROTO_TRIS];
    int i;
    for (i = 0 ; i < PROTO_TRIS ; i++)
    {
      polygons[i].num_vertices = 3;
      polygons[i].vertices = &triangles[i].a;
    }
  }
  return polygons;
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csProtoMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csProtoMeshObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csProtoMeshObjectType)


csProtoMeshObjectType::csProtoMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csProtoMeshObjectType::~csProtoMeshObjectType ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObjectFactory> csProtoMeshObjectType::NewFactory ()
{
  csProtoMeshObjectFactory* cm = new csProtoMeshObjectFactory (this,
    object_reg);
  csRef<iMeshObjectFactory> ifact (
    SCF_QUERY_INTERFACE (cm, iMeshObjectFactory));
  cm->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}

bool csProtoMeshObjectType::Initialize (iObjectRegistry* object_reg)
{
  csProtoMeshObjectType::object_reg = object_reg;
  return true;
}

