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

#include "crystalspace.h"

#include "common.h"
#include "directlight.h"
#include "lighter.h"
#include "raytracer.h"
#include "scene.h"
#include "statistics.h"

namespace lighter
{
  class PartialElementIgnoreCallback : public HitIgnoreCallback,
    public HitPointCallback
  {
  public:
    PartialElementIgnoreCallback (const Primitive* originalPrim) 
      : originalPrim (originalPrim), pass (0)
    {
    }

    void SetSampleInside (bool inside)
    {
      this->inside = inside;
    }
    void SetPass2 () { pass = 1; }

    virtual bool IgnoreHit (const Primitive* prim);
    virtual void RegisterHit (const Ray &ray, const HitPoint &hit);

    inline csSet<csConstPtrKey<Primitive> >& GetSet ()
    {
      return insideHits;
    }

  protected:
    const Primitive* originalPrim;
    bool inside;
    csSet<csConstPtrKey<Primitive> > insideHits;
    int pass;
  };

  bool PartialElementIgnoreCallback::IgnoreHit (const Primitive* prim)
  {
    const csPlane3& plane = originalPrim->GetPlane();

    // Ignore coplanar primitives.
    if ((fabsf (prim->GetPlane().DD - plane.DD) < SMALL_EPSILON)
        && ((prim->GetPlane().norm - plane.norm).IsZero (SMALL_EPSILON)))
      return true;

    // For inside sample points, no other primitives are ignored
    if (inside)
      return false;

    // Pass 0 is only to collect inside hits.
    if (pass == 0) return !inside;

    // If an inside sample hit that primitive, don't ignore it.
    if (insideHits.Contains (prim))
      return false;

    // Ignore neighbouring primitives.
    for (size_t i = 0; i < 3; i++)
    {
      const size_t vi = prim->GetTriangle()[i];
      const csVector3& pi = prim->GetVertexData().vertexArray[vi].position;

      for (size_t j = 0; j < 3; j++)
      {
        const size_t vj = originalPrim->GetTriangle()[j];
        const csVector3& pj = originalPrim->GetVertexData().vertexArray[vj].position;

        if ((pi - pj).IsZero (SMALL_EPSILON))
          return true;
      }
    }

    return false;
  }

  void PartialElementIgnoreCallback::RegisterHit (const Ray &ray, const HitPoint &hit)
  {
    if (inside)
      insideHits.Add (hit.primitive);
  }

  //-------------------------------------------------------------------------

  static const float ElementQuadrantConstants[][2] =
  {
    {-0.25f, -0.25f},
    {-0.25f,  0.25f},
    { 0.25f, -0.25f},
    { 0.25f,  0.25f}
  };

  DirectLighting::PVLPointShader DirectLighting::pvlPointShader = 0;
  DirectLighting::LMElementShader DirectLighting::lmElementShader = 0;
  DirectLighting::LMElementCollectShadowPrims DirectLighting::lmElementCollect = 0;

  void DirectLighting::Initialize ()
  {
    if (globalLighter->configMgr->GetBool ("lighter2.DirectLightRandom", false))
    {
      pvlPointShader = UniformShadeRndLightNonPD;
      lmElementShader = UniformShadeRndLightNonPD;
      lmElementCollect = CollectShadowRndLightNonPD;
    }
    else
    {
      pvlPointShader = UniformShadeAllLightsNonPD;
      lmElementShader = UniformShadeAllLightsNonPD;
      lmElementCollect = CollectShadowAllLightsNonPD;
    }    
  }

  // Shade a single point in space with direct lighting
  csColor DirectLighting::UniformShadeAllLightsNonPD (Sector* sector, 
    const csVector3& point, const csVector3& normal, 
    SamplerSequence<2>& lightSampler)
  {
    csColor res (0);
    const LightRefArray& allLights = sector->allNonPDLights;

    for (size_t i = 0; i < allLights.GetSize (); ++i)
    {
      float rndValues[2];
      lightSampler.GetNext (rndValues);

      res += ShadeLight (allLights[i], point, normal, lightSampler);
    }

    return res;
  }

  // Shade a single point in space with direct lighting using a single light
  csColor DirectLighting::UniformShadeRndLightNonPD (Sector* sector, 
    const csVector3& point, const csVector3& normal, 
    SamplerSequence<2>& sampler)
  {
    SamplerSequence<3> lightSampler (sampler);

    csColor res (0);
    const LightRefArray& allLights = sector->allNonPDLights;

    // Select random light
    float rndValues[3];
    lightSampler.GetNext (rndValues);

    size_t lightIdx = (size_t) floorf (allLights.GetSize () * rndValues[2]);

    return ShadeLight (allLights[lightIdx], point, normal, sampler) * 
      allLights.GetSize ();
  }

  // Shade a primitive element with direct lighting
  csColor DirectLighting::UniformShadeAllLightsNonPD (Sector* sector, 
    ElementProxy element, SamplerSequence<2>& lightSampler, 
    PartialElementIgnoreCallback* ignoreCB)
  {
    csColor res (0);
    const LightRefArray& allLights = sector->allNonPDLights;

    // Get some data
    const csVector3& uVec = element.primitive.GetuFormVector ();
    const csVector3& vVec = element.primitive.GetvFormVector ();
    const csVector3& elementC = element.primitive.ComputeElementCenter (element.element); 

    // Add handling of "half" elements
    for (size_t qi = 0; qi < 4; ++qi)
    {
      const csVector3 offsetVector = uVec * ElementQuadrantConstants[qi][0] +
        vVec * ElementQuadrantConstants[qi][1];

      const csVector3 pos = elementC + offsetVector;
      const csVector3 normal = element.primitive.ComputeNormal (pos);

      if (ignoreCB)
      {
        ignoreCB->SetSampleInside (element.primitive.PointInside (pos));
        for (size_t i = 0; i < allLights.GetSize (); ++i)
        {
          res += ShadeLight (allLights[i], pos, normal, lightSampler, ignoreCB);
        }
      }
      else
      {
        for (size_t i = 0; i < allLights.GetSize (); ++i)
        {
          res += ShadeLight (allLights[i], pos, normal, lightSampler, &element.primitive);
        }
      }
    }

    
    
    return res * 0.25f;
  }

  // Shade a primitive element with direct lighting using a single light
  csColor DirectLighting::UniformShadeRndLightNonPD (Sector* sector, 
    ElementProxy element, SamplerSequence<2>& sampler,
    PartialElementIgnoreCallback* ignoreCB)
  {
    SamplerSequence<3> lightSampler (sampler);

    csColor res (0);
    const LightRefArray& allLights = sector->allNonPDLights;

    // Get some data
    const csVector3& uVec = element.primitive.GetuFormVector ();
    const csVector3& vVec = element.primitive.GetvFormVector ();
    const csVector3& elementC = element.primitive.ComputeElementCenter (element.element); 

    // Add handling of "half" elements

    for (size_t qi = 0; qi < 4; ++qi)
    {
      float rndValues[3];
      lightSampler.GetNext (rndValues);

      const csVector3 offsetVector = uVec * ElementQuadrantConstants[qi][0] +
        vVec * ElementQuadrantConstants[qi][1];

      const csVector3 pos = elementC + offsetVector;
      const csVector3 normal = element.primitive.ComputeNormal (pos);

      size_t lightIdx = (size_t) floorf (allLights.GetSize () * rndValues[2]);

      if (ignoreCB)
      {
        ignoreCB->SetSampleInside (element.primitive.PointInside (pos));
        res += ShadeLight (allLights[lightIdx], pos, normal, sampler, ignoreCB);
      }
      else
        res += ShadeLight (allLights[lightIdx], pos, normal, sampler, &element.primitive);
    }

    return res * 0.25f * allLights.GetSize ();
  }

  csColor DirectLighting::UniformShadeOneLight (Sector* sector, const csVector3& point, 
    const csVector3& normal, Light* light, SamplerSequence<2>& sampler)
  {
    return ShadeLight (light, point, normal, sampler);
  }

  csColor DirectLighting::UniformShadeOneLight (Sector* sector, ElementProxy element,
    Light* light, SamplerSequence<2>& sampler, 
    PartialElementIgnoreCallback* ignoreCB)
  {
    csColor res (0);

    // Get some data
    const csVector3& uVec = element.primitive.GetuFormVector ();
    const csVector3& vVec = element.primitive.GetvFormVector ();
    const csVector3& elementC = element.primitive.ComputeElementCenter (element.element); 

    // Add handling of "half" elements
    for (size_t qi = 0; qi < 4; ++qi)
    {
      const csVector3 offsetVector = uVec * ElementQuadrantConstants[qi][0] +
        vVec * ElementQuadrantConstants[qi][1];

      const csVector3 pos = elementC + offsetVector;
      const csVector3 normal = element.primitive.ComputeNormal (pos);

      if (ignoreCB)
      {
        ignoreCB->SetSampleInside (element.primitive.PointInside (pos));
        res += ShadeLight (light, pos, normal, sampler, ignoreCB);
      }
      else
        res += ShadeLight (light, pos, normal, sampler, &element.primitive);
    }

    return res * 0.25f;
  }

  csColor DirectLighting::ShadeLight (Light* light, const csVector3& point,
    const csVector3& normal, SamplerSequence<2>& lightSampler,
    const Primitive* shadowIgnorePrimitive)
  {
    // Some variables..
    VisibilityTester visTester;
    float lightPdf, cosineTerm = 0;
    csVector3 lightVec;

    float lightSamples[2];
    lightSampler.GetNext (lightSamples);

    csColor lightColor = light->SampleLight (point, normal, lightSamples[0],
      lightSamples[1], lightVec, lightPdf, visTester);

    if (lightPdf > 0.0f && !lightColor.IsBlack () &&
      (cosineTerm = normal * lightVec) > 0)
    {
      //@@TODO add material...
      if (visTester.Unoccluded (shadowIgnorePrimitive))
      {
        if (light->IsDeltaLight ())
          return lightColor * fabsf (cosineTerm) / lightPdf;
        else
          // Properly handle area sources! See pbrt page 732
          return lightColor * fabsf (cosineTerm) / lightPdf;
      }
    }

    return csColor (0,0,0);
  }

  csColor DirectLighting::ShadeLight (Light* light, const csVector3& point,
    const csVector3& normal, SamplerSequence<2>& lightSampler,
    PartialElementIgnoreCallback* ignoreCB)
  {
    // Some variables..
    VisibilityTester visTester;
    float lightPdf, cosineTerm = 0;
    csVector3 lightVec;

    float lightSamples[2];
    lightSampler.GetNext (lightSamples);

    csColor lightColor = light->SampleLight (point, normal, lightSamples[0],
      lightSamples[1], lightVec, lightPdf, visTester);

    if (lightPdf > 0.0f && !lightColor.IsBlack () &&
      (cosineTerm = normal * lightVec) > 0)
    {
      //@@TODO add material...
      if (visTester.Unoccluded (ignoreCB))
      {
        if (light->IsDeltaLight ())
          return lightColor * fabsf (cosineTerm) / lightPdf;
        else
          // Properly handle area sources! See pbrt page 732
          return lightColor * fabsf (cosineTerm) / lightPdf;
      }
    }

    return csColor (0,0,0);
  }

  void DirectLighting::CollectShadowAllLightsNonPD(Sector* sector, ElementProxy element,
    SamplerSequence<2>& lightSampler, PartialElementIgnoreCallback* ignoreCB)
  {
    const LightRefArray& allLights = sector->allNonPDLights;

    // Get some data
    const csVector3& uVec = element.primitive.GetuFormVector ();
    const csVector3& vVec = element.primitive.GetvFormVector ();
    const csVector3& elementC = element.primitive.ComputeElementCenter (element.element); 

    // Add handling of "half" elements
    for (size_t qi = 0; qi < 4; ++qi)
    {
      const csVector3 offsetVector = uVec * ElementQuadrantConstants[qi][0] +
        vVec * ElementQuadrantConstants[qi][1];

      const csVector3 pos = elementC + offsetVector;
      const csVector3 normal = element.primitive.ComputeNormal (pos);

      ignoreCB->SetSampleInside (element.primitive.PointInside (pos));
      for (size_t i = 0; i < allLights.GetSize (); ++i)
      {
        CollectShadowLight (allLights[i], pos, normal, lightSampler, ignoreCB);
      }
    }
  }

  void DirectLighting::CollectShadowRndLightNonPD(Sector* sector, ElementProxy element,
    SamplerSequence<2>& sampler, PartialElementIgnoreCallback* ignoreCB)
  {
    SamplerSequence<3> lightSampler (sampler);

    csColor res (0);
    const LightRefArray& allLights = sector->allNonPDLights;

    // Get some data
    const csVector3& uVec = element.primitive.GetuFormVector ();
    const csVector3& vVec = element.primitive.GetvFormVector ();
    const csVector3& elementC = element.primitive.ComputeElementCenter (element.element); 

    // Add handling of "half" elements

    for (size_t qi = 0; qi < 4; ++qi)
    {
      float rndValues[3];
      lightSampler.GetNext (rndValues);

      const csVector3 offsetVector = uVec * ElementQuadrantConstants[qi][0] +
        vVec * ElementQuadrantConstants[qi][1];

      const csVector3 pos = elementC + offsetVector;
      const csVector3 normal = element.primitive.ComputeNormal (pos);

      size_t lightIdx = (size_t) floorf (allLights.GetSize () * rndValues[2]);

      ignoreCB->SetSampleInside (element.primitive.PointInside (pos));
      CollectShadowLight (allLights[lightIdx], pos, normal, sampler, ignoreCB);
    }
  }

  void DirectLighting::CollectShadowOneLight(Sector* sector, ElementProxy element,
    Light* light, SamplerSequence<2>& lightSampler, PartialElementIgnoreCallback* ignoreCB)
  {
    const csVector3& uVec = element.primitive.GetuFormVector ();
    const csVector3& vVec = element.primitive.GetvFormVector ();
    const csVector3& elementC = element.primitive.ComputeElementCenter (element.element); 

    for (size_t qi = 0; qi < 4; ++qi)
    {
      const csVector3 offsetVector = uVec * ElementQuadrantConstants[qi][0] +
        vVec * ElementQuadrantConstants[qi][1];

      const csVector3 pos = elementC + offsetVector;
      const csVector3 normal = element.primitive.ComputeNormal (pos);

      ignoreCB->SetSampleInside (element.primitive.PointInside (pos));
      CollectShadowLight (light, pos, normal, lightSampler, ignoreCB);
    }
  }

  void DirectLighting::CollectShadowLight (Light* light, const csVector3& point,
    const csVector3& normal, SamplerSequence<2>& lightSampler, 
    PartialElementIgnoreCallback* ignoreCB)
  {
    // Some variables..
    VisibilityTester visTester;
    float lightPdf/*, cosineTerm = 0*/;
    csVector3 lightVec;

    float lightSamples[2];
    lightSampler.GetNext (lightSamples);

    //@@TODO: Change to real sampling for visibility
    csColor lightColor = light->SampleLight (point, normal, lightSamples[0],
      lightSamples[1], lightVec, lightPdf, visTester);

    if (lightPdf > 0.0f && !lightColor.IsBlack () &&
      (normal * lightVec) > 0)
    {
      visTester.CollectHits (ignoreCB, ignoreCB);
    }
  }

  //--------------------------------------------------------------------------
  void DirectLighting::ShadeDirectLighting (Sector* sector, 
                                            Statistics::SubProgress& progress,
                                            float progressStep)
  {
    // Setup some common stuff
    SamplerSequence<2> masterSampler;

    float progressPerObject = progressStep / sector->allObjects.GetSize ();

    ObjectHash::GlobalIterator giter = sector->allObjects.GetIterator ();

    while (giter.HasNext ())
    {
      csRef<Object> obj = giter.Next ();

      if (!obj->GetFlags ().Check (OBJECT_FLAG_NOLIGHT))
      {
        if (obj->lightPerVertex)
          ShadePerVertex (sector, obj, masterSampler);
        else
          ShadeLightmap (sector, obj, masterSampler);
      }

      progress.IncProgress (progressPerObject);
      globalTUI.Redraw (TUI::TUI_DRAW_RAYCORE);
    }

    return;
  }
  
  void DirectLighting::ShadeLightmap (Sector* sector, Object* obj, 
    SamplerSequence<2>& masterSampler)
  {
    csArray<PrimitiveArray>& submeshArray = obj->GetPrimitives ();
    const LightRefArray& allPDLights = sector->allPDLights;

    csArray<Lightmap*> pdLightLMs;

    for (size_t submesh = 0; submesh < submeshArray.GetSize (); ++submesh)
    {
      PrimitiveArray& primArray = submeshArray[submesh];

      float area2pixel = 1.0f;

      for (size_t pidx = 0; pidx < primArray.GetSize (); ++pidx)
      {
        Primitive& prim = primArray[pidx];

        area2pixel = 
          1.0f / (prim.GetuFormVector () % prim.GetvFormVector ()).Norm();

        const ElementAreas& areas = prim.GetElementAreas ();
        size_t numElements = prim.GetElementCount ();        
        Lightmap* normalLM = sector->scene->GetLightmap (prim.GetGlobalLightmapID (), (Light*)0);

        globalLMCache->LockLM (normalLM);

        pdLightLMs.Empty ();
        for (size_t pdli = 0; pdli < allPDLights.GetSize (); ++pdli)
        {
          Lightmap* lm = sector->scene->GetLightmap (prim.GetGlobalLightmapID (),
            allPDLights[pdli]);

          globalLMCache->LockLM (lm);
          pdLightLMs.Push (lm);
        }


        csVector2 minUV = prim.GetMinUV ();

        // Iterate all elements
        for (size_t eidx = 0; eidx < numElements; ++eidx)
        {
          const float elArea = areas.GetElementArea (eidx);
          if (elArea == 0)
            continue;

          float pixelArea = (elArea*area2pixel);
          bool borderElement = fabsf (pixelArea - 1.0f) > SMALL_EPSILON;

          // Shade non-PD lights
          ElementProxy ep = prim.GetElement (eidx);
          csColor c;
          if (borderElement)
          {
            PartialElementIgnoreCallback callback (&prim);
            // Pass 1: collect inside hits
            //lmElementShader (sector, ep, masterSampler, &callback);
            lmElementCollect (sector, ep, masterSampler, &callback);
            // Pass 2: "real" lighting
            callback.SetPass2 ();
            c = lmElementShader (sector, ep, masterSampler, &callback);
          }
          else
            c = lmElementShader (sector, ep, masterSampler, 0);

          size_t u, v;
          prim.GetElementUV (eidx, u, v);
          u += size_t (minUV.x);
          v += size_t (minUV.y);

          normalLM->SetAddPixel (u, v, c * pixelArea);

          // Shade PD lights
          for (size_t pdli = 0; pdli < allPDLights.GetSize (); ++pdli)
          {
            csColor c;
            if (borderElement)
            {
              PartialElementIgnoreCallback callback (&prim);
              // Pass 1: collect inside hits
              CollectShadowOneLight (sector, ep, allPDLights[pdli],
                masterSampler, &callback);
              //UniformShadeOneLight (sector, ep, allPDLights[pdli],
                //masterSampler, &callback);
              // Pass 2: "real" lighting
              callback.SetPass2 ();
              c = UniformShadeOneLight (sector, ep, allPDLights[pdli],
                masterSampler, &callback);
            }
            else
              c = UniformShadeOneLight (sector, ep, allPDLights[pdli],
                masterSampler, 0);

            Lightmap* lm = pdLightLMs[pdli];

            lm->SetAddPixel (u, v, c * pixelArea);
          }
        }

        globalLMCache->UnlockAll ();
      }
    }
  }

  void DirectLighting::ShadePerVertex (Sector* sector, Object* obj, 
    SamplerSequence<2>& masterSampler)
  {
    const LightRefArray& allPDLights = sector->allPDLights;

    Object::LitColorArray* litColors = obj->GetLitColors ();
    const ObjectVertexData& vdata = obj->GetVertexData ();

    for (size_t i = 0; i < vdata.vertexArray.GetSize (); ++i)
    {
      const csVector3& pos = vdata.vertexArray[i].position;
      const csVector3& normal = vdata.vertexArray[i].normal;

      csColor& c = litColors->Get (i);

      c = pvlPointShader (sector, pos, normal, masterSampler);

      // Shade PD lights
      for (size_t pdli = 0; pdli < allPDLights.GetSize (); ++pdli)
      {
        Object::LitColorArray* pdlColors = obj->GetLitColorsPD (allPDLights[pdli]);
        pdlColors->Get (i) += UniformShadeOneLight (sector, pos, normal, allPDLights[pdli],
          masterSampler);
      }
    }

  }

}
