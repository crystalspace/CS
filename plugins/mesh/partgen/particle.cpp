/*
    Copyright (C) 2003 by Martin Geisse <mgeisse@gmx.net>

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
#include "plugins/mesh/partgen/particle.h"
#include "iengine/rview.h"
#include "iengine/camera.h"
#include "iengine/movable.h"
#include "iengine/light.h"
#include "ivideo/material.h"
#include "ivideo/vbufmgr.h"
#include "iutil/objreg.h"
#include "qint.h"

SCF_IMPLEMENT_IBASE_EXT (csNewParticleSystem)
#ifndef CS_USE_NEW_RENDERER
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iVertexBufferManagerClient)
#endif
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iParticleState)
SCF_IMPLEMENT_IBASE_EXT_END

#ifndef CS_USE_NEW_RENDERER
SCF_IMPLEMENT_EMBEDDED_IBASE (csNewParticleSystem::eiVertexBufferManagerClient)
  SCF_IMPLEMENTS_INTERFACE (iVertexBufferManagerClient)
SCF_IMPLEMENT_EMBEDDED_IBASE_END
#endif

SCF_IMPLEMENT_EMBEDDED_IBASE (csNewParticleSystem::eiParticleState)
  SCF_IMPLEMENTS_INTERFACE (iParticleState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csNewParticleSystem::csNewParticleSystem (
	iEngine *eng, iMeshObjectFactory *fact, int flags) : csMeshObject (eng)
{
#ifndef CS_USE_NEW_RENDERER 
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiVertexBufferManagerClient);
#endif
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiParticleState);
  Factory = fact;
  ParticleFlags = flags;
  ParticleCount = 0;
  StorageCount = 0;
  PositionArray = 0;
  Scale = csVector2 (1.0f, 1.0f);
  Angle = 0.0f;
  Color = csColor (0, 0, 0);
  Material = 0;
  Axis = csVector3 (0, 1, 0);
  PrevTime = 0;
  MixMode = CS_FX_COPY;
  Lighting = false;
  LitColors = 0;
  csMeshFactory* mf = (csMeshFactory*)fact;
  iObjectRegistry* object_reg = mf->GetObjectRegistry ();
  light_mgr = CS_QUERY_REGISTRY (object_reg, iLightManager);

  self_destruct = false;
  time_to_live = 0;

  change_size = false;
  change_color = false;
  change_alpha = false;
  change_rotation = false;
  alphapersecond = 0.0f;
  alpha_now = 1.0f;

#ifdef CS_USE_NEW_RENDERER 
  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);

  vertices = 0;

  svcontext.AttachNew (new csShaderVariableContext);
  /*mesh.variablecontext.AttachNew (new csShaderVariableContext);
  mesh.object2camera = csReversibleTransform ();
  mesh.meshtype = CS_MESHTYPE_TRIANGLES;
  meshPtr = &mesh;*/
#endif

  texels = 0;
  triangles = 0;
  colors = 0;
  initialized = false;
}

csNewParticleSystem::~csNewParticleSystem ()
{
  delete[] PositionArray;
  delete[] LitColors;
  delete[] texels;
  delete[] triangles;
  delete[] colors;
#ifdef CS_USE_NEW_RENDERER 
  delete[] vertices;
#else
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiVertexBufferManagerClient);
#endif
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiParticleState);
}

#define UPDATE_ARRAY_ALWAYS(NAME,TYPE) {                                \
  TYPE *old = NAME;                                                     \
  NAME = new TYPE [newsize];                                            \
  memcpy (NAME, old, sizeof (TYPE) * copysize);                         \
  delete[] old;                                                         \
}

void csNewParticleSystem::Update (csTicks elapsed_time)
{
  SetupObject ();
  if (self_destruct)
  {
    if (elapsed_time >= time_to_live)
    {
      if (Engine)
      {
        csRef<iMeshWrapper> m = SCF_QUERY_INTERFACE (LogParent, iMeshWrapper);
	if (m)
          Engine->WantToDie (m);
      }
      time_to_live = 0;
      /// and a calling virtual function can process without crashing
      return;
    }
    time_to_live -= elapsed_time;
  }
  float elapsed_seconds = ((float)elapsed_time) / 1000.0;
  if (change_color)
    AddColor (colorpersecond * elapsed_seconds);
  if (change_size)
  {
    Scale.x *= pow (scalepersecond, elapsed_seconds);
    Scale.y *= pow (scalepersecond, elapsed_seconds);
  }
  if (change_alpha)
  {
    alpha_now += alphapersecond * elapsed_seconds;
    if (alpha_now < 0.0f) alpha_now = 0.0f;
    else if (alpha_now > 1.0f) alpha_now = 1.0f;
    MixMode = CS_FX_SETALPHA (alpha_now);
  }
  if (change_rotation)
  {
    Angle += anglepersecond * elapsed_seconds;
  }
}

void csNewParticleSystem::Allocate (int newsize, int copysize)
{
  UPDATE_ARRAY_ALWAYS (PositionArray, csVector3);
  if (Lighting) UPDATE_ARRAY_ALWAYS (LitColors, csColor);

  StorageCount = newsize;
}

void csNewParticleSystem::SetCount (int c)
{
  if (c > StorageCount)
    Allocate (c, ParticleCount);
  ParticleCount = c;
  initialized = false;
}

void csNewParticleSystem::Compact ()
{
  if (ParticleCount < StorageCount)
    Allocate (ParticleCount, ParticleCount);
}

void csNewParticleSystem::UpdateBounds ()
{
  if (ParticleCount <= 0)
    Bounds.StartBoundingBox ();
  else
  {
    Bounds.StartBoundingBox (PositionArray[0]);
    for (int i=1; i<ParticleCount; i++)
      Bounds.AddBoundingVertexSmart (PositionArray [i]);
  }
}

iMeshObjectFactory* csNewParticleSystem::GetFactory () const
{
  return Factory;
}

void csNewParticleSystem::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;
    delete[] texels;
    texels = new csVector2 [ParticleCount * 4];
    delete[] triangles;
    triangles = new csTriangle [ParticleCount * 2];
    delete[] colors;
    colors = new csColor [ParticleCount * 4];

    int i;
    csVector2* txt = texels;
    csTriangle* tri = triangles;
    csColor* c = colors;
    for (i = 0 ; i < ParticleCount ; i++)
    {
      // fill the texel table
      *txt++ = csVector2 (0, 0);
      *txt++ = csVector2 (0, 1);
      *txt++ = csVector2 (1, 1);
      *txt++ = csVector2 (1, 0);
      // fill the triangle table
      *tri++ = csTriangle (i*4+0, i*4+1, i*4+2);
      *tri++ = csTriangle (i*4+0, i*4+2, i*4+3);
      // fill the color table
      *c++ = Color;
      *c++ = Color;
      *c++ = Color;
      *c++ = Color;
    }

#ifdef CS_USE_NEW_RENDERER
    csStringID vertex_name, texel_name, normal_name, color_name, index_name;
    csMeshFactory* mf = (csMeshFactory*)Factory;
    iObjectRegistry* object_reg = mf->GetObjectRegistry ();
    csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
	"crystalspace.shared.stringset", iStringSet);
    vertex_name = strings->Request ("vertices");
    texel_name = strings->Request ("texture coordinates");
    normal_name = strings->Request ("normals");
    color_name = strings->Request ("colors");
    index_name = strings->Request ("indices");

    delete[] vertices;
    VertexCount = ParticleCount * 4;
    TriangleCount = ParticleCount * 2;
    vertices = new csVector3 [VertexCount];
    vertex_buffer = g3d->CreateRenderBuffer (
        sizeof (csVector3)*VertexCount, CS_BUF_DYNAMIC, 
        CS_BUFCOMP_FLOAT, 3);
    texel_buffer = g3d->CreateRenderBuffer (
        sizeof (csVector2)*VertexCount, CS_BUF_DYNAMIC, 
        CS_BUFCOMP_FLOAT, 2);
#if 0
    normal_buffer = g3d->CreateRenderBuffer (
        sizeof (csVector3)*VertexCount, CS_BUF_DYNAMIC,
        CS_BUFCOMP_FLOAT, 3);
#endif
    color_buffer = g3d->CreateRenderBuffer (
        sizeof (csColor)*VertexCount, CS_BUF_DYNAMIC,
        CS_BUFCOMP_FLOAT, 3);
    index_buffer = g3d->CreateIndexRenderBuffer (
        sizeof (unsigned int)*TriangleCount*3, CS_BUF_DYNAMIC,
        CS_BUFCOMP_UNSIGNED_INT, 0, VertexCount - 1);
    csShaderVariable *sv;
    sv = svcontext->GetVariableAdd (vertex_name);
    sv->SetValue (vertex_buffer);
    sv = svcontext->GetVariableAdd (texel_name);
    sv->SetValue (texel_buffer);
#if 0
    sv = svcontext->GetVariableAdd (normal_name);
    sv->SetValue (normal_buffer);
#endif
    sv = svcontext->GetVariableAdd (color_name);
    sv->SetValue (color_buffer);
    sv = svcontext->GetVariableAdd (index_name);
    sv->SetValue (index_buffer);
#endif
  }
}

void csNewParticleSystem::SetupParticles (
  const csReversibleTransform& trans,
  csVector3* vertices,
  csBox3& bbox)	// Not for CS_USE_NEW_RENDERER
{
  int i;

  // compute modified axes for rotation or axis alignment
  csVector3 x_axis, y_axis;

  if (ParticleFlags & CS_PARTICLE_ROTATE)
  {
    float cosa, sina;
    cosa = cos (Angle);
    sina = sin (Angle);
    x_axis = csVector3 (cosa, sina, 0);
    y_axis = csVector3 (-sina, cosa, 0);
  }
  else if (ParticleFlags & CS_PARTICLE_AXIS)
  {
    csVector3 effectiveAxis = trans.Other2ThisRelative (Axis);
    if (ParticleFlags & CS_PARTICLE_ALIGN_Y)
    {
      y_axis = effectiveAxis;
      x_axis = csVector3 (0, 0, -1) % y_axis;
    }
    else
    {
      x_axis = effectiveAxis;
      y_axis = csVector3 (0, 0, +1) % x_axis;
    }

    float norm = x_axis.Norm ();
    if (ABS (norm) < EPSILON) return;	// @@@?
    x_axis /= norm;

    norm = y_axis.Norm ();
    if (ABS (norm) < EPSILON) return;	// @@@?
    y_axis /= norm;
  }
  else
  {
    x_axis = csVector3 (1, 0, 0);
    y_axis = csVector3 (0, 1, 0);
  }

  // apply scaling
  if (ParticleFlags & CS_PARTICLE_SCALE)
  {
    x_axis *= Scale.x;
    y_axis *= Scale.y;
  }

  // compute the actual vertices
  x_axis /= 2;
  y_axis /= 2;

  // set up the data for DrawTriangleMesh
  for (i = 0; i<ParticleCount; i++)
  {
    // transform to eye coordinates
    csVector3 pos = trans.Other2This (PositionArray [i]);

    *vertices = pos - x_axis - y_axis;
#ifndef CS_USE_NEW_RENDERER
    bbox.AddBoundingVertex (*vertices++);
#else
    vertices++;
#endif
    *vertices = pos - x_axis + y_axis;
#ifndef CS_USE_NEW_RENDERER
    bbox.AddBoundingVertex (*vertices++);
#else
    vertices++;
#endif
    *vertices = pos + x_axis + y_axis;
#ifndef CS_USE_NEW_RENDERER
    bbox.AddBoundingVertex (*vertices++);
#else
    vertices++;
#endif
    *vertices = pos + x_axis - y_axis;
#ifndef CS_USE_NEW_RENDERER
    bbox.AddBoundingVertex (*vertices++);
#else
    vertices++;
#endif
  }
}

bool csNewParticleSystem::DrawTest (iRenderView* rview, iMovable* movable,
	uint32 frustum_mask)
{
  SetupObject ();

  // get the object-to-camera transformation
  iCamera *camera = rview->GetCamera ();
  csReversibleTransform trans = camera->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    trans /= movable->GetFullTransform ();

  rview->CalculateClipSettings (frustum_mask, ClipPortal, ClipPlane, ClipZ);

  if (Lighting && light_mgr)
  {
    const csArray<iLight*>& relevant_lights = light_mgr
    	->GetRelevantLights (LogParent, -1, false);
    UpdateLighting (relevant_lights, movable);
  }

  return true;
}

void csNewParticleSystem::UpdateLighting (const csArray<iLight*>& lights,
  iMovable* movable)
{
  if (!Lighting) return;
  const csReversibleTransform &transform = movable->GetTransform ();

  csColor* c = colors;
  for (int i=0; i<ParticleCount; i++)
  {
    csColor lightColor = Color;
    csVector3 wpos = transform.This2Other (PositionArray [i]);

    int num = lights.Length ();
    for (int j=0; j<num; j++)
    {
      float d = (wpos - lights [j]->GetCenter ()).Norm ();
      float br = lights [j]->GetBrightnessAtDistance (d);
      lightColor += br * lights [j]->GetColor ();
    }

    LitColors [i] = lightColor;
    *c++ = lightColor;
    *c++ = lightColor;
    *c++ = lightColor;
    *c++ = lightColor;
  }
}

#ifndef CS_USE_NEW_RENDERER
void csNewParticleSystem::eiVertexBufferManagerClient::ManagerClosing ()
{
  scfParent->vbuf = 0;
}
#endif

bool csNewParticleSystem::Draw (iRenderView* rview, iMovable* mov,
	csZBufMode mode)
{
#ifndef CS_USE_NEW_RENDERER
  // some generic setup
  if (VisCallback) VisCallback->BeforeDrawing (this, rview);
  iGraphics3D* g3d = rview->GetGraphics3D ();
  iCamera* camera = rview->GetCamera ();
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, mode);

  // Get object space in view coordinates
  csReversibleTransform objspace = camera->GetTransform ();
  if (!mov->IsFullTransformIdentity ())
    objspace /= mov->GetFullTransform ();
  csReversibleTransform tr_ident;
  g3d->SetObjectToCamera (&tr_ident);

  // these are the final data chunks for DrawTriangleMesh
  int VertexCount = ParticleCount * 4;
  CS_ALLOC_STACK_ARRAY (csVector3, vertices, VertexCount);
  csBox3 bbox;

  Material->Visit ();

  SetupParticles (objspace, vertices, bbox);

  // set up a vertex buffer
  iVertexBufferManager *vbufmgr = g3d->GetVertexBufferManager ();
  if (vbuf == 0)
  {
    vbufmgr->AddClient (&scfiVertexBufferManagerClient);
    vbuf = vbufmgr->CreateBuffer (100);
  }

  // set up the G3DTriangleMesh
  G3DTriangleMesh trimesh;
  trimesh.num_vertices_pool = 1;
  trimesh.num_triangles = ParticleCount * 2;
  trimesh.triangles = triangles;
  trimesh.clip_portal = ClipPortal;
  trimesh.clip_plane = ClipPlane;
  trimesh.clip_z_plane = ClipZ;
  trimesh.use_vertex_color = false;
  trimesh.do_fog = false;
  trimesh.do_mirror = false;
  trimesh.do_morph_texels = false;
  trimesh.do_morph_colors = false;
  trimesh.vertex_mode = G3DTriangleMesh::VM_VIEWSPACE;
  trimesh.mixmode = MixMode;
  trimesh.morph_factor = 0;
  trimesh.buffers[0] = vbuf;
  trimesh.mat_handle = Material->GetMaterialHandle ();
  trimesh.vertex_fog = 0;

  // draw it!
  vbufmgr->LockBuffer (vbuf, vertices, texels, colors, VertexCount, 0, bbox);
  g3d->DrawTriangleMesh (trimesh);
  vbufmgr->UnlockBuffer (vbuf);
#endif

  return true;
}

csRenderMesh **csNewParticleSystem::GetRenderMeshes (int &num,
	iRenderView* rview, 
	iMovable* movable, uint32 frustum_mask)
{
#ifdef CS_USE_NEW_RENDERER
  num = 0;
  SetupObject ();

  rview->CalculateClipSettings (frustum_mask, ClipPortal, ClipPlane, ClipZ);

  // get the object-to-camera transformation
  iCamera *camera = rview->GetCamera ();
  csReversibleTransform trans = camera->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    trans /= movable->GetFullTransform ();
  
  csBox3 box;
  SetupParticles (trans, vertices, box);

  if (Lighting && light_mgr)
  {
    const csArray<iLight*>& relevant_lights = light_mgr
      ->GetRelevantLights (LogParent, -1, false);
    UpdateLighting (relevant_lights, movable);
  }
  
  // some generic setup
  //@@@?if (VisCallback) VisCallback->BeforeDrawing (this, rview);

  Material->Visit ();

  vertex_buffer->CopyToBuffer (vertices, sizeof (csVector3) * VertexCount);
  texel_buffer->CopyToBuffer (texels, sizeof (csVector2) * VertexCount);
  color_buffer->CopyToBuffer (colors, sizeof (csColor) * VertexCount);
  index_buffer->CopyToBuffer (triangles,
      	sizeof (unsigned int) * TriangleCount *3);

  bool meshCreated;
  csRenderMesh*& rm = rmHolder.GetUnusedMesh (meshCreated, 
    rview->GetCurrentFrameNumber ());

  if (meshCreated)
  {
    rm->variablecontext = svcontext;
  }

  // Prepare for rendering.
  rm->mixmode = MixMode;
  rm->clip_portal = ClipPortal;
  rm->clip_plane = ClipPlane;
  rm->clip_z_plane = ClipZ;
  rm->do_mirror = false/* camera->IsMirrored () */; 
    /*
      Force to false as the front-face culling will let the particle 
      disappear. 
     */
  rm->meshtype = CS_MESHTYPE_TRIANGLES;
  rm->indexstart = 0;
  rm->indexend = TriangleCount * 3;
  rm->material = Material;
  rm->object2camera = csReversibleTransform ();
  rm->camera_transform = &camera->GetTransform();
 
  num = 1;
  return &rm;
#else
  num = 0;
  return 0;
#endif
}

void csNewParticleSystem::NextFrame (csTicks current, const csVector3&)
{
  if (PrevTime != 0)
    Update (current - PrevTime);
  PrevTime = current;
}

bool csNewParticleSystem::SetColor (const csColor& c)
{
  Color = c;
  // If already initialized we fix the color table.
  if (initialized)
  {
    csColor* c = colors;
    int i;
    for (i = 0 ; i < ParticleCount ; i++)
    {
      // fill the color table
      *c++ = Color;
      *c++ = Color;
      *c++ = Color;
      *c++ = Color;
    }
  }
  return true;
}

void csNewParticleSystem::AddColor (const csColor& c)
{
  SetColor (Color + c);
  if (LitColors)
  {
    int i;
    csColor* clr = colors;
    for (i = 0 ; i < ParticleCount ; i++)
    {
      csColor l = LitColors[i];
      l += c;
      LitColors[i] = l;
      *clr++ = l;
      *clr++ = l;
      *clr++ = l;
      *clr++ = l;
    }
  }
}

const csColor& csNewParticleSystem::GetColor () const
{
  return Color;
}

bool csNewParticleSystem::SetMaterialWrapper (iMaterialWrapper* m)
{
  Material = m;
  return true;
}

iMaterialWrapper* csNewParticleSystem::GetMaterialWrapper () const
{
  return Material;
}

bool csNewParticleSystem::GetLighting () const
{
  return Lighting;
}

void csNewParticleSystem::SetLighting (bool enable)
{
  delete[] LitColors;
  Lighting = enable;
  if (Lighting) LitColors = new csColor [StorageCount];
  else LitColors = 0;
  initialized = false;
}

