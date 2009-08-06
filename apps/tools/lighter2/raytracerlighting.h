/*
  Copyright (C) 2006 by Marten Svanfeldt
  Portions Copyright (C) 2009 by Seth Berrier

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

#ifndef __DIRECTLIGHT_H__
#define __DIRECTLIGHT_H__

#include "csutil/noncopyable.h"

#include "lightcomponent.h"
#include "light.h"
#include "lightmap.h"
#include "primitive.h"
#include "raytracer.h"
#include "sampler.h"

namespace lighter
{
  class Sector;
  class Raytracer;
  class Primitive;

  class PartialElementIgnoreCallback;  
  
  // Class to calculate direct lighting
  class RaytracerLighting : public LightComponent
  {
  public:
    // Setup
    RaytracerLighting (const csVector3& tangentSpaceNorm, size_t subLightmapNum);
    ~RaytracerLighting ();

    virtual csColor ComputeElementLightingComponent(Sector* sector, 
      ElementProxy element, SamplerSequence<2>& lightSampler,
      bool recordInfluence);

    virtual csColor ComputePointLightingComponent(Sector* sector, 
      Object* obj, const csVector3& point, const csVector3& normal, 
      SamplerSequence<2>& lightSampler);

    virtual csColor ComputeElementLightingComponent (Sector* sector,
      ElementProxy element, SamplerSequence<2>& lightSampler,
      bool recordInfluence, Light* light);

    virtual csColor ComputePointLightingComponent(Sector* sector, 
      Object* obj, const csVector3& point, const csVector3& normal, 
      SamplerSequence<2>& lightSampler, Light* light);

  private:
    // Shade by using all primitives within range
    //void ShadeDirectLighting (Sector* sector, 
    //  Statistics::Progress& progress);

    // Shade a single point in space with direct lighting
    csColor UniformShadeAllLightsNonPD (Sector* sector, Object* obj, 
      const csVector3& point, const csVector3& normal, 
      SamplerSequence<2>& lightSampler);

    // Shade a single point in space with direct lighting using a single light
    csColor UniformShadeRndLightNonPD (Sector* sector, Object* obj,
      const csVector3& point, const csVector3& normal, 
      SamplerSequence<2>& lightSampler);

    // Shade a primitive element with direct lighting
    csColor UniformShadeAllLightsNonPD (Sector* sector, ElementProxy element,
      SamplerSequence<2>& lightSampler, bool recordInfluence);

    // Shade a primitive element with direct lighting using a single light
    csColor UniformShadeRndLightNonPD (Sector* sector, ElementProxy element,
      SamplerSequence<2>& lightSampler, bool recordInfluence);

    // Shade a primitive element with direct lighting
    csColor UniformShadeOneLight (Sector* sector, Object* obj, 
      const csVector3& point, const csVector3& normal, Light* light, 
      SamplerSequence<2>& lightSampler);

    // Shade a primitive element with direct lighting
    csColor UniformShadeOneLight (Sector* sector, ElementProxy element,
      Light* light, SamplerSequence<2>& lightSampler, bool recordInfluence);

    template<typename T>
    inline csColor UniformShadeElement (T& shade, ElementProxy element, 
      SamplerSequence<2>& lightSampler, bool recordInfluence);

    struct InfluenceRecorder
    {
      Object* obj;
      size_t u, v;
      const csMatrix3& ts;
      uint primGroup;
      float weight;
    
      InfluenceRecorder (Object* obj, size_t u, size_t v,
        const csMatrix3& ts, uint primGroup,
        float weight) : obj (obj), u (u), v (v), ts (ts),
        primGroup (primGroup), weight (weight) {}
      
      void RecordInfluence (Light* light, const csVector3& direction,
        const csColor& color);
    };
  
    struct ShadeAllLightsNonPD
    {
      RaytracerLighting& lighting;
      const LightRefArray& allLights;
      ShadeAllLightsNonPD (RaytracerLighting& lighting, 
        const LightRefArray& allLights) : lighting (lighting), 
        allLights (allLights) {}
      inline csColor ShadeLight (Object* obj, const csVector3& point, 
        const csVector3& normal, SamplerSequence<2>& lightSampler, 
        const Primitive* shadowIgnorePrimitive = 0, 
        bool fullIgnore = false, InfluenceRecorder* influenceRec = 0);
    };

    struct ShadeRndLightNonPD
    {
      RaytracerLighting& lighting;
      const LightRefArray& allLights;
      SamplerSequence<3>& lightSampler;
      ShadeRndLightNonPD (RaytracerLighting& lighting, 
        const LightRefArray& allLights, SamplerSequence<3>& lightSampler) : 
        lighting (lighting), allLights (allLights), 
        lightSampler (lightSampler) {}
      inline csColor ShadeLight (Object* obj, const csVector3& point, 
        const csVector3& normal, SamplerSequence<2>& sampler, 
        const Primitive* shadowIgnorePrimitive = 0, 
        bool fullIgnore = false, InfluenceRecorder* influenceRec = 0);
    };

    struct ShadeOneLight
    {
      RaytracerLighting& lighting;
      Light* light;
      ShadeOneLight (RaytracerLighting& lighting, Light* light) : 
        lighting (lighting), light (light) {}
      inline csColor ShadeLight (Object* obj, const csVector3& point, 
        const csVector3& normal, SamplerSequence<2>& sampler, 
        const Primitive* shadowIgnorePrimitive = 0, 
        bool fullIgnore = false, InfluenceRecorder* influenceRec = 0);
    };

    // Methods...
    inline csColor ShadeLight (Light* light, Object* obj, 
      const csVector3& point, const csVector3& normal, 
      SamplerSequence<2>& lightSampler, 
      const Primitive* shadowIgnorePrimitive = 0, 
      bool fullIgnore = false, csVector3* incomingLightVec = 0);

    // Helpers
    csVector3 ComputeElementNormal (ElementProxy element, const csVector3& pt) const;
    
    // Function Pointer -- Shade a point
    typedef csColor (RaytracerLighting::*PVLPointShader)(Sector* sector, 
      Object* obj, const csVector3& point, const csVector3& normal, 
      SamplerSequence<2>& lightSampler);

    // Function Pointer -- Shade a lightmap element
    typedef csColor (RaytracerLighting::*LMElementShader)(Sector* sector, 
      ElementProxy element, SamplerSequence<2>& lightSampler,
      bool recordInfluence);

    // Normal Calculation data
    csVector3 tangentSpaceNorm;
    bool fancyTangentSpaceNorm;

    // Pointers to the shading functions
    PVLPointShader pvlPointShader;
    LMElementShader lmElementShader;
  };
}

#endif
