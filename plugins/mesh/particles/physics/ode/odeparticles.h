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

#ifndef __CS_ODEPARTICLES_H__
#define __CS_ODEPARTICLES_H__

#include "csgeom/vector3.h"
#include "iutil/comp.h"
#include "ivaria/dynamics.h"
#include "ivaria/ode.h"
#include "imesh/particles.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"

struct iVirtualClock;

class csODEParticlePhysics : public iParticlesPhysics
{
  iObjectRegistry* objreg;

  csRef<iDynamics> dyn;
  csRef<iVirtualClock> clock;
  csRef<iODEDynamicState> odestate;

  struct ParticleObjects {
    iParticlesObjectState *particles;
    csArray<csParticlesData> *data;
    csRef<iDynamicSystem> dynsys;
    csArray< csRef<iRigidBody> > bodies;
    int active_count;
  };
  csArray<ParticleObjects> partobjects;

public:
  SCF_DECLARE_IBASE;

  csODEParticlePhysics (iBase *p);
  virtual ~csODEParticlePhysics ();

  bool Initialize (iObjectRegistry* reg);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csODEParticlePhysics);
    bool Initialize (iObjectRegistry* reg)
    { return scfParent->Initialize (reg); }
  } scfiComponent;

  /// Passes particle parameters to physics just before step function
  void Execute (float stepsize);

  struct eiODEFrameUpdateCallback : public iODEFrameUpdateCallback
  {
    SCF_DECLARE_EMBEDDED_IBASE (csODEParticlePhysics);
    void Execute (float stepsize)
    { scfParent->Execute (stepsize); }
  } scfiODEFrameUpdateCallback;

  /// Passes the results of the physics back to particles and updates them
  bool HandleEvent (iEvent &event);

  struct eiEventHandler : public iEventHandler 
  {
    SCF_DECLARE_EMBEDDED_IBASE (csODEParticlePhysics);
    bool HandleEvent (iEvent &event)
    { return scfParent->HandleEvent (event); }
  } scfiEventHandler;

  void RegisterParticles (iParticlesObjectState *particles,
    csArray<csParticlesData> *data);

  void RemoveParticles (iParticlesObjectState *particles);

};

#endif // __CS_ODEPARTICLES_H__
