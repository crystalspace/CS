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
  eq->RegisterListener (&scfiEventHandler, CSMASK_Broadcast);

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
  for (int i = 0; i < partobjects.Length(); i ++)
  {
    ParticleObjects &po = partobjects[i];
    if (po.data->Length () < po.active_count) 
    {
      int removes = po.active_count - po.data->Length();
      for (int j = po.active_count - removes; j < po.active_count; j ++) 
      {
        po.dynsys->RemoveBody (po.bodies[j]);
      }
    } 
    po.bodies.SetLength (po.data->Length());

    int new_active_count = 0;
    for (int j = 0; j < po.data->Length (); j ++)
    {
      csParticlesData &part = po.data->Get (j);
      if (part.time_to_live < 0) { break; }
      new_active_count ++;
      if (new_active_count > po.active_count) {
        po.bodies[j] = po.dynsys->CreateBody ();
	po.bodies[j]->AttachColliderSphere (po.particles->GetParticleRadius(), 
	  csVector3 (0,0,0), po.particles->GetDampener(),
	  po.particles->GetMass (), po.particles->GetDiffusion ());
      }
      po.bodies[j]->SetPosition(part.position);

      csVector3 emitter;
      po.particles->GetEmitPosition (emitter);
      csVector3 dir = part.position - emitter;
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

      po.bodies[j]->AddForce (dir * (po.particles->GetForce() * falloff));
    }
    po.active_count = new_active_count;
  }
}

bool csODEParticlePhysics::HandleEvent (iEvent &event)
{
  if (event.Type != csevBroadcast || event.Command.Code != cscmdPreProcess)
  {
    return false;
  }


  for (int i = 0; i < partobjects.Length(); i ++)
  { 
    ParticleObjects &po = partobjects[i];
    for (int j = 0; j < po.data->Length (); j ++)
    {
      csParticlesData &part = po.data->Get (j);
      CS_ASSERT (part.time_to_live >= 0);

      part.velocity = po.bodies[i]->GetLinearVelocity ();
      part.position = po.bodies[i]->GetPosition ();
    }
    po.particles->Update ((float)clock->GetElapsedTicks () / 1000.0);
  }
  return true;
}

void csODEParticlePhysics::RegisterParticles (iParticlesObjectState *particles,
	csArray<csParticlesData> *data)
{
  if (!dyn) 
  {
      csReport (objreg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.particles.physics.ode",
	"Cannot register particles objects until initialize is called");
      return;
  }
  ParticleObjects &po = partobjects.GetExtend (partobjects.Length());
  po.particles = particles;
  po.data = data;
  po.dynsys = dyn->CreateSystem ();
  po.bodies.SetLength (data->Length ());
  po.active_count = 0;

  for (int i = 0; i < data->Length(); i ++) 
  {
    csParticlesData &part = data->Get (i);
    if (part.time_to_live < 0) { break; }
    po.bodies[i] = po.dynsys->CreateBody ();
    po.bodies[i]->AttachColliderSphere (po.particles->GetParticleRadius(), 
	csVector3 (0,0,0), po.particles->GetDampener(),
	po.particles->GetMass (), po.particles->GetDiffusion ());
    po.bodies[i]->SetPosition (part.position);
    po.active_count ++;
  }
}

void csODEParticlePhysics::RemoveParticles (iParticlesObjectState *particles)
{
  for (int i = 0; i < partobjects.Length(); i ++) {
    if (partobjects[i].particles == particles) {
      partobjects[i].bodies.SetLength (0);
      dyn->RemoveSystem (partobjects[i].dynsys);
      partobjects.DeleteIndex (i);
      break;
    }
  }
}

