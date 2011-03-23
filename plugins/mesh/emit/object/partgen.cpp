/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    (C) W.C.A. Wijngaards, 2000

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
#include <math.h>

#include "csgfx/renderbuffer.h"
#include "csgeom/box.h"
#include "csgeom/matrix3.h"
#include "csgeom/transfrm.h"
#include "csgeom/tri.h"
#include "cstool/rbuflock.h"
#include "cstool/rviewclipper.h"
#include "csutil/scfarray.h"

#include "imesh/object.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "imesh/sprite2d.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/plugin.h"
#include "iutil/objreg.h"
#include "iutil/strset.h"
#include "ivideo/rendermesh.h"

#include "partgen.h"

csParticleSystem::csParticleSystem (
  iObjectRegistry* object_reg, iMeshObjectFactory* factory) :
  scfImplementationType(this, factory)
{
  initialized = false;
  csParticleSystem::factory = factory;
  csParticleSystem::object_reg = object_reg;
  logparent = 0;
  self_destruct = false;
  time_to_live = 0;
  // defaults
  change_size = false;
  change_color = false;
  change_alpha = false;
  change_rotation = false;
  alphapersecond = 0.0f;
  alpha_now = 1.0f;
  // bbox is empty.
  prev_time = 0;
  MixMode = 0;
  vis_cb = 0;
  mat = 0;
  radius = 0.0f;
  color.Set (0, 0, 0);
  csRef<iMeshObjectType> type = csLoadPluginCheck<iMeshObjectType> (
  	object_reg, "crystalspace.mesh.object.sprite.2d");
  if (!type) return;
  spr_factory = type->NewFactory ();
  current_lod = 1;
  current_features = 0;
  csRef<iEngine> eng = csQueryRegistry<iEngine> (object_reg);
  engine = eng;	// We don't want to keep a reference.
  light_mgr = csQueryRegistry<iLightManager> (object_reg);

  g3d = csQueryRegistry<iGraphics3D> (object_reg);

  part_sides = 0;
}

csParticleSystem::~csParticleSystem()
{
  if (vis_cb) vis_cb->DecRef ();
  RemoveParticles ();
}

void csParticleSystem::SetupObject ()
{
  if (!initialized)
  {
    part_sides = 0;
  }
}

void csParticleSystem::SetupBuffers (size_t part_sides)
{
  if (csParticleSystem::part_sides == part_sides) return;
  csParticleSystem::part_sides = part_sides;

  VertexCount = number * part_sides;
  TriangleCount = number * (part_sides-2);

  index_buffer = csRenderBuffer::CreateIndexRenderBuffer (
        TriangleCount*3, CS_BUF_STATIC,
        CS_BUFCOMP_UNSIGNED_INT, 0, VertexCount - 1);

  {
    csRenderBufferLock<csTriangle> trianglesLock (index_buffer);

    size_t i;
    csTriangle* tri = trianglesLock.Lock(); 
    for (i = 0 ; i < number ; i++)
    {
      // fill the triangle table
      size_t j;
      for (j = 2 ; j < part_sides ; j++)
      {
	*tri++ = csTriangle ((int)(i*part_sides+0), 
	  (int)(i*part_sides+j-1), (int)(i*part_sides+j));
      }
    }
  }
}

void csParticleSystem::RemoveParticles ()
{
  if (particles.GetSize () <= 0) return;

  particles.DeleteAll ();
  sprite2ds.DeleteAll ();
  partmeshes.DeleteAll ();
  ShapeChanged ();
}

void csParticleSystem::AppendRectSprite (float width, float height,
  iMaterialWrapper *mat, bool lighted)
{
  csRef<iMeshObject> sprmesh (spr_factory->NewInstance ());
  csRef<iParticle> part (scfQueryInterface<iParticle> (sprmesh));
  csRef<iSprite2DState> state (scfQueryInterface<iSprite2DState> (sprmesh));
  state->EnsureVertexCopy();
  csRef<iColoredVertices> vs = state->GetVertices();

  vs->SetSize (4);
  vs->Get (0).pos.Set (-width,-height); 
  vs->Get (0).u=0.; vs->Get (0).v=1.;
  vs->Get (0).color.Set (0, 0, 0);
  vs->Get (1).pos.Set (-width,+height); 
  vs->Get (1).u=0.; vs->Get (1).v=0.;
  vs->Get (1).color.Set (0, 0, 0);
  vs->Get (2).pos.Set (+width,+height); 
  vs->Get (2).u=1.; vs->Get (2).v=0.;
  vs->Get (2).color.Set (0, 0, 0);
  vs->Get (3).pos.Set (+width,-height); 
  vs->Get (3).u=1.; vs->Get (3).v=1.;
  vs->Get (3).color.Set (0, 0, 0);
  state->SetLighting (lighted);
  sprmesh->SetColor (csColor (1.0, 1.0, 1.0));
  sprmesh->SetMaterialWrapper (mat);
  AppendParticle (sprmesh, part, state);
  ShapeChanged ();
}


void csParticleSystem::AppendRegularSprite (int n, float radius,
  iMaterialWrapper* mat, bool lighted)
{
  csRef<iMeshObject> sprmesh (spr_factory->NewInstance ());
  csRef<iParticle> part (scfQueryInterface<iParticle> (sprmesh));
  csRef<iSprite2DState> state (scfQueryInterface<iSprite2DState> (sprmesh));
  state->EnsureVertexCopy();
  state->CreateRegularVertices (n, true);
  part->ScaleBy (radius);
  if (mat) sprmesh->SetMaterialWrapper (mat);
  state->SetLighting (lighted);
  sprmesh->SetColor (csColor (1.0, 1.0, 1.0));

  AppendParticle (sprmesh, part, state);
  ShapeChanged ();
}


void csParticleSystem::SetupMixMode ()
{
  size_t i;
  for (i = 0 ; i < particles.GetSize () ; i++)
  {
    csRef<iMeshObject> sprmesh = scfQueryInterface<iMeshObject> (GetParticle (i));
    sprmesh->SetMixMode (MixMode);
  }
}


void csParticleSystem::SetupColor ()
{
  size_t i;
  for(i = 0 ; i < particles.GetSize () ; i++)
  {
    csRef<iMeshObject> sprmesh = scfQueryInterface<iMeshObject> (GetParticle (i));
    sprmesh->SetColor (color);
  }
}


void csParticleSystem::AddColor (const csColor& col)
{
  size_t i;
  for(i = 0; i<particles.GetSize (); i++)
    GetParticle(i)->AddColor(col);
}


//void csParticleSystem::SetPosition (const csVector3& pos)
//{
  //for(int i = 0; i<particles.GetSize (); i++)
    //GetParticle(i)->SetPosition(pos);
//}


//void csParticleSystem::MovePosition (const csVector3& move)
//{
  //for(int i = 0; i<particles.GetSize (); i++)
    //GetParticle(i)->MovePosition(move);
//}


void csParticleSystem::ScaleBy (float factor)
{
  size_t i;
  for (i = 0 ; i<particles.GetSize () ; i++)
    GetParticle (i)->ScaleBy (factor);
  ShapeChanged ();
}


void csParticleSystem::Rotate (float angle)
{
  size_t i;
  for (i = 0 ; i<particles.GetSize () ; i++)
    GetParticle (i)->Rotate (angle);
  ShapeChanged ();
}


void csParticleSystem::Update (csTicks elapsed_time)
{
  if (self_destruct)
  {
    if (elapsed_time >= time_to_live)
    {
      if (engine)
      {
        csRef<iMeshWrapper> m = scfQueryInterface<iMeshWrapper> (logparent);
	if (m)
          engine->WantToDie (m);
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
    ScaleBy (pow (scalepersecond, elapsed_seconds));
  if (change_alpha)
  {
    alpha_now += alphapersecond * elapsed_seconds;
    if (alpha_now < 0.0f) alpha_now = 0.0f;
    else if (alpha_now > 1.0f) alpha_now = 1.0f;
    MixMode = CS_FX_SETALPHA (alpha_now);
    SetupMixMode ();
  }
  if (change_rotation)
    Rotate (anglepersecond * elapsed_seconds);
}

bool csParticleSystem::PreGetRenderMeshes (iRenderView*, iMovable* movable,
	uint32)
{
  SetupObject ();

  if (light_mgr)
  {
    csSafeCopyArray<csLightInfluence> lightInfluences;
    scfArrayWrap<iLightInfluenceArray, csSafeCopyArray<csLightInfluence> > 
      relevantLights (lightInfluences); //Yes, know, its on the stack...

    light_mgr->GetRelevantLights (logparent, &relevantLights, -1);
    UpdateLighting (lightInfluences, movable);
  }

  return true;
}

csRenderMesh** csParticleSystem::GetRenderMeshes (int& n, iRenderView* rview, 
						  iMovable* movable,
						  uint32 frustum_mask)
{
  if ((sprite2ds.GetSize () == 0)
  	|| !PreGetRenderMeshes (rview, movable, frustum_mask))
  {
    n = 0;
    return 0;
  }

  iMaterialWrapper* m = partmeshes[0]->GetMaterialWrapper ();
  if (!m)
  {
    n = 0;
    return 0;
  }

  int ClipPortal, ClipPlane, ClipZ;
  CS::RenderViewClipper::CalculateClipSettings (rview->GetRenderContext(),
    frustum_mask, ClipPortal, ClipPlane, ClipZ);

  // get the object-to-camera transformation
  iCamera *camera = rview->GetCamera ();
  csReversibleTransform trans = camera->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    trans /= movable->GetFullTransform ();

  SetupBuffers (sprite2ds[0]->GetVertices ()->GetSize());

  const uint currentFrame = rview->GetCurrentFrameNumber ();

  bool frameDataCreated;
  PerFrameData& frameData = perFrameHolder.GetUnusedData (frameDataCreated,
    currentFrame);
  if (frameDataCreated 
    || (frameData.vertex_buffer->GetElementCount() != (uint)VertexCount))
  {
    frameData.vertex_buffer = csRenderBuffer::CreateRenderBuffer (
      VertexCount, CS_BUF_DYNAMIC, CS_BUFCOMP_FLOAT, 3);
    frameData.texel_buffer = csRenderBuffer::CreateRenderBuffer (
      VertexCount, CS_BUF_DYNAMIC, CS_BUFCOMP_FLOAT, 2);
    frameData.color_buffer = csRenderBuffer::CreateRenderBuffer (
      VertexCount, CS_BUF_DYNAMIC, CS_BUFCOMP_FLOAT, 4);

    frameData.bufferHolder.AttachNew (new csRenderBufferHolder);
    frameData.bufferHolder->SetRenderBuffer (CS_BUFFER_INDEX, index_buffer);
    frameData.bufferHolder->SetRenderBuffer (CS_BUFFER_POSITION, 
      frameData.vertex_buffer);
    frameData.bufferHolder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, 
      frameData.texel_buffer);
    frameData.bufferHolder->SetRenderBuffer (CS_BUFFER_COLOR, 
      frameData.color_buffer);
  }

  size_t i;
  //csColor* c = colors;
  //csVector3* vt = vertices;
  //csVector2* txt = texels;
  size_t p = 0;
  csRenderBufferLock<csVector4> c (frameData.color_buffer);
  csRenderBufferLock<csVector3> vt (frameData.vertex_buffer);
  csRenderBufferLock<csVector2> txt (frameData.texel_buffer);
  for (i = 0 ; i < sprite2ds.GetSize () ; i++)
  {
    iColoredVertices* sprvt = sprite2ds[i]->GetVertices ();
    // transform to eye coordinates
    csVector3 pos = trans.Other2This (particles[i]->GetPosition ());
    uint mixmode = partmeshes[i]->GetMixMode ();
    float alpha = 1.0f - ((mixmode & CS_FX_MASK_ALPHA) / 255.0f);

    size_t j;
    for (j = 0 ; j < part_sides ; j++)
    {
      const csSprite2DVertex& vtx = sprvt->Get (j);
      vt[p] = pos + csVector3 (vtx.pos.x, vtx.pos.y, 0); 
      c[p].Set (vtx.color.red, vtx.color.green, vtx.color.blue, 
        alpha);
      txt[p].Set (vtx.u, vtx.v);
      p++;
    }
  }

  m->Visit ();

  //index_buffer->CopyToBuffer (triangles,
  //    	sizeof (unsigned int) * TriangleCount *3);

  bool meshCreated;
  csRenderMesh*& rm = rmHolder.GetUnusedMesh (meshCreated, currentFrame);

  if (meshCreated)
  {
#include "csutil/custom_new_disable.h"
    rm->variablecontext.AttachNew (new csShaderVariableContext);
#include "csutil/custom_new_enable.h"
  }
  rm->buffers = frameData.bufferHolder;

  // Prepare for rendering.
  uint mixmode = partmeshes[0]->GetMixMode ();
  if ((mixmode & CS_FX_MASK_MIXMODE) == CS_FX_COPY)
    // Hack to force alpha blending...
    mixmode = CS_FX_ALPHA | (mixmode & CS_FX_MASK_ALPHA);
  else
    rm->mixmode = mixmode & ~CS_FX_MASK_ALPHA;
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
  rm->indexend = (uint)TriangleCount * 3;
  rm->material = m;
  rm->worldspace_origin = movable->GetFullPosition ();
  rm->object2world = camera->GetTransform ();
  rm->bbox = GetObjectBoundingBox();  

  n = 1;
  return &rm;
}

void csParticleSystem::UpdateLighting (
    const csSafeCopyArray<csLightInfluence>& lights,
    iMovable* movable)
{
  SetupObject ();
  csReversibleTransform trans = movable->GetFullTransform ();
  size_t i;
  for (i = 0 ; i < particles.GetSize () ; i++)
    GetParticle (i)->UpdateLighting (lights, trans);
}

//---------------------------------------------------------------------

csVector3 csParticleSystem::GetRandomDirection ()
{
  csVector3 dir;
  dir.x = 2.0 * randgen.Get() - 1.0;
  dir.y = 2.0 * randgen.Get() - 1.0;
  dir.z = 2.0 * randgen.Get() - 1.0;
  return dir;
}

csVector3 csParticleSystem::GetRandomDirection (csVector3 const& magnitude,
	csVector3 const& offset)
{
  csVector3 dir;
  dir.x = randgen.Get() * magnitude.x;
  dir.y = randgen.Get() * magnitude.y;
  dir.z = randgen.Get() * magnitude.z;
  dir += offset;
  return dir;
}

csVector3 csParticleSystem::GetRandomPosition (csBox3 const& box)
{
  csVector3 dir;
  dir = box.Max() - box.Min();
  dir.x *= randgen.Get();
  dir.y *= randgen.Get();
  dir.z *= randgen.Get();
  dir += box.Min();
  return dir;
}
//-- csNewtonianParticleSystem ------------------------------------------

csNewtonianParticleSystem::csNewtonianParticleSystem (
	iObjectRegistry* object_reg,
	iMeshObjectFactory* factory)
  : scfImplementationType(this, object_reg, factory)
{
  // create csVector3's
  part_speed = 0;
  part_accel = 0;
}

void csNewtonianParticleSystem::SetCount (int max)
{
  delete[] part_speed;
  delete[] part_accel;
  part_speed = new csVector3 [max];
  part_accel = new csVector3 [max];
}

csNewtonianParticleSystem::~csNewtonianParticleSystem ()
{
  delete[] part_speed;
  delete[] part_accel;
}


void csNewtonianParticleSystem::Update (csTicks elapsed_time)
{
  csVector3 move;
  csParticleSystem::Update (elapsed_time);
  // time passed; together with CS 1 unit = 1 meter makes units right.
  float delta_t = elapsed_time / 1000.0f; // in seconds
  size_t i;
  for (i=0 ; i < particles.GetSize () ; i++)
  {
    // notice that the ordering of the lines (1) and (2) makes the
    // resulting newpos = a*dt^2 + v*dt + oldposition (i.e. paraboloid).
    part_speed[i] += part_accel[i] * delta_t; // (1)
    move = part_speed[i] * delta_t; // (2)
    GetParticle (i)->MovePosition (move);
  }
}

