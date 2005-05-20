/*
    Copyright (C) 2003 by Jorrit Tyberghein, John Harger, Daniel Duhprey

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
#include "csutil/ref.h"
#include "csutil/cscolor.h"
#include "csutil/event.h"
#include "iutil/objreg.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "imesh/particles.h"
#include "simplephys.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csParticlesPhysicsSimple)
  SCF_IMPLEMENTS_INTERFACE (iParticlesPhysics)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csParticlesPhysicsSimple::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csParticlesPhysicsSimple::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csParticlesPhysicsSimple)

csParticlesPhysicsSimple::csParticlesPhysicsSimple (iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiEventHandler);
}

csParticlesPhysicsSimple::~csParticlesPhysicsSimple ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiEventHandler);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csParticlesPhysicsSimple::Initialize (iObjectRegistry* reg)
{
  object_reg = reg;
  // hook into eventqueue
  csRef<iEventQueue> eq (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (eq == 0) return false;
  eq->RegisterListener (&scfiEventHandler, CSMASK_Nothing);

  vclock = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  leftover_time = 0;

  return true;
}

const csArray<csParticlesData> *csParticlesPhysicsSimple::RegisterParticles (
  iParticlesObjectState *particles)
{
  particles_object *part = new particles_object;
  part->particles = particles;
  part->dead_particles = 0;
  part->new_particles = 0;
  part->total_elapsed_time = 0.0f;
  partobjects.Push (part);
  return &part->data;
}

void csParticlesPhysicsSimple::RemoveParticles (
  iParticlesObjectState *particles)
{
  particles_object *temp;
  int end = (int)partobjects.Length () - 1;
  for (int i=end;i>=0;--i) {
    temp = partobjects[i];
    if (temp->particles == particles) {
      partobjects[i] = partobjects[end];
      partobjects[end] = temp;
      temp = partobjects.Pop ();
      delete temp;
      return;
    }
  }
}

void csParticlesPhysicsSimple::Start (iParticlesObjectState *particles)
{
  particles_object *part = FindParticles (particles);
  if(!part) return;
  
  int initial_particles = part->particles->GetInitialParticleCount();

  if(part->data.Length () < 1)
  {
    int start_size = 1000;
    if (initial_particles > start_size) start_size = initial_particles;

    part->data.SetLength (start_size);
    for (int i = 0; i < start_size; i++)
    {
      csParticlesData &p = part->data.Get (i);
      p.sort = -FLT_MAX;
      p.color.w = 0.0f;
      p.time_to_live = -1.0f;
    }
    part->dead_particles = start_size;
  }

  part->new_particles = (float)initial_particles;
  part->total_elapsed_time = 0.0f;
}

void csParticlesPhysicsSimple::Stop (iParticlesObjectState *particles)
{
  particles_object *part = FindParticles (particles);
  if(!part) return;

  part->total_elapsed_time = part->particles->GetEmitTime () + 50.0f;
  part->new_particles = 0.0f;
}

bool csParticlesPhysicsSimple::HandleEvent (iEvent &event)
{
  if (event.Type == csevBroadcast && csCommandEventHelper::GetCode(&event) == cscmdPreProcess)
  {
    csTicks elapsed = vclock->GetElapsedTicks ();
    int updates = (elapsed + leftover_time) / 20;
    leftover_time = (elapsed + leftover_time) - (updates * 20);

    float elapsedSecs = (float)elapsed * 0.001f;
    for (size_t i=0; i < partobjects.Length (); i++)
    {
      StepPhysics (elapsedSecs, partobjects[i]);
    }
  }
  return false;
}

void csParticlesPhysicsSimple::StepPhysics (float true_elapsed_time,
  particles_object *part)
{
  size_t i;
  float emit_time = part->particles->GetEmitTime ();
  if (part->total_elapsed_time < emit_time)
  {
    part->total_elapsed_time += true_elapsed_time;
    part->new_particles += true_elapsed_time * 
      (float)part->particles->GetParticlesPerSecond ();
  }

  if ((part->dead_particles-part->new_particles) <
    part->data.Length () * 0.30f)
  {
    int oldlen = (int)part->data.Length ();
    int newlen = (oldlen > (int)part->new_particles) ?
      oldlen << 1 : (int)part->new_particles << 1;
    part->data.SetLength (newlen);
    part->dead_particles += (int)part->data.Length() - oldlen;
    for(i = oldlen; i < part->data.Length (); i++) {
      csParticlesData &p = part->data.Get (i);
      p.sort = -FLT_MAX;
      p.color.w = 0.0f;
      p.time_to_live = -1.0f;
    }
  }
  else if (part->dead_particles - part->new_particles >
    part->data.Length () * 0.70f && part->data.Length() > 1)
  {
    int oldlen = (int)part->data.Length();
    part->data.Truncate ((part->data.Length () >> 1));
    part->dead_particles -= oldlen - (int)part->data.Length();
  }

  size_t dead_offset = part->data.Length() - part->dead_particles;

  csVector3 emitter;
  part->particles->GetEmitPosition (emitter);

  for (i = 0; i < (size_t)part->new_particles; i++)
  {
    csParticlesData &point = part->data.Get(i + dead_offset);
    // Emission
    csVector3 start;

    switch (part->particles->GetEmitType ())
    {
    case CS_PART_EMIT_SPHERE:
    {
      start = csVector3((rng.Get() - 0.5f) * 2.0f,
                        (rng.Get() - 0.5f) * 2.0f,
			                  (rng.Get() - 0.5f) * 2.0f);
      start.Normalize ();
      float inner_radius = part->particles->GetSphereEmitInnerRadius ();
      float outer_radius = part->particles->GetSphereEmitOuterRadius ();
      start = emitter +
        (start * ((rng.Get() * (outer_radius - inner_radius ))
        + inner_radius ));
      break;
    }
    case CS_PART_EMIT_PLANE:
    {
      start = csVector3((rng.Get() - 0.5f) * part->particles->GetEmitXSize(),
        0.0f, (rng.Get() - 0.5f) * part->particles->GetEmitYSize());
      start = part->particles->GetRotation () * start;
      start += emitter;
      break;
    }
    case CS_PART_EMIT_BOX:
      start = csVector3((rng.Get() - 0.5f) * part->particles->GetEmitXSize(),
        (rng.Get() - 0.5f) * part->particles->GetEmitYSize(),
        (rng.Get() - 0.5f) * part->particles->GetEmitZSize());
      start = part->particles->GetRotation () * start;
      start += emitter;
      break;
    case CS_PART_EMIT_CYLINDER:
      start = csVector3((rng.Get() - 0.5f) * 2.0f,
        0.0f,
			  (rng.Get() - 0.5f) * 2.0f);
      start.Normalize ();
      start *= part->particles->GetEmitXSize () * rng.Get();
      start.y = (rng.Get() - 0.5f) * part->particles->GetEmitYSize ();
      start = part->particles->GetRotation () * start;
      start += emitter;
      break;
    }

    point.position = start;
    point.color = csVector4 (0.0f, 0.0f, 0.0f, 0.0f);
    point.velocity = csVector3 (0.0f, 0.0f, 0.0f);
    point.time_to_live = part->particles->GetTimeToLive() +
      (part->particles->GetTimeVariation () * rng.Get());
    point.mass = part->particles->GetMass() +
      (rng.Get() * part->particles->GetMassVariation ());
  }

  float time_increment = true_elapsed_time / part->new_particles;

  dead_offset += (int)part->new_particles;
  part->dead_particles -= (int)part->new_particles;
  part->new_particles -= (int)part->new_particles;

  float elapsed_time = 0.0f;

  float force_range = part->particles->GetForceRange ();
  for (i=0; i < dead_offset; i++)
  {
    if(elapsed_time < true_elapsed_time - time_increment)
    {
      elapsed_time += time_increment;
    }
    else
    {
      elapsed_time = true_elapsed_time;
    }

    // Setup for this particle
    csParticlesData &point = part->data.Get (i);

    // Time until death
    point.time_to_live -= elapsed_time;
    if (point.time_to_live < 0.0f)
    {
      // Deletion :(
      point.color.w = 0.0f;
      point.sort = -FLT_MAX;
      part->dead_particles ++;
      continue;
    }

    // Diffusion
    csVector3 diff((rng.Get() * 2.0f) - 1.0f, (rng.Get() * 2.0f) - 1.0f,
      (rng.Get() * 2.0f) - 1.0f);
    diff *= part->particles->GetDiffusion ();
    point.position += (diff * elapsed_time);

    // Force related stuff
    float force_range_squared = (force_range * force_range);
    csVector3 dir = point.position - emitter;
    float dist_squared = dir.SquaredNorm();
    float falloff = 1.0f;

    csParticleForceType force_type;
    force_type = part->particles->GetForceType ();

    switch (force_type)
    {
    case CS_PART_FORCE_RADIAL:
      dir.Normalize ();
      break;
    case CS_PART_FORCE_LINEAR:
      part->particles->GetForceDirection (dir);
      break;
    case CS_PART_FORCE_CONE:
      part->particles->GetForceDirection (dir);
      break;
    }

    csParticleFalloffType force_falloff, cone_falloff;

    part->particles->GetFalloffType (force_falloff, cone_falloff);

    switch (force_falloff) {
    case CS_PART_FALLOFF_CONSTANT:
      if (dist_squared > force_range_squared) falloff = 0.0f;
      break;
    case CS_PART_FALLOFF_LINEAR:
      if (dist_squared > force_range_squared) falloff = 0.0f;
      else falloff = 1.0f - (dist_squared / force_range_squared);
      break;
    case CS_PART_FALLOFF_PARABOLIC:
      if (dist_squared < force_range_squared)
        falloff = (1.0f / (force_range_squared - dist_squared));
      break;
    }

    csVector3 gravity;
    part->particles->GetGravity (gravity);

    point.velocity += dir * (((part->particles->GetForce() * falloff -
      fabs(point.velocity.Norm()) * part->particles->GetDampener ()) /
      point.mass) * elapsed_time) + gravity * elapsed_time;
    point.position += point.velocity * elapsed_time;

    // The color functions
    switch (part->particles->GetParticleColorMethod ())
    {
    case CS_PART_COLOR_CONSTANT:
    {
      csColor4 constant_color;
      part->particles->GetConstantColor (constant_color);
      point.color.x = constant_color.red;
      point.color.y = constant_color.green;
      point.color.z = constant_color.blue;
      point.color.w = constant_color.alpha;//1.0f;
      break;
    }
    case CS_PART_COLOR_LINEAR:
    {
      float colortime = point.time_to_live
        / (part->particles->GetTimeToLive ()
        + part->particles->GetTimeVariation ());
      const csArray<csColor4> &gradient_colors =
        part->particles->GetGradient ();
      int color_len = (int)gradient_colors.Length();
      if (color_len)
      {
        // With a gradient
        float cref = (1.0f - colortime) * (float)(color_len-1);
        int index = (int)floor(cref);
        csColor4 color1 = gradient_colors.Get(index);
        csColor4 color2 = color1;
        if (index != color_len - 1)
        {
          color2 = gradient_colors.Get(index + 1);
        }

        float pos = cref - floor(cref);

        point.color.x = ((1.0f - pos) * color1.red) + (pos * color2.red);
        point.color.y = ((1.0f - pos) * color1.green) + (pos * color2.green);
        point.color.z = ((1.0f - pos) * color1.blue) + (pos * color2.blue);
        point.color.w = ((1.0f - pos) * color1.alpha) + (pos * color2.alpha);
      }
      else
      {
        // With no gradient set, use magic pink instead (fade to black)
        // Note, that this case is technically an error
        point.color.x = 1.0f * colortime;
        point.color.y = 0.0f * colortime;
        point.color.z = 1.0f * colortime;
	point.color.w = 1.0f;
      }
      //point.color.w = colortime;
      break;
    }
    case CS_PART_COLOR_HEAT:
      // @@@ TODO: Do this
      break;
    case CS_PART_COLOR_CALLBACK:
    {
      csRef<iParticlesColorCallback> color_callback =
        part->particles->GetColorCallback ();
      if (color_callback.IsValid())
      {
        float colortime = point.time_to_live
          / (part->particles->GetTimeToLive ()
          + part->particles->GetTimeVariation ());
        csColor color = color_callback->GetColor(colortime);
        point.color.x = color.red;
        point.color.y = color.green;
        point.color.z = color.blue;
        point.color.w = colortime;
      }
      break;
    }
    case CS_PART_COLOR_LOOPING:
      // @@@ TODO: Do this
      break;
    }

    csVector3 transformed =
      part->particles->GetObjectToCamera ().Other2This(point.position);
    point.sort = transformed.z;
  }
  part->data.Sort (ZSort);
}

csParticlesPhysicsSimple::particles_object*
  csParticlesPhysicsSimple::FindParticles(iParticlesObjectState *p)
{
  for (size_t i = 0; i < partobjects.Length (); i++) 
  {
    if (partobjects[i]->particles == p) 
    {
      return partobjects[i];
    }
  }
  return NULL;
}

int csParticlesPhysicsSimple::ZSort (csParticlesData const& i1,
				     csParticlesData const& i2)
{
  if (i1.sort < i2.sort) return 1;
  return -1;
}

