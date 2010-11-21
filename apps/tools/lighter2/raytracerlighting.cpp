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

#include "common.h"

#include "raytracerlighting.h"

#include "lighter.h"
#include "raytracer.h"
#include "scene.h"
#include "statistics.h"

namespace lighter
{
  //-------------------------------------------------------------------------

  RaytracerLighting::RaytracerLighting (const csVector3& tangentSpaceNorm,
    size_t subLightmapNum) : tangentSpaceNorm (tangentSpaceNorm),
    fancyTangentSpaceNorm (!(tangentSpaceNorm - csVector3 (0, 0, 1)).IsZero ())
  {
    PDLightsSupported = true;
    if (globalLighter->configMgr->GetBool ("lighter2.DirectLightRandom", false))
    {
      pvlPointShader = &RaytracerLighting::UniformShadeRndLightNonPD;
      lmElementShader = &RaytracerLighting::UniformShadeRndLightNonPD;
    }
    else
    {
      pvlPointShader = &RaytracerLighting::UniformShadeAllLightsNonPD;
      lmElementShader = &RaytracerLighting::UniformShadeAllLightsNonPD;
    }    
  }

  RaytracerLighting::~RaytracerLighting () {}

  csColor RaytracerLighting::ComputeElementLightingComponent(
    Sector* sector, ElementProxy element,
    SamplerSequence<2>& lightSampler,
    bool recordInfluence)
  {
    return (this->*lmElementShader) (sector, element, lightSampler, recordInfluence);
  }

  csColor RaytracerLighting::ComputePointLightingComponent(
    Sector* sector, Object* obj, const csVector3& point,
    const csVector3& normal, SamplerSequence<2>& lightSampler)
  {
    return (this->*pvlPointShader) (sector, obj, point, normal, lightSampler);
  }

  csColor RaytracerLighting::ComputeElementLightingComponent (
    Sector* sector, ElementProxy element, SamplerSequence<2>& lightSampler,
    bool recordInfluence, Light* light)
  {
    return UniformShadeOneLight(sector, element, light,
      lightSampler, recordInfluence);
  }

  csColor RaytracerLighting::ComputePointLightingComponent(
    Sector* sector, Object* obj, const csVector3& point,
    const csVector3& normal, SamplerSequence<2>& lightSampler,
    Light* light)
  {
    return UniformShadeOneLight(sector, obj, point,
      normal, light, lightSampler);
  }

  // Shade a single point in space with direct lighting
  csColor RaytracerLighting::UniformShadeAllLightsNonPD (Sector* sector, 
    Object* obj, const csVector3& point, const csVector3& normal, 
    SamplerSequence<2>& lightSampler)
  {
    csColor res (0);
    const LightRefArray& allLights = sector->allNonPDLights;

    for (size_t i = 0; i < allLights.GetSize (); ++i)
    {
      if (!affectingLights.IsBitSet (i)) 
        continue;

      res += ShadeLight (allLights[i], obj, point, normal, lightSampler);
    }

    return res;
  }

  // Shade a single point in space with direct lighting using a single light
  csColor RaytracerLighting::UniformShadeRndLightNonPD (Sector* sector, 
    Object* obj, const csVector3& point, const csVector3& normal, 
    SamplerSequence<2>& sampler)
  {
    SamplerSequence<3> lightSampler (sampler);

    const LightRefArray& allLights = sector->allNonPDLights;

    // Select random light
    float rndValues[3];
    lightSampler.GetNext (rndValues);

    size_t lightIdx = (size_t) floorf (allLights.GetSize () * rndValues[2]);
    if (!affectingLights.IsBitSet (lightIdx)) 
      return csColor (0);

    return ShadeLight (allLights[lightIdx], obj, point, normal, sampler) * 
      allLights.GetSize ();
  }

  template<typename T>
  csColor RaytracerLighting::UniformShadeElement (T& shade, ElementProxy element, 
    SamplerSequence<2>& lightSampler, bool recordInfluence)
  {
    csColor res (0);

    // Get some data
    const csVector3& uVec = element.primitive.GetuFormVector ();
    const csVector3& vVec = element.primitive.GetvFormVector ();
    const csVector3& elementC = element.primitive.ComputeElementCenter (element.element); 
  
    const csVector2 defElementQuadrantConstants[4] = {
      csVector2(-0.25f, -0.25f),
      csVector2(-0.25f,  0.25f),
      csVector2( 0.25f, -0.25f),
      csVector2( 0.25f,  0.25f)
    };
    csVector2 clippedElementQuadrantConstants[4];
    const csVector2* ElementQuadrantConstants = defElementQuadrantConstants;

    Primitive::ElementType elemType = 
      element.primitive.GetElementType (element.element);
    if (elemType == Primitive::ELEMENT_BORDER)
    {
      element.primitive.RecomputeQuadrantOffset (element.element, 
        defElementQuadrantConstants, clippedElementQuadrantConstants);
      ElementQuadrantConstants = clippedElementQuadrantConstants;
    }
    
#ifdef DUMP_NORMALS
    {
      const csVector3 normal = ComputeElementNormal (element, elementC);
      const csVector3 normalBiased = normal*0.5f + csVector3 (0.5f);
      return csColor (normalBiased.x, normalBiased.y, normalBiased.z);
    }
#endif

    size_t u, v;
    if (recordInfluence)
    {
      // @@@ redundant, outer primitive loop already does this
      element.primitive.GetElementUV (element.element, u, v);
      const csVector2& minUV = element.primitive.GetMinUV();
      u += size_t (floorf (minUV.x));
      v += size_t (floorf (minUV.y));
    }

    for (size_t qi = 0; qi < 4; ++qi)
    {
      const csVector3 offsetVector = uVec * ElementQuadrantConstants[qi].x +
        vVec * ElementQuadrantConstants[qi].y;

      csVector3 pos = elementC + offsetVector;
      
      const csVector3 normal = ComputeElementNormal (element, pos);
      if (elemType == Primitive::ELEMENT_BORDER)
      {
        const float fudge = EPSILON;
        /* Slightly offset the point on the primitive to work around unwanted 
           occlusions by e.g. neighbouring prims */
        pos -= element.primitive.GetPlane().Normal() * fudge;
      }

      if (recordInfluence)
      {
        csMatrix3 ts (element.primitive.GetObject()->ComputeTangentSpace (
          &element.primitive, pos));
        
        InfluenceRecorder inflRec (element.primitive.GetObject(),
          u, v, ts, element.primitive.GetGroupID(), 0.25f);
	res += shade.ShadeLight (element.primitive.GetObject(), 
	  pos, normal, lightSampler,
	  &element.primitive,
	  elemType == Primitive::ELEMENT_BORDER,
	  &inflRec);
      }
      else
      {
	res += shade.ShadeLight (element.primitive.GetObject(), 
	  pos, normal, lightSampler,
	  &element.primitive,
	  elemType == Primitive::ELEMENT_BORDER);
      }
    }
    
    return 0.25f * res;
  }

  void RaytracerLighting::InfluenceRecorder::RecordInfluence (Light* light,
    const csVector3& direction, const csColor& color)
  {
    csVector3 dirW_n (direction);
    dirW_n.Normalize();
    csVector3 dirO = obj->GetObjectToWorld().Other2ThisRelative (dirW_n);
    csVector3 dirT = ts.GetInverse() * dirO;
    dirT.Normalize();
    
    LightInfluences& influences =
      obj->GetLightInfluences (primGroup, light);
    ScopedSwapLock<LightInfluences> l (influences);
    influences.AddDirection (u, v, dirT, color.Luminance() * weight);
  }
  
  csColor RaytracerLighting::ShadeAllLightsNonPD::ShadeLight (Object* obj, 
    const csVector3& point, const csVector3& normal, 
    SamplerSequence<2>& lightSampler,
    const Primitive* shadowIgnorePrimitive, 
    bool fullIgnore, InfluenceRecorder* influenceRec)
  {
    csColor res (0);
    for (size_t i = 0; i < allLights.GetSize (); ++i)
    {
      if (!lighting.affectingLights.IsBitSet (i)) 
        continue;

      csVector3 lightVec;
      csColor litColor (lighting.ShadeLight (allLights[i], obj, point,
        normal, lightSampler,
        shadowIgnorePrimitive, fullIgnore, &lightVec));
      
      if (influenceRec)
        influenceRec->RecordInfluence (allLights[i], lightVec, litColor);
      res += litColor;
    }
    return res;
  }

  // Shade a primitive element with direct lighting
  csColor RaytracerLighting::UniformShadeAllLightsNonPD (Sector* sector, 
    ElementProxy element, SamplerSequence<2>& lightSampler,
    bool recordInfluence)
  {
    const LightRefArray& allLights = sector->allNonPDLights;
    ShadeAllLightsNonPD shade (*this, allLights);
    return UniformShadeElement (shade, element, lightSampler, recordInfluence);
  }

  csColor RaytracerLighting::ShadeRndLightNonPD::ShadeLight (Object* obj, 
    const csVector3& point, const csVector3& normal, 
    SamplerSequence<2>& sampler, 
    const Primitive* shadowIgnorePrimitive, 
    bool fullIgnore, InfluenceRecorder* influenceRec)
  {
    float rndValues[3];
    
    lightSampler.GetNext (rndValues);
    size_t lightIdx = (size_t) floorf (allLights.GetSize () * rndValues[2]);

    if (!lighting.affectingLights.IsBitSet (lightIdx)) 
      return csColor (0);

    csVector3 lightVec;
    csColor litColor (lighting.ShadeLight (allLights[lightIdx], obj, point,
      normal, sampler,
      shadowIgnorePrimitive, fullIgnore, &lightVec));
    if (influenceRec)
      influenceRec->RecordInfluence (allLights[lightIdx], lightVec, litColor);
    return litColor;
  }

  // Shade a primitive element with direct lighting using a single light
  csColor RaytracerLighting::UniformShadeRndLightNonPD (Sector* sector, 
    ElementProxy element, SamplerSequence<2>& sampler, bool recordInfluence)
  {
    SamplerSequence<3> lightSampler (sampler);
    const LightRefArray& allLights = sector->allNonPDLights;

    ShadeRndLightNonPD shade (*this, allLights, lightSampler);
    return UniformShadeElement (shade, element, sampler, recordInfluence) 
      * allLights.GetSize ();
  }

  csColor RaytracerLighting::UniformShadeOneLight (Sector* sector, Object* obj, 
    const csVector3& point, const csVector3& normal, Light* light, 
    SamplerSequence<2>& sampler)
  {
    return ShadeLight (light, obj, point, normal, sampler);
  }

  csColor RaytracerLighting::ShadeOneLight::ShadeLight (Object* obj, 
    const csVector3& point, const csVector3& normal, 
    SamplerSequence<2>& sampler,
    const Primitive* shadowIgnorePrimitive,
    bool fullIgnore, InfluenceRecorder* influenceRec)
  {
    csVector3 lightVec;
    csColor litColor (lighting.ShadeLight (light, obj, point,
      normal, sampler,
      shadowIgnorePrimitive, fullIgnore, &lightVec));
    if (influenceRec)
      influenceRec->RecordInfluence (light, lightVec, litColor);
    return litColor;
  }

  csColor RaytracerLighting::UniformShadeOneLight (Sector* sector, ElementProxy element,
    Light* light, SamplerSequence<2>& sampler, bool recordInfluence)
  {
    ShadeOneLight shade (*this, light);
    return UniformShadeElement (shade, element, sampler, recordInfluence);
  }

  class RaytracerLightingBorderIgnoreCb : public HitIgnoreCallback
  {
  public:
    explicit RaytracerLightingBorderIgnoreCb (const Primitive* ignorePrim,
      const csVector3& rayDir, const csVector3& point) : ignorePrim (ignorePrim), 
        rayDir (rayDir), point (point)
    {}

    virtual bool IgnoreHit (const Primitive* prim)
    {
      // Ignore coplanar primitives.
      if (prim->GetPlane() == ignorePrim->GetPlane ())
        return true;

      return false;
    }

  private:
    const Primitive* ignorePrim;
    const csVector3 rayDir;
    const csVector3 point;
  };
  
  csColor RaytracerLighting::ShadeLight (Light* light, Object* obj, 
    const csVector3& point, const csVector3& normal, 
    SamplerSequence<2>& lightSampler, 
    const Primitive* shadowIgnorePrimitive, bool fullIgnore,
    csVector3* incomingLightVec)
  {
    // Some variables..
    VisibilityTester visTester (light, obj);
    float lightPdf, cosineTerm = 0;
    csVector3 lightVec;
    const Object* ignObj = obj->GetFlags().Check (OBJECT_FLAG_NOSELFSHADOW)
      ? obj : 0;

    const bool isDelta = true; //light->IsDeltaLight (); no support for area lights yet

    float lightSamples[2] = {0};
    if (!isDelta)
      lightSampler.GetNext (lightSamples);

    csColor lightColor = light->SampleLight (point, normal, lightSamples[0],
      lightSamples[1], lightVec, lightPdf, visTester);

    if (incomingLightVec) *incomingLightVec = lightVec;
    if (lightPdf > 0.0f && !lightColor.IsBlack () &&
      (cosineTerm = normal * lightVec) > 0)
    {
      VisibilityTester::OcclusionState occlusion;
      if (fullIgnore)
      {
        RaytracerLightingBorderIgnoreCb icb (shadowIgnorePrimitive, -lightVec,
          point);
        occlusion = visTester.Occlusion (ignObj, &icb);
      }
      else
      {
        occlusion = visTester.Occlusion (ignObj, shadowIgnorePrimitive);
      }
      if (occlusion == VisibilityTester::occlOccluded)
        return csColor (0, 0, 0);
      else if (occlusion == VisibilityTester::occlPartial)
        lightColor *= visTester.GetFilterColor ();
        
      if (isDelta)
        return lightColor * fabsf (cosineTerm) / lightPdf;
      else
        // Properly handle area sources! See pbrt page 732
        return lightColor * fabsf (cosineTerm) / lightPdf;
    }

    return csColor (0,0,0);
  }

  csVector3 RaytracerLighting::ComputeElementNormal (ElementProxy element,
                                                  const csVector3& pt) const
  {
    if (fancyTangentSpaceNorm)
    {
      csMatrix3 ts = element.primitive.GetObject()->ComputeTangentSpace (
        &element.primitive, pt);
      csVector3 v = ts * tangentSpaceNorm;
      v.Normalize();
      return v;
    }
    else
      return element.primitive.ComputeNormal (pt);
  }

}
