/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#include "csutil/scf.h"

#include "builtineffectors.h"

CS_PLUGIN_NAMESPACE_BEGIN(Particles)
{

  SCF_IMPLEMENT_FACTORY(ParticleEffectorFactory);

  csPtr<iParticleBuiltinEffectorForce> ParticleEffectorFactory::CreateForce () const
  {
    return new ParticleEffectorForce;
  }


  csPtr<iParticleEffector> ParticleEffectorForce::Clone () const
  {
    return 0;
  }

  void ParticleEffectorForce::EffectParticles (iParticleSystemBase* system,
    const csParticleBuffer& particleBuffer, float dt, float totalTime)
  {
    for (size_t idx = 0; idx < particleBuffer.particleCount; ++idx)
    {
      csParticle& particle = particleBuffer.particleData[idx];
      csParticleAux& particleAux = particleBuffer.particleAuxData[idx];

      particle.linearVelocity += (acceleration + force / particle.mass) * dt;
    }
  }
}
CS_PLUGIN_NAMESPACE_END(Particles)