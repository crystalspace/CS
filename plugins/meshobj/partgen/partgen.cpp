/*
    Copyright (C) 2000 by Jorrit Tyberghein
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
#include "csgeom/fastsqrt.h"
#include "plugins/meshobj/partgen/partgen.h"
#include "imeshobj.h"
#include "isystem.h"
#include "imspr2d.h"
#include <math.h>
#include <stdlib.h>

IMPLEMENT_IBASE (csParticleSystem)
  IMPLEMENTS_INTERFACE (iMeshObject)
  IMPLEMENTS_EMBEDDED_INTERFACE (iParticleState)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csParticleSystem::ParticleState)
  IMPLEMENTS_INTERFACE (iParticleState)
IMPLEMENT_EMBEDDED_IBASE_END

csParticleSystem::csParticleSystem (iSystem* system)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiParticleState);
  particles.SetLength(0);
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

  iMeshObjectType* type = QUERY_PLUGIN_CLASS (system, "crystalspace.meshobj.spr2d",
      "MeshObj", iMeshObjectType);
  if (!type) type = LOAD_PLUGIN (system, "crystalspace.meshobj.spr2d", "MeshObj", iMeshObjectType);
  spr_factory = type->NewFactory ();
}

csParticleSystem::~csParticleSystem()
{
  RemoveParticles ();
  if (spr_factory) spr_factory->DecRef ();
}

void csParticleSystem::RemoveParticles ()
{
  for (int i=0 ; i < particles.Length () ; i++)
    if (particles[i])
      GetParticle (i)->DecRef ();
  particles.DeleteAll ();
}

void csParticleSystem::AppendRectSprite (float width, float height, 
  iMaterialWrapper *mat, bool lighted)
{
  iParticle *pTicle;
  iMeshObject* sprmesh = spr_factory->NewInstance ();
  iParticle* part = QUERY_INTERFACE (sprmesh, iParticle);
  iSprite2DState* state = QUERY_INTERFACE (sprmesh, iSprite2DState);
  csColoredVertices& vs = state->GetVertices();

  vs.SetLimit(4);
  vs.SetLength(4);
  vs[0].pos.Set(-width,-height); vs[0].u=0.; vs[0].v=1.;
  vs[1].pos.Set(-width,+height); vs[1].u=0.; vs[1].v=0.;
  vs[2].pos.Set(+width,+height); vs[2].u=1.; vs[2].v=0.;
  vs[3].pos.Set(+width,-height); vs[3].u=1.; vs[3].v=1.;
  state->SetLighting( lighted );
  part->SetColor( csColor(1.0, 1.0, 1.0) );
  state->SetMaterialWrapper (mat);
  AppendParticle(pTicle = part);
  pTicle->DecRef ();
  part->DecRef(); 
  
}


void csParticleSystem::AppendRegularSprite (int n, float radius, 
  iMaterialWrapper* mat, bool lighted)
{
  iParticle *pTicle;
  iMeshObject* sprmesh = spr_factory->NewInstance ();
  iParticle* part = QUERY_INTERFACE (sprmesh, iParticle);
  iSprite2DState* state = QUERY_INTERFACE (sprmesh, iSprite2DState);
  state->CreateRegularVertices (n, true);
  part->ScaleBy(radius);
  state->SetMaterialWrapper (mat);
  state->SetLighting( lighted );
  part->SetColor( csColor(1.0, 1.0, 1.0) );

  AppendParticle(pTicle = part);
  pTicle->DecRef ();
  part->DecRef(); 
}


void csParticleSystem::SetupMixMode ()
{
  for (int i = 0 ; i < particles.Length () ; i++)
    GetParticle (i)->SetMixmode (MixMode);
}


void csParticleSystem::SetupColor ()
{
  for(int i = 0 ; i < particles.Length () ; i++)
    GetParticle (i)->SetColor (color);
}


void csParticleSystem::AddColor (const csColor& col)
{
  for(int i = 0; i<particles.Length(); i++)
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
  for (int i = 0 ; i<particles.Length () ; i++)
    GetParticle (i)->ScaleBy (factor);
}


void csParticleSystem::Rotate (float angle)
{
  for (int i = 0 ; i<particles.Length () ; i++)
    GetParticle (i)->Rotate (angle);
}


void csParticleSystem::Update (cs_time elapsed_time)
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

bool csParticleSystem::DrawTest (iRenderView*, iMovable*)
{
  SetupObject ();
  return true;
}

bool csParticleSystem::Draw (iRenderView* rview, iMovable* /*movable*/)
{
  for (int i = 0 ; i < particles.Length() ; i++)
    GetParticle (i)->Draw (rview);
  return true;
}

void csParticleSystem::UpdateLighting (iLight** lights, int num_lights,
    iMovable* movable)
{
  SetupObject ();
  for (int i = 0 ; i < particles.Length() ; i++)
    GetParticle (i)->UpdateLighting (lights, num_lights);
}

//---------------------------------------------------------------------

csVector3& csParticleSystem::GetRandomDirection ()
{
  static csVector3 dir;
  dir.x = 2.0 * rand() / (1.0+RAND_MAX) - 1.0;
  dir.y = 2.0 * rand() / (1.0+RAND_MAX) - 1.0;
  dir.z = 2.0 * rand() / (1.0+RAND_MAX) - 1.0;
  return dir;
}

csVector3& csParticleSystem::GetRandomDirection (const csVector3& magnitude,
	const csVector3& offset)
{
  static csVector3 dir;
  dir.x = (rand() / (1.0+RAND_MAX)) * magnitude.x;
  dir.y = (rand() / (1.0+RAND_MAX)) * magnitude.y;
  dir.z = (rand() / (1.0+RAND_MAX)) * magnitude.z;
  dir = dir + offset;
  return dir;
}

//-- csNewtonianParticleSystem ------------------------------------------

csNewtonianParticleSystem::csNewtonianParticleSystem (iSystem* system)
  : csParticleSystem (system)
{
  // create csVector3's
  part_speed = NULL;
  part_accel = NULL;
}

void csNewtonianParticleSystem::SetNumber (int max)
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


void csNewtonianParticleSystem::Update (cs_time elapsed_time)
{
  csVector3 move;
  csParticleSystem::Update(elapsed_time);
  // time passed; together with CS 1 unit = 1 meter makes units right.
  float delta_t = elapsed_time / 1000.0f; // in seconds
  for (int i=0; i<particles.Length(); i++)
  {
    // notice that the ordering of the lines (1) and (2) makes the
    // resulting newpos = a*dt^2 + v*dt + oldposition (i.e. paraboloid).
    part_speed[i] += part_accel[i] * delta_t; // (1)
    move = part_speed[i] * delta_t; // (2)
    GetParticle(i)->MovePosition (move); 
  }
}

