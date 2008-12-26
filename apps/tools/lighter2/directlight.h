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

#ifndef __DIRECTLIGHT_H__
#define __DIRECTLIGHT_H__

#include "csutil/noncopyable.h"

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
  class DirectLighting : private CS::NonCopyable
  {
  public:
    // Setup
    DirectLighting (const csVector3& tangentSpaceNorm, size_t subLightmapNum);

    // Shade by using all primitives within range
    void ShadeDirectLighting (Sector* sector, 
      Statistics::Progress& progress);

    //-- Shade a point
    typedef csColor (DirectLighting::*PVLPointShader)(Sector* sector, 
      Object* obj, const csVector3& point, const csVector3& normal, 
      SamplerSequence<2>& lightSampler);

    // Shade a single point in space with direct lighting
    csColor UniformShadeAllLightsNonPD (Sector* sector, Object* obj, 
      const csVector3& point, const csVector3& normal, 
      SamplerSequence<2>& lightSampler);

    // Shade a single point in space with direct lighting using a single light
    csColor UniformShadeRndLightNonPD (Sector* sector, Object* obj,
      const csVector3& point, const csVector3& normal, 
      SamplerSequence<2>& lightSampler);

    //-- Shade a lightmap element
    typedef csColor (DirectLighting::*LMElementShader)(Sector* sector, 
      ElementProxy element, SamplerSequence<2>& lightSampler,
      bool recordInfluence);

    // Shade a primitive element with direct lighting
    csColor UniformShadeAllLightsNonPD (Sector* sector, ElementProxy element,
      SamplerSequence<2>& lightSampler, bool recordInfluence);

    // Shade a primitive element with direct lighting using a single light
    csColor UniformShadeRndLightNonPD (Sector* sector, ElementProxy element,
      SamplerSequence<2>& lightSampler, bool recordInfluence);

    //-- Shade using one light
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

  private:
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
      DirectLighting& lighting;
      const LightRefArray& allLights;
      ShadeAllLightsNonPD (DirectLighting& lighting, 
        const LightRefArray& allLights) : lighting (lighting), 
        allLights (allLights) {}
      inline csColor ShadeLight (Object* obj, const csVector3& point, 
        const csVector3& normal, SamplerSequence<2>& lightSampler, 
        const Primitive* shadowIgnorePrimitive = 0, 
        bool fullIgnore = false, InfluenceRecorder* influenceRec = 0);
    };

    struct ShadeRndLightNonPD
    {
      DirectLighting& lighting;
      const LightRefArray& allLights;
      SamplerSequence<3>& lightSampler;
      ShadeRndLightNonPD (DirectLighting& lighting, 
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
      DirectLighting& lighting;
      Light* light;
      ShadeOneLight (DirectLighting& lighting, Light* light) : 
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

    class ProgressState
    {
      Statistics::Progress& progress;
      size_t updateFreq;
      size_t u;
      float progressStep;

    public:
      ProgressState (Statistics::Progress& progress, size_t total) : 
        progress (progress), 
        updateFreq (progress.GetUpdateFrequency (total)), u (updateFreq),
        progressStep (float (updateFreq) / total) {}

      CS_FORCEINLINE void Advance ()
      {
        if (--u == 0)
        {
          progress.IncProgress (progressStep);
          u = updateFreq;
          globalTUI.Redraw (TUI::TUI_DRAW_RAYCORE);
        }
      }
    };

    void ShadeLightmap (Sector* sector, Object* obj, 
      SamplerSequence<2>& masterSampler, ProgressState& progress);

    void ShadePerVertex (Sector* sector, Object* obj,
      SamplerSequence<2>& masterSampler, ProgressState& progress);

    // Helpers
    csVector3 ComputeElementNormal (ElementProxy element,
      const csVector3& pt) const;
    csVector3 ComputeVertexNormal (Object* obj, size_t index) const;
    
    void ComputeAffectingLights (Object* obj);

    // Data...
    csVector3 tangentSpaceNorm;
    bool fancyTangentSpaceNorm;
    size_t subLightmapNum;
    PVLPointShader pvlPointShader;
    LMElementShader lmElementShader;
    csBitArray affectingLights;
  };
}

#endif
