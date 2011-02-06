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
#include "csgfx/trianglestream.h"
#include "csgfx/vertexlistwalker.h"
#include "csutil/csendian.h"
#include "csutil/csmd5.h"
#include "csutil/memfile.h"
#include "csutil/scfstr.h"
#include "csutil/sysfunc.h"
#include "cstool/rbuflock.h"
#include "cstool/rviewclipper.h"

#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
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
#include "csgeom/poly3d.h"
#include "ivaria/decal.h"

#include "genmesh.h"



CS_PLUGIN_NAMESPACE_BEGIN(Genmesh)
{

CS_LEAKGUARD_IMPLEMENT (csGenmeshMeshObject);
CS_LEAKGUARD_IMPLEMENT (csGenmeshMeshObjectFactory);
CS_LEAKGUARD_IMPLEMENT (csGenmeshMeshObject::RenderBufferAccessor);
CS_LEAKGUARD_IMPLEMENT (csGenmeshMeshObjectFactory::RenderBufferAccessor);

csGenmeshMeshObject::csGenmeshMeshObject (csGenmeshMeshObjectFactory* factory) :
        scfImplementationType (this), factorySubMeshesChangeNum (~0)
{
  shaderVariableAccessor.AttachNew (new ShaderVariableAccessor (this));
  renderBufferAccessor.AttachNew (new RenderBufferAccessor (this));
  csGenmeshMeshObject::factory = factory;
  vc = factory->vc;
  logparent = 0;
  initialized = false;
  cur_movablenr = -1;
  //material = 0;
  //MixMode = 0;
  do_manual_colors = false;
  base_color.red = 0;
  base_color.green = 0;
  base_color.blue = 0;
  current_lod = 1;
  current_features = 0;
  factory_user_rb_state = 0;
  mesh_user_rb_dirty_flag = false;

  anim_ctrl_verts = false;
  anim_ctrl_texels = false;
  anim_ctrl_normals = false;
  anim_ctrl_colors = false;
  anim_ctrl_bbox = false;

  subMeshes.GetDefaultSubmesh()->parentSubMesh =
    factory->subMeshes.GetDefaultSubmesh();

  svcontext.AttachNew (new csShaderVariableContext);
  bufferHolder.AttachNew (new csRenderBufferHolder);

  g3d = csQueryRegistry<iGraphics3D> (factory->object_reg);
  buffers_version = (uint)-1;
  mesh_colors_dirty_flag = true;
}

csGenmeshMeshObject::~csGenmeshMeshObject ()
{
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

const csBox3& csGenmeshMeshObject::AnimControlGetBbox ()
{
  return anim_ctrl->UpdateBoundingBox (vc->GetCurrentTicks (),
				       factory->GetShapeNumber (),
				       factory->GetObjectBoundingBox ());
}

const csBox3* csGenmeshMeshObject::AnimControlGetBboxes ()
{
  return anim_ctrl->UpdateBoundingBoxes (vc->GetCurrentTicks (),
					 factory->GetShapeNumber ());
}

const float csGenmeshMeshObject::AnimControlGetRadius ()
{
  return anim_ctrl->UpdateRadius (vc->GetCurrentTicks (),
				  factory->GetShapeNumber (),
				  factory->GetRadius ());
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
    anim_ctrl_bbox = ac->AnimatesBBoxRadius ();
    
    // small hack to force the initialization of the animation
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
    anim_ctrl_bbox = false;
  }
  SetupShaderVariableContext ();

  // Initialize the object model
  if (anim_ctrl_bbox)
    objectModel.AttachNew (new csAnimatedModel (this));
  else
    objectModel = (iObjectModel*) nullptr;
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
      SubMeshProxiesContainer newSubMeshes (subMeshes.GetDefaultSubmesh());
      for (size_t i = 0; i < sm.GetSize(); i++)
      {
        const char* name = sm[i]->GetName();
        csRef<SubMeshProxy> proxy = subMeshes.FindSubMesh (name);
        if (!proxy.IsValid())
          proxy.AttachNew (new SubMeshProxy);
        proxy->parentSubMesh = sm[i];
        // Exploit fact that factory SMs are sorted already
        newSubMeshes.Push (proxy);
      }
      subMeshes = newSubMeshes;
    }
    factorySubMeshesChangeNum = sm.GetChangeNum();
  }
}

bool csGenmeshMeshObject::SetMaterialWrapper (iMaterialWrapper* mat)
{
  UpdateSubMeshProxies ();
  subMeshes.SetMaterialWrapper (mat);
  return true;
}

void csGenmeshMeshObject::SetupShaderVariableContext ()
{
  csShaderVariable* sv;
  
  uint bufferMask = (uint)CS_BUFFER_ALL_MASK;

  size_t i;
  iShaderVarStringSet* strings = factory->GetSVStrings();
  const csArray<CS::ShaderVarStringID>& factoryUBNs = factory->GetUserBufferNames();
  // Set up factorys user buffers...
  for (i = 0; i < factoryUBNs.GetSize (); i++)
  {
    const CS::ShaderVarStringID userBuf = factoryUBNs.Get(i);
    const char* bufName = strings->Request (userBuf);
    csRenderBufferName userName = 
      csRenderBuffer::GetBufferNameFromDescr (bufName);
    if (userName >= CS_BUFFER_INDEX)
    {
      bufferHolder->SetRenderBuffer (userName, 
	factory->GetUserBuffers().GetRenderBuffer (userBuf));
      bufferMask &= ~CS_BUFFER_MAKE_MASKABLE(userName);
    }
    else
    {
      sv = svcontext->GetVariableAdd (userBuf);
      sv->SetAccessor (factory->shaderVariableAccessor);
    }
  }
  // Set up meshs user buffers...
  for (i = 0; i < user_buffer_names.GetSize (); i++)
  {
    const CS::ShaderVarStringID userBuf = user_buffer_names.Get(i);
    const char* bufName = strings->Request (userBuf);
    csRenderBufferName userName = 
      csRenderBuffer::GetBufferNameFromDescr (bufName);
    if (userName >= CS_BUFFER_INDEX)
    {
      bufferHolder->SetRenderBuffer (userName, 
	userBuffers.GetRenderBuffer (userBuf));
      bufferMask &= ~CS_BUFFER_MAKE_MASKABLE(userName);
    }
    else
    {
      sv = svcontext->GetVariableAdd (userBuf);
      sv->SetAccessor (shaderVariableAccessor);
    }
  }
  
  /* Set accessor flags for animated buffers again since they might have
     been masked out */
  if (anim_ctrl_verts) bufferMask |= CS_BUFFER_POSITION_MASK;
  if (anim_ctrl_texels) bufferMask |= CS_BUFFER_TEXCOORD0_MASK;
  if (anim_ctrl_normals) bufferMask |= CS_BUFFER_NORMAL_MASK;
  if (!do_manual_colors || anim_ctrl_colors) bufferMask |= CS_BUFFER_COLOR_MASK;
  
  bufferHolder->SetAccessor (renderBufferAccessor, bufferMask);
}
  
void csGenmeshMeshObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;
    legacyLighting.Free();
    if (!do_manual_colors)
    {
      legacyLighting.SetColorNum (factory->GetVertexCount ());
      legacyLighting.Clear(base_color);
    }
    /*iMaterialWrapper* mater = material;
    if (!mater) mater = factory->GetMaterialWrapper ();
    material_needs_visit = mater ? mater->IsVisitRequired () : false;*/
    SetupShaderVariableContext ();
  }
  if (factory->user_buffer_change != factory_user_rb_state || mesh_user_rb_dirty_flag)
  {
    mesh_user_rb_dirty_flag = false;
    factory_user_rb_state = factory->user_buffer_change;
    SetupShaderVariableContext ();
  }
}

iRenderBuffer* csGenmeshMeshObject::GetPositions ()
{
  if (anim_ctrl)
  {
    // If we have an animation control then we must get the vertex data
    // here.
    int num_mesh_vertices = factory->GetVertexCount ();
    if (!animBuffers.position)
      animBuffers.position = csRenderBuffer::CreateRenderBuffer (
	num_mesh_vertices, CS_BUF_STATIC,
	CS_BUFCOMP_FLOAT, 3);
    const csVector3* mesh_vertices = AnimControlGetVertices ();
    if (!mesh_vertices) mesh_vertices = factory->GetVertices ();
    animBuffers.position->SetData (mesh_vertices);
    return animBuffers.position;
  }
  
  return factory->GetPositions ();
}

const csVector3* csGenmeshMeshObject::GetVertices ()
{
  if (anim_ctrl)
  {
    const csVector3* mesh_vertices = AnimControlGetVertices ();
    if (!mesh_vertices) mesh_vertices = factory->GetVertices ();
    return mesh_vertices;
  }
  
  return factory->GetVertices ();
}

#include "csutil/custom_new_disable.h"

int csGenmeshMeshObject::ComputeProgLODLevel (const SubMeshProxy& subMesh, const csVector3& camera_pos)
{
  float min, max;
  factory->GetProgLODDistances(min, max);
  if (min >= max)
    return 0;
  csVector3 objpos = logparent->GetMovable()->GetPosition();
  const csBox3& bbox = anim_ctrl_bbox ? objectModel->GetObjectBoundingBox () : factory->GetObjectBoundingBox ();
  csVector3 bbpos = bbox.GetCenter();
  float dist = (camera_pos - (objpos + bbpos)).Norm();
  csRef<iGeneralFactoryState> fstate = scfQueryInterface<iGeneralFactoryState>(factory);
  int nlod = subMesh.GetSlidingWindowSize() - 1;
  // Function of square root of distance
  float t = (dist - min) / (max - min);

  // Adaptive LODs
  float mult = factory->engine->GetAdaptiveLODsMultiplier();
  t *= mult;

  if (t > 1.0f)
    t = 1.0f;
  else if (t < 0.0f)
    t = 0.0f;
  return (sqrtf(t) * nlod);
}  

csRenderMesh** csGenmeshMeshObject::GetRenderMeshes (
	int& n, iRenderView* rview, 
	iMovable* movable, uint32 frustum_mask)
{
  SetupObject ();

  n = 0;

  iCamera* camera = rview->GetCamera ();

  int clip_portal, clip_plane, clip_z_plane;
  CS::RenderViewClipper::CalculateClipSettings (rview->GetRenderContext (),
      frustum_mask, clip_portal, clip_plane, clip_z_plane);

  if (anim_ctrl)
    anim_ctrl->Update (vc->GetCurrentTicks (), factory->GetVertexCount(), 
      factory->GetShapeNumber());
    
  iRenderBuffer* positions = GetPositions();

  const csReversibleTransform o2wt = movable->GetFullTransform ();
  const csVector3& wo = o2wt.GetOrigin ();

  UpdateSubMeshProxies ();
  SubMeshProxiesContainer& sm = subMeshes;

  const bool factoryB2F = factory->back2front;
  const csVector3 b2fPos (o2wt.Other2This (camera->GetTransform ().GetOrigin()));
  uint frameNum = rview->GetCurrentFrameNumber ();
  renderMeshes.Empty();
  renderMeshes.SetCapacity (sm.GetSize ());

  const csBox3* bboxes;
  if (anim_ctrl_bbox)
    bboxes = AnimControlGetBboxes ();

  for (size_t i = 0; i<sm.GetSize (); ++i)
  {
    SubMeshProxy& subMesh = *(sm[i]);
    iMaterialWrapper* mater = subMesh.SubMeshProxy::GetMaterial();
    if (!mater)
      mater = subMeshes.GetDefaultSubmesh()->SubMeshProxy::GetMaterial();
    //if (!mater) mater = factory->GetMaterialWrapper ();
    if (!mater)
    {
      csPrintf ("INTERNAL ERROR: mesh used without material!\n");
      return 0;
    }

    if (mater->IsVisitRequired ()) mater->Visit ();

    bool rmCreated;
    csRenderMesh*& meshPtr = subMesh.rmHolder.GetUnusedMesh (rmCreated,
      frameNum);
      
    iRenderBuffer* index_buffer;
    bool b2f = subMesh.SubMeshProxy::GetBack2Front () || factoryB2F;
    if (b2f)
      index_buffer = subMesh.SubMeshProxy::GetIndicesB2F (b2fPos, frameNum,
	GetVertices(), factory->GetVertexCount());
    else
      index_buffer = subMesh.SubMeshProxy::GetIndices();
    if (!index_buffer)
      // Possible with empty submesh ...
      continue;
    csRenderBufferHolder* smBufferHolder =
      subMesh.SubMeshProxy::GetBufferHolder (bufferHolder);

    uint smMixMode = subMesh.SubMeshProxy::GetMixmode();
    meshPtr->mixmode = (smMixMode != (uint)~0) ? smMixMode 
      : subMeshes.GetDefaultSubmesh()->SubMeshProxy::GetMixmode ();;
    meshPtr->z_buf_mode = subMesh.SubMeshProxy::GetZMode();
    meshPtr->renderPrio = subMesh.SubMeshProxy::GetRenderPriority();
    meshPtr->clip_portal = clip_portal;
    meshPtr->clip_plane = clip_plane;
    meshPtr->clip_z_plane = clip_z_plane;
    meshPtr->do_mirror = camera->IsMirrored ();
    meshPtr->meshtype = CS_MESHTYPE_TRIANGLES;
    int start_index, end_index;
    if (subMesh.GetSlidingWindowSize() == 0)
    {
      start_index = 0;
      end_index = (uint)index_buffer->GetElementCount();
    }
    else
    {
      int prog_lod_level = (subMesh.GetForcedProgLODLevel() == -1)
        ? ComputeProgLODLevel(subMesh, camera->GetTransform().GetOrigin())
        : subMesh.GetForcedProgLODLevel();
      subMesh.GetSlidingWindow(prog_lod_level, start_index, end_index);
    }
    meshPtr->indexstart = start_index;
    meshPtr->indexend = end_index;
    meshPtr->material = mater;
    CS_ASSERT (mater != 0);
    meshPtr->worldspace_origin = wo;
    csRef<MergedSVContext> mergedSVContext;
    mergedSVContext.AttachNew (
      new (factory->genmesh_type->mergedSVContextPool) MergedSVContext (
      static_cast<iShaderVariableContext*> (&subMesh), svcontext));
    meshPtr->variablecontext = mergedSVContext;
    meshPtr->object2world = o2wt;
    meshPtr->bbox = anim_ctrl_bbox ? bboxes[i] : subMesh.parentSubMesh->GetObjectBoundingBox (positions);

    meshPtr->buffers = smBufferHolder;
    meshPtr->geometryInstance = (void*)factory;

    renderMeshes.Push (meshPtr);
  }

  n = (int)renderMeshes.GetSize ();
  return renderMeshes.GetArray ();
}

#include "csutil/custom_new_enable.h"

csGenmeshMeshObject::csAnimatedModel::csAnimatedModel (csGenmeshMeshObject* object)
  : scfImplementationType (this), object (object)
{}

csGenmeshMeshObject::csAnimatedModel::~csAnimatedModel ()
{}

const csBox3& csGenmeshMeshObject::csAnimatedModel::GetObjectBoundingBox ()
{
  const csBox3& bbox = object->AnimControlGetBbox ();
  return bbox;
}

void csGenmeshMeshObject::csAnimatedModel::SetObjectBoundingBox (const csBox3& bbox)
{
}

void csGenmeshMeshObject::csAnimatedModel::GetRadius (float& rad, csVector3& cent)
{
  rad = object->AnimControlGetRadius ();
  cent = object->AnimControlGetBbox ().GetCenter ();
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
  const csVector3 *vrt = GetVertices ();
  for (size_t s = 0; s < sm.GetSize(); s++)
  {
    iRenderBuffer* indexBuffer = sm[s]->GetIndices();
    CS::TriangleIndicesStream<uint> triangles (indexBuffer,
      CS_MESHTYPE_TRIANGLES);
    while (triangles.HasNext())
    {
      CS::TriangleT<uint> t (triangles.Next());
      if (csIntersect3::SegmentTriangle (seg, 
	vrt[t.a], vrt[t.b], vrt[t.c], 
	isect))
      {
	if (pr) *pr = csQsqrt (csSquaredDist::PointPoint (start, isect) /
	  csSquaredDist::PointPoint (start, end));
	return true;
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
  const csVector3 *vrt = GetVertices ();
  csVector3 tmp;
  iMaterialWrapper* mat = 0;
  for (size_t s = 0; s < sm.GetSize(); s++)
  {
    iRenderBuffer* indexBuffer = sm[s]->GetIndices();
    CS::TriangleIndicesStream<uint> triangles (indexBuffer,
      CS_MESHTYPE_TRIANGLES);
    while (triangles.HasNext())
    {
      CS::TriangleT<uint> t (triangles.Next());
      if (csIntersect3::SegmentTriangle (seg, 
	vrt[t.a], vrt[t.b], vrt[t.c], 
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
    }
  }
  if (pr) *pr = csQsqrt (dist * itot_dist);
  if (dist >= tot_dist)
    return false;

  if (material) *material = mat;

  return true;
}

void csGenmeshMeshObject::BuildDecal(const csVector3* pos, float decalRadius,
          iDecalBuilder* decalBuilder)
{
  UpdateSubMeshProxies ();
  SubMeshProxiesContainer& sm = subMeshes;

  csPoly3D poly;
  poly.SetVertexCount(3);
  const csVector3* vertices = factory->GetVertices();

  for (size_t s = 0; s < sm.GetSize(); s++)
  {
    iRenderBuffer* indexBuffer = sm[s]->GetIndices();
    CS::TriangleIndicesStream<uint> triangles (indexBuffer,
      CS_MESHTYPE_TRIANGLES);
    while (triangles.HasNext())
    {
      CS::TriangleT<uint> t (triangles.Next());
      poly[0] = vertices[t.a];
      poly[1] = vertices[t.b];
      poly[2] = vertices[t.c];

      if (poly.InSphere(*pos, decalRadius))
	decalBuilder->AddStaticPoly(poly);
    }
  }
}

iObjectModel* csGenmeshMeshObject::GetObjectModel ()
{
  if (anim_ctrl_bbox)
    return objectModel;
  return factory->GetObjectModel ();
}

void csGenmeshMeshObject::PreGetShaderVariableValue (csShaderVariable* var)
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
      holder->SetRenderBuffer (buffer, GetPositions());
      return;
    }
    if (buffer == CS_BUFFER_TEXCOORD0)
    {
      if (!animBuffers.texcoord)
        animBuffers.texcoord = csRenderBuffer::CreateRenderBuffer (
          num_mesh_vertices, CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 2);
      const csVector2* mesh_texels = AnimControlGetTexels ();
      if (!mesh_texels) mesh_texels = factory->GetTexels ();
      animBuffers.texcoord->SetData (mesh_texels);
      holder->SetRenderBuffer (buffer, animBuffers.texcoord);
      return;
    }
    if (buffer == CS_BUFFER_NORMAL)
    {
      if (!animBuffers.normal)
        animBuffers.normal = csRenderBuffer::CreateRenderBuffer (
          num_mesh_vertices, CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3);
      const csVector3* mesh_normals = AnimControlGetNormals ();
      if (!mesh_normals) mesh_normals = factory->GetNormals ();
      animBuffers.normal->SetData (mesh_normals);
      holder->SetRenderBuffer (buffer, animBuffers.normal);
      return;
    }
  }

  if (buffer == CS_BUFFER_COLOR)
  {
    if (mesh_colors_dirty_flag || anim_ctrl_colors)
    {
      if (!do_manual_colors)
      {
        if (!legacyLighting.color_buffer ||
          (legacyLighting.color_buffer->GetSize() != (sizeof (csColor4) * 
          legacyLighting.num_lit_mesh_colors)))
        {
          // Recreate the render buffer only if the new data cannot fit inside
          //  the existing buffer.
          legacyLighting.color_buffer = csRenderBuffer::CreateRenderBuffer (
              legacyLighting.num_lit_mesh_colors, 
              CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 4);
        }
        mesh_colors_dirty_flag = false;
        const csColor4* mesh_colors = 0;
        if (anim_ctrl_colors)
          mesh_colors = AnimControlGetColors (legacyLighting.lit_mesh_colors);
        else
          mesh_colors = legacyLighting.lit_mesh_colors;
        legacyLighting.color_buffer->SetData (mesh_colors);
      }
      else
      {
        mesh_colors_dirty_flag = false;
        const csColor4* mesh_colors = 0;
        if (anim_ctrl_colors)
          mesh_colors = AnimControlGetColors (factory->GetColors (false));
        else
          mesh_colors = factory->GetColors (false);
        if (mesh_colors)
        {
          if (!legacyLighting.color_buffer || 
            (legacyLighting.color_buffer->GetSize() != (sizeof (csColor4) * 
            factory->GetVertexCount())))
          {
            // Recreate the render buffer only if the new data cannot fit inside
            //  the existing buffer.
            legacyLighting.color_buffer = csRenderBuffer::CreateRenderBuffer (
              factory->GetVertexCount(), CS_BUF_STATIC,
              CS_BUFCOMP_FLOAT, 4);
          }
          legacyLighting.color_buffer->SetData (mesh_colors);
        }
      }
    }
    if (legacyLighting.color_buffer.IsValid())
      holder->SetRenderBuffer (buffer, legacyLighting.color_buffer);
    return;
  }

  factory->PreGetBuffer (holder, buffer);
}

void csGenmeshMeshObject::ForceProgLODLevel(int level)
{
  for (size_t s = 0; s < subMeshes.GetSize(); s++)
  {
    int the_level = level;
    if (level >= subMeshes[s]->GetSlidingWindowSize())
      the_level = subMeshes[s]->GetSlidingWindowSize() - 1;
    subMeshes[s]->ForceProgLODLevel(the_level);
  }
}

iGeneralMeshSubMesh* csGenmeshMeshObject::FindSubMesh (const char* name) const
{
  UpdateSubMeshProxies();
  return static_cast<iGeneralMeshSubMesh*> (subMeshes.FindSubMesh (name));
}

bool csGenmeshMeshObject::AddRenderBuffer (const char *name,
					   iRenderBuffer* buffer)
{
  CS::ShaderVarStringID bufID = factory->GetSVStrings()->Request (name);
  if (userBuffers.AddRenderBuffer (bufID, buffer))
  {
    user_buffer_names.Push (bufID);
    mesh_user_rb_dirty_flag = true;
    return true;
  }
  return false;
}

bool csGenmeshMeshObject::AddRenderBuffer (csRenderBufferName name,
					   iRenderBuffer* buffer)
{
  return AddRenderBuffer (csRenderBuffer::GetDescrFromBufferName (name), buffer);
}

bool csGenmeshMeshObject::RemoveRenderBuffer (const char *name)
{
  CS::ShaderVarStringID bufID = factory->GetSVStrings()->Request (name);
  if (userBuffers.RemoveRenderBuffer (bufID))
  {
    user_buffer_names.Delete (bufID);
    mesh_user_rb_dirty_flag = true;
    return true;
  }
  return false;
}

bool csGenmeshMeshObject::RemoveRenderBuffer (csRenderBufferName name)
{
  return RemoveRenderBuffer (csRenderBuffer::GetDescrFromBufferName (name));
}

iRenderBuffer* csGenmeshMeshObject::GetRenderBuffer (int index)
{
  CS::ShaderVarStringID bufID = user_buffer_names[index];
  return userBuffers.GetRenderBuffer (bufID);
}

csRef<iString> csGenmeshMeshObject::GetRenderBufferName (int index) const
{
  csRef<iString> name; 
  name.AttachNew (new scfString (factory->GetSVStrings ()->Request 
    (user_buffer_names[index])));
  return name;
}

iRenderBuffer* csGenmeshMeshObject::GetRenderBuffer (const char* name)
{
  CS::ShaderVarStringID bufID = factory->GetSVStrings()->Request (name);
  iRenderBuffer* buf = userBuffers.GetRenderBuffer (bufID);
  if (buf != 0) return 0;

  return factory->GetRenderBuffer (name);
}

iRenderBuffer* csGenmeshMeshObject::GetRenderBuffer (csRenderBufferName name)
{
  const char* nameStr = csRenderBuffer::GetDescrFromBufferName (name);
  CS::ShaderVarStringID bufID = factory->GetSVStrings()->Request (nameStr);
  iRenderBuffer* buf = userBuffers.GetRenderBuffer (bufID);
  if (buf != 0) return 0;

  return factory->GetRenderBuffer (name);
}

iMeshObjectFactory* csGenmeshMeshObject::GetFactory () const
{
  return factory;
}

//----------------------------------------------------------------------

csGenmeshMeshObjectFactory::csGenmeshMeshObjectFactory (
  csGenmeshMeshObjectType* pParent, iObjectRegistry* object_reg) : 
  scfImplementationType (this, static_cast<iBase*> (pParent))
{
  shaderVariableAccessor.AttachNew (new ShaderVariableAccessor (this));
  renderBufferAccessor.AttachNew (new RenderBufferAccessor (this));

  csGenmeshMeshObjectFactory::object_reg = object_reg;

  genmesh_type = pParent;
  SetPolyMeshStandard ();

  logparent = 0;
  initialized = false;
  object_bbox_valid = false;

  //material = 0;
  back2front = false;
  back2front_tree = 0;

  g3d = csQueryRegistry<iGraphics3D> (object_reg);
  svstrings = csQueryRegistryTagInterface<iShaderVarStringSet>
    (object_reg, "crystalspace.shader.variablenameset");

  user_buffer_change = 0;

  buffers_version = 0;

  autonormals = false;
  autonormals_compress = true;

  subMeshes.GetDefaultSubmesh()->SubMesh::SetMixmode (0);
  //default_mixmode = 0;
  default_lighting = true;
  default_color.Set (0, 0, 0);
  default_manualcolors = false;
  default_shadowcasting = true;
  default_shadowreceiving = false;

  csRef<iEngine> eng = csQueryRegistry<iEngine> (object_reg);
  engine = eng; // We don't want a circular reference!

  vc = csQueryRegistry<iVirtualClock> (object_reg);

  csRef<iCommandLineParser> cmdline = 
  	csQueryRegistry<iCommandLineParser> (object_reg);
  do_fullbright = (cmdline->GetOption ("fullbright") != 0);

  prog_lod_min_dist = 0.0;
  prog_lod_max_dist = 0.0;
}

csGenmeshMeshObjectFactory::~csGenmeshMeshObjectFactory ()
{
  ClearSubMeshes ();

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
  UpdateFromLegacyBuffers();

  object_bbox_valid = true;
  
  for (size_t s = 0; s < subMeshes.GetSize(); s++)
  {
    object_bbox += subMeshes[s]->GetObjectBoundingBox (knownBuffers.position);
  }
  
  float max_sqradius = 0.0f;
  const csVector3& center = object_bbox.GetCenter ();
  for (size_t s = 0; s < subMeshes.GetSize(); s++)
  {
    max_sqradius = csMax (max_sqradius,
      subMeshes[s]->ComputeMaxSqRadius (knownBuffers.position, center));
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

// TODO: Optimize. Cache this value in the class.
int csGenmeshMeshObjectFactory::GetNumProgLODLevels() const
{
  int max_size = 0;
  for (size_t s = 0; s < subMeshes.GetSize(); s++)
  {
    if (max_size < subMeshes[s]->GetSlidingWindowSize())
      max_size = subMeshes[s]->GetSlidingWindowSize();
  }
  return max_size;
}
  
void csGenmeshMeshObjectFactory::SetupFactory ()
{
  if (!initialized)
  {
    initialized = true;
    object_bbox_valid = false;
  }
}

void csGenmeshMeshObjectFactory::UpdateTangentsBitangents ()
{
  if (!knownBuffers.tangent.IsValid() || !knownBuffers.bitangent.IsValid())
  {
    int vertCount = GetVertexCount();
    if (!knownBuffers.tangent.IsValid())
      knownBuffers.tangent = csRenderBuffer::CreateRenderBuffer (
        vertCount, CS_BUF_STATIC,
        CS_BUFCOMP_FLOAT, 3);
    if (!knownBuffers.bitangent.IsValid())
      knownBuffers.bitangent = csRenderBuffer::CreateRenderBuffer (
        vertCount, CS_BUF_STATIC,
        CS_BUFCOMP_FLOAT, 3);

    
    size_t triNum;
    const csTriangle* tris;
    csDirtyAccessArray<csTriangle> triangleScratch;
    for (size_t i = 0; i < subMeshes.GetSize(); i++)
    {
      iRenderBuffer* indexBuffer = subMeshes[i]->SubMesh::GetIndices();
      size_t scratchPos = triangleScratch.GetSize();
      size_t indexTris = indexBuffer->GetElementCount() / 3;
      if ((indexBuffer->GetComponentType() == CS_BUFCOMP_INT)
	  || (indexBuffer->GetComponentType() == CS_BUFCOMP_UNSIGNED_INT))
      {
        triangleScratch.SetSize (scratchPos + indexTris);
	csRenderBufferLock<uint8> indexLock (indexBuffer, CS_BUF_LOCK_READ);
	memcpy (triangleScratch.GetArray() + scratchPos,
	  indexLock.Lock(), indexTris * sizeof (csTriangle));
      }
      else
      {
        triangleScratch.SetCapacity (scratchPos + indexTris);
	CS::TriangleIndicesStream<int> triangles (indexBuffer,
	  CS_MESHTYPE_TRIANGLES);
	while (triangles.HasNext())
	  triangleScratch.Push (triangles.Next());
      }
    }
    triNum = triangleScratch.GetSize ();
    tris = triangleScratch.GetArray ();
    
    // Workaround for meshes that don't have normals set
    csVector3* normals = GetNormals();
    csDirtyAccessArray<csVector3> normalsScratch;
    if (normals == 0)
    {
      normalsScratch.SetSize (vertCount, csVector3 (0, 0, 1));
      normals = normalsScratch.GetArray();
    }
    
    // Workaround for meshes that don't have texcoords set
    csVector2* texels = GetTexels();
    csDirtyAccessArray<csVector2> texcoordScratch;
    if (texels == 0)
    {
      texcoordScratch.SetSize (vertCount, csVector2 (0));
      texels = texcoordScratch.GetArray();
    }
    
    csVector3* tangentData = (csVector3*)cs_malloc (
      sizeof (csVector3) * vertCount * 2);
    csVector3* bitangentData = tangentData + vertCount;
    csNormalMappingTools::CalculateTangents (triNum, tris, 
      vertCount, GetVertices(), 
      normals, texels, 
      tangentData, bitangentData);
  
    knownBuffers.tangent->CopyInto (tangentData, vertCount);
    knownBuffers.bitangent->CopyInto (bitangentData, vertCount);
  
    cs_free (tangentData);
  }
}

iRenderBuffer* csGenmeshMeshObjectFactory::GetPositions()
{
  if (legacyBuffers.mesh_vertices_dirty_flag)
    UpdateFromLegacyBuffers ();
  return knownBuffers.position;
}

template<typename T>
static void RemapIndexBuffer (csRef<iRenderBuffer>& index_buffer,
                              size_t* vt,
			      size_t lastIndex)
{
  csRef<iRenderBuffer> newBuffer;
  {
    csRenderBufferLock<T> indices (index_buffer, CS_BUF_LOCK_READ);
    newBuffer = csRenderBuffer::CreateIndexRenderBuffer (
      index_buffer->GetElementCount(), index_buffer->GetBufferType(), 
      index_buffer->GetComponentType(), 0, lastIndex);
    csRenderBufferLock<T> newIndices (newBuffer);
    for (size_t n = 0; n < indices.GetSize(); n++)
    {
      size_t index = size_t (indices[n]);
      size_t newIndex = vt[index];
      newIndices[n] = T (newIndex);
    }
  }
  index_buffer = newBuffer;
}

void csGenmeshMeshObjectFactory::Compress ()
{
  UpdateFromLegacyBuffers ();
  ClearLegacyBuffers ();
  
  csDirtyAccessArray<csRef<iRenderBuffer> > buffers;
  buffers.SetCapacity (user_buffer_names.GetSize());
  for (size_t i = 0; i < user_buffer_names.GetSize(); i++)
  {
    buffers.Push (userBuffers.GetRenderBuffer (user_buffer_names[i]));
  }
  
  size_t newVtNum;
  size_t* vt = csVertexCompressor::Compress (buffers.GetArray(),
					     buffers.GetSize(),
					     newVtNum);
  if (vt)
  {
    for (size_t s = 0; s < subMeshes.GetSize(); s++)
    {
      SubMesh* subMesh = subMeshes[s];
      csRef<iRenderBuffer> indices (subMesh->GetIndices());
      csRenderBufferComponentType compType = 
	indices->GetComponentType ();
      switch (compType & ~CS_BUFCOMP_NORMALIZED)
      {
	case CS_BUFCOMP_BYTE:
	  RemapIndexBuffer<char> (indices, vt, newVtNum-1);
	  break;
	case CS_BUFCOMP_UNSIGNED_BYTE:
	  RemapIndexBuffer<unsigned char> (indices, vt, newVtNum-1);
	  break;
	case CS_BUFCOMP_SHORT:
	  RemapIndexBuffer<short> (indices, vt, newVtNum-1);
	  break;
	case CS_BUFCOMP_UNSIGNED_SHORT:
	  RemapIndexBuffer<unsigned short> (indices, vt, newVtNum-1);
	  break;
	case CS_BUFCOMP_INT:
	  RemapIndexBuffer<int> (indices, vt, newVtNum-1);
	  break;
	case CS_BUFCOMP_UNSIGNED_INT:
	  RemapIndexBuffer<unsigned int> (indices, vt, newVtNum-1);
	  break;
	case CS_BUFCOMP_FLOAT:
	  RemapIndexBuffer<float> (indices, vt, newVtNum-1);
	  break;
	case CS_BUFCOMP_DOUBLE:
	  RemapIndexBuffer<double> (indices, vt, newVtNum-1);
	  break;
	default:
	  CS_ASSERT_MSG("invalid component type", false);
      }
      subMesh->SetIndices (indices);
    }
    delete[] vt;
    
    for (size_t b = 0; b < buffers.GetSize(); b++)
    {
      csRenderBufferName bufName = csRenderBuffer::GetBufferNameFromDescr (
	svstrings->Request (user_buffer_names[b]));
      InternalSetBuffer (user_buffer_names[b], buffers[b], bufName);
    }
  }
}

class csTriangleMeshGenMesh :
  public scfImplementation1<csTriangleMeshGenMesh,iTriangleMesh>
{
private:
  csGenmeshMeshObjectFactory* factory;
  csFlags flags;
  uint32 change_nr;

public:
  csTriangleMeshGenMesh () : scfImplementationType(this)
  {
    change_nr = 0;
  }

  virtual ~csTriangleMeshGenMesh ()
  {
  }

  void SetFactory (csGenmeshMeshObjectFactory* Factory)
  { factory = Factory; }

  virtual size_t GetVertexCount () { return factory->GetVertexCount (); }
  virtual csVector3* GetVertices () { return factory->GetVertices (); }
  virtual size_t GetTriangleCount () { return factory->GetTriangleCount (); }
  virtual csTriangle* GetTriangles () { return factory->GetTriangles (); }
  virtual void Lock () { }
  virtual void Unlock () { }
  virtual csFlags& GetFlags () { return flags; }
  virtual uint32 GetChangeNumber () const { return change_nr; }
};


void csGenmeshMeshObjectFactory::SetPolyMeshStandard ()
{
  polyMeshType = Standard;

  csRef<csTriangleMeshGenMesh> trimesh;
  trimesh.AttachNew (new csTriangleMeshGenMesh ());
  trimesh->SetFactory (this);
  SetTriangleData (genmesh_type->base_id, trimesh);
}

void csGenmeshMeshObjectFactory::SetPolyMeshSubmeshes ()
{
  polyMeshType = Submeshes;

  csRef<SubMeshesTriMesh> trimesh;
  trimesh.AttachNew (new SubMeshesTriMesh (this, subMeshes));
  SetTriangleData (genmesh_type->base_id, trimesh);
}

void csGenmeshMeshObjectFactory::PreGetShaderVariableValue (
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
  switch (buffer)
  {
    case CS_BUFFER_POSITION:
      {
	holder->SetRenderBuffer (buffer, GetPositions());
	return;
      }
    case CS_BUFFER_TEXCOORD0:
      {
	if (legacyBuffers.mesh_texels_dirty_flag)
	  UpdateFromLegacyBuffers ();
	holder->SetRenderBuffer (buffer, knownBuffers.texcoord);
	return;
      }
    case CS_BUFFER_NORMAL:
      {
	if (legacyBuffers.mesh_normals_dirty_flag)
	  UpdateFromLegacyBuffers ();
	holder->SetRenderBuffer (buffer, knownBuffers.normal);
	return;
      }
    case CS_BUFFER_TANGENT:
    case CS_BUFFER_BINORMAL:
      {
	UpdateTangentsBitangents();
	holder->SetRenderBuffer (buffer, (buffer == CS_BUFFER_TANGENT) ?
	  knownBuffers.tangent : knownBuffers.bitangent);
	return;
      }
    default:
      break;
  }
}

void csGenmeshMeshObjectFactory::AddVertex (const csVector3& v,
      const csVector2& uv, const csVector3& normal,
      const csColor4& color)
{
  CreateLegacyBuffers();
  GetColors (true);
  legacyBuffers.mesh_vertices.Push (v);
  legacyBuffers.mesh_texels.Push (uv);
  legacyBuffers.mesh_normals.Push (normal);
  legacyBuffers.mesh_colors.Push (color);
  Invalidate ();
}

void csGenmeshMeshObjectFactory::AddTriangle (const csTriangle& tri)
{
  subMeshes.GetDefaultSubmesh()->CreateLegacyBuffer();
  subMeshes.GetDefaultSubmesh()->legacyTris.mesh_triangles.Push (tri);

  Invalidate ();
}

void csGenmeshMeshObjectFactory::SetVertexCount (int n)
{
  CreateLegacyBuffers();
  legacyBuffers.mesh_vertices.SetCapacity (n); legacyBuffers.mesh_vertices.SetSize (n);
  legacyBuffers.mesh_texels.SetCapacity (n); legacyBuffers.mesh_texels.SetSize (n);
  if (legacyBuffers.mesh_colors.GetSize () > 0)
  {
    legacyBuffers.mesh_colors.SetCapacity (n); 
    legacyBuffers.mesh_colors.SetSize (n, csColor4 (0, 0, 0, 1));
  }
  legacyBuffers.mesh_normals.SetCapacity (n); legacyBuffers.mesh_normals.SetSize (n, csVector3 (0));
  initialized = false;

  legacyBuffers.mesh_vertices_dirty_flag = true;
  legacyBuffers.mesh_texels_dirty_flag = true;
  legacyBuffers.mesh_normals_dirty_flag = true;
  legacyBuffers.mesh_colors_dirty_flag = true;
}

void csGenmeshMeshObjectFactory::SetTriangleCount (int n)
{
  subMeshes.GetDefaultSubmesh()->CreateLegacyBuffer();
  subMeshes.GetDefaultSubmesh()->legacyTris.mesh_triangles.SetSize (n);
  subMeshes.GetDefaultSubmesh()->legacyTris.mesh_triangle_dirty_flag = true;

  initialized = false;
}

void csGenmeshMeshObjectFactory::CalculateNormals (bool compress)
{
  CreateLegacyBuffers ();
  csDirtyAccessArray<csTriangle> newTriangles;
  csArray<size_t> submeshTriangleStarts;
  for (size_t s = 0; s < subMeshes.GetSize (); s++)
  {
    submeshTriangleStarts.Push (newTriangles.GetSize());

    SubMesh* subMesh (subMeshes[s]);
    csRef<iRenderBuffer> indices (subMesh->GetIndices());

    CS::TriangleIndicesStream<int> triangles;
    triangles.BeginTriangulate (indices, CS_MESHTYPE_TRIANGLES);
    while (triangles.HasNext ())
    {
      csTriangle tri (triangles.Next ());
      newTriangles.Push (tri);
    }
  }
  submeshTriangleStarts.Push (newTriangles.GetSize());
  csNormalCalculator::CalculateNormals (
    legacyBuffers.mesh_vertices, newTriangles, legacyBuffers.mesh_normals, 
    compress);
  if (compress)
  {
    /* When compression is enabled, indices in the triangles may have 
      * changed. Thus, refill the submesh index buffers with the new
      * data. */
    for (size_t s = 0; s < subMeshes.GetSize (); s++)
    {
      size_t smTriStart = submeshTriangleStarts[s]; 
      size_t smTriEnd = submeshTriangleStarts[s+1]; 
      size_t rangeMin = (size_t)~0, rangeMax = 0;
      for (size_t t = smTriStart; t < smTriEnd; t++)
      {
	const csTriangle& tri = newTriangles[t];
	int lowestIndex, highestIndex;
	if (tri.a > tri.b)
	{
	  if (tri.b > tri.c)
	  {
	    // a > b > c
	    highestIndex = tri.a;
	    lowestIndex = tri.c;
	  }
	  else if (tri.a > tri.c)
	  {
	    // a > c > b
	    highestIndex = tri.a;
	    lowestIndex = tri.b;
	  }
	  else
	  {
	    // c > a > b
	    highestIndex = tri.c;
	    lowestIndex = tri.b;
	  }
	}
	else
	{
	  if (tri.a > tri.c)
	  {
	    // b > a > c
	    highestIndex = tri.b;
	    lowestIndex = tri.c;
	  }
	  else if (tri.b > tri.c)
	  {
	    // b > c > a
	    highestIndex = tri.b;
	    lowestIndex = tri.a;
	  }
	  else
	  {
	    // c > b > a
	    highestIndex = tri.c;
	    lowestIndex = tri.a;
	  }
	}
	rangeMin = csMin (rangeMin, size_t (lowestIndex));
	rangeMax = csMax (rangeMax, size_t (highestIndex));
      }
      csRef<iRenderBuffer> newBuffer;
      // FIXME: try to take original component type into account.
      size_t triNum = smTriEnd - smTriStart;
      newBuffer = csRenderBuffer::CreateIndexRenderBuffer (
	triNum * 3, CS_BUF_STATIC, 
	CS_BUFCOMP_UNSIGNED_INT, rangeMin, rangeMax);
      newBuffer->CopyInto (newTriangles.GetArray() + smTriStart, triNum*3);
      SubMesh* subMesh (subMeshes[s]);
      subMesh->SetIndices (newBuffer);
    }
  }
  autonormals = true;
  autonormals_compress = compress;

  legacyBuffers.mesh_normals_dirty_flag = true;
}

void csGenmeshMeshObjectFactory::GenerateCylinder (float l, float r, uint sides)
{
  CreateLegacyBuffers();
  subMeshes.GetDefaultSubmesh()->CreateLegacyBuffer();
  CS::Geometry::DensityTextureMapper mapper (10);
  CS::Geometry::Primitives::GenerateCylinder (
      l, r, sides, legacyBuffers.mesh_vertices, legacyBuffers.mesh_texels,
      legacyBuffers.mesh_normals, 
      subMeshes.GetDefaultSubmesh()->legacyTris.mesh_triangles, &mapper);
  legacyBuffers.mesh_colors.DeleteAll ();
  Invalidate ();
}

void csGenmeshMeshObjectFactory::GenerateCapsule (float l, float r, uint sides)
{
  CreateLegacyBuffers();
  subMeshes.GetDefaultSubmesh()->CreateLegacyBuffer();
  CS::Geometry::DensityTextureMapper mapper (10);
  CS::Geometry::Primitives::GenerateCapsule (
      l, r, sides, legacyBuffers.mesh_vertices, legacyBuffers.mesh_texels,
      legacyBuffers.mesh_normals, 
      subMeshes.GetDefaultSubmesh()->legacyTris.mesh_triangles, &mapper);
  legacyBuffers.mesh_colors.DeleteAll ();
  Invalidate ();
}

void csGenmeshMeshObjectFactory::GenerateSphere (const csEllipsoid& ellips,
    int num, bool cyl_mapping, bool toponly, bool reversed)
{
  CreateLegacyBuffers();
  subMeshes.GetDefaultSubmesh()->CreateLegacyBuffer();
  CS::Geometry::Primitives::GenerateSphere (
      ellips, num, legacyBuffers.mesh_vertices, legacyBuffers.mesh_texels,
      legacyBuffers.mesh_normals, 
      subMeshes.GetDefaultSubmesh()->legacyTris.mesh_triangles, 
      cyl_mapping, toponly, reversed);
  legacyBuffers.mesh_colors.DeleteAll();
  Invalidate();
}

void csGenmeshMeshObjectFactory::GenerateBox (const csBox3& box)
{
  CreateLegacyBuffers();
  subMeshes.GetDefaultSubmesh()->CreateLegacyBuffer();
  CS::Geometry::Primitives::GenerateBox (box, legacyBuffers.mesh_vertices, 
      legacyBuffers.mesh_texels,
      legacyBuffers.mesh_normals, 
      subMeshes.GetDefaultSubmesh()->legacyTris.mesh_triangles);
  legacyBuffers.mesh_colors.DeleteAll();
  Invalidate();
}

bool csGenmeshMeshObjectFactory::InternalSetBuffer (csRenderBufferName name,
						    iRenderBuffer* buffer)
{
  const char* nameStr = csRenderBuffer::GetDescrFromBufferName (name);

  CS::ShaderVarStringID bufID = svstrings->Request (nameStr);
  return InternalSetBuffer (bufID, buffer, name);
}

bool csGenmeshMeshObjectFactory::InternalSetBuffer (CS::ShaderVarStringID bufID,
						    iRenderBuffer* buffer,
						    csRenderBufferName name)
{
  bool hasName = false;
  if (userBuffers.RemoveRenderBuffer (bufID))
  {
    userBuffers.RemoveRenderBuffer (bufID);
    hasName = true;
  }
  if (userBuffers.AddRenderBuffer (bufID, buffer))
  {
    if (!hasName) user_buffer_names.Push (bufID);
    user_buffer_change++;
    
    switch (name)
    {
      case CS_BUFFER_INDEX:
        //knownBuffers.index = buffer;
	subMeshes.GetDefaultSubmesh()->SetIndices (buffer);
        break;
      case CS_BUFFER_POSITION:
        knownBuffers.position = buffer;
        break;
      case CS_BUFFER_TEXCOORD0:
        knownBuffers.texcoord = buffer;
        break;
      case CS_BUFFER_NORMAL:
        knownBuffers.normal = buffer;
        break;
      case CS_BUFFER_COLOR:
        knownBuffers.color = buffer;
        break;
      case CS_BUFFER_TANGENT:
        knownBuffers.tangent = buffer;
        break;
      case CS_BUFFER_BINORMAL:
        knownBuffers.bitangent = buffer;
        break;
      default:
        break;
    }
    
    return true;
  }
  return false;
}

bool csGenmeshMeshObjectFactory::AddRenderBuffer (const char *name,
						  iRenderBuffer* buffer)
{
  CS::ShaderVarStringID bufID = svstrings->Request (name);
  if (userBuffers.AddRenderBuffer (bufID, buffer))
  {
    user_buffer_names.Push (bufID);
    user_buffer_change++;
    
    csRenderBufferName userName = 
      csRenderBuffer::GetBufferNameFromDescr (name);
    if (userName != CS_BUFFER_NONE)
      ClearLegacyBuffers (CS_BUFFER_MAKE_MASKABLE (userName));
    
    switch (userName)
    {
      case CS_BUFFER_INDEX:
        subMeshes.GetDefaultSubmesh()->SetIndices (buffer);
        break;
      case CS_BUFFER_POSITION:
        knownBuffers.position = buffer;
        break;
      case CS_BUFFER_TEXCOORD0:
        knownBuffers.texcoord = buffer;
        break;
      case CS_BUFFER_NORMAL:
        knownBuffers.normal = buffer;
        break;
      case CS_BUFFER_COLOR:
        knownBuffers.color = buffer;
        break;
      case CS_BUFFER_TANGENT:
        knownBuffers.tangent = buffer;
        break;
      case CS_BUFFER_BINORMAL:
        knownBuffers.bitangent = buffer;
        break;
      default:
        break;
    }
    
    return true;
  }
  return false;
}

bool csGenmeshMeshObjectFactory::AddRenderBuffer (csRenderBufferName name,
						  iRenderBuffer* buffer)
{
  const char* nameStr = csRenderBuffer::GetDescrFromBufferName (name);
  if (nameStr == 0) return false;

  CS::ShaderVarStringID bufID = svstrings->Request (nameStr);
  if (userBuffers.AddRenderBuffer (bufID, buffer))
  {
    user_buffer_names.Push (bufID);
    user_buffer_change++;
    
    ClearLegacyBuffers (CS_BUFFER_MAKE_MASKABLE (name));
    
    switch (name)
    {
      case CS_BUFFER_INDEX:
        subMeshes.GetDefaultSubmesh()->SetIndices (buffer);
        break;
      case CS_BUFFER_POSITION:
        knownBuffers.position = buffer;
        break;
      case CS_BUFFER_TEXCOORD0:
        knownBuffers.texcoord = buffer;
        break;
      case CS_BUFFER_NORMAL:
        knownBuffers.normal = buffer;
        break;
      case CS_BUFFER_COLOR:
        knownBuffers.color = buffer;
        break;
      case CS_BUFFER_TANGENT:
        knownBuffers.tangent = buffer;
        break;
      case CS_BUFFER_BINORMAL:
        knownBuffers.bitangent = buffer;
        break;
      default:
        break;
    }
    
    return true;
  }
  return false;
}

bool csGenmeshMeshObjectFactory::RemoveRenderBuffer (const char *name)
{
  CS::ShaderVarStringID bufID = svstrings->Request (name);
  if (userBuffers.RemoveRenderBuffer (bufID))
  {
    user_buffer_names.Delete (bufID);
    user_buffer_change++;
    
    csRenderBufferName userName = 
      csRenderBuffer::GetBufferNameFromDescr (name);
    switch (userName)
    {
      case CS_BUFFER_INDEX:
        subMeshes.GetDefaultSubmesh()->SetIndices (0);
        break;
      case CS_BUFFER_POSITION:
        knownBuffers.position.Invalidate();
        break;
      case CS_BUFFER_TEXCOORD0:
        knownBuffers.texcoord.Invalidate();
        break;
      case CS_BUFFER_NORMAL:
        knownBuffers.normal.Invalidate();
        break;
      case CS_BUFFER_COLOR:
        knownBuffers.color.Invalidate();
        break;
      case CS_BUFFER_TANGENT:
        knownBuffers.tangent.Invalidate();
        break;
      case CS_BUFFER_BINORMAL:
        knownBuffers.bitangent.Invalidate();
        break;
      default:
        break;
    }
    
    return true;
  }
  return false;
}

bool csGenmeshMeshObjectFactory::RemoveRenderBuffer (csRenderBufferName name)
{
  const char* nameStr = csRenderBuffer::GetDescrFromBufferName (name);
  if (nameStr == 0) return false;

  CS::ShaderVarStringID bufID = svstrings->Request (nameStr);
  if (userBuffers.RemoveRenderBuffer (bufID))
  {
    user_buffer_names.Delete (bufID);
    user_buffer_change++;
    
    switch (name)
    {
      case CS_BUFFER_INDEX:
        subMeshes.GetDefaultSubmesh()->SetIndices (0);
        break;
      case CS_BUFFER_POSITION:
        knownBuffers.position.Invalidate();
        break;
      case CS_BUFFER_TEXCOORD0:
        knownBuffers.texcoord.Invalidate();
        break;
      case CS_BUFFER_NORMAL:
        knownBuffers.normal.Invalidate();
        break;
      case CS_BUFFER_COLOR:
        knownBuffers.color.Invalidate();
        break;
      case CS_BUFFER_TANGENT:
        knownBuffers.tangent.Invalidate();
        break;
      case CS_BUFFER_BINORMAL:
        knownBuffers.bitangent.Invalidate();
        break;
      default:
        break;
    }
    
    return true;
  }
  return false;
}

iRenderBuffer* csGenmeshMeshObjectFactory::GetRenderBuffer (int index)
{
  UpdateFromLegacyBuffers();
  CS::ShaderVarStringID bufID = user_buffer_names[index];
  return userBuffers.GetRenderBuffer (bufID);
}

csRef<iString> csGenmeshMeshObjectFactory::GetRenderBufferName (int index) const
{
  csRef<iString> name; 
  name.AttachNew (new scfString (svstrings->Request (user_buffer_names[index])));
  return name;
}

iRenderBuffer* csGenmeshMeshObjectFactory::GetRenderBuffer (const char* name)
{
  UpdateFromLegacyBuffers();
  CS::ShaderVarStringID bufID = svstrings->Request (name);
  iRenderBuffer* buf = userBuffers.GetRenderBuffer (bufID);
  if (buf != 0) return buf;

  if (strcmp (name, "tangent") == 0)
  {
    UpdateTangentsBitangents ();
    return knownBuffers.tangent;
  }
  else if (strcmp (name, "bitangent") == 0)
  {
    UpdateTangentsBitangents ();
    return knownBuffers.bitangent;
  }
  return 0;
}

iRenderBuffer* csGenmeshMeshObjectFactory::GetRenderBuffer (csRenderBufferName name)
{
  const char* nameStr = csRenderBuffer::GetDescrFromBufferName (name);
  if (nameStr == 0) return 0;

  UpdateFromLegacyBuffers();
  CS::ShaderVarStringID bufID = svstrings->Request (nameStr);
  iRenderBuffer* buf = userBuffers.GetRenderBuffer (bufID);
  if (buf != 0) return buf;

  if (name == CS_BUFFER_TANGENT)
  {
    UpdateTangentsBitangents ();
    return knownBuffers.tangent;
  }
  else if (name == CS_BUFFER_BINORMAL)
  {
    UpdateTangentsBitangents ();
    return knownBuffers.bitangent;
  }
  return 0;
}


void csGenmeshMeshObjectFactory::Invalidate ()
{
  object_bbox_valid = false;
  initialized = false;

  legacyBuffers.mesh_vertices_dirty_flag = true;
  legacyBuffers.mesh_texels_dirty_flag = true;
  legacyBuffers.mesh_normals_dirty_flag = true;
  legacyBuffers.mesh_colors_dirty_flag = true;
  subMeshes.GetDefaultSubmesh()->legacyTris.mesh_triangle_dirty_flag = true;
  
  for (size_t s = 0; s < subMeshes.GetSize(); s++)
    subMeshes[s]->InvalidateBoundingBox();

  ShapeChanged ();
}

void csGenmeshMeshObjectFactory::HardTransform (
    const csReversibleTransform& t)
{
  CreateLegacyBuffers();
  size_t i;
  for (i = 0 ; i < legacyBuffers.mesh_vertices.GetSize () ; i++)
  {
    legacyBuffers.mesh_vertices[i] = t.This2Other (
      legacyBuffers.mesh_vertices[i]);
    legacyBuffers.mesh_normals[i] = t.This2OtherRelative (
      legacyBuffers.mesh_normals[i]);
  }

  legacyBuffers.mesh_vertices_dirty_flag = true;
  legacyBuffers.mesh_normals_dirty_flag = true;

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
  csRef<iStringSet> strset = csQueryRegistryTagInterface<iStringSet> (
      object_reg, "crystalspace.shared.stringset");
  base_id = strset->Request ("base");
  csRef<iVerbosityManager> verbosemgr (
    csQueryRegistry<iVerbosityManager> (object_reg));
  if (verbosemgr) 
    do_verbose = verbosemgr->Enabled ("genmesh");

  return true;
}

}
CS_PLUGIN_NAMESPACE_END(Genmesh)
