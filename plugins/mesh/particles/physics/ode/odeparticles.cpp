/*
    Copyright (C) 2004 by Jorrit Tyberghein, John Harger, Daniel Duhprey

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
#include "csutil/event.h"
#include "csutil/cscolor.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/virtclk.h"
#include "ivaria/reporter.h"
#include "odeparticles.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csODEParticlePhysics)
  SCF_IMPLEMENTS_INTERFACE (iParticlesPhysics)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iODEFrameUpdateCallback)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csODEParticlePhysics::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csODEParticlePhysics::eiODEFrameUpdateCallback)
  SCF_IMPLEMENTS_INTERFACE (iODEFrameUpdateCallback)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csODEParticlePhysics::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csODEParticlePhysics)

csODEParticlePhysics::csODEParticlePhysics (iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiODEFrameUpdateCallback);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiEventHandler);

  objreg = 0;
  dyn = 0;
  partobjects.SetLength (0);
}

csODEParticlePhysics::~csODEParticlePhysics ()
{
  odestate->RemoveFrameUpdateCallback (&scfiODEFrameUpdateCallback);

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiEventHandler);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiODEFrameUpdateCallback);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}


bool csODEParticlePhysics::Initialize (iObjectRegistry* reg)
{
  objreg = reg;
  dyn = CS_QUERY_REGISTRY (objreg, iDynamics);
  if (dyn == 0) {
    csRef<iPluginManager> pluginmgr = CS_QUERY_REGISTRY (objreg, iPluginManager);
    if (pluginmgr == 0) {
      csReport (objreg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.particles.physics.ode",
	"No ode or any dynamics plugin loadable");
      return false;
    }
    dyn = csPtr<iDynamics> (CS_LOAD_PLUGIN (pluginmgr,
      "crystalspace.dynamics.ode", iDynamics));
    if (dyn == 0) 
    {
      csReport (objreg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.particles.physics.ode",
	"Unable to find ode dynamics plugin");
      return false;
    }
  }
  odestate = SCF_QUERY_INTERFACE (dyn, iODEDynamicState);
  if (odestate == 0) {
    csRef<iPluginManager> pluginmgr = CS_QUERY_REGISTRY (objreg, iPluginManager);
    if (pluginmgr == 0) {
      csReport (objreg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.particles.physics.ode",
	"Dynamics is not ode and unable to load ode plugin");
      return false;
    }
    dyn = csPtr<iDynamics> (CS_LOAD_PLUGIN (pluginmgr,
      "crystalspace.dynamics.ode", iDynamics));
    if (dyn == 0) 
    {
      csReport (objreg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.particles.physics.ode",
	"Dynamics is not ode and unable to find ode dynamics plugin");
      return false;
    }
    odestate = SCF_QUERY_INTERFACE (dyn, iODEDynamicState);
    if (!odestate)
    {
      csReport (objreg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.particles.physics.ode",
	"Plugin claiming to be odedynam doesn't implement iODEDynamicState");
      return false;
    }
  }
  if (!odestate->EventProcessingEnabled())
  {
      csReport (objreg, CS_REPORTER_SEVERITY_WARNING,
        "crystalspace.particles.physics.ode",
	"EventProcessing disabled in odedynam, will enable Step() now invalid");
  }
  odestate->EnableEventProcessing (true);
  odestate->AddFrameUpdateCallback (&scfiODEFrameUpdateCallback);

  csRef<iEventQueue> eq = CS_QUERY_REGISTRY (objreg, iEventQueue);
  if (eq == 0) 
  {
      csReport (objreg, CS_REPORTER_SEVERITY_WARNING,
        "crystalspace.particles.physics.ode",
	"No event queue available");
      return false;
  }
  eq->RegisterListener (&scfiEventHandler, CSMASK_Nothing);

  clock = CS_QUERY_REGISTRY (objreg, iVirtualClock);
  if (clock == 0) 
  {
      csReport (objreg, CS_REPORTER_SEVERITY_WARNING,
        "crystalspace.particles.physics.ode",
	"No virtual clock available");
      return false;
  }

  return true;
}

void csODEParticlePhysics::Execute (float stepsize)
{
  for (size_t i = 0; i < partobjects.Length(); i ++)
  {
    ParticleObjects &po = partobjects[i];

    if (po.total_elapsed_time < po.particles->GetEmitTime ())
    {
      po.new_particles += stepsize * (float)po.particles->GetParticlesPerSecond();
      po.total_elapsed_time += stepsize;
    }
    float newdead = po.dead_particles - po.new_particles;
    if (newdead < po.data.Length() * 0.3)
    {
      int oldlen = (int)po.data.Length ();
      int newlen = (oldlen > (int)po.new_particles) ?
        oldlen : (int)po.new_particles;
      newlen <<= 1;
      po.data.SetLength (newlen);
      po.bodies.SetLength (newlen);
      po.dead_particles += newlen - oldlen;
      for (int i = oldlen; i < newlen; i ++)
      {
        po.data[i].sort = -FLT_MAX;
	po.data[i].color.w = 0.0f;
	po.data[i].time_to_live = 0.0;
	po.bodies[i].sort = -FLT_MAX;
	po.bodies[i].body = 0;
      }
    }
    else if (newdead > po.data.Length () * 0.7 && po.data.Length() > 1)
    {
      int oldlen = (int)po.data.Length ();
      int newlen = oldlen >> 1;
      po.data.Truncate (newlen);
      po.bodies.Truncate (newlen);
      po.dead_particles -= oldlen - newlen;
    }

    int dead_offset = (int)po.data.Length () - po.dead_particles;
    
    csVector3 emitter;
    po.particles->GetEmitPosition (emitter);
    int j;
    for (j = 0; j < (int)po.new_particles; j ++)
    {
      csParticlesData &point = po.data.Get(j+dead_offset);
      csVector3 start;
      switch (po.particles->GetEmitType())
      {
      case CS_PART_EMIT_SPHERE:
      {
        start = csVector3((rng.Get() - 0.5) * 2,
                          (rng.Get() - 0.5) * 2,
			  (rng.Get() - 0.5) * 2);
        start.Normalize ();
        float inner_radius = po.particles->GetSphereEmitInnerRadius ();
        float outer_radius = po.particles->GetSphereEmitOuterRadius ();
        start = emitter +
          (start * ((rng.Get() * (outer_radius - inner_radius ))
          + inner_radius ));
        break;
      }
      case CS_PART_EMIT_PLANE:
        break;
      case CS_PART_EMIT_BOX:
        break;
      case CS_PART_EMIT_CYLINDER:
        // @@@ FIXME: Implement this?
        break;
      }
      point.position = start;
      point.velocity = csVector3 ();
      point.mass = po.particles->GetMass () +
        (rng.Get() * po.particles->GetMassVariation ());
      point.time_to_live = po.particles->GetTimeToLive () +
        (rng.Get() * po.particles->GetTimeVariation ());
      point.color = csVector4 ();

      csRef<iRigidBody> b = po.dynsys->CreateBody ();
      b->SetPosition(point.position);
      b->AttachColliderSphere (po.particles->GetParticleRadius(), 
        csVector3 (0,0,0), po.particles->GetDampener(),
        point.mass, po.particles->GetDiffusion ());
      po.bodies[j+dead_offset].body = b;
    }

    dead_offset += (int)po.new_particles;
    po.dead_particles -= (int)po.new_particles;
    po.new_particles -= (int)po.new_particles;

    for (j = 0; j < dead_offset; j ++)
    {
      CS_ASSERT (po.data[j].time_to_live > 0);
      po.data[j].time_to_live -= stepsize;
      if (po.data[j].time_to_live <= 0.0)
      {
        po.data[j].color.w = 0.0;
        po.data[j].sort = -FLT_MAX;
        po.dynsys->RemoveBody (po.bodies[j].body);
	po.bodies[j].sort = -FLT_MAX;
	po.bodies[j].body = 0;
	continue;
      }
      csRef<iRigidBody> b = po.bodies[j].body;

      csVector3 diff (rng.Get() * 2.0 - 1,
        rng.Get() * 2.0 - 1,
        rng.Get() * 2.0 - 1);
      diff *= po.particles->GetDiffusion () * stepsize;
      // b->SetPosition(b->GetPosition() + diff);

      csVector3 emitter;
      po.particles->GetEmitPosition (emitter);
      csVector3 dir = b->GetPosition() - emitter;
      float sq_dist = dir.SquaredNorm ();
      float sq_range = po.particles->GetForceRange ();
      sq_range *= sq_range;
      if (sq_range <= sq_dist) { continue; }

      switch (po.particles->GetForceType ()) 
      {
      case CS_PART_FORCE_RADIAL:
        dir.Normalize ();
        break;
      case CS_PART_FORCE_LINEAR:
        po.particles->GetForceDirection (dir);
        break;
      case CS_PART_FORCE_CONE:
        po.particles->GetForceDirection (dir);
        break;
      }

      float falloff;
      csParticleFalloffType primary, cone;
      po.particles->GetFalloffType (primary, cone);
      switch (primary)
      {
      case CS_PART_FALLOFF_CONSTANT:
        falloff = 1.0;
	break;
      case CS_PART_FALLOFF_LINEAR:
        falloff = 1.0 - (sq_dist / sq_range);
	break;
      case CS_PART_FALLOFF_PARABOLIC:
        falloff = 1.0 - (sq_dist * sq_dist / sq_range);
	break;
      }

      // po.bodies[j].body->AddForce (dir * (po.particles->GetForce() * falloff));
    }
  }
}

bool csODEParticlePhysics::HandleEvent (iEvent &event)
{
  if (event.Type != csevBroadcast || csCommandEventHelper::GetCode(&event) != cscmdPreProcess)
  {
    return false;
  }


  for (size_t i = 0; i < partobjects.Length(); i ++)
  { 
    ParticleObjects &po = partobjects[i];
    for (size_t j = 0; j < (po.data.Length() - po.dead_particles); j ++)
    {
      csParticlesData &part = po.data.Get (j);
      CS_ASSERT (part.time_to_live > 0);

      part.velocity = po.bodies[j].body->GetLinearVelocity ();
      part.position = po.bodies[j].body->GetPosition ();

      switch (po.particles->GetParticleColorMethod ())
      {
      case CS_PART_COLOR_CONSTANT:
      {
	csColor4 constant;
	po.particles->GetConstantColor (constant);
	part.color.x = constant.red;
	part.color.y = constant.green;
	part.color.z = constant.blue;
	part.color.w = 1.0f;
        break;
      }
      case CS_PART_COLOR_LINEAR:
      {
        float normaltime = part.time_to_live / 
	  (po.particles->GetTimeToLive() + po.particles->GetTimeVariation());
	const csArray<csColor4> &grad = po.particles->GetGradient();
        if (grad.Length())
	{
	  float cref = (1.0 - normaltime) * (float)grad.Length();
	  size_t index = (size_t)floor (cref);
	  const csColor4& c1 = grad[index];
	  const csColor4& c2 = grad[(index == grad.Length()-1)?index:index+1];
	  float interp = cref - floor(cref);
	  part.color.x = ((1.0 - interp) * c1.red) + (interp * c2.red);
	  part.color.y = ((1.0 - interp) * c1.green) + (interp * c2.green);
	  part.color.z = ((1.0 - interp) * c1.blue) + (interp * c2.blue);
	  part.color.w = ((1.0 - interp) * c1.alpha) + (interp * c2.alpha);
	}
        break;
      }
      case CS_PART_COLOR_HEAT:
        break;
      case CS_PART_COLOR_CALLBACK:
        break;
      case CS_PART_COLOR_LOOPING:
        break;
      }
      part.sort = po.particles->GetObjectToCamera ().Other2This (part.position).z;
      po.bodies[j].sort = part.sort;
    }
    po.data.Sort (DataSort);
    po.bodies.Sort (BodySort);
  }
  return true;
}

const csArray<csParticlesData> *csODEParticlePhysics::RegisterParticles (iParticlesObjectState *particles)
{
  if (!dyn) 
  {
      csReport (objreg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.particles.physics.ode",
	"Cannot register particles objects until initialize is called");
      return 0;
  }
  ParticleObjects &po = partobjects.GetExtend (partobjects.Length());
  po.particles = particles;
  po.dynsys = dyn->CreateSystem ();
  return &po.data;
}

void csODEParticlePhysics::RemoveParticles (iParticlesObjectState *particles)
{
  for (size_t i = 0; i < partobjects.Length(); i ++)
  {
    if (partobjects[i].particles == particles)
    {
      partobjects[i].bodies.SetLength (0);
      dyn->RemoveSystem (partobjects[i].dynsys);
      partobjects.DeleteIndex (i);
      break;
    }
  }
}

void csODEParticlePhysics::Start (iParticlesObjectState *particles)
{
  ParticleObjects* po = Find (particles);
  CS_ASSERT (po);

  if (po->data.Length () == 0) 
  {
    int start_size = particles->GetInitialParticleCount() * 2;
csPrintf ("Initial size = %d\n", start_size);
    po->data.SetLength (start_size);
    po->bodies.SetLength (start_size);
    for (int i = 0; i < start_size; i ++)
    {
      po->data[i].sort = -FLT_MAX;
      po->data[i].color.w = 0.0;
      po->data[i].time_to_live = 0.0;
      po->bodies[i].sort = -FLT_MAX;
      po->bodies[i].body = 0;
    }
csPrintf ("Setting dead parts to %d\n", start_size);
    po->dead_particles = start_size;
  }

  po->new_particles = (float)po->particles->GetInitialParticleCount();
  po->total_elapsed_time = 0.0;
}

void csODEParticlePhysics::Stop (iParticlesObjectState *particles)
{
  ParticleObjects* po = Find (particles);
  CS_ASSERT (po);
    
  po->new_particles = 0.0;
  po->total_elapsed_time = particles->GetEmitTime ();
}
