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
#include "csgfx/renderbuffer.h"
#include "csgeom/matrix3.h"
#include "csgeom/transfrm.h"
#include "csgeom/box.h"
#include "cstool/rbuflock.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/rview.h"
#include "iengine/camera.h"
#include "iengine/material.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/strset.h"
#include "imesh/sprite2d.h"
#include "iengine/movable.h"
#include "ivideo/rendermesh.h"
#include "csplugincommon/particlesys/partgen.h"
#include <math.h>
#include <stdlib.h>

SCF_IMPLEMENT_IBASE (csParticleSystem)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iParticleState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csParticleSystem::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csParticleSystem::ParticleState)
  SCF_IMPLEMENTS_INTERFACE (iParticleState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csParticleSystem::csParticleSystem (iObjectRegistry* object_reg,
				    iMeshObjectFactory* factory) 
{
  SCF_CONSTRUCT_IBASE (factory);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiParticleState);
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
  radius.Set (0, 0, 0);
  color.Set (0, 0, 0);
  csRef<iPluginManager> plugin_mgr (
	  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.sprite.2d", iMeshObjectType));
  if (!type) type = CS_LOAD_PLUGIN (plugin_mgr,
  	"crystalspace.mesh.object.sprite.2d", iMeshObjectType);
  spr_factory = type->NewFactory ();
  current_lod = 1;
  current_features = 0;
  csRef<iEngine> eng = CS_QUERY_REGISTRY (object_reg, iEngine);
  engine = eng;	// We don't want to keep a reference.
  light_mgr = CS_QUERY_REGISTRY (object_reg, iLightManager);

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg,
    "crystalspace.shared.stringset", iStringSet);
  string_object2world = strings->Request ("object2world transform");

  vertices = 0;
  texels = 0;
  colors = 0;
  part_sides = 0;

}

csParticleSystem::~csParticleSystem()
{
  delete[] vertices;
  delete[] texels;
  delete[] colors;

  if (vis_cb) vis_cb->DecRef ();
  RemoveParticles ();
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiParticleState);
  SCF_DESTRUCT_IBASE ();
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

  delete[] texels;
  texels = new csVector2 [VertexCount];
  delete[] colors;
  colors = new csColor [VertexCount];
  delete[] vertices;
  vertices = new csVector3 [VertexCount];

  vertex_buffer = csRenderBuffer::CreateRenderBuffer (
        VertexCount, CS_BUF_DYNAMIC, 
        CS_BUFCOMP_FLOAT, 3);
  texel_buffer = csRenderBuffer::CreateRenderBuffer (
        VertexCount, CS_BUF_DYNAMIC, 
        CS_BUFCOMP_FLOAT, 2);
  color_buffer = csRenderBuffer::CreateRenderBuffer (
        VertexCount, CS_BUF_DYNAMIC,
        CS_BUFCOMP_FLOAT, 3);
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

  bufferHolder.AttachNew (new csRenderBufferHolder);
  bufferHolder->SetRenderBuffer (CS_BUFFER_INDEX, index_buffer);
  bufferHolder->SetRenderBuffer (CS_BUFFER_POSITION, vertex_buffer);
  bufferHolder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, texel_buffer);
  bufferHolder->SetRenderBuffer (CS_BUFFER_COLOR, color_buffer);

}

void csParticleSystem::RemoveParticles ()
{
  if (particles.Length () <= 0) return;

  particles.DeleteAll ();
  sprite2ds.DeleteAll ();
  scfiObjectModel.ShapeChanged ();
}

void csParticleSystem::AppendRectSprite (float width, float height,
  iMaterialWrapper *mat, bool lighted)
{
  csRef<iMeshObject> sprmesh (spr_factory->NewInstance ());
  csRef<iParticle> part (SCF_QUERY_INTERFACE (sprmesh, iParticle));
  csRef<iSprite2DState> state (SCF_QUERY_INTERFACE (sprmesh, iSprite2DState));
  csColoredVertices& vs = state->GetVertices();

  vs.SetLength (4);
  vs[0].pos.Set (-width,-height); vs[0].u=0.; vs[0].v=1.;
  vs[0].color.Set (0, 0, 0);
  vs[1].pos.Set (-width,+height); vs[1].u=0.; vs[1].v=0.;
  vs[1].color.Set (0, 0, 0);
  vs[2].pos.Set (+width,+height); vs[2].u=1.; vs[2].v=0.;
  vs[2].color.Set (0, 0, 0);
  vs[3].pos.Set (+width,-height); vs[3].u=1.; vs[3].v=1.;
  vs[3].color.Set (0, 0, 0);
  state->SetLighting (lighted);
  part->SetColor (csColor (1.0, 1.0, 1.0));
  state->SetMaterialWrapper (mat);
  AppendParticle (part, state);
  scfiObjectModel.ShapeChanged ();
}


void csParticleSystem::AppendRegularSprite (int n, float radius,
  iMaterialWrapper* mat, bool lighted)
{
  csRef<iMeshObject> sprmesh (spr_factory->NewInstance ());
  csRef<iParticle> part (SCF_QUERY_INTERFACE (sprmesh, iParticle));
  csRef<iSprite2DState> state (SCF_QUERY_INTERFACE (sprmesh, iSprite2DState));
  state->CreateRegularVertices (n, true);
  part->ScaleBy (radius);
  if (mat) state->SetMaterialWrapper (mat);
  state->SetLighting (lighted);
  part->SetColor (csColor (1.0, 1.0, 1.0));

  AppendParticle (part, state);
  scfiObjectModel.ShapeChanged ();
}


void csParticleSystem::SetupMixMode ()
{
  size_t i;
  for (i = 0 ; i < particles.Length () ; i++)
    GetParticle (i)->SetMixMode (MixMode);
}


void csParticleSystem::SetupColor ()
{
  size_t i;
  for(i = 0 ; i < particles.Length () ; i++)
    GetParticle (i)->SetColor (color);
}


void csParticleSystem::AddColor (const csColor& col)
{
  size_t i;
  for(i = 0; i<particles.Length(); i++)
    GetParticle(i)->AddColor(col);
}


//void csParticleSystem::SetPosition (const csVector3& pos)
//{
  //for(int i = 0; i<particles.Length(); i++)
    //GetParticle(i)->SetPosition(pos);
//}


//void csParticleSystem::MovePosition (const csVector3& move)
//{
  //for(int i = 0; i<particles.Length(); i++)
    //GetParticle(i)->MovePosition(move);
//}


void csParticleSystem::ScaleBy (float factor)
{
  size_t i;
  for (i = 0 ; i<particles.Length () ; i++)
    GetParticle (i)->ScaleBy (factor);
  scfiObjectModel.ShapeChanged ();
}


void csParticleSystem::Rotate (float angle)
{
  size_t i;
  for (i = 0 ; i<particles.Length () ; i++)
    GetParticle (i)->Rotate (angle);
  scfiObjectModel.ShapeChanged ();
}


void csParticleSystem::Update (csTicks elapsed_time)
{
  if (self_destruct)
  {
    if (elapsed_time >= time_to_live)
    {
      if (engine)
      {
        csRef<iMeshWrapper> m = SCF_QUERY_INTERFACE (logparent, iMeshWrapper);
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

bool csParticleSystem::PreGetRenderMeshes (iRenderView*, iMovable* movable, uint32)
{
  SetupObject ();

  if (light_mgr)
  {
    const csArray<iLight*>& relevant_lights = light_mgr
    	->GetRelevantLights (logparent, -1, false);
    UpdateLighting (relevant_lights, movable);
  }

  return true;
}

csRenderMesh** csParticleSystem::GetRenderMeshes (int& n, iRenderView* rview, 
						  iMovable* movable,
						  uint32 frustum_mask)
{
  if ((sprite2ds.Length() == 0) || !PreGetRenderMeshes (rview, movable, frustum_mask))
  {
    n = 0;
    return 0;
  }

  int ClipPortal, ClipPlane, ClipZ;
  rview->CalculateClipSettings (frustum_mask, ClipPortal, ClipPlane, ClipZ);

  // get the object-to-camera transformation
  iCamera *camera = rview->GetCamera ();
  csReversibleTransform trans = camera->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    trans /= movable->GetFullTransform ();

  SetupBuffers (sprite2ds[0]->GetVertices ().Length ());

  size_t i;
  csColor* c = colors;
  csVector3* vt = vertices;
  csVector2* txt = texels;
  for (i = 0 ; i < sprite2ds.Length () ; i++)
  {
    csColoredVertices& sprvt = sprite2ds[i]->GetVertices ();
    // transform to eye coordinates
    csVector3 pos = trans.Other2This (particles[i]->GetPosition ());

    size_t j;
    for (j = 0 ; j < part_sides ; j++)
    {
      *vt++ = pos + csVector3 (sprvt[j].pos.x, sprvt[j].pos.y, 0);
      *c++ = sprvt[j].color;
      (*txt++).Set (sprvt[j].u, sprvt[j].v);
    }
  }

  iMaterialWrapper* m = sprite2ds[0]->GetMaterialWrapper ();
  m->Visit ();

  vertex_buffer->CopyInto (vertices, VertexCount);
  texel_buffer->CopyInto (texels, VertexCount);
  color_buffer->CopyInto (colors, VertexCount);
  //index_buffer->CopyToBuffer (triangles,
  //    	sizeof (unsigned int) * TriangleCount *3);

  bool meshCreated;
  csRenderMesh*& rm = rmHolder.GetUnusedMesh (meshCreated, 
    rview->GetCurrentFrameNumber ());

  if (meshCreated)
  {
    rm->buffers = bufferHolder;
    rm->variablecontext.AttachNew (new csShaderVariableContext);
  }

  // Prepare for rendering.
  rm->mixmode = sprite2ds[0]->GetMixMode ();
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
  CS_ASSERT (m != 0);
  rm->worldspace_origin = movable->GetFullPosition ();
  rm->variablecontext->GetVariableAdd (string_object2world)->SetValue (camera->GetTransform ());  

  n = 1;
  return &rm;
}

void csParticleSystem::UpdateLighting (const csArray<iLight*>& lights,
    iMovable* movable)
{
  SetupObject ();
  csReversibleTransform trans = movable->GetFullTransform ();
  size_t i;
  for (i = 0 ; i < particles.Length () ; i++)
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
  : csParticleSystem (object_reg, factory)
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
  for (i=0 ; i < particles.Length () ; i++)
  {
    // notice that the ordering of the lines (1) and (2) makes the
    // resulting newpos = a*dt^2 + v*dt + oldposition (i.e. paraboloid).
    part_speed[i] += part_accel[i] * delta_t; // (1)
    move = part_speed[i] * delta_t; // (2)
    GetParticle (i)->MovePosition (move);
  }
}

