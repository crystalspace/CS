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

#include "csgeom/math.h"
#include "csutil/scf.h"
#include "csutil/floatrand.h"

#include "builtineffectors.h"


CS_PLUGIN_NAMESPACE_BEGIN(Particles)
{
  SCF_IMPLEMENT_FACTORY(ParticleEffectorFactory);

  CS_IMPLEMENT_STATIC_VAR(GetVGen, csRandomVectorGen,);

  csPtr<iParticleBuiltinEffectorForce> ParticleEffectorFactory::CreateForce () const
  {
    return new ParticleEffectorForce;
  }

  csPtr<iParticleBuiltinEffectorLinColor> 
    ParticleEffectorFactory::CreateLinColor () const
  {
    return new ParticleEffectorLinColor;
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

      csVector3 a = acceleration;

      if (randomAcceleration > 0)
        a += GetVGen()->Get () * randomAcceleration;

      particle.linearVelocity += (a + force / particle.mass) * dt;
    }
  }

  ParticleEffectorLinColor::ParticleEffectorLinColor ()
    : scfImplementationType (this),
    precalcInvalid (true)
  {

  }

  csPtr<iParticleEffector> ParticleEffectorLinColor::Clone () const
  {
    return new ParticleEffectorLinColor (*this);
  }

  void ParticleEffectorLinColor::EffectParticles (iParticleSystemBase* system,
    const csParticleBuffer& particleBuffer, float dt, float totalTime)
  {
    Precalc ();

    if (precalcList.GetSize () == 0)
      return;

    for (size_t idx = 0; idx < particleBuffer.particleCount; ++idx)
    {
      csParticle& particle = particleBuffer.particleData[idx];
      csParticleAux& particleAux = particleBuffer.particleAuxData[idx];

      float ttl = particle.timeToLive;

      //Classify
      size_t aSpan;
      for(aSpan = 0; aSpan < precalcList.GetSize (); ++aSpan)
      {
        if (ttl < precalcList[aSpan].maxTTL)
          break;
      }

      aSpan = csMin (aSpan, precalcList.GetSize ()-1);

      const PrecalcEntry& ei = precalcList[aSpan];
      particleAux.color = ei.add + ei.mult * ttl;
    }
  }

  size_t ParticleEffectorLinColor::AddColor (const csColor4& color, float maxTTL)
  {
    ColorEntry c;
    c.color = color;
    c.maxTTL = maxTTL;

    colorList.Push (c);

    precalcInvalid = true;
    return colorList.GetSize () - 1;
  }

  void ParticleEffectorLinColor::SetColor (size_t index, const csColor4& color)
  {
    if (index >= colorList.GetSize ())
      return;

    colorList[index].color = color;

    precalcInvalid = true;
  }

  void ParticleEffectorLinColor::Precalc ()
  {
    if (!precalcInvalid)
      return;

    precalcList.SetSize (colorList.GetSize ());
    
    if (precalcList.GetSize () == 0)
        return;

    PrecalcEntry& e0 = precalcList[0];
    e0.add = colorList[0].color;
    e0.mult.Set (0,0,0);

    for (size_t i = 1; i < precalcList.GetSize (); ++i)
    {
      PrecalcEntry& ei = precalcList[i];
      const ColorEntry& ci = colorList[i];
      const ColorEntry& cip = colorList[i-1];

      ei.maxTTL = ci.maxTTL;
      ei.mult = (ci.color - cip.color) / (ci.maxTTL - cip.maxTTL);
      ei.add = ci.color - ei.mult * ei.maxTTL;
    }

    precalcInvalid = false;
  }
}
CS_PLUGIN_NAMESPACE_END(Particles)