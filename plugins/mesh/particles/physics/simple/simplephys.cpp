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

void csParticlesPhysicsSimple::RegisterParticles (iParticlesObjectState *particles,
  csArray<csParticlesData> *data)
{
  particles_object *part = new particles_object;
  part->particles = particles;
  part->data = data;
  partobjects.Push (part);
}

void csParticlesPhysicsSimple::RemoveParticles (iParticlesObjectState *particles)
{
  particles_object *temp;
  int end = partobjects.Length () - 1;
  for(int i=end;i>=0;i++) {
    temp = partobjects[i];
    if(temp->particles == particles) {
      partobjects[i] = partobjects[end];
      partobjects[end] = temp;
      temp = partobjects.Pop ();
      delete temp;
      return;
    }
  }
}

bool csParticlesPhysicsSimple::HandleEvent (iEvent &event)
{
  if(event.Type == csevBroadcast && event.Command.Code == cscmdPreProcess) 
  {
    csTicks elapsed = vclock->GetElapsedTicks ();
    int updates = (elapsed + leftover_time) / 20;
    leftover_time = (elapsed + leftover_time) - (updates * 20);
    
    float elapsedSecs = (float)elapsed * 0.001f;
    for(int i=0; i < partobjects.Length (); i++) 
    {
      StepPhysics (elapsedSecs, partobjects[i]->particles,
        partobjects[i]->data);
      partobjects[i]->particles->Update (elapsedSecs);
    }
  }
  return false;
}

void csParticlesPhysicsSimple::StepPhysics (float elapsed_time,
  iParticlesObjectState *particles, csArray<csParticlesData> *data)
{
  float force_range = particles->GetForceRange ();
  for(int i=0;i<data->Length ();i++) 
  {
    // Setup for this particle
    csParticlesData &part = data->Get (i);

    if(part.time_to_live < 0.0f) break;

    // Diffusion
    csVector3 diff((rng.Get() * 2.0f) - 1.0f, (rng.Get() * 2.0f) - 1.0f, (rng.Get() * 2.0f) - 1.0f);
    diff *= particles->GetDiffusion ();
    part.position += (diff * elapsed_time);

    // Force related stuff
    csVector3 emitter;
    particles->GetEmitPosition (emitter);
    float force_range_squared = (force_range * force_range);
    csVector3 dir = part.position - emitter; 
    float dist_squared = dir.SquaredNorm();
    float falloff = 1.0f;

    csParticleForceType force_type;
    force_type = particles->GetForceType ();

    switch(force_type)
    {
    case CS_PART_FORCE_RADIAL:
      dir.Normalize ();
      break;
    case CS_PART_FORCE_LINEAR:
      particles->GetForceDirection (dir);
      break;
    case CS_PART_FORCE_CONE:
      particles->GetForceDirection (dir);
      break;
    }

    csParticleFalloffType force_falloff, cone_falloff;

    particles->GetFalloffType (force_falloff, cone_falloff);

    switch(force_falloff) {
    case CS_PART_FALLOFF_CONSTANT:
      if(dist_squared > force_range_squared) falloff = 0.0f;
      break;
    case CS_PART_FALLOFF_LINEAR:
      if(dist_squared > force_range_squared) falloff = 0.0f;
      else falloff = 1.0f - (dist_squared / force_range_squared);
      break;
    case CS_PART_FALLOFF_PARABOLIC:
      if(dist_squared < force_range_squared)
        falloff = (1.0f / (force_range_squared - dist_squared));
      break;
    }

    csVector3 gravity;
    particles->GetGravity (gravity);

    part.velocity += dir * (((particles->GetForce() * falloff -
      fabs(part.velocity.Norm()) * particles->GetDampener ())
      / particles->GetMass ()) * elapsed_time) + gravity * elapsed_time;
    part.position += part.velocity * elapsed_time;
  }
}
