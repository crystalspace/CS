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
#include "csgeom/matrix3.h"
#include "csgeom/transfrm.h"
#include "csgeom/box.h"
#include "partgen.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/rview.h"
#include "iengine/camera.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imesh/sprite2d.h"
#include "iengine/movable.h"
#include "iutil/objreg.h"
#include <math.h>
#include <stdlib.h>

SCF_IMPLEMENT_IBASE (csParticleSystem)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iParticleState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csParticleSystem::ParticleState)
  SCF_IMPLEMENTS_INTERFACE (iParticleState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csParticleSystem::csParticleSystem (iObjectRegistry* object_reg,
	iMeshObjectFactory* factory)
{
  SCF_CONSTRUCT_IBASE (factory);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiParticleState);
  initialized = false;
  csParticleSystem::factory = factory;
  logparent = NULL;
  particles.SetLength (0);
  self_destruct = false;
  time_to_live = 0;
  to_delete = false;
  // defaults
  change_size = false;
  change_color = false;
  change_alpha = false;
  change_rotation = false;
  // bbox is empty.
  spr_factory = NULL;
  prev_time = 0;
  MixMode = 0;
  vis_cb = NULL;
  mat = NULL;
  radius.Set (0, 0, 0);

  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.sprite.2d", iMeshObjectType);
  if (!type) type = CS_LOAD_PLUGIN (plugin_mgr,
  	"crystalspace.mesh.object.sprite.2d", iMeshObjectType);
  plugin_mgr->DecRef ();
  spr_factory = type->NewFactory ();
  type->DecRef ();
  shapenr = 0;
  current_lod = 1;
  current_features = 0;
}

csParticleSystem::~csParticleSystem()
{
  if (vis_cb) vis_cb->DecRef ();
  RemoveParticles ();
  if (spr_factory) spr_factory->DecRef ();
}

void csParticleSystem::RemoveParticles ()
{
  int i;
  for (i=0 ; i < particles.Length () ; i++)
    if (particles[i])
      GetParticle (i)->DecRef ();
  particles.DeleteAll ();
  shapenr++;
}

void csParticleSystem::AppendRectSprite (float width, float height,
  iMaterialWrapper *mat, bool lighted)
{
  iMeshObject* sprmesh = spr_factory->NewInstance ();
  iParticle* part = SCF_QUERY_INTERFACE (sprmesh, iParticle);
  iSprite2DState* state = SCF_QUERY_INTERFACE (sprmesh, iSprite2DState);
  csColoredVertices& vs = state->GetVertices();

  vs.SetLimit (4);
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
  AppendParticle (part);
  part->DecRef ();
  state->DecRef ();
  sprmesh->DecRef ();
  shapenr++;
}


void csParticleSystem::AppendRegularSprite (int n, float radius,
  iMaterialWrapper* mat, bool lighted)
{
  iMeshObject* sprmesh = spr_factory->NewInstance ();
  iParticle* part = SCF_QUERY_INTERFACE (sprmesh, iParticle);
  iSprite2DState* state = SCF_QUERY_INTERFACE (sprmesh, iSprite2DState);
  state->CreateRegularVertices (n, true);
  part->ScaleBy (radius);
  if (mat) state->SetMaterialWrapper (mat);
  state->SetLighting (lighted);
  part->SetColor (csColor (1.0, 1.0, 1.0));

  AppendParticle (part);
  part->DecRef ();
  sprmesh->DecRef ();
  state->DecRef ();
  shapenr++;
}


void csParticleSystem::SetupMixMode ()
{
  int i;
  for (i = 0 ; i < particles.Length () ; i++)
    GetParticle (i)->SetMixMode (MixMode);
}


void csParticleSystem::SetupColor ()
{
  int i;
  for(i = 0 ; i < particles.Length () ; i++)
    GetParticle (i)->SetColor (color);
}


void csParticleSystem::AddColor (const csColor& col)
{
  int i;
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
  int i;
  for (i = 0 ; i<particles.Length () ; i++)
    GetParticle (i)->ScaleBy (factor);
  shapenr++;
}


void csParticleSystem::Rotate (float angle)
{
  int i;
  for (i = 0 ; i<particles.Length () ; i++)
    GetParticle (i)->Rotate (angle);
  shapenr++;
}


void csParticleSystem::Update (csTicks elapsed_time)
{
  if (self_destruct)
  {
    if (elapsed_time >= time_to_live)
    {
      to_delete = true;
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

bool csParticleSystem::DrawTest (iRenderView* rview, iMovable* movable)
{
  SetupObject ();
  // Based and copied this code on csSprite3DMeshObject's
  //   DrawTest, GetScreenBoundingBox, GetTransformedBoundingBox

  const int BOX_3D_CORNERS = 8;

  iCamera *camera = rview->GetCamera ();

   // Transform the particle's world bounding box (bbox)
   // from world to camera coordinates, including the movable
  csBox3 particle_cbox;
  csReversibleTransform trans = camera->GetTransform () *
    	movable->GetFullTransform ().GetInverse ();
  particle_cbox.AddBoundingVertex (trans * bbox.GetCorner (0));
  int i;
  for (i=1; i<BOX_3D_CORNERS; i++)
    particle_cbox.AddBoundingVertexSmart (trans * bbox.GetCorner (i));

   // Not visible if completely behind the camera
  if (particle_cbox.MaxZ () < 0) return false;

   // Transform from camera to screen space
  csBox2 particle_sbox;
  if (particle_cbox.MinZ () <= 0)
    particle_sbox.Set (-CS_BOUNDINGBOX_MAXVALUE, -CS_BOUNDINGBOX_MAXVALUE,
	CS_BOUNDINGBOX_MAXVALUE, CS_BOUNDINGBOX_MAXVALUE);
  else
  {
    float fov = camera->GetFOV ();
    float shift_x = camera->GetShiftX ();
    float shift_y = camera->GetShiftY ();

    csVector3 cv(particle_cbox.GetCorner (0));
    float inv_z = fov/cv.z;
    float sx = cv.x*inv_z + shift_x;
    float sy = cv.y*inv_z + shift_y;
    particle_sbox.StartBoundingBox (sx, sy);

    for (i=1 ; i<BOX_3D_CORNERS ; i++)
    {
      cv = particle_cbox.GetCorner (i);
      inv_z = fov/cv.z;
      sx = cv.x*inv_z + shift_x;
      sy = cv.y*inv_z + shift_y;
      particle_sbox.AddBoundingVertexSmart (sx, sy);
    }
  }

   // Test visibility of bounding box with the clipper
  int clip_portal, clip_plane, clip_z_plane;
  if (!rview->ClipBBox (particle_sbox, particle_cbox,
                      clip_portal,clip_plane,clip_z_plane))
    return false;

  return true;
}

bool csParticleSystem::Draw (iRenderView* rview, iMovable* movable,
	csZBufMode mode)
{
  if (vis_cb) if (!vis_cb->BeforeDrawing (this, rview)) return false;
  csReversibleTransform trans = movable->GetFullTransform ();
  int i;
  for (i = 0 ; i < particles.Length() ; i++)
    GetParticle (i)->Draw (rview, trans, mode);
  return true;
}

void csParticleSystem::UpdateLighting (iLight** lights, int num_lights,
    iMovable* movable)
{
  SetupObject ();
  csReversibleTransform trans = movable->GetFullTransform ();
  int i;
  for (i = 0 ; i < particles.Length () ; i++)
    GetParticle (i)->UpdateLighting (lights, num_lights, trans);
}

//---------------------------------------------------------------------

CS_IMPLEMENT_STATIC_VAR (RandDir1, csVector3, ())

csVector3& csParticleSystem::GetRandomDirection ()
{
  static csVector3 *dir = RandDir1 ();
  dir->x = 2.0 * rand() / (1.0+RAND_MAX) - 1.0;
  dir->y = 2.0 * rand() / (1.0+RAND_MAX) - 1.0;
  dir->z = 2.0 * rand() / (1.0+RAND_MAX) - 1.0;
  return *dir;
}

CS_IMPLEMENT_STATIC_VAR (RandDir2, csVector3, ())

csVector3& csParticleSystem::GetRandomDirection (const csVector3& magnitude,
	const csVector3& offset)
{
  static csVector3 *dir = RandDir2 ();
  dir->x = (rand() / (1.0+RAND_MAX)) * magnitude.x;
  dir->y = (rand() / (1.0+RAND_MAX)) * magnitude.y;
  dir->z = (rand() / (1.0+RAND_MAX)) * magnitude.z;
  *dir = *dir + offset;
  return *dir;
}

CS_IMPLEMENT_STATIC_VAR (RandDir3, csVector3, ())

csVector3& csParticleSystem::GetRandomPosition (const csBox3& box)
{
  static csVector3 *dir = RandDir3 ();
  *dir = box.Max() - box.Min();
  dir->x *= rand() / (1.0+RAND_MAX);
  dir->y *= rand() / (1.0+RAND_MAX);
  dir->z *= rand() / (1.0+RAND_MAX);
  *dir += box.Min();
  return *dir;
}
//-- csNewtonianParticleSystem ------------------------------------------

csNewtonianParticleSystem::csNewtonianParticleSystem (
	iObjectRegistry* object_reg,
	iMeshObjectFactory* factory)
  : csParticleSystem (object_reg, factory)
{
  // create csVector3's
  part_speed = NULL;
  part_accel = NULL;
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
  int i;
  for (i=0 ; i < particles.Length () ; i++)
  {
    // notice that the ordering of the lines (1) and (2) makes the
    // resulting newpos = a*dt^2 + v*dt + oldposition (i.e. paraboloid).
    part_speed[i] += part_accel[i] * delta_t; // (1)
    move = part_speed[i] * delta_t; // (2)
    GetParticle (i)->MovePosition (move);
  }
}

