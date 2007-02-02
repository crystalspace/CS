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
  csColor DirectLighting::UniformShadeAllLights (Sector* sector, 
    const csVector3& point, const csVector3& normal, 
    SamplerSequence<2>& lightSampler, Raytracer& rt)
  {
    csColor res (0);
    const LightRefArray& allLights = sector->allLights;

    for (size_t i = 0; i < allLights.GetSize (); ++i)
    {
      float rndValues[2];
      lightSampler.GetNext (rndValues);

      res += ShadeLight (allLights[i], point, normal, rt, rndValues);
    }

    return res;
  }

  // Shade a single point in space with direct lighting using a single light
  csColor DirectLighting::UniformShadeOneLight (Sector* sector, 
    const csVector3& point, const csVector3& normal, 
    SamplerSequence<3>& lightSampler, Raytracer& rt)
  {
    csColor res (0);
    const LightRefArray& allLights = sector->allLights;

    // Select random light
    float rndValues[3];
    lightSampler.GetNext (rndValues);

    size_t lightIdx = (size_t) floorf (allLights.GetSize () * rndValues[2]);

    return ShadeLight (allLights[lightIdx], point, normal, rt, rndValues) * 
      allLights.GetSize ();
  }

  // Shade a primitive element with direct lighting
  csColor DirectLighting::UniformShadeAllLights (Sector* sector, 
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
    const LightRefArray& allLights = sector->allLights;

    // Get some data
    const csVector3& uVec = element.primitive.GetuFormVector ();
    const csVector3& vVec = element.primitive.GetvFormVector ();
    const csVector3& elementC = element.primitive.ComputeElementCenter (element.element); 

    const csVector3& normal = element.primitive.GetPlane ().Normal ();

    // Add handling of "half" elements

    for (size_t i = 0; i < allLights.GetSize (); ++i)
    {
      for (size_t qi = 0; qi < 4; ++qi)
      {
        float rndValues[4];
        lightSampler.GetNext (rndValues);

        csVector3 offsetVector = uVec * (rndValues[2]*quadratConstants[qi][0]) +
          vVec * (rndValues[3]*quadratConstants[qi][1]);

        res += ShadeLight (allLights[i], elementC + offsetVector, 
          normal, rt, rndValues);
      }
    }
    
    return res * 0.25f;
  }

  // Shade a primitive element with direct lighting using a single light
  csColor DirectLighting::UniformShadeOneLight (Sector* sector, 
    ElementProxy element, SamplerSequence<5>& lightSampler, Raytracer& rt)
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
    const LightRefArray& allLights = sector->allLights;

    // Get some data
    const csVector3& uVec = element.primitive.GetuFormVector ();
    const csVector3& vVec = element.primitive.GetvFormVector ();
    const csVector3& elementC = element.primitive.ComputeElementCenter (element.element); 

    const csVector3& normal = element.primitive.GetPlane ().Normal ();

    // Add handling of "half" elements

    for (size_t qi = 0; qi < 4; ++qi)
    {
      float rndValues[5];
      lightSampler.GetNext (rndValues);

      csVector3 offsetVector = uVec * (rndValues[2]*quadratConstants[qi][0]) +
        vVec * (rndValues[3]*quadratConstants[qi][1]);

      size_t lightIdx = (size_t) floorf (allLights.GetSize () * rndValues[5]); 

      res += ShadeLight (allLights[lightIdx], elementC + offsetVector, 
        normal, rt, rndValues);
    }

    return res * 0.25f * allLights.GetSize ();
  }

  csColor DirectLighting::ShadeLight (Light* light, const csVector3& point,
    const csVector3& normal, Raytracer& rt, float lightSamples[2])
  {
    // Some variables..
    VisibilityTester visTester;
    float lightPdf;
    csVector3 lightVec;

    csColor lightColor = light->SampleLight (point, normal, lightSamples[0],
      lightSamples[1], lightVec, lightPdf, visTester);

    if (lightPdf > 0.0f && !lightColor.IsBlack ())
    {
      //@@TODO add material...
      if (!visTester.Unoccluded (rt))
      {
        if (light->IsDeltaLight ())
          return lightColor * fabsf (lightVec * normal) / lightPdf;
        else
          // Properly handle area sources! See pbrt page 732
          return lightColor * fabsf (lightVec * normal) / lightPdf;
      }
    }


    return csColor (0,0,0);
  }

  class Data_Lightmap
  {
    Sector* sector;
    Primitive& prim;
    csVector3 elementCenter;
    float area2pixel;
    int minU, minV, maxU, maxV;
    uint findex;
    csVector3 normals[3];
    struct State
    {
      float lmArea;
      csVector3 ec;
      int u, v;
    };
    State state;
    State next;
    bool nextValid;
    csVector3 norm;

    void SeekNext()
    {
      nextValid = false;
      while (!nextValid)
      {
        next.u++;
        next.ec += prim.GetuFormVector ();
        if (next.u > maxU)
        {
          next.u = minU;
          elementCenter += prim.GetvFormVector ();
          next.v++;
          next.ec = elementCenter;
        }
        if (next.v > maxV)
        {
          return;
        }

        const float elemArea = prim.GetElementAreas ()[findex++];
        if (elemArea > 0.0f) 
        {
          //need an area
	  next.lmArea = elemArea * area2pixel;
          nextValid = true;
        }
      };
    }
  public:
    Data_Lightmap (Sector* sector, Primitive& prim) : sector (sector), 
      prim (prim), findex (0)
    {
      elementCenter = prim.GetMinCoord () 
	+ prim.GetuFormVector () * 0.5f 
	+ prim.GetvFormVector () * 0.5f;

      area2pixel = 
	1.0f / (prim.GetuFormVector () % prim.GetvFormVector ()).Norm();

      prim.ComputeMinMaxUV (minU, maxU, minV, maxV);

      const Primitive::TriangleType& t = prim.GetTriangle ();
      const ObjectVertexData& vertexData = prim.GetVertexData ();

      normals[0] = vertexData.vertexArray[t.a].normal;
      normals[1] = vertexData.vertexArray[t.b].normal;
      normals[2] = vertexData.vertexArray[t.c].normal;

      next.u = minU-1;
      next.v = minV;
      next.ec = elementCenter - prim.GetuFormVector ();
      SeekNext();
    }

    /// Returns whether a next element is available
    bool HasNext() 
    {
      return nextValid;
    }
    /// Go to next element
    void Next()
    {
      state = next;
      SeekNext();
    }
    /// Get position of element to light
    const csVector3& GetPosition() 
    {
      return state.ec;
    }
    /// Get normal of element to light
    const csVector3& GetNormal() 
    {
      csVector3 testPt (state.ec);
      if (!prim.PointInside (state.ec))
      {
	/* "Snap" the point for normal computation to the nearest point on
	 * the primitive */
	float minDistSq = FLT_MAX;
	for (size_t i = 0; i < 3; i++)
	{
	  const csVector3& v1 = 
	    prim.GetVertexData().vertexArray[prim.GetTriangle()[i]].position;
	  const csVector3& v2 = 
	    prim.GetVertexData().vertexArray[prim.GetTriangle()[(i+1)%3]].position;
	  csVector3 d = v2-v1;
	  csVector3 dn (d); dn.Normalize();
	  float f = (state.ec-v1) * dn;
	  f = csClamp (f, d.Norm(), 0.0f);
	  csVector3 p = v1 + f*dn;
	  float distSq = (state.ec - p).SquaredNorm();
	  if (distSq < minDistSq)
	  {
	    minDistSq = distSq;
            testPt = p;
          }
        }
      }
      float lambda, my;
      prim.ComputeBaryCoords (testPt, lambda, my);
      norm = lambda * normals[0] + my * normals[1] + 
        (1 - lambda - my) * normals[2];
      norm.Normalize();
      return norm;
    }
    /**
     * Returns whether the current element should be culled, given the vector
     * to the light. 
     */
    bool CullElement (const csVector3& jiVec) 
    {
      // Backface culling
      return (jiVec * prim.GetPlane().Normal() >= 0.0f);
    }
    /// Apply computed light value to light value for element.
    void ApplyLight (Light_old* light, const csColor& reflected)
    {
      Lightmap* lm = sector->scene->GetLightmap (
        prim.GetGlobalLightmapID (), light);
#ifndef DUMP_NORMALS
      lm->GetData ()[state.v*lm->GetWidth ()+state.u] += 
        reflected * state.lmArea;
#else
      lm->GetData ()[state.v*lm->GetWidth ()+state.u].Set (reflected.x*0.5+0.5, 
        reflected.y*0.5+0.5, reflected.z*0.5+0.5);
#endif
    }
    /// Determine visibility of element
    float GetVisibility (Raytracer &tracer, csVector3& lightPos)
    {
      // Do a 5 ray visibility test
      return RaytraceFunctions::Vistest5 (tracer, 
	state.ec, prim.GetuFormVector (), prim.GetvFormVector (), lightPos, 
        prim);
    }
  };

  class Data_PerVertex
  {
    PrimitivePtrArray& prims;
    Object::LitColorArray* litColors;

    struct State
    {
      size_t primIndex;
      int vertex;
      size_t vertIndex;
      ObjectVertexData::Vertex* v;
    };
    State state, next;
    bool nextValid;
    csSet<size_t> litVertices;

    void SeekNext ()
    {
      nextValid = false;
      while (!nextValid)
      {
        next.vertex++;
        if (next.vertex >= 3)
        {
          next.primIndex++;
          if (next.primIndex >= prims.GetSize()) return;
          next.vertex = 0;
        }
        Primitive* prim = prims[next.primIndex];
        size_t v = prim->GetTriangle()[next.vertex];
        if (!litVertices.Contains (v))
        {
          litVertices.AddNoTest (v);
          next.v = &prim->GetVertexData().vertexArray[v];
          next.vertIndex = v;
          nextValid = true;
        }
      }
    }
  public:
    Data_PerVertex (PrimitivePtrArray& prims, Object* obj) : 
      prims (prims), litColors (obj->GetLitColors())
    {
      next.primIndex = 0;
      next.vertex = -1;
      SeekNext ();
    }

    /// Returns whether a next element is available
    bool HasNext() 
    {
      return nextValid;
    }
    /// Go to next element
    void Next()
    {
      state = next;
      SeekNext();
    }
    /// Get position of element to light
    const csVector3& GetPosition() 
    {
      return state.v->position;
    }
    /// Get normal of element to light
    const csVector3& GetNormal() 
    {
      return state.v->normal;
    }
    /**
     * Returns whether the current element should be culled, given the vector
     * to the light. 
     */
    bool CullElement (const csVector3& jiVec) 
    {
      return false;
    }
    /// Apply computed light value to light value for element.
    void ApplyLight (Light_old* /*light*/, const csColor& reflected)
    {
#ifndef DUMP_NORMALS
      (*litColors)[state.vertIndex] += reflected;
#else
      (*litColors)[state.vertIndex].Set (reflected.x*0.5+0.5, 
        reflected.y*0.5+0.5, reflected.z*0.5+0.5);
#endif
    }
    /// Determine visibility of element
    float GetVisibility (Raytracer &tracer, csVector3& lightPos)
    {
      // Do a 1 ray visibility test
      return RaytraceFunctions::Vistest1 (tracer, GetPosition(), lightPos);
    }
  };

  typedef csHash<PrimitivePtrArray,
    csPtrKey<Object> > PrimitivesPerObjectHash;

  template<class Attenuation>
  struct Shade
  {
    // Shade a single rad primitive
    template<class Data>
    static void ShadeElements (Data& data, const Attenuation& attn, 
      Raytracer &tracer, Light_old* light)
    {
      // Compute shading for each point on the primitive, using normal Phong shading
      // for diffuse surfaces

      while (data.HasNext())
      {
        data.Next();

        csVector3 jiVec = light->position - data.GetPosition();

        float distSq = jiVec.SquaredNorm ();
        jiVec.Normalize ();

  #ifndef DUMP_NORMALS
        // Backface culling
        if (data.CullElement (jiVec)) continue;
  #endif

        const csVector3& norm = data.GetNormal();
	float cosTheta_j = (norm * jiVec);

  #ifndef DUMP_NORMALS
        // Also cull if interpolated normal points away
        if (cosTheta_j <= 0.0f) continue; 
  #endif

        float visFact = data.GetVisibility (tracer, light->position);
        //float visFact = 1.0f;
        
	// refl = reflectance * lightsource * cos(theta) / distance^2 * visfact
	float phongConst = attn (cosTheta_j, distSq);

	// energy
	csColor energy = light->freeEnergy * phongConst * visFact;
	csColor reflected = energy 
          /* @@@ FIXME: * prim.GetOriginalPrimitive ()->GetReflectanceColor()*/;

	// Store the reflected color
        data.ApplyLight (light, reflected);
      }
    }

    // Shade a number of primitives
    static void Primitives (Sector* sector, const Attenuation& attn, 
      Raytracer &tracer, PrimitivesPerObjectHash& prims, Light_old* light, 
      float progressStep)
    {
      // Iterate over primitives
      float objProgressStep = progressStep / prims.GetSize ();
      PrimitivesPerObjectHash::GlobalIterator objIt = prims.GetIterator();
      while (objIt.HasNext ())
      {
        csPtrKey<Object> obj;
        PrimitivePtrArray& objPrims = objIt.Next (obj);
        if (obj->lightPerVertex)
        {
          Data_PerVertex data (objPrims, obj);
          ShadeElements (data, attn, tracer, light);
        }
        else
        {
          for (unsigned i = 0; i < objPrims.GetSize (); i++)
          {
            Data_Lightmap data (sector, *objPrims[i]);
            ShadeElements (data, attn, tracer, light);
          }
        }
        globalStats.IncTaskProgress (objProgressStep);
        globalTUI.Redraw (TUI::TUI_DRAW_RAYCORE | TUI::TUI_DRAW_PROGRESS);
      }
    }

  };

  void DirectLighting::ShootDirectLighting (Sector* sector, float progressStep)
  {
    // Need a raytracer for shadow-rays
    Raytracer rayTracer (sector->kdTree);

    // Iterate all lights
    LightOldRefArray::Iterator lightIt = sector->allLightsOld.GetIterator ();
    float lightProgressStep = progressStep / sector->allLightsOld.GetSize ();

    while (lightIt.HasNext ())
    {
      csRef<Light_old> radLight = lightIt.Next ();

      // Rework to use more correct scheme
      radLight->freeEnergy = radLight->color * 
        globalConfig.GetDIProperties ().pointLightMultiplier;

      // Get all primitives
      PrimitivePtrArray prims;

      if (!KDTreeHelper::CollectPrimitives (sector->kdTree, prims, radLight->boundingBox))
        continue;

      PrimitivesPerObjectHash primitivesPerObject;
      for (unsigned i = 0; i < prims.GetSize (); i++)
      {
        Primitive& prim = *prims[i];
        if (!primitivesPerObject.Contains (prim.GetObject()))
        {
          primitivesPerObject.Put (prim.GetObject(), PrimitivePtrArray ());
        }
        PrimitivePtrArray* objPrims = 
          primitivesPerObject.GetElementPointer (prim.GetObject());
        objPrims->Push (&prim);
      }

      switch (radLight->attenuation)
      {
        default:
        case CS_ATTN_NONE:
          {
            NoAttenuation attn (*radLight);
	    Shade<NoAttenuation>::Primitives (sector, attn, rayTracer, 
	      primitivesPerObject, radLight, lightProgressStep);
          }
          break;
        case CS_ATTN_LINEAR:
          {
            LinearAttenuation attn (*radLight);
            Shade<LinearAttenuation>::Primitives (sector, attn, rayTracer, 
	      primitivesPerObject, radLight, lightProgressStep);
          }
          break;
        case CS_ATTN_INVERSE:
          {
            InverseAttenuation attn (*radLight);
            Shade<InverseAttenuation>::Primitives (sector, attn, rayTracer, 
	      primitivesPerObject, radLight, lightProgressStep);
          }
          break;
        case CS_ATTN_REALISTIC:
          {
            RealisticAttenuation attn (*radLight);
            Shade<RealisticAttenuation>::Primitives (sector, attn, 
	      rayTracer, primitivesPerObject, radLight, lightProgressStep);
          }
          break;
        case CS_ATTN_CLQ:
          {
            CLQAttenuation attn (*radLight);
	    Shade<CLQAttenuation>::Primitives (sector, attn, rayTracer, 
	      primitivesPerObject, radLight, lightProgressStep);
          }
          break;
      }
    }
  }


}
