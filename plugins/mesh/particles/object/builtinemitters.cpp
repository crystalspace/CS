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
#include "csutil/floatrand.h"
#include "csutil/randomgen.h"

#include "builtinemitters.h"

CS_PLUGIN_NAMESPACE_BEGIN(Particles)
{
  // General random number generators
  CS_IMPLEMENT_STATIC_VAR(GetVGen, csRandomVectorGen,);
  CS_IMPLEMENT_STATIC_VAR(GetFGen, csRandomFloatGen,);
  CS_IMPLEMENT_STATIC_VAR(GetIGen, csRandomGen,);  

  /**
   * Small helper-function to calculate x^(1/3) using Newton iteration.
   * The parameters are tuned for x=[0;1]
   */
  inline float ApproximateCubeRoot (float x)
  {
    float xm = x + 0.22f;
    for (uint i = 0; i < 2; ++i)
    {
      xm = (1/3.0f) * (x / (xm * xm) + 2*xm); 
    }

    return xm;
  }


  SCF_IMPLEMENT_FACTORY(ParticleEmitterFactory);

  csPtr<iParticleBuiltinEmitterSphere> ParticleEmitterFactory::CreateSphere () const
  {
    return new ParticleEmitterSphere;
  }

  csPtr<iParticleBuiltinEmitterCone> ParticleEmitterFactory::CreateCone () const
  {
    return new ParticleEmitterCone;
  }

  csPtr<iParticleBuiltinEmitterBox> ParticleEmitterFactory::CreateBox () const
  {
    return new ParticleEmitterBox;
  }

  csPtr<iParticleBuiltinEmitterCylinder> ParticleEmitterFactory::CreateCylinder () const
  {
    return new ParticleEmitterCylinder;
  }



  ParticleEmitterSphere::ParticleEmitterSphere ()
    : radius (1.0f)
  {
  }

  ParticleEmitterSphere::~ParticleEmitterSphere ()
  {
  }

  csPtr<iParticleEmitter> ParticleEmitterSphere::Clone () const
  {
    return 0;
  }


  void ParticleEmitterSphere::EmitParticles (iParticleSystemBase* system,
    const csParticleBuffer& particleBuffer, float dt, float totalTime)
  {
    const csVector2& size = system->GetParticleSize ();

    for (size_t idx = 0; idx < particleBuffer.particleCount; ++idx)
    {
      csParticle& particle = particleBuffer.particleData[idx];
      csParticleAux& particleAux = particleBuffer.particleAuxData[idx];

      particle.position = position;

      csVector3 posOffset = GetVGen()->Get ();

      if (placement == CS_PARTICLE_BUILTIN_VOLUME)
        particle.position += posOffset * sqrtf (GetFGen ()->Get ()) * radius;
      else if (placement == CS_PARTICLE_BUILTIN_SURFACE)
        particle.position += posOffset * radius;

      particle.orientation.SetIdentity ();
      if (uniformVelocity)
      {
        particle.linearVelocity = initialLinearVelocity;  
      }
      else
      {
        particle.linearVelocity = posOffset * initialVelocityMag;
      }

      particle.angularVelocity = initialAngularVelocity;
      
      particle.timeToLive = GetFGen ()->Get (initialTTLMin, initialTTLMax);
      particle.mass = GetFGen ()->Get (initialMassMin, initialMassMax);

      particleAux.color = csColor4 (1.0f, 1.0f, 1.0f);
      particleAux.particleSize = size;
    }
  }


  ParticleEmitterBox::ParticleEmitterBox ()
    : genBox (csBox3 (csVector3 (-0.5f), csVector3 (0.5f)))
  {
  }

  ParticleEmitterBox::~ParticleEmitterBox ()
  {
  }

  csPtr<iParticleEmitter> ParticleEmitterBox::Clone () const
  {
    return 0;
  }


  void ParticleEmitterBox::EmitParticles (iParticleSystemBase* system,
    const csParticleBuffer& particleBuffer, float dt, float totalTime)
  {
    const csVector2& size = system->GetParticleSize ();
    const csVector3& boxSize = genBox.GetSize ();

    csVector3 globalPos = position + genBox.GetCenter ();

    for (size_t idx = 0; idx < particleBuffer.particleCount; ++idx)
    {
      csParticle& particle = particleBuffer.particleData[idx];
      csParticleAux& particleAux = particleBuffer.particleAuxData[idx];

      particle.position = globalPos;

      csVector3 posOffset (0.0f);
      csVector3 velVector;

      const csMatrix3& mat = genBox.GetMatrix ();

      if (placement == CS_PARTICLE_BUILTIN_VOLUME)
      {
        posOffset.x = (GetFGen ()->Get (-1, 1) * boxSize.x);
        posOffset.y = (GetFGen ()->Get (-1, 1) * boxSize.y);
        posOffset.z = (GetFGen ()->Get (-1, 1) * boxSize.z);
      }
      else if (placement == CS_PARTICLE_BUILTIN_SURFACE)
      {
        float axTemp = GetFGen ()->Get (-3.0f, 3.0f);
        
        unsigned int axis = abs ((int)axTemp);
        unsigned int axu = (axis+1) % 3;
        unsigned int axv = (axis+2) % 3;

        float sign = (axTemp > 0) ? 1 : -1;

        posOffset[axis] = sign * boxSize[axis];
        posOffset[axu] = (GetFGen ()->Get (-1, 1) * boxSize[axu]);
        posOffset[axv] = (GetFGen ()->Get (-1, 1) * boxSize[axv]);
      }

      if (!posOffset.IsZero ())
        particle.position += mat * posOffset;

      if (uniformVelocity)
      {
        particle.linearVelocity = initialLinearVelocity;
      }
      else
      {
        particle.linearVelocity = mat * (posOffset.UnitAxisClamped () * initialVelocityMag);
      }

      particle.angularVelocity = initialAngularVelocity;

      particle.timeToLive = GetFGen ()->Get (initialTTLMin, initialTTLMax);
      particle.mass = GetFGen ()->Get (initialMassMin, initialMassMax);

      particleAux.color = csColor4 (1.0f, 1.0f, 1.0f);
      particleAux.particleSize = size;
    }
  }


  ParticleEmitterCylinder::ParticleEmitterCylinder ()
    : radius (1.0f), extent (1.0f, 0.0f, 0.0f), normal0 (0.0f, 1.0f, 0.0f),
    normal1 (0.0f, 0.0f, 1.0f)
  {}

  ParticleEmitterCylinder::~ParticleEmitterCylinder ()
  {}

  void ParticleEmitterCylinder::SetExtent (const csVector3& extent)
  {
    this->extent = extent;

    //Compute two normals which are normalized, and orthogonal to extent as
    //well as eachother
    normal0 = csVector3 (extent.y, -extent.x, 0).Unit ();
    normal1 = (extent % normal0).Unit ();
  }

  csPtr<iParticleEmitter> ParticleEmitterCylinder::Clone () const
  {
    return 0;
  }

  void ParticleEmitterCylinder::EmitParticles (iParticleSystemBase* system,
    const csParticleBuffer& particleBuffer, float dt, float totalTime)
  {
    const csVector2& size = system->GetParticleSize ();

    for (size_t idx = 0; idx < particleBuffer.particleCount; ++idx)
    {
      csParticle& particle = particleBuffer.particleData[idx];
      csParticleAux& particleAux = particleBuffer.particleAuxData[idx];

      particle.position = position;

      //Offset along axis
      csVector3 posOffset = GetFGen ()->Get (-1, 1) * extent;

      //Point along circle
      float phi = GetFGen ()->GetAngle ();
      csVector2 circlePoint (cosf (phi), sinf (phi));

      csVector3 radialVec = normal0 * circlePoint.x + normal1 * circlePoint.y;

      if (placement == CS_PARTICLE_BUILTIN_VOLUME)
      {
        posOffset += radialVec * sqrtf (GetFGen ()->Get ()) * radius;
      }
      else if (placement == CS_PARTICLE_BUILTIN_SURFACE)
      {
        posOffset += radialVec * radius;
      }

      particle.position += posOffset;

      if (uniformVelocity)
      {
        particle.linearVelocity = initialLinearVelocity;
      }
      else
      {
        particle.linearVelocity = radialVec * initialVelocityMag;
      }

      particle.angularVelocity = initialAngularVelocity;

      particle.timeToLive = GetFGen ()->Get (initialTTLMin, initialTTLMax);
      particle.mass = GetFGen ()->Get (initialMassMin, initialMassMax);

      particleAux.color = csColor4 (1.0f, 1.0f, 1.0f);
      particleAux.particleSize = size;
    }
  }

  
  ParticleEmitterCone::ParticleEmitterCone ()
    : coneAngle (PI/8.0f), extent (1.0f, 0.0f, 0.0f), normal0 (0.0f, 1.0f, 0.0f),
    normal1 (0.0f, 0.0f, 1.0f)
  {}

  ParticleEmitterCone::~ParticleEmitterCone ()
  {}

  void ParticleEmitterCone::SetExtent (const csVector3& extent)
  {
    this->extent = extent;

    //Compute two normals which are normalized, and orthogonal to extent as
    //well as eachother
    normal0 = csVector3 (extent.y, -extent.x, 0).Unit ();
    normal1 = (extent % normal0).Unit ();
  }

  csPtr<iParticleEmitter> ParticleEmitterCone::Clone () const
  {
    return 0;
  }

  void ParticleEmitterCone::EmitParticles (iParticleSystemBase* system,
    const csParticleBuffer& particleBuffer, float dt, float totalTime)
  {
    const csVector2& size = system->GetParticleSize ();

    const float tanConeAngTimeExt = tanf (coneAngle) * extent.Norm ();

    for (size_t idx = 0; idx < particleBuffer.particleCount; ++idx)
    {
      csParticle& particle = particleBuffer.particleData[idx];
      csParticleAux& particleAux = particleBuffer.particleAuxData[idx];

      particle.position = position;

      //Offset along axis
      float offsetVal = sqrtf (GetFGen ()->Get ());
      csVector3 posOffset = offsetVal * extent;

      //Point in disk
      float phi = GetFGen ()->GetAngle ();
      csVector2 circlePoint (cosf (phi), sinf (phi));
      circlePoint *= sqrtf (GetFGen ()->Get ()) * tanConeAngTimeExt * offsetVal;

      csVector3 radialVec = normal0 * circlePoint.x + normal1 * circlePoint.y;

      if (placement == CS_PARTICLE_BUILTIN_VOLUME)
      {
        particle.position += posOffset + radialVec;
      }
      else if (placement == CS_PARTICLE_BUILTIN_SURFACE)
      {
        particle.position += extent + radialVec;
      }


      if (uniformVelocity)
      {
        particle.linearVelocity = initialLinearVelocity;
      }
      else
      {
        particle.linearVelocity = (extent + radialVec).Unit () * initialVelocityMag;
      }

      particle.angularVelocity = initialAngularVelocity;

      particle.timeToLive = GetFGen ()->Get (initialTTLMin, initialTTLMax);
      particle.mass = GetFGen ()->Get (initialMassMin, initialMassMax);

      particleAux.color = csColor4 (1.0f, 1.0f, 1.0f);
      particleAux.particleSize = size;
    }
  }

}
CS_PLUGIN_NAMESPACE_END(Particles)

