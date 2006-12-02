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
#include "lightprop.h"
#include "raytracer.h"
#include "scene.h"
#include "statistics.h"

namespace lighter
{

  // Shade a single point in space with direct lighting
  csColor DirectLighting::UniformShadeAllLightsNonPD (Sector* sector, 
    const csVector3& point, const csVector3& normal, 
    SamplerSequence<2>& lightSampler, Raytracer& rt)
  {
    csColor res (0);
    const LightRefArray& allLights = sector->allNonPDLights;

    for (size_t i = 0; i < allLights.GetSize (); ++i)
    {
      float rndValues[2];
      lightSampler.GetNext (rndValues);

      res += ShadeLight (allLights[i], point, normal, rt, rndValues);
    }

    return res;
  }

  // Shade a single point in space with direct lighting using a single light
  csColor DirectLighting::UniformShadeRndLightNonPD (Sector* sector, 
    const csVector3& point, const csVector3& normal, 
    SamplerSequence<3>& lightSampler, Raytracer& rt)
  {
    csColor res (0);
    const LightRefArray& allLights = sector->allNonPDLights;

    // Select random light
    float rndValues[3];
    lightSampler.GetNext (rndValues);

    size_t lightIdx = (size_t) floorf (allLights.GetSize () * rndValues[2]);

    return ShadeLight (allLights[lightIdx], point, normal, rt, rndValues) * 
      allLights.GetSize ();
  }

  // Shade a primitive element with direct lighting
  csColor DirectLighting::UniformShadeAllLightsNonPD (Sector* sector, 
    ElementProxy element, SamplerSequence<4>& lightSampler, Raytracer& rt)
  {
    // Sample each quadrant of element using stratified sampling
    // and random values 2 and 3 for coordinates
    static float quadratConstants[][2] =
    {
      {-0.5f, -0.5f},
      {-0.5f,  0.5f},
      { 0.5f, -0.5f},
      { 0.5f,  0.5f}
    };

    csColor res (0);
    const LightRefArray& allLights = sector->allNonPDLights;

    // Get some data
    const csVector3& uVec = element.primitive.GetuFormVector ();
    const csVector3& vVec = element.primitive.GetvFormVector ();
    const csVector3& elementC = element.primitive.ComputeElementCenter (element.element); 

    // Add handling of "half" elements

    for (size_t i = 0; i < allLights.GetSize (); ++i)
    {
      for (size_t qi = 0; qi < 4; ++qi)
      {
        float rndValues[4];
        lightSampler.GetNext (rndValues);

        csVector3 offsetVector = uVec * (rndValues[2]*quadratConstants[qi][0]) +
          vVec * (rndValues[3]*quadratConstants[qi][1]);

        csVector3 pos = elementC + offsetVector;

        res += ShadeLight (allLights[i], pos, element.primitive.ComputeNormal (pos), 
          rt, rndValues);
      }
    }
    
    return res * 0.25f;
  }

  // Shade a primitive element with direct lighting using a single light
  csColor DirectLighting::UniformShadeRndLightNonPD (Sector* sector, 
    ElementProxy element, SamplerSequence<5>& lightSampler, Raytracer& rt)
  {
    const LightRefArray& allLights = sector->allNonPDLights;
    float rndValues[5];
    lightSampler.GetNext (rndValues);

    size_t lightIdx = (size_t) floorf (allLights.GetSize () * rndValues[2]);

    return UniformShadeOneLight (sector, element, allLights[lightIdx],
      SamplerSequence<4> (lightSampler), rt) * allLights.GetSize ();
  }

  csColor DirectLighting::UniformShadeOneLight (Sector* sector, const csVector3& point, 
    const csVector3& normal, Light* light, SamplerSequence<2>& lightSampler, Raytracer& rt)
  {
    float rndValues[2];
    lightSampler.GetNext (rndValues);

    return ShadeLight (light, point, normal, rt, rndValues);
  }

  csColor DirectLighting::UniformShadeOneLight (Sector* sector, ElementProxy element,
    Light* light, SamplerSequence<4>& lightSampler, Raytracer& rt)
  {
    // Sample each quadrant of element using stratified sampling
    // and random values 2 and 3 for coordinates
    // Use different light per quadrant
    static float quadratConstants[][2] =
    {
      {-0.5f, -0.5f},
      {-0.5f,  0.5f},
      { 0.5f, -0.5f},
      { 0.5f,  0.5f}
    };

    csColor res (0);

    // Get some data
    const csVector3& uVec = element.primitive.GetuFormVector ();
    const csVector3& vVec = element.primitive.GetvFormVector ();
    const csVector3& elementC = element.primitive.ComputeElementCenter (element.element); 

    // Add handling of "half" elements

    for (size_t qi = 0; qi < 4; ++qi)
    {
      float rndValues[4];
      lightSampler.GetNext (rndValues);

      csVector3 offsetVector = uVec * (rndValues[2]*quadratConstants[qi][0]) +
        vVec * (rndValues[3]*quadratConstants[qi][1]);

      csVector3 pos = elementC + offsetVector;

      res += ShadeLight (light, pos, element.primitive.ComputeNormal (pos), 
        rt, rndValues);
    }

    return res * 0.25f;
  }

  csColor DirectLighting::ShadeLight (Light* light, const csVector3& point,
    const csVector3& normal, Raytracer& rt, float lightSamples[2])
  {
    // Some variables..
    VisibilityTester visTester;
    float lightPdf, cosineTerm = 0;
    csVector3 lightVec;

    csColor lightColor = light->SampleLight (point, normal, lightSamples[0],
      lightSamples[1], lightVec, lightPdf, visTester);

    

    if (lightPdf > 0.0f && !lightColor.IsBlack () &&
      (cosineTerm = normal * lightVec) > 0)
    {
      //@@TODO add material...
      if (visTester.Unoccluded (rt))
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



  void DirectLighting::ShadeDirectLighting (Sector* sector, float progressStep)
  {
    // Setup some common stuff
    SamplerSequence<1> masterSampler;
    Raytracer rt (sector->kdTree);

    float progressPerObject = progressStep / sector->allObjects.GetSize ();

    ObjectHash::GlobalIterator giter = sector->allObjects.GetIterator ();

    while (giter.HasNext ())
    {
      csRef<Object> obj = giter.Next ();

      if (obj->lightPerVertex)
        ShadePerVertex (sector, obj, rt, masterSampler);
      else
        ShadeLightmap (sector, obj, rt, masterSampler);

      globalStats.IncTaskProgress (progressPerObject);
      globalTUI.Redraw (TUI::TUI_DRAW_RAYCORE);
    }

    return;
  }
  
  void DirectLighting::ShadeLightmap (Sector* sector, Object* obj, Raytracer& rt, 
    SamplerSequence<1>& masterSampler)
  {
    SamplerSequence<4> subSampler (masterSampler);
    csArray<PrimitiveArray>& submeshArray = obj->GetPrimitives ();

    const LightRefArray& allPDLights = sector->allPDLights;

    for (size_t submesh = 0; submesh < submeshArray.GetSize (); ++submesh)
    {
      PrimitiveArray& primArray = submeshArray[submesh];

      float area2pixel = 1.0f;

      for (size_t pidx = 0; pidx < primArray.GetSize (); ++pidx)
      {
        Primitive& prim = primArray[pidx];

        if (pidx == 0)
        {
          area2pixel = 
            1.0f / (prim.GetuFormVector () % prim.GetvFormVector ()).Norm();
        }

        const FloatDArray& areaArray = prim.GetElementAreas ();
        size_t numElements = prim.GetElementCount ();        
        Lightmap* normalLM = sector->scene->GetLightmap (prim.GetGlobalLightmapID (), (Light*)0);

        csVector2 minUV = prim.GetMinUV ();

        // Iterate all elements
        for (size_t eidx = 0; eidx < numElements; ++eidx)
        {
          if (areaArray[eidx] == 0)
            continue;

          float pixelArea = (areaArray[eidx]*area2pixel);

          // Shade non-PD lights
          ElementProxy ep = prim.GetElement (eidx);
          csColor c = UniformShadeAllLightsNonPD (sector, ep, subSampler, rt);

          uint u, v;
          prim.GetElementUV (eidx, u, v);
          u += minUV.x;
          v += minUV.y;

          normalLM->SetAddPixel (u, v, c * pixelArea);

          // Shade PD lights
          for (size_t pdli = 0; pdli < allPDLights.GetSize (); ++pdli)
          {
            csColor c = UniformShadeOneLight (sector, ep, allPDLights[pdli],
              subSampler, rt);

            Lightmap* lm = sector->scene->GetLightmap (prim.GetGlobalLightmapID (),
              allPDLights[pdli]);
            lm->SetAddPixel (u, v, c * pixelArea);
          }
        }

      }
    }
  }

  void DirectLighting::ShadePerVertex (Sector* sector, Object* obj, Raytracer& rt, 
    SamplerSequence<1>& masterSampler)
  {
    SamplerSequence<2> subSampler (masterSampler);

    const LightRefArray& allPDLights = sector->allPDLights;

    Object::LitColorArray* litColors = obj->GetLitColors ();
    const ObjectVertexData& vdata = obj->GetVertexData ();

    for (size_t i = 0; i < vdata.vertexArray.GetSize (); ++i)
    {
      const csVector3& pos = vdata.vertexArray[i].position;
      const csVector3& normal = vdata.vertexArray[i].normal;

      csColor& c = litColors->Get (i);

      c = UniformShadeAllLightsNonPD (sector, pos, normal, subSampler, rt);

      // Shade PD lights
      for (size_t pdli = 0; pdli < allPDLights.GetSize (); ++pdli)
      {
        c += UniformShadeOneLight (sector, pos, normal, allPDLights[pdli],
          subSampler, rt);
      }
    }

  }

}
