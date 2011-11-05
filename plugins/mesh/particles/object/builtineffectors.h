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

#ifndef __CS_MESH_BUILTINEFFECTORS_H__
#define __CS_MESH_BUILTINEFFECTORS_H__

#include "csutil/scf_implementation.h"

#include "imesh/particles.h"
#include "iutil/comp.h"

struct iLight;

CS_PLUGIN_NAMESPACE_BEGIN(Particles)
{

  //------------------------------------------------------------------------

  class ParticleEffectorFactory : public 
    scfImplementation2<ParticleEffectorFactory,
                       iParticleBuiltinEffectorFactory,
                       iComponent>
  {
  public:
    ParticleEffectorFactory (iBase* parent)
      : scfImplementationType (this, parent)
    {
    }

    //-- iParticleBuiltinEffectorFactory
    virtual csPtr<iParticleBuiltinEffectorForce> CreateForce () const;
    virtual csPtr<iParticleBuiltinEffectorLinear> CreateLinear () const;
    virtual csPtr<iParticleBuiltinEffectorLinColor> CreateLinColor () const;
    virtual csPtr<iParticleBuiltinEffectorVelocityField> 
      CreateVelocityField () const;
    virtual csPtr<iParticleBuiltinEffectorLight> CreateLight () const;

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*)
    {
      return true;
    }
  };

  //------------------------------------------------------------------------

  class ParticleEffectorForce : public 
    scfImplementation2<ParticleEffectorForce,
                       iParticleBuiltinEffectorForce,
                       scfFakeInterface<iParticleEffector> >
  {
  public:
    ParticleEffectorForce ()
      : scfImplementationType (this),
      acceleration (0.0f), force (0.0f), randomAcceleration (0.0f, 0.0f, 0.0f),
      do_randomAcceleration (false)
    {
    }

    //-- iParticleEffector
    virtual csPtr<iParticleEffector> Clone () const;

    virtual void EffectParticles (iParticleSystemBase* system,
      const csParticleBuffer& particleBuffer, float dt, float totalTime);

    //-- iParticleBuiltinEffectorForce
    virtual void SetAcceleration (const csVector3& acceleration)
    {
      this->acceleration = acceleration;
    }

    virtual const csVector3& GetAcceleration () const
    {
      return acceleration;
    }

    virtual void SetForce (const csVector3& force)
    {
      this->force = force;
    }

    virtual const csVector3& GetForce () const
    {
      return force; 
    }

    virtual void SetRandomAcceleration (const csVector3& magnitude)
    {
      randomAcceleration = magnitude;
      if (randomAcceleration < .000001f)
	do_randomAcceleration = false;
      else
	do_randomAcceleration = true;
    }

    virtual const csVector3& GetRandomAcceleration () const
    {
      return randomAcceleration;
    }

  private:
    csVector3 acceleration;
    csVector3 force;
    csVector3 randomAcceleration;
    bool do_randomAcceleration;
  };

  //------------------------------------------------------------------------

  class ParticleEffectorLinColor : public
    scfImplementation2<ParticleEffectorLinColor,
                       iParticleBuiltinEffectorLinColor,
                       scfFakeInterface<iParticleEffector> >
  {
  public:
    //-- ParticleEffectorLinColor
    ParticleEffectorLinColor ();

    //-- iParticleEffector
    virtual csPtr<iParticleEffector> Clone () const;

    virtual void EffectParticles (iParticleSystemBase* system,
      const csParticleBuffer& particleBuffer, float dt, float totalTime);


    //-- iParticleBuiltinEffectorLinColor
    virtual size_t AddColor (const csColor4& color, float maxTTL);
    virtual void RemoveColor (size_t index);
    virtual void Clear ();

    virtual void SetColor (size_t index, const csColor4& color);
    virtual void SetEndTTL (size_t index, float ttl);

    virtual void GetColor (size_t index, csColor4& color, float& maxTTL) const
    {
      if (index >= colorList.GetSize ())
        return;

      color = colorList[index].color;
      maxTTL = colorList[index].maxTTL;
    }
    virtual const csColor4& GetColor (size_t index) const
    {
      return colorList[index].color;
    }
    virtual float GetEndTTL (size_t index) const
    {
      return colorList[index].maxTTL;
    }

    virtual size_t GetColorCount () const
    {
      return colorList.GetSize ();
    }

  private:
    void Precalc ();

    struct ColorEntry
    {
      csColor4 color;
      float maxTTL;
    };
    csArray<ColorEntry> colorList;

    static int ColorEntryCompare(const ColorEntry& e0, const ColorEntry& e1);

    struct PrecalcEntry
    {
      csColor4 mult;
      csColor4 add;
      float maxTTL;
    };
    bool precalcInvalid;
    csArray<PrecalcEntry> precalcList;
  };

  //------------------------------------------------------------------------

  class ParticleEffectorLinear : public
    scfImplementation2<ParticleEffectorLinear,
                       iParticleBuiltinEffectorLinear,
                       scfFakeInterface<iParticleEffector> >
  {
  public:
    //-- ParticleEffectorLinear
    ParticleEffectorLinear ();

    //-- iParticleEffector
    virtual csPtr<iParticleEffector> Clone () const;

    virtual void EffectParticles (iParticleSystemBase* system,
      const csParticleBuffer& particleBuffer, float dt, float totalTime);

    //-- iParticleBuiltinEffectorLinear
    virtual void SetMask (int mask)
    {
      ParticleEffectorLinear::mask = mask;
    }

    virtual int GetMask () const
    {
      return mask;
    }

    virtual size_t AddParameterSet (const csParticleParameterSet& param, float endTTL);
    virtual void RemoveParameterSet (size_t index);
    virtual void Clear ();
    virtual void SetParameterSet (size_t index, const csParticleParameterSet& param);
    virtual void SetEndTTL (size_t index, float ttl);
    virtual void GetParameterSet (size_t index, csParticleParameterSet& param, float& maxTTL) const
    {
      if (index >= paramList.GetSize ())
        return;

      param = paramList[index].param;
      maxTTL = paramList[index].maxTTL;
    }
    virtual const csParticleParameterSet& GetParameterSet (size_t index) const
    {
      return paramList[index].param;
    }
    virtual float GetEndTTL (size_t index) const
    {
      return paramList[index].maxTTL;
    }

    virtual size_t GetParameterSetCount () const
    {
      return paramList.GetSize ();
    }

  private:
    void Precalc ();

    int mask;

    struct ParamEntry
    {
      csParticleParameterSet param;
      float maxTTL;
    };
    csArray<ParamEntry> paramList;

    static int ParamEntryCompare(const ParamEntry& e0, const ParamEntry& e1);

    struct PrecalcEntry
    {
      csParticleParameterSet mult;
      csParticleParameterSet add;
      float maxTTL;
    };
    bool precalcInvalid;
    csArray<PrecalcEntry> precalcList;
  };

  //------------------------------------------------------------------------

  class ParticleEffectorVelocityField : public 
    scfImplementation2<ParticleEffectorVelocityField,
                       iParticleBuiltinEffectorVelocityField,
                       scfFakeInterface<iParticleEffector> >
  {
  public:
    ParticleEffectorVelocityField  ()
      : scfImplementationType (this),
      type (CS_PARTICLE_BUILTIN_SPIRAL)
    {
    }

    //-- iParticleEffector
    virtual csPtr<iParticleEffector> Clone () const;

    virtual void EffectParticles (iParticleSystemBase* system,
      const csParticleBuffer& particleBuffer, float dt, float totalTime);

    //-- iParticleBuiltinEffectorForce
    virtual void SetType (csParticleBuiltinEffectorVFType type)
    {
      this->type = type;
    }

    virtual csParticleBuiltinEffectorVFType GetType () const
    {
      return type;
    }

    virtual void SetFParameter (size_t parameterNumber, float value)
    {
      fparams.Put (parameterNumber, value);
    }

    virtual float GetFParameter (size_t parameterNumber) const
    {
      if (parameterNumber >= fparams.GetSize ())
        return 0;

      return fparams[parameterNumber];
    }

    virtual size_t GetFParameterCount () const
    {
      return fparams.GetSize ();
    }

    virtual void AddFParameter(float value)
    {
      fparams.Push(value);
    }

    virtual void RemoveFParameter(size_t index)
    {
      fparams.DeleteIndex(index);
    }

    virtual void SetVParameter (size_t parameterNumber, const csVector3& value)
    {
      vparams.Put (parameterNumber, value);
    }

    virtual csVector3 GetVParameter (size_t parameterNumber) const
    {
      if (parameterNumber >= vparams.GetSize ())
        return csVector3(0,0,0);

      return vparams[parameterNumber];
    }

    virtual size_t GetVParameterCount () const
    {
      return vparams.GetSize ();
    }

    virtual void AddVParameter(const csVector3& value)
    {
      vparams.Push(value);
    }

    virtual void RemoveVParameter(size_t index)
    {
      vparams.DeleteIndex(index);
    }

  private:
    csParticleBuiltinEffectorVFType type;
    csArray<csVector3> vparams;
    csArray<float> fparams;
  };

  //------------------------------------------------------------------------

  class ParticleEffectorLight : public
    scfImplementation2<ParticleEffectorLight,
                       iParticleBuiltinEffectorLight,
                       scfFakeInterface<iParticleEffector> >
  {
  public:
    //-- ParticleEffectorLight
    ParticleEffectorLight ();
    ~ParticleEffectorLight ();

    //-- iParticleEffector
    virtual csPtr<iParticleEffector> Clone () const;

    virtual void EffectParticles (iParticleSystemBase* system,
      const csParticleBuffer& particleBuffer, float dt, float totalTime);

    //-- iParticleBuiltinEffectorLight
    virtual void SetInitialCutoffDistance (float distance);
    virtual float GetInitialCutoffDistance () const;

  private:
    float cutoffDistance;

    csRef<iEngine> engine;
    csRefArray<iLight> lights;
    csRefArray<iLight> allocatedLights;
  };

}
CS_PLUGIN_NAMESPACE_END(Particles)

#endif

