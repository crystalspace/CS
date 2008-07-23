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
#include "csqsqrt.h"

#include "csgeom/box.h"
#include "csgeom/frustum.h"
#include "csgeom/math3d.h"
#include "csgeom/trimesh.h"
#include "csgeom/polyclip.h"
#include "csgeom/transfrm.h"
#include "csgeom/tri.h"
#include "csgeom/matrix4.h"
#include "csgfx/renderbuffer.h"
#include "csgfx/shadervarcontext.h"
#include "csgfx/shadervar.h"
#include "csgfx/imagecubemapmaker.h"
#include "cstool/rviewclipper.h"
#include "iengine/camera.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "iengine/texture.h"
#include "igraphic/image.h"
#include "iutil/objreg.h"
#include "iutil/document.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "iutil/strset.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/rendermesh.h"
#include "ivideo/fontserv.h"
#include "ivideo/shader/shader.h"
#include "ivideo/txtmgr.h"

#include "watermesh.h"

#define OCEAN_BBOX_RADIUS 50000.0f
#define OCEAN_NP_WID	40.0f
#define OCEAN_NP_LEN	40.0f

CS_IMPLEMENT_PLUGIN

using namespace CS::Plugins::WaterMesh;

csWaterMeshObject::csWaterMeshObject (csWaterMeshObjectFactory* factory) :
  scfImplementationType (this)
{
  myRenderBufferAccessor.AttachNew (new RenderBufferAccessor (this));

  csWaterMeshObject::factory = factory;
  logparent = 0;
  initialized = false;
  vertsChanged = false;

  material = 0;

  MixMode = CS_FX_ALPHA;

  color.red = 0;
  color.green = 0;
  color.blue = 0;
  factory_color_nr = (uint)~0;
  mesh_colors_dirty_flag = true;
	
  current_lod = 1;
  current_features = 0;

  g3d = csQueryRegistry<iGraphics3D> (factory->object_reg);
  engine = csQueryRegistry<iEngine> (factory->object_reg);

  strings = csQueryRegistryTagInterface<iStringSet> 
  	(factory->object_reg, "crystalspace.shared.stringset");

  variableContext.AttachNew (new csShaderVariableContext);
  if(factory->isOcean())
	farPatchVariableContext.AttachNew (new csShaderVariableContext);

  factory->AddMeshObject(this);
}

csWaterMeshObject::~csWaterMeshObject ()
{
	factory->RemoveMeshObject(this);
	
	bool meshesCreated;
	csDirtyAccessArray<csRenderMesh*>& renderMeshes =
    	meshesHolder.GetUnusedData (meshesCreated, 0);

    renderMeshes.DeleteAll ();
}

iMeshObjectFactory* csWaterMeshObject::GetFactory () const
{
  return (csWaterMeshObjectFactory*)factory;
}

bool csWaterMeshObject::SetMaterialWrapper (iMaterialWrapper* mat)
{
  material = mat;
  return true;
}

void csWaterMeshObject::SetupBufferHolder ()
{
  if (bufferHolder == 0)
    bufferHolder.AttachNew (new csRenderBufferHolder);

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
  bufferHolder->SetRenderBuffer (CS_BUFFER_POSITION, vertex_buffer);

  // Texels are fetched from the factory.
  bufferHolder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, factory->texel_buffer);

  // Normals are fetched from the factory but we use an accessor
  // for those because they are not always needed.
  // Colors are fetched from the object because we need to add the mesh
  // base color to the static colors in the factory.
  bufferHolder->SetAccessor (myRenderBufferAccessor, CS_BUFFER_NORMAL_MASK | CS_BUFFER_COLOR_MASK);

  if(factory->isOcean())
  {
	if(farPatchBufferHolder == 0)
		farPatchBufferHolder.AttachNew(new csRenderBufferHolder);
	
	farPatchBufferHolder->SetRenderBuffer(CS_BUFFER_INDEX, factory->far_index_buffer);
	farPatchBufferHolder->SetRenderBuffer(CS_BUFFER_POSITION, factory->far_vertex_buffer);
	farPatchBufferHolder->SetRenderBuffer(CS_BUFFER_TEXCOORD0, factory->far_texel_buffer);
	//Far patch colors and normals shouldn't change..
	farPatchBufferHolder->SetRenderBuffer(CS_BUFFER_NORMAL, factory->far_normal_buffer);
	farPatchBufferHolder->SetRenderBuffer(CS_BUFFER_COLOR, factory->far_color_buffer);
  }
}

void csWaterMeshObject::SetupVertexBuffer()
{
  if (vertsChanged)
  {
    if (!vertex_buffer)
    {
      // Create a buffer that doesn't copy the data.
      vertex_buffer = csRenderBuffer::CreateRenderBuffer (
        factory->numVerts, CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
        3);
    }
    vertex_buffer->CopyInto (verts.GetArray(), factory->numVerts);
  }
}

void csWaterMeshObject::SetupObject ()
{	
  if (!initialized || vertsChanged)
  {
    initialized = true;

	// Make sure the factory is ok and his its buffers.
	factory->SetupFactory ();
	
	if(vertsChanged)
	{
		verts.DeleteAll();
		norms.DeleteAll();
		for(uint i = 0; i < factory->verts.GetSize(); i++)
		{
			verts.Push(factory->verts[i]);
			norms.Push(factory->norms[i]);
		}

		SetupVertexBuffer ();
			
		vertsChanged = false;
	}
	
    SetupBufferHolder ();
  }

  if(factory->murkChanged)
  {
	csShaderVariable *murkVar = variableContext->GetVariableAdd(strings->Request("murkiness"));
	murkVar->SetType(csShaderVariable::FLOAT);
	murkVar->SetValue(factory->waterAlpha);
	
	if(factory->isOcean())
	{
		murkVar = farPatchVariableContext->GetVariableAdd(strings->Request("murkiness"));
		murkVar->SetType(csShaderVariable::FLOAT);
		murkVar->SetValue(factory->waterAlpha);
	}
	
	factory->murkChanged = false;	
  }

	/*
		<shadervar name="amps" type="vector3">0.1, 0.03, 0.05</shadervar>
		<shadervar name="kxs" type="vector3">1.4, -1.1, 0.5</shadervar>
		<shadervar name="kys" type="vector3">1.6, 0.7, -2.5</shadervar>
		<shadervar name="freqs" type="vector3">2.0, 1.7, 1.6</shadervar>
		<shadervar name="phases" type="vector3">0.0, 1.0, 1.41</shadervar>
	*/

  if(factory->amplitudes_changed)
  {
	csShaderVariable *ampsVar = variableContext->GetVariableAdd(strings->Request("amps"));
	ampsVar->SetType(csShaderVariable::VECTOR3);
	ampsVar->SetValue(factory->GetAmplitudes());
	
	factory->amplitudes_changed = false;	
  }

  if(factory->frequencies_changed)
  {
	csShaderVariable *freqsVar = variableContext->GetVariableAdd(strings->Request("freqs"));
	freqsVar->SetType(csShaderVariable::VECTOR3);
	freqsVar->SetValue(factory->GetFrequencies());
	
	factory->frequencies_changed = false;	
  }

  if(factory->phases_changed)
  {
	csShaderVariable *phasesVar = variableContext->GetVariableAdd(strings->Request("phases"));
	phasesVar->SetType(csShaderVariable::VECTOR3);
	phasesVar->SetValue(factory->GetPhases());
	
	factory->phases_changed = false;	
  }

  if(factory->directions_changed)
  {
	csShaderVariable *kxsVar = variableContext->GetVariableAdd(strings->Request("kxs"));
	kxsVar->SetType(csShaderVariable::VECTOR3);
	kxsVar->SetValue(factory->GetDirsX());
	
	csShaderVariable *kysVar = variableContext->GetVariableAdd(strings->Request("kys"));
	kysVar->SetType(csShaderVariable::VECTOR3);
	kysVar->SetValue(factory->GetDirsY());

	factory->directions_changed = false;
  }
}

/*
* This function actually supplies the meshes to render to the
* 3D renderer (which will call g3d->DrawMesh()). In this simple
* case there is only one render mesh but more complex objects
* can have multiple render meshes.
*/
csRenderMesh** csWaterMeshObject::GetRenderMeshes (
  int& n, iRenderView* rview, 
  iMovable* movable, uint32 frustum_mask)
{
  n = 0;

  if (vis_cb) if (!vis_cb->BeforeDrawing (this, rview)) return false;

  iCamera* camera = rview->GetCamera ();

  SetupObject ();

//  float camY = camera->GetTransform().GetOrigin().y;
  csReversibleTransform trans;
  float camY = camera->GetTransform().GetOrigin().y;
  if(factory->isOcean())	
  {	
	// csOrthoTransform ot = camera->GetTransform();
	// ot.Translate(-(camera->GetTransform().GetOrigin()));
	// const csVector3 camDir = csVector3(0, 0, 1) * ot.GetInverse();
	// 
	// csVector3 camXZ = csVector3(camDir.x, 0, camDir.z);
	// camXZ.Normalize();
	// trans.RotateThis(csVector3(0, 1, 0), -acos(camXZ * csVector3(0, 0, 1)));
  
	float camX = camera->GetTransform().GetOrigin().x;
	float camZ = camera->GetTransform().GetOrigin().z;
	trans.Translate(csVector3(camX, 0, camZ));
  }
  else
  {
	trans.Identity();
	updateLocal();
  }

  int clip_portal, clip_plane, clip_z_plane;
  CS::RenderViewClipper::CalculateClipSettings (rview->GetRenderContext (),
      frustum_mask, clip_portal, clip_plane, clip_z_plane);

  const csReversibleTransform o2wt = movable->GetFullTransform ();
  const csVector3& wo = o2wt.GetOrigin ();
	
  csReversibleTransform o2world (o2wt);
  //o2world.SetO2T(o2world.GetO2T());

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

  logparent->SetZBufMode(CS_ZBUF_TEST);
  logparent->SetRenderPriority (factory->engine->GetRenderPriority ("alpha"));

  const uint currentFrame = rview->GetCurrentFrameNumber ();
  bool meshesCreated;
  csDirtyAccessArray<csRenderMesh*>& renderMeshes =
    meshesHolder.GetUnusedData (meshesCreated, currentFrame);

  if(renderMeshes.GetSize() == 0)
  {
 	bool rmCreated;
  	renderMeshes.Push(rmHolder.GetUnusedMesh (rmCreated, currentFrame));

  	renderMeshes[0]->mixmode = MixMode;
  	renderMeshes[0]->clip_portal = clip_portal;
	renderMeshes[0]->clip_plane = clip_plane;
	renderMeshes[0]->clip_z_plane = clip_z_plane;
	renderMeshes[0]->do_mirror = camera->IsMirrored ();
	renderMeshes[0]->meshtype = CS_MESHTYPE_TRIANGLES;
	renderMeshes[0]->indexstart = 0;
	renderMeshes[0]->indexend = factory->numTris * 3;
	renderMeshes[0]->material = material;		
  	renderMeshes[0]->worldspace_origin = wo;

    renderMeshes[0]->geometryInstance = (void*)factory;

  	if (rmCreated)
	  {
	    renderMeshes[0]->buffers = bufferHolder;
	    renderMeshes[0]->variablecontext = variableContext;
	  }
  }

  renderMeshes[0]->object2world = o2world * trans;

  //update shader variable
  csShaderVariable *o2wtVar = variableContext->GetVariableAdd(strings->Request("o2w transform"));
  o2wtVar->SetType(csShaderVariable::MATRIX);
  o2wtVar->SetValue(renderMeshes[0]->object2world);

  if(factory->isOcean())
  {
	/* TODO: Calculate required voodoo to get mesh to conform to what's left of the 
		y = 0 plane in the view frustum... should require some nifty inverse projective
		transformation....
	*/
	/*
	csOrthoTransform invCam = camera->GetTransform();
	
	csMatrix3 invCamMat = invCam.GetO2T();
	csMatrix3 fpTransMat (invCamMat);
	fpTransMat.m21 = fpTransMat.m23 = 0;
	fpTransMat.m22 = 1;
	
	csOrthoTransform fpTrans;
	fpTrans.SetO2T(fpTransMat);
	fpTrans.SetOrigin(wo);
	*/

	// float e = n * tanAlph;
	// 
	// float DminE = (d - e);
	// float FminN = (f - n);
	// 
	// printf("n: %f\n", n);
	// printf("halfDminE: %f\n", halfDminE);
	// printf("halfFminN: %f\n", halfFminN);
	// csMatrix3 tf (n, 0, halfDminE, 0, 1, 0, 0, 0, halfFminN);
	// csVector3 tfv (halfDminE, 0, halfFminN);
	
	csPlane3* fp = camera->GetFarPlane();
	float f;
	if(fp)
		f = fp->DD;
	else
		f = 500.0f;
		
	float tanAlph = tan((camera->GetFOVAngle() * PI) / 360);
	float d = f * tanAlph;
	
	CS::Math::Matrix4 fpFirstTransMat (1, 0, 1, -1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	CS::Math::Matrix4 fpSecondTransMat (d, 0, 0, 0, 0, 1, 0, 0, 0, 0, f, 0, 0, 0, 0, 1);
	CS::Math::Matrix4 fpTransMat = fpFirstTransMat * fpSecondTransMat;
	csTransform fpTrans = fpTransMat.GetTransform();
	CS::Math::Matrix4 testMat (fpTrans);	

	printf("testMat: |%.2f\t%.2f\t%.2f\t%.2f\t|\n", testMat.Row(0).x, testMat.Row(0).y, testMat.Row(0).z, testMat.Row(0).w);
	printf("testMat: |%.2f\t%.2f\t%.2f\t%.2f\t|\n", testMat.Row(1).x, testMat.Row(1).y, testMat.Row(1).z, testMat.Row(1).w);
	printf("testMat: |%.2f\t%.2f\t%.2f\t%.2f\t|\n", testMat.Row(2).x, testMat.Row(2).y, testMat.Row(2).z, testMat.Row(2).w);
	printf("testMat: |%.2f\t%.2f\t%.2f\t%.2f\t|\n", testMat.Row(3).x, testMat.Row(3).y, testMat.Row(3).z, testMat.Row(3).w);
	
	
	
	csVector3 test = fpTrans * csVector3(-1.0, 0.0, 1.0);
	printf("Vertex: <%.2f, %.2f, %.2f, %.2f> ===> <%.2f, %.2f, %.2f, %.2f>\n",-1.0, 0.0, 1.0, 1.0, test.x, test.y, test.z, 1.0);
	// test = testMat.Row2();
	// printf("fpTransMat: <%f, %f, %f, %f>\n", test.x, test.y, test.z, test.w);
	// test = testMat.Row3();
	// printf("fpTransMat: <%f, %f, %f, %f>\n", test.x, test.y, test.z, test.w);
	// test = testMat.Row4();
	// printf("fpTransMat: <%f, %f, %f, %f>\n", test.x, test.y, test.z, test.w);
	// 
	// test = csVector4(1.0, 0.0, -1.0, 1.0) * fpTrans;
	// printf("Vertex: <%f, %f, %f> => <%f, %f, %f>\n", 1.0, 0.0, -1.0, test.x, test.y, test.z);
	// test = csVector4(-1.0, 0.0, 1.0, 1.0) * fpTrans;
	// printf("Vertex: <%f, %f, %f> => <%f, %f, %f>\n", -1.0, 0.0, 1.0, test.x, test.y, test.z);
	// test = csVector4(1.0, 0.0, 1.0, 1.0) * fpTrans;
	// printf("Vertex: <%f, %f, %f> => <%f, %f, %f>\n", 1.0, 0.0, 1.0, test.x, test.y, test.z);
	// printf("\n");
	bool fpCreated;
	if(renderMeshes.GetSize() == 1)
	{
		renderMeshes.Push(rmHolder.GetUnusedMesh (fpCreated,
		rview->GetCurrentFrameNumber ()));

		renderMeshes[1]->mixmode = MixMode;
		renderMeshes[1]->clip_portal = clip_portal;
		renderMeshes[1]->clip_plane = clip_plane;
		renderMeshes[1]->clip_z_plane = clip_z_plane;
		renderMeshes[1]->do_mirror = camera->IsMirrored ();
		renderMeshes[1]->meshtype = CS_MESHTYPE_TRIANGLES;
		renderMeshes[1]->indexstart = 0;
		renderMeshes[1]->indexend = factory->far_tris.GetSize() * 3;
		renderMeshes[1]->material = material;		
		renderMeshes[1]->worldspace_origin = wo;

		renderMeshes[1]->geometryInstance = (void*)factory;

		if (fpCreated)
		{
			renderMeshes[1]->buffers = farPatchBufferHolder;
			renderMeshes[1]->variablecontext = farPatchVariableContext;
		}
	}
	renderMeshes[1]->object2world = o2world * fpTrans * trans;
  }

  //update shader variable
  // o2wtVar = farPatchVariableContext->GetVariableAdd(strings->Request("o2w transform"));
  // o2wtVar->SetType(csShaderVariable::MATRIX);
  // o2wtVar->SetValue(renderMeshes[1]->object2world);

  n = renderMeshes.GetSize();

  return renderMeshes.GetArray();
}

void csWaterMeshObject::NextFrame (csTicks, const csVector3&, uint)
{
	// for(uint i = 0; i < verts.GetSize(); i++)
	// {
	// 	printf("Vertex %d: <%f, %f, %f>\n", i, verts[i].x, verts[i].y, verts[i].z);
	// }
	
	//printf("Vertex count: %d\n", verts.GetSize());
}

bool csWaterMeshObject::HitBeamOutline (const csVector3& start,
                                        const csVector3& end, csVector3& isect, float* pr)
{
  // This is now closer to an outline hitting method. It will
  // return as soon as it touches any triangle in the mesh, and
  // will be a bit faster than its more accurate cousin (below).

  csSegment3 seg (start, end);
  int i, max = factory->numTris;
  csTriangle *tr = factory->tris.GetArray();
  csVector3 *vrt = factory->verts.GetArray();
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

bool csWaterMeshObject::HitBeamObject (const csVector3& start,
                                       const csVector3& end, csVector3& isect, float *pr, int* polygon_idx,
                                       iMaterialWrapper** material)
{
	
  if (material) *material = csWaterMeshObject::material;
  if (polygon_idx) *polygon_idx = -1;
  // This is the slow version. Use for an accurate hit on the object.
  // It will cycle through every triangle in the mesh serching for the
  // closest intersection. Slower, but returns the closest hit.
  // Usage is optional.

  csSegment3 seg (start, end);
  int i, max = factory->numTris;
  float tot_dist = csSquaredDist::PointPoint (start, end);
  float dist, temp;
  float itot_dist = 1 / tot_dist;
  dist = temp = tot_dist;
  csVector3 *vrt = factory->verts.GetArray(), tmp;
  csTriangle *tr = factory->tris.GetArray();
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

void csWaterMeshObject::updateLocal()
{
	
}

iObjectModel* csWaterMeshObject::GetObjectModel ()
{
  return factory->GetObjectModel ();
}

void csWaterMeshObject::SetNormalMap(iTextureWrapper *map)
{ 
	nMap = map;
	nMapVar = variableContext->GetVariableAdd(strings->Request("texture normal"));
	nMapVar->SetType(csShaderVariable::TEXTURE);
	nMapVar->SetValue(nMap);
	
	if(factory->isOcean())
	{
		csShaderVariable *nMapFP = farPatchVariableContext->GetVariableAdd(strings->Request("texture normal"));
		nMapFP->SetType(csShaderVariable::TEXTURE);
		nMapFP->SetValue(nMap);
	}
}

iTextureWrapper* csWaterMeshObject::GetNormalMap()
{
	return nMap;
}

void csWaterMeshObject::PreGetBuffer (csRenderBufferHolder *holder, 
                                      csRenderBufferName buffer)
{
  if (buffer == CS_BUFFER_COLOR)
  {
    if (mesh_colors_dirty_flag)
    {
      if (!color_buffer)
      {
        color_buffer = csRenderBuffer::CreateRenderBuffer (
          factory->numVerts, CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3);
      }
      mesh_colors_dirty_flag = false;
      const csColor* factory_colors = factory->cols.GetArray();
      int i;
      csColor colors[factory->numVerts];
      for (i = 0 ; i < factory->numVerts ; i++)
        colors[i] = factory_colors[i]+color;
      // Copy the data into the render buffer
      // since we don't keep a local copy of the color buffer here.
      color_buffer->CopyInto (colors, factory->numVerts);
    }
    holder->SetRenderBuffer (CS_BUFFER_COLOR, color_buffer);
  } 
  else if (buffer == CS_BUFFER_NORMAL)
  {
    if (vertsChanged)
    {
      if (!normal_buffer)
      {
        normal_buffer = csRenderBuffer::CreateRenderBuffer (
          factory->numVerts, CS_BUF_STATIC,
          CS_BUFCOMP_FLOAT, 3);
      }
      // Don't copy the data, have the buffer store a pointer instead.
      normal_buffer->SetData (norms.GetArray());
    }
    holder->SetRenderBuffer (CS_BUFFER_NORMAL, normal_buffer);
  }
}

//----------------------------------------------------------------------

csWaterMeshObjectFactory::csWaterMeshObjectFactory (
    iMeshObjectType *pParent, iObjectRegistry* object_reg)
  : scfImplementationType (this, pParent)
{
  csWaterMeshObjectFactory::object_reg = object_reg;

  csStringID base_mesh_id = GetBaseID (object_reg);
  csRef<csTriangleMeshPointer> trimesh_base;
  trimesh_base.AttachNew (new csTriangleMeshPointer (
	verts.GetArray(), numVerts, tris.GetArray(), numTris));
  SetTriangleData (base_mesh_id, trimesh_base);

  logparent = 0;
  water_type = pParent;
  initialized = false;
  object_bbox_valid = false;
  color_nr = 0;

  g3d = csQueryRegistry<iGraphics3D> (object_reg);
  engine = csQueryRegistry<iEngine> (object_reg);

  mesh_vertices_dirty_flag = true;
  mesh_texels_dirty_flag = true;
  mesh_normals_dirty_flag = true;
  mesh_triangle_dirty_flag = true;

  mesh_far_patch_dirty_flag = true;

  changedVerts = false;

  amplitudes_changed = false;
  frequencies_changed = false;
  phases_changed = false;
  directions_changed = false;

  type = WATER_TYPE_LOCAL;
  len = 2;
  wid = 2;
  gran = 1;
	
  numVerts = 4;
  numTris = 2;
	
  size_changed = false;

  detail = 1;

  waterAlpha = 0.3;
  murkChanged = true;
}

csWaterMeshObjectFactory::~csWaterMeshObjectFactory ()
{
}

void csWaterMeshObjectFactory::AddMeshObject (csWaterMeshObject* meshObj)
{
	children.Push(meshObj);
}

void csWaterMeshObjectFactory::RemoveMeshObject (csWaterMeshObject* meshObj)
{
	children.Delete(children[children.Find(meshObj)]);
}

void csWaterMeshObjectFactory::CalculateBBoxRadius ()
{
  object_bbox_valid = true;
  csVector3& v0 = verts[0];
  object_bbox.StartBoundingBox (v0);
  int i;
  for (i = 1 ; i < numVerts ; i++)
  {
    csVector3& v = verts[i];
    object_bbox.AddBoundingVertexSmart (v);
  }

  const csVector3& center = object_bbox.GetCenter ();
  float max_sqradius = 0.0f;
  for (i = 0 ; i < numVerts ; i++)
  {
    csVector3& v = verts[i];
    float sqradius = csSquaredDist::PointPoint (center, v);
    if (sqradius > max_sqradius) max_sqradius = sqradius;
  }

  if(isOcean())
	object_bbox.SetSize(csVector3(OCEAN_BBOX_RADIUS, OCEAN_BBOX_RADIUS, OCEAN_BBOX_RADIUS));
	  
  radius = csQsqrt (max_sqradius);
}

void csWaterMeshObjectFactory::GetRadius (float& radius, csVector3& center)
{
  SetupFactory ();
  if (!object_bbox_valid) CalculateBBoxRadius ();
  radius = this->radius;
  center = object_bbox.GetCenter();
}

const csBox3& csWaterMeshObjectFactory::GetObjectBoundingBox ()
{
  SetupFactory ();
  if (!object_bbox_valid) CalculateBBoxRadius ();
  return object_bbox;
}

void csWaterMeshObjectFactory::SetObjectBoundingBox (const csBox3& bbox)
{
  object_bbox_valid = true;
  object_bbox = bbox;
}

void csWaterMeshObjectFactory::SetWaterType(waterMeshType waterType)
{
	type = waterType;
	if(type == WATER_TYPE_OCEAN)
	{
		wid = OCEAN_NP_WID;
		len = OCEAN_NP_LEN;
		gran = 1;
		
		SetMurkiness(0.2);
		
		//Setup Ocean defaults
		SetAmplitudes(0.1, 0.03, 0.05);
		SetFrequencies(2.0, 1.7, 1.6);
		SetPhases(0.0, 1.0, 1.41);
		
		SetDirections(csVector2(1.4, 1.6), csVector2(-1.1, 0.7), csVector2(0.5, -2.5));
		
		size_changed = true;
	}
}

void csWaterMeshObjectFactory::SetupFactory ()
{
  if (!initialized || size_changed)
  {
    initialized = true;
    object_bbox_valid = false;
	size_changed = false;

	verts.DeleteAll();
	norms.DeleteAll();
	cols.DeleteAll();
	texs.DeleteAll();
	tris.DeleteAll();

	float offx, offz;
	offx = offz = 0.0;
	if(type == WATER_TYPE_OCEAN)
	{
		offx = (wid * gran) / 2;
		offz = (len * gran) / 2;
	}
		
	for(uint j = 0; j < len * gran; j++)
	{
		for(uint i = 0; i < wid * gran; i++)
		{
			verts.Push(csVector3 ((i / gran) - offx, 0, (j / gran) - offz));
			norms.Push(csVector3 (0, 1, 0));
			cols.Push(csColor (0.17,0.27,0.26));
			texs.Push(csVector2((i / gran) / (1.5 * detail), (j / gran) / (1.5 * detail)));
		}
	}
	
	for(uint j = 0; j < (len * gran) - 1; j++)
	{
		for(uint i = 0; i < (wid * gran) - 1; i++)
		{
			tris.Push(csTriangle (j * (wid * gran) + i, 
									(j + 1) * (wid * gran) + i, 
									j * (wid * gran) + i + 1));
			tris.Push(csTriangle (j * (wid * gran) + i + 1,
									(j + 1) * (wid * gran) + i,
									(j + 1) * (wid * gran) + i + 1));
		}
	}
	
	if(isOcean()) //make far patch
	{		
		far_verts.DeleteAll();
		far_norms.DeleteAll();
		far_cols.DeleteAll();
		far_texs.DeleteAll();
		far_tris.DeleteAll();
		
		float fpatchH = len * gran;
		float fpatchW = wid * gran;
				
		for(float j = 0; j < fpatchH; j+=1)
		{
			for(float i = 0; i < fpatchW; i+=1)
			{
				far_verts.Push(csVector3(((j * 2) / fpatchW) - 1 , -0.5f , i / fpatchH));
				far_norms.Push(csVector3 (0, 1, 0));
				far_cols.Push(csColor (0.17,0.27,0.26));
				far_texs.Push(csVector2((i / gran) / (0.1 * detail), (j / gran) / (0.1 * detail)));
			}
		}

		for(uint j = 0; j < (fpatchH) - 1; j++)
		{
			for(uint i = 0; i < (fpatchW) - 1; i++)
			{
				far_tris.Push(csTriangle (j * (fpatchW) + i + 1, 
										(j + 1) * (fpatchW) + i, 
										(j * (fpatchW) + i)));
				far_tris.Push(csTriangle ((j + 1) * (fpatchW) + i + 1,
										(j + 1) * (fpatchW) + i,
										(j * (fpatchW) + i + 1)));
			}
		}
	}
	
	numVerts = verts.GetSize();
	numTris = tris.GetSize();
	
	for(uint i = 0; i < children.GetSize(); i++)
	{
		children[i]->vertsChanged = true;
	}
		
	Invalidate();

    PrepareBuffers ();
  }
}

void csWaterMeshObjectFactory::SetMurkiness(float murk) 
{ 
	waterAlpha = murk;
	murkChanged = true;
}

float csWaterMeshObjectFactory::GetMurkiness()
{
	return waterAlpha;
}

void csWaterMeshObjectFactory::SetAmplitudes(float amp1, float amp2, float amp3)
{
	amps[0] = amp1;
	amps[1] = amp2;
	amps[2] = amp3;
  amplitudes_changed = true;
}

void csWaterMeshObjectFactory::SetFrequencies(float freq1, float freq2, float freq3)
{
	freqs[0] = freq1;
	freqs[1] = freq2;
	freqs[2] = freq3;
  frequencies_changed = true;
}

void csWaterMeshObjectFactory::SetPhases(float phase1, float phase2, float phase3)
{
	phases[0] = phase1;
	phases[1] = phase2;
	phases[2] = phase3;
  phases_changed = true;
}

void csWaterMeshObjectFactory::SetDirections(const csVector2 dir1, 
				const csVector2 dir2, const csVector2 dir3)
{
	k1 = csVector2(dir1);
	k2 = csVector2(dir2);
	k3 = csVector2(dir3);
  directions_changed = true;
}

csRef<iTextureWrapper> csWaterMeshObjectFactory::MakeFresnelTex(int size)
{
	if(size < 0) return 0;
	
	float buf[size * size];
	
	int i, j;
	
	int maxDist = size >> 1;
	
	int n = 1.0 / 1.33333333;
	float g, ratio;
	int dist;
	
	float a, b, d, e;
	
	for(i = 0; i < size; i++)
	{
		for(j = 0; j < size; j++)
		{
			dist = (i - maxDist) * (i - maxDist) + (j - maxDist) * (j - maxDist);
			
			if(dist > maxDist * maxDist)
			{
				buf[i * size + j] = 1.0;
			}
			else
			{
				ratio = sqrt(dist / (maxDist * maxDist));
				g = sqrt((ratio * ratio) - 1.0 + (n * n));
				a = g - ratio;
				b = g + ratio;
				
				d = (ratio * b - 1) * (ratio * b - 1);
				e = (ratio * a + 1) * (ratio * a + 1);
				
				buf[i * size + j] = (0.5 * ((a * a) / (b * b))) * (1 + (d / e));
			}
		}
	}
	
	// csRef<iTextureManager> texManager = csQueryRegistry<iTextureManager> (object_reg);
	// csPtr<iTextureHandle> texHandle = texManager->CreateTexture(size, size, 
	// 	csimg2D, "abgr8", CS_TEXTURE_2D);
	// texHandle->Blit(0, 0, size, size, buf, iTextureHandle::BGRA8888);
	
	csRef<iTextureWrapper> fresnelTexWrapper;
	// fresnelTexWrapper->SetTextureHandle(texHandle);
	return fresnelTexWrapper;
}

void csWaterMeshObjectFactory::PreGetBuffer (csRenderBufferHolder* holder, 
                                             csRenderBufferName buffer)
{
}

void csWaterMeshObjectFactory::Invalidate ()
{
  object_bbox_valid = false;

  mesh_vertices_dirty_flag = true;
  mesh_texels_dirty_flag = true;
  mesh_normals_dirty_flag = true;
  mesh_triangle_dirty_flag = true;
	
  mesh_far_patch_dirty_flag = true;
  
  color_nr++;

  ShapeChanged ();
}

void csWaterMeshObjectFactory::PrepareBuffers ()
{
  if (mesh_vertices_dirty_flag)
  {
    mesh_vertices_dirty_flag = false;
    if (!vertex_buffer)
    {
      // Create a buffer that doesn't copy the data.
      vertex_buffer = csRenderBuffer::CreateRenderBuffer (
        numVerts, CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
        3);
    }
    vertex_buffer->CopyInto (verts.GetArray(), numVerts);
  }
  if (mesh_texels_dirty_flag)
  {
    mesh_texels_dirty_flag = false;
    if (!texel_buffer)
    {
      // Create a buffer that doesn't copy the data.
      texel_buffer = csRenderBuffer::CreateRenderBuffer (
        numVerts, CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
        2);
    }
    texel_buffer->CopyInto (texs.GetArray(), numVerts);
  }
  if (mesh_triangle_dirty_flag)
  {
    mesh_triangle_dirty_flag = false;
    if (!index_buffer)
      index_buffer = csRenderBuffer::CreateIndexRenderBuffer (
      numTris*3,
      CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT,
      0, numVerts-1);
    index_buffer->CopyInto (tris.GetArray(), numTris*3);
  }

  if (mesh_far_patch_dirty_flag)
  {
    mesh_far_patch_dirty_flag = false;

    if (!far_vertex_buffer)
    {
      // Create a buffer that doesn't copy the data.
      far_vertex_buffer = csRenderBuffer::CreateRenderBuffer (
        far_verts.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
        3);
    }
    far_vertex_buffer->CopyInto (far_verts.GetArray(), far_verts.GetSize());

    if (!far_texel_buffer)
    {
      // Create a buffer that doesn't copy the data.
      far_texel_buffer = csRenderBuffer::CreateRenderBuffer (
		far_verts.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
        2);
    }
    far_texel_buffer->CopyInto (far_texs.GetArray(), far_verts.GetSize());

    if (!far_index_buffer)
	{
      far_index_buffer = csRenderBuffer::CreateIndexRenderBuffer (
      far_tris.GetSize()*3,
      CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT,
      0, far_verts.GetSize()-1);
	}
    far_index_buffer->CopyInto (far_tris.GetArray(), far_tris.GetSize()*3);

    if (!far_normal_buffer)
	{            
	  // Create a buffer that doesn't copy the data.
      far_normal_buffer = csRenderBuffer::CreateRenderBuffer (
        far_norms.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
        3);
	}
    far_normal_buffer->CopyInto (far_norms.GetArray(), far_norms.GetSize());

    if (!far_color_buffer)
	{            
	  // Create a buffer that doesn't copy the data.
      far_color_buffer = csRenderBuffer::CreateRenderBuffer (
        far_cols.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
        3);
	}
    far_color_buffer->CopyInto (far_cols.GetArray(), far_cols.GetSize());
  }
}

csPtr<iMeshObject> csWaterMeshObjectFactory::NewInstance ()
{
  csRef<csWaterMeshObject> cm;
  cm.AttachNew (new csWaterMeshObject (this));

  csRef<iMeshObject> im = scfQueryInterface<iMeshObject> (cm);
  return csPtr<iMeshObject> (im);
}

void csWaterMeshObjectFactory::SetLength(uint length) 
{ 
	if(type == WATER_TYPE_OCEAN)
		return;
	
	len = length; 
	size_changed = true; 
}

void csWaterMeshObjectFactory::SetWidth(uint width) 
{ 
	if(type == WATER_TYPE_OCEAN)
		return;
		
	wid = width; 
	size_changed = true; 
}

void csWaterMeshObjectFactory::SetGranularity(uint granularity) 
{ 
	if(type == WATER_TYPE_OCEAN)
		return;
		
	gran = granularity; 
	size_changed = true; 
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY (csWaterMeshObjectType)


csWaterMeshObjectType::csWaterMeshObjectType (iBase* pParent) : 
scfImplementationType (this, pParent), object_reg(0)
{
}

csWaterMeshObjectType::~csWaterMeshObjectType ()
{
}

csPtr<iMeshObjectFactory> csWaterMeshObjectType::NewFactory ()
{
  csRef<csWaterMeshObjectFactory> cm;
  cm.AttachNew (new csWaterMeshObjectFactory (this,
    object_reg));
  csRef<iMeshObjectFactory> ifact (
    scfQueryInterface<iMeshObjectFactory> (cm));
  return csPtr<iMeshObjectFactory> (ifact);
}

bool csWaterMeshObjectType::Initialize (iObjectRegistry* object_reg)
{
  csWaterMeshObjectType::object_reg = object_reg;
  return true;
}

