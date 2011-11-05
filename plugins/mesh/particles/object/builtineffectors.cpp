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
#include "csgeom/odesolver.h"
#include "csutil/scf.h"
#include "csutil/floatrand.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/scenenode.h"
#include "iengine/sector.h"
#include "ivideo/rendermesh.h"

#include "builtineffectors.h"
#include "particles.h"


CS_PLUGIN_NAMESPACE_BEGIN(Particles)
{
  SCF_IMPLEMENT_FACTORY(ParticleEffectorFactory);

  CS_IMPLEMENT_STATIC_VAR(GetVGen, csRandomVectorGen, ());

  csPtr<iParticleBuiltinEffectorForce> ParticleEffectorFactory::CreateForce () const
  {
    return new ParticleEffectorForce;
  }

  csPtr<iParticleBuiltinEffectorLinColor> 
    ParticleEffectorFactory::CreateLinColor () const
  {
    return new ParticleEffectorLinColor;
  }

  csPtr<iParticleBuiltinEffectorLinear> 
    ParticleEffectorFactory::CreateLinear () const
  {
    return new ParticleEffectorLinear;
  }

  csPtr<iParticleBuiltinEffectorVelocityField> 
    ParticleEffectorFactory::CreateVelocityField () const
  {
    return new ParticleEffectorVelocityField;
  }

  csPtr<iParticleBuiltinEffectorLight> 
    ParticleEffectorFactory::CreateLight () const
  {
    return new ParticleEffectorLight;
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
      //csParticleAux& particleAux = particleBuffer.particleAuxData[idx];

      csVector3 a = acceleration;

      if (do_randomAcceleration)
      {
        csVector3 r = GetVGen()->Get ();
	a.x += r.x * randomAcceleration.x;
	a.y += r.y * randomAcceleration.y;
	a.z += r.z * randomAcceleration.z;
      }

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
    // Update the color list if needed
    Precalc ();

    if (precalcList.GetSize () == 0)
      return;

    // Iterate on all particles
    for (size_t idx = 0; idx < particleBuffer.particleCount; ++idx)
    {
      csParticle& particle = particleBuffer.particleData[idx];
      csParticleAux& particleAux = particleBuffer.particleAuxData[idx];

      float ttl = particle.timeToLive;

      // Find the color entry corresponding to this ttl
      size_t aSpan;
      for (aSpan = 0; aSpan < precalcList.GetSize (); ++aSpan)
      {
        if (ttl < precalcList[aSpan].maxTTL)
          break;
      }

      aSpan = csMin (aSpan, precalcList.GetSize () - 1);

      // Apply the new color
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

  void ParticleEffectorLinColor::RemoveColor (size_t index)
  {
    colorList.DeleteIndex (index);
    precalcInvalid = true;
  }

  void ParticleEffectorLinColor::Clear ()
  {
    colorList.DeleteAll ();
    precalcInvalid = true;
  }

  void ParticleEffectorLinColor::SetColor (size_t index, const csColor4& color)
  {
    if (index >= colorList.GetSize ())
      return;

    colorList[index].color = color;

    precalcInvalid = true;
  }

  void ParticleEffectorLinColor::SetEndTTL (size_t index, float ttl)
  {
    if (index >= colorList.GetSize ())
      return;

    colorList[index].maxTTL = ttl;

    precalcInvalid = true;
  }

  int ParticleEffectorLinColor::ColorEntryCompare(
    const ParticleEffectorLinColor::ColorEntry &e0, 
    const ParticleEffectorLinColor::ColorEntry &e1)
  {
    if (e0.maxTTL < e1.maxTTL)
      return -1;
    else if (e0.maxTTL > e1.maxTTL)
      return 1;
    return 0;
  }

  void ParticleEffectorLinColor::Precalc ()
  {
    if (!precalcInvalid)
      return;

    precalcList.SetSize (colorList.GetSize ());
    
    if (precalcList.GetSize () == 0)
        return;

    csArray<ColorEntry> localList = colorList;
    localList.Sort (ColorEntryCompare);

    PrecalcEntry& e0 = precalcList[0];
    e0.add = localList[0].color;
    e0.mult.Set (0,0,0);
    e0.maxTTL = localList[0].maxTTL;

    for (size_t i = 1; i < precalcList.GetSize (); ++i)
    {
      PrecalcEntry& ei = precalcList[i];
      const ColorEntry& ci = localList[i];
      const ColorEntry& cip = localList[i-1];

      ei.maxTTL = ci.maxTTL;
      ei.mult = (ci.color - cip.color) / (ci.maxTTL - cip.maxTTL);
      ei.add = ci.color - ei.mult * ei.maxTTL;
    }

    PrecalcEntry copyLast;
    copyLast.maxTTL = FLT_MAX;
    copyLast.add = localList.Top ().color;
    copyLast.mult.Set (0,0,0);
    precalcList.Push (copyLast);

    precalcInvalid = false;
  }

  //------------------------------------------------------------------------

  ParticleEffectorLinear::ParticleEffectorLinear ()
    : scfImplementationType (this),
      mask (CS_PARTICLE_MASK_ALL), precalcInvalid (true)
  {

  }

  csPtr<iParticleEffector> ParticleEffectorLinear::Clone () const
  {
    return new ParticleEffectorLinear (*this);
  }

#define INTERPOLATE_PARAMETER(parname) \
      particle.parname = ei.add.parname + ei.mult.parname * ttl;
#define INTERPOLATE_PARAMETER_AUX(parname) \
      particleAux.parname = ei.add.parname + ei.mult.parname * ttl;

  void ParticleEffectorLinear::EffectParticles (iParticleSystemBase* system,
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
      if (mask & CS_PARTICLE_MASK_MASS) { INTERPOLATE_PARAMETER (mass) }
      if (mask & CS_PARTICLE_MASK_LINEARVELOCITY) { INTERPOLATE_PARAMETER (linearVelocity) }
      if (mask & CS_PARTICLE_MASK_ANGULARVELOCITY) { INTERPOLATE_PARAMETER (angularVelocity) }
      if (mask & CS_PARTICLE_MASK_COLOR) { INTERPOLATE_PARAMETER_AUX (color) }
      if (mask & CS_PARTICLE_MASK_PARTICLESIZE) { INTERPOLATE_PARAMETER_AUX (particleSize) }
    }
  }

  size_t ParticleEffectorLinear::AddParameterSet (const csParticleParameterSet& param, float maxTTL)
  {
    ParamEntry c;
    c.param = param;
    c.maxTTL = maxTTL;

    paramList.Push (c);

    precalcInvalid = true;
    return paramList.GetSize () - 1;
  }

  void ParticleEffectorLinear::RemoveParameterSet (size_t index)
  {
    paramList.DeleteIndex (index);
    precalcInvalid = true;
  }

  void ParticleEffectorLinear::Clear ()
  {
    paramList.DeleteAll ();
    precalcInvalid = true;
  }

  void ParticleEffectorLinear::SetParameterSet (size_t index, const csParticleParameterSet& param)
  {
    if (index >= paramList.GetSize ())
      return;

    paramList[index].param = param;

    precalcInvalid = true;
  }

  void ParticleEffectorLinear::SetEndTTL (size_t index, float ttl)
  {
    if (index >= paramList.GetSize ())
      return;

    paramList[index].maxTTL = ttl;

    precalcInvalid = true;
  }

  int ParticleEffectorLinear::ParamEntryCompare(
    const ParticleEffectorLinear::ParamEntry &e0, 
    const ParticleEffectorLinear::ParamEntry &e1)
  {
    if (e0.maxTTL < e1.maxTTL)
      return -1;
    else if (e0.maxTTL > e1.maxTTL)
      return 1;
    return 0;
  }

#define INIT_PARAM_PRECALC(parname) \
	ei.mult.parname = (ci.param.parname - cip.param.parname) / (ci.maxTTL - cip.maxTTL); \
	ei.add.parname = ci.param.parname - ei.mult.parname * ei.maxTTL;

  void ParticleEffectorLinear::Precalc ()
  {
    if (!precalcInvalid)
      return;

    precalcList.SetSize (paramList.GetSize ());
    
    if (precalcList.GetSize () == 0)
        return;

    csArray<ParamEntry> localList = paramList;
    localList.Sort (ParamEntryCompare);

    PrecalcEntry& e0 = precalcList[0];
    e0.add = localList[0].param;
    e0.mult.Clear ();
    e0.maxTTL = localList[0].maxTTL;

    for (size_t i = 1; i < precalcList.GetSize (); ++i)
    {
      PrecalcEntry& ei = precalcList[i];
      const ParamEntry& ci = localList[i];
      const ParamEntry& cip = localList[i-1];

      ei.maxTTL = ci.maxTTL;
      if (mask & CS_PARTICLE_MASK_MASS) { INIT_PARAM_PRECALC (mass) }
      if (mask & CS_PARTICLE_MASK_LINEARVELOCITY) { INIT_PARAM_PRECALC (linearVelocity) }
      if (mask & CS_PARTICLE_MASK_ANGULARVELOCITY) { INIT_PARAM_PRECALC (angularVelocity) }
      if (mask & CS_PARTICLE_MASK_COLOR) { INIT_PARAM_PRECALC (color) }
      if (mask & CS_PARTICLE_MASK_PARTICLESIZE) { INIT_PARAM_PRECALC (particleSize) }
    }

    PrecalcEntry copyLast;
    copyLast.maxTTL = FLT_MAX;
    copyLast.add = localList.Top ().param;
    copyLast.mult.Clear ();
    precalcList.Push (copyLast);

    precalcInvalid = false;
  }

  //------------------------------------------------------------------------
  
  namespace
  {
    // Helper method for stepping system one step using fn
    template<typename FnType>
    void StepParticles (FnType& fn, const csParticleBuffer& particleBuffer, 
      float dt, float t0 = 0)
    {
      // Calculate stepping
      const float maxDt = 1/30.0f;
      dt = csMin (dt, maxDt);

      for (size_t idx = 0; idx < particleBuffer.particleCount; ++idx)
      {
        csParticle& particle = particleBuffer.particleData[idx];

        const csVector3& oldPos = particle.position;

         //Calculate new position
        /*float err = */CS::Math::Ode45::Step<FnType, float> 
          (fn, dt, t0, oldPos, particle.position);
          
      }
    }

    // Spiral functor
    struct SpiralFunc
    {
      SpiralFunc (const csVector3& O, const csVector3& D)
        : lineO (O), lineD (D), velScale (1.0f), velOffset (0.0f),
        spreadFactor (0)
      {
      }

      // y is position, give y' = v
      csVector3 operator()(float t, const csVector3& y)
      {
        float c1 = lineD * (y - lineO);
        const csVector3 pl = lineO + c1 * lineD;
        
        // PP is vector from line to particle
        const csVector3 PP = (y - pl);

        csVector3 result =  PP%lineD;
        
        // fix result
        result.x *= velScale.x;
        result.y *= velScale.y;
        result.z *= velScale.z;

        result += PP*spreadFactor;

        return result + velOffset;
      }

      csVector3 lineO, lineD;
      csVector3 velScale, velOffset;
      float spreadFactor;
    };

    // Radial push/pull functor
    struct RadialPointFunc
    {
      RadialPointFunc (const csVector3& O, float scale)
        : origin (O), scale1 (scale), scale2 (0.0f)
      {
      }

      // y is position, give y' = v
      csVector3 operator()(float t, const csVector3& y)
      {
        return (y - origin).Unit () * (scale1 +  scale2 * sinf(t));
      }

      csVector3 origin;
      float scale1, scale2;
    };
  }

  void ParticleEffectorVelocityField::EffectParticles (iParticleSystemBase* system,
    const csParticleBuffer& particleBuffer, float dt, float totalTime)
  {
    if (particleBuffer.particleCount == 0)
      return;

    switch (type)
    {
    case CS_PARTICLE_BUILTIN_SPIRAL:
      {
        if (vparams.GetSize () < 2)
          vparams.SetSize (2);

        // First make sure we have enough parameters to evaluate the function
        SpiralFunc func (vparams[0], vparams[1].Unit ());

        if (vparams.GetSize () >= 3)
          func.velScale = vparams[2];
        if (vparams.GetSize () >= 4)
          func.velOffset = vparams[3];
        if (fparams.GetSize () >= 1)
          func.spreadFactor = fparams[0];

        StepParticles (func, particleBuffer, dt, totalTime);
      }
      break;
    case CS_PARTICLE_BUILTIN_RADIALPOINT:
      {
        if (vparams.GetSize () < 1)
          vparams.SetSize (1);

        if (fparams.GetSize () < 1)
          fparams.SetSize (1);

        // First make sure we have enough parameters to evaluate the function
        RadialPointFunc func (vparams[0], fparams[0]);

        if (fparams.GetSize () >= 2)
          func.scale2 = fparams[1];

        StepParticles (func, particleBuffer, dt, totalTime);
      }
      break;
    default:
      break;
    }
  }


  csPtr<iParticleEffector> ParticleEffectorVelocityField::Clone () const
  {
    return 0;
  }

  //------------------------------------------------------------------------

  ParticleEffectorLight::ParticleEffectorLight ()
    : scfImplementationType (this), cutoffDistance (5.0f)
  {

  }

  ParticleEffectorLight::~ParticleEffectorLight ()
  {
    // Remove all lights from the scene
    for (size_t idx = 0; idx < lights.GetSize (); ++idx)
    {
      csRef<iLight> light = lights.Get (idx);
      light->GetMovable ()->ClearSectors ();
      light->GetMovable ()->GetSceneNode ()->SetParent (nullptr);
      engine->RemoveLight (light);
    }

    for (size_t idx = 0; idx < allocatedLights.GetSize (); ++idx)
    {
      csRef<iLight> light = allocatedLights.Get (idx);
      engine->RemoveLight (light);
    }
  }

  void ParticleEffectorLight::SetInitialCutoffDistance (float distance)
  {
    cutoffDistance = distance;
  }

  float ParticleEffectorLight::GetInitialCutoffDistance () const
  {
    return cutoffDistance;
  }

  csPtr<iParticleEffector> ParticleEffectorLight::Clone () const
  {
    ParticleEffectorLight* newPtr = new ParticleEffectorLight (*this);
    newPtr->lights.DeleteAll ();
    newPtr->allocatedLights.DeleteAll ();
    return newPtr;
  }

  void ParticleEffectorLight::EffectParticles (iParticleSystemBase* system,
    const csParticleBuffer& particleBuffer, float dt, float totalTime)
  {
    ParticlesMeshObject* meshObject = dynamic_cast<ParticlesMeshObject*> (system);
    iMovable* meshMovable = meshObject->GetMeshWrapper ()->GetMovable ();
    iSectorList* meshSectors = meshMovable->GetSectors ();

    if (!engine)
      engine = csQueryRegistry<iEngine> (meshObject->factory->objectType->object_reg);
    if (!engine) return;

    // Remove the lights of the particles that are no more active
    while (lights.GetSize () > particleBuffer.particleCount)
    {
      // Remove the light from the scene
      csRef<iLight> light = lights.Pop ();
      light->GetMovable ()->ClearSectors ();
      light->GetMovable ()->GetSceneNode ()->SetParent (nullptr);

      // Put the light in a temporary buffer
      allocatedLights.Push (light);
    }

    // Create the lights for the new particles
    while (lights.GetSize () < particleBuffer.particleCount)
    {
      csRef<iLight> light = allocatedLights.GetSize () ? allocatedLights.Pop ()
	: engine->CreateLight (0, csVector3 (0.0f),
			       cutoffDistance, csColor4 (0.0f),
			       CS_LIGHT_DYNAMICTYPE_DYNAMIC);

      // Put the light in the scene
      for (int i = 0; i < meshSectors->GetCount (); i++)
	light->GetMovable ()->GetSectors ()->Add (meshSectors->Get (i));
      light->GetMovable ()->GetSceneNode ()->SetParent (meshMovable->GetSceneNode ());

      lights.Push (light);
    }

    // Update the light parameters
    for (size_t idx = 0; idx < particleBuffer.particleCount; ++idx)
    {
      csParticle& particle = particleBuffer.particleData[idx];
      csParticleAux& particleAux = particleBuffer.particleAuxData[idx];
      iLight* light = lights[idx];

      light->GetMovable ()->SetPosition (particle.position);
      light->GetMovable ()->SetTransform (csMatrix3 (particle.orientation));
      light->GetMovable ()->UpdateMove ();
      light->SetColor (particleAux.color);
      light->SetSpecularColor (particleAux.color);
      light->SetCutoffDistance (particleAux.color.alpha * cutoffDistance);
    }
  }

}
CS_PLUGIN_NAMESPACE_END(Particles)
