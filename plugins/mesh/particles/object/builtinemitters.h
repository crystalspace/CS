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

#ifndef __CS_MESH_BUILTINEMITTERS_H__
#define __CS_MESH_BUILTINEMITTERS_H__

#include "csutil/scf_implementation.h"

#include "imesh/particles.h"
#include "iutil/comp.h"

CS_PLUGIN_NAMESPACE_BEGIN(Particles)
{
  // Helper-class for calculation of emission rates etc
  class ParticleEmissionHelper
  {
  public:
    ParticleEmissionHelper ()
      : emissionRate (0), reminder (0)
    {}

    void SetEmissionRate (float rate)
    {
      emissionRate = rate;
    }

    float GetEmissionRate () const
    {
      return emissionRate;
    }

    size_t GetParticlesToEmit (float dt)
    {
      reminder += emissionRate * dt;
      size_t intRem = (size_t)reminder;
      reminder -= intRem;

      return intRem;
    }

  private:
    float emissionRate;
    float reminder;
  };

  class ParticleEmitterFactory : public scfImplementation2<ParticleEmitterFactory,
                                                           iParticleBuiltinEmitterFactory,
                                                           iComponent>
  {
  public:
    ParticleEmitterFactory (iBase* parent)
      : scfImplementationType (this, parent)
    {
    }

    //-- iParticleBuiltinEmitterFactory
    virtual csPtr<iParticleBuiltinEmitterSphere> CreateSphere () const;
    virtual csPtr<iParticleBuiltinEmitterCone> CreateCone () const;
    virtual csPtr<iParticleBuiltinEmitterBox> CreateBox () const;
    virtual csPtr<iParticleBuiltinEmitterCylinder> CreateCylinder () const;

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*)
    {
      return true;
    }
  };


  template<class T>
  class ParticleEmitterHelper : public scfImplementation3<ParticleEmitterHelper<T>,
                                                          T,
                                                          scfFakeInterface<iParticleBuiltinEmitterBase>,
                                                          scfFakeInterface<iParticleEmitter> >
  {
  public:
    typedef scfImplementation3<ParticleEmitterHelper<T>,
                               T,
                               scfFakeInterface<iParticleBuiltinEmitterBase>,
                               scfFakeInterface<iParticleEmitter> > base;

    ParticleEmitterHelper ()
      : base (this),
      isEnabled (true), startTime (0), duration (FLT_MAX),
      initialTTLMin(1.0f), initialTTLMax (1.0f), initialMassMin (1.0f), 
      initialMassMax (1.0f),
      position (0.0f), placement (CS_PARTICLE_BUILTIN_VOLUME), 
      initialLinearVelocity (0.0f), initialAngularVelocity (0.0f), 
      initialVelocityMag (0.0f), uniformVelocity (false)
    {}

    //-- iParticleEmitter
    virtual size_t ParticlesToEmit (iParticleSystemBase* system,
      float dt, float totalTime)
    {
      if (!isEnabled ||
        startTime > totalTime ||
        startTime+duration < totalTime)
        return 0; //Don't emit any particles

      return emissionHelper.GetParticlesToEmit (dt);
    }

    virtual void SetEnabled (bool enabled)
    {
      isEnabled = enabled;
    }

    virtual bool GetEnabled () const
    {
      return isEnabled;
    }

    virtual void SetStartTime (float time) 
    {
      startTime = time;
    }

    virtual float GetStartTime () const
    {
      return startTime;
    }

    virtual void SetDuration (float time)
    {
      duration = time;
    }

    virtual float GetDuration () const
    {
      return duration;
    }

    virtual void SetEmissionRate (float particlesPerSecond)
    {
      emissionHelper.SetEmissionRate (particlesPerSecond);
    }

    virtual float GetEmissionRate () const
    {
      return emissionHelper.GetEmissionRate ();
    }

    virtual void SetInitialTTL (float min, float max)
    {
      initialTTLMin = min;
      initialTTLMax = max;
    }

    virtual void GetInitialTTL (float& min, float& max) const
    {
      min = initialTTLMin;
      max = initialTTLMax;
    }

    virtual void SetInitialMass (float min, float max)
    {
      initialMassMin = min;
      initialMassMax = max;
    }

    virtual void GetInitialMass (float& min, float& max) const
    {
      min = initialMassMin;
      max = initialMassMax;
    }

    //-- iParticleBuiltinEmitterBase
    virtual void SetPosition (const csVector3& position)
    {
      this->position = position;
    }

    virtual const csVector3& GetPosition () const
    {
      return position;
    }

    virtual void SetParticlePlacement (csParticleBuiltinEmitterPlacement place)
    {
      placement = place;
    }

    virtual csParticleBuiltinEmitterPlacement GetParticlePlacement () const
    {
      return placement;
    }

    virtual void SetUniformVelocity (bool uniform)
    {
      uniformVelocity = uniform;
    }

    virtual bool GetUniformVelocity () const
    {
      return uniformVelocity;
    }

    virtual void SetInitialVelocity (const csVector3& linear, 
      const csVector3& angular) 
    {
      initialLinearVelocity = linear;
      initialVelocityMag = linear.Norm ();
      initialAngularVelocity = angular;
    }

    virtual void GetInitialVelocity (csVector3& linear, 
      csVector3& angular) const
    {
      linear = initialLinearVelocity;
      angular = initialAngularVelocity;
    }

  protected:
    //-- iParticleEmitter
    bool isEnabled;
    float startTime;
    float duration;
    float initialTTLMin, initialTTLMax;
    float initialMassMin, initialMassMax;

    ParticleEmissionHelper emissionHelper;

    //-- iParticleBuiltinEmitterBase
    csVector3 position;
    csParticleBuiltinEmitterPlacement placement;
    csVector3 initialLinearVelocity;
    csVector3 initialAngularVelocity;
    float initialVelocityMag;
    bool uniformVelocity;

    typedef ParticleEmitterHelper<T> BaseType;
  };

  class ParticleEmitterSphere : public ParticleEmitterHelper<iParticleBuiltinEmitterSphere>
  {
  public:
    ParticleEmitterSphere ();
    virtual ~ParticleEmitterSphere ();

    virtual csPtr<iParticleEmitter> Clone () const;

    virtual void EmitParticles (iParticleSystemBase* system,
      const csParticleBuffer& particleBuffer, float dt, float totalTime,
      const csReversibleTransform* const emitterToParticle);

    //-- iParticleBuiltinEmitterSphere
    virtual void SetRadius (float radius) 
    {
      this->radius = radius;
    }

    virtual float GetRadius () const
    {
      return radius;
    }

  private:
    //-- iParticleBuiltinEmitterSphere
    float radius;
  };


  class ParticleEmitterBox : public ParticleEmitterHelper<iParticleBuiltinEmitterBox>
  {
  public:
    ParticleEmitterBox ();
    virtual ~ParticleEmitterBox ();

    virtual csPtr<iParticleEmitter> Clone () const;

    virtual void EmitParticles (iParticleSystemBase* system,
      const csParticleBuffer& particleBuffer, float dt, float totalTime,
      const csReversibleTransform* const emitterToParticle);


    //-- iParticleBuiltinEmitterBox
    virtual void SetBox (const csOBB& box)
    {
      genBox = box;
    }

    virtual const csOBB& GetBox () const
    {
      return genBox;
    }

  private:
    //-- iParticleBuiltinEmitterBox
    csOBB genBox;
  };

  class ParticleEmitterCylinder : public 
    ParticleEmitterHelper<iParticleBuiltinEmitterCylinder>
  {
  public:
    ParticleEmitterCylinder ();
    virtual ~ParticleEmitterCylinder ();

    virtual csPtr<iParticleEmitter> Clone () const;

    virtual void EmitParticles (iParticleSystemBase* system,
      const csParticleBuffer& particleBuffer, float dt, float totalTime,
      const csReversibleTransform* const emitterToParticle);

    //-- iParticleBuiltinEmitterCylinder
    virtual void SetRadius (float radius)
    {
      this->radius = radius;
    }

    virtual float GetRadius () const
    {
      return radius;
    }

    virtual void SetExtent (const csVector3& extent);

    virtual const csVector3& GetExtent () const
    {
      return extent;
    }

  private:
    float radius;
    csVector3 extent;
    csVector3 normal0, normal1;
  };


  class ParticleEmitterCone : public 
    ParticleEmitterHelper<iParticleBuiltinEmitterCone>
  {
  public:
    ParticleEmitterCone ();
    virtual ~ParticleEmitterCone ();

    virtual csPtr<iParticleEmitter> Clone () const;

    virtual void EmitParticles (iParticleSystemBase* system,
      const csParticleBuffer& particleBuffer, float dt, float totalTime,
      const csReversibleTransform* const emitterToParticle);

    //-- iParticleBuiltinEmitterCone
    virtual void SetExtent (const csVector3& extent);
  
    virtual const csVector3& GetExtent () const
    {
      return extent;
    }

    virtual void SetConeAngle (float angle)
    {
      coneAngle = angle;
    }

    virtual float GetConeAngle () const
    {
      return coneAngle;
    }

  private:
    float coneAngle;
    csVector3 extent;
    csVector3 normal0, normal1;
  };

}
CS_PLUGIN_NAMESPACE_END(Particles)

#endif
