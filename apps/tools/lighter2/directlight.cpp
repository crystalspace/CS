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

  void DirectLighting::ShootDirectLighting (Sector* sector, float progressStep)
  {
    // Need a raytracer for shadow-rays
    Raytracer rayTracer (sector->kdTree);

    // Iterate all lights
    LightRefArray::Iterator lightIt = sector->allLights.GetIterator ();
    float lightProgressStep = progressStep / sector->allLights.GetSize ();

    while (lightIt.HasNext ())
    {
      csRef<Light> radLight = lightIt.Next ();

      // Rework to use more correct scheme
      radLight->freeEnergy = radLight->color * 
        globalConfig.GetDIProperties ().pointLightMultiplier;

      // Get all primitives
      RadPrimitivePtrArray prims;
      if (!KDTreeHelper::CollectPrimitives (sector->kdTree, prims, radLight->boundingBox))
        continue;

      // Iterate over primitives
      switch (radLight->attenuation)
      {
        default:
        case CS_ATTN_NONE:
          {
            NoAttenuation attn (*radLight);
            ShadeRadPrimitives (sector, attn, rayTracer, prims, radLight, 
              lightProgressStep);
          }
          break;
        case CS_ATTN_LINEAR:
          {
            LinearAttenuation attn (*radLight);
            ShadeRadPrimitives (sector, attn, rayTracer, prims, radLight, 
              lightProgressStep);
          }
          break;
        case CS_ATTN_INVERSE:
          {
            InverseAttenuation attn (*radLight);
            ShadeRadPrimitives (sector, attn, rayTracer, prims, radLight, 
              lightProgressStep);
          }
          break;
        case CS_ATTN_REALISTIC:
          {
            RealisticAttenuation attn (*radLight);
            ShadeRadPrimitives (sector, attn, rayTracer, prims, radLight, 
              lightProgressStep);
          }
          break;
        case CS_ATTN_CLQ:
          {
            CLQAttenuation attn (*radLight);
            ShadeRadPrimitives (sector, attn, rayTracer, prims, radLight, 
              lightProgressStep);
          }
          break;
      }
    }
  }

  template<class Attenuation>
  void DirectLighting::ShadeRadPrimitives (Sector* sector, 
                                           const Attenuation& attn, 
                                           Raytracer &rayTracer, 
                                           RadPrimitivePtrArray& prims, 
                                           Light* light, 
                                           float progressStep)
  {
    float primProgressStep = progressStep / prims.GetSize ();
    for (unsigned i = 0; i < prims.GetSize (); i++)
    {
      globalStats.IncTaskProgress (primProgressStep);
      ShadeRadPrimitive (sector, attn, rayTracer, *prims[i], light);
      globalTUI.Redraw (TUI::TUI_DRAW_RAYCORE | TUI::TUI_DRAW_PROGRESS);
    }
  }

  template<class Attenuation>
  void DirectLighting::ShadeRadPrimitive (Sector* sector, 
    const Attenuation& attn, Raytracer &tracer, RadPrimitive &prim, 
    Light *light)
  {
    // Compute shading for each point on the primitive, using normal Phong shading
    // for diffuse surfaces
    csVector3 elementCenter = prim.GetMinCoord () 
      + prim.GetuFormVector () * 0.5f 
      + prim.GetvFormVector () * 0.5f;

    float area2pixel = 
      1.0f / (prim.GetuFormVector () % prim.GetvFormVector ()).Norm();

    int minU, minV, maxU, maxV;
    prim.ComputeMinMaxUV (minU, maxU, minV, maxV);

    uint findex = 0;

    for (int v = minV; v <= maxV; v++)
    {
      csVector3 ec = elementCenter;
      for (int u = minU; u <= maxU; u++, findex++, 
        ec += prim.GetuFormVector ())
      {
        const float elemArea = prim.GetElementAreas ()[findex];
        if (elemArea <= 0.0f) continue; //need an area
        const float lmArea = elemArea * area2pixel;

        csVector3 jiVec = light->position - ec;

        float distSq = jiVec.SquaredNorm ();
        jiVec.Normalize ();

        float lambda, my;
        prim.ComputeBaryCoords (ec, lambda, my);
        csVector3 norm = 
          lambda * prim.GetVertexData().vertexArray[prim.GetIndexArray()[0]].normal +
          my * prim.GetVertexData().vertexArray[prim.GetIndexArray()[1]].normal +
          (1 - lambda - my) * prim.GetVertexData().vertexArray[prim.GetIndexArray()[2]].normal;
        norm.Normalize();
        float cosTheta_j = (norm * jiVec);

#ifndef DUMP_NORMALS
        if (cosTheta_j <= 0.0f) continue; //backface culling
#endif

        // Do a 5 ray visibility test
        float visFact = RaytraceFunctions::Vistest5 (tracer, 
          ec, prim.GetuFormVector (), prim.GetvFormVector (), light->position, prim);
        //float visFact = 1.0f;
        
        // refl = reflectance * lightsource * cos(theta) / distance^2 * visfact
        float phongConst = attn (cosTheta_j, distSq);

        // energy
        csColor energy = light->freeEnergy * phongConst * visFact;
        csColor reflected = energy * prim.GetOriginalPrimitive ()->GetReflectanceColor();

        // Store the reflected color
        Lightmap* lm = sector->scene->GetLightmap (
          prim.GetGlobalLightmapID (), light);
#ifndef DUMP_NORMALS
        lm->GetData ()[v*lm->GetWidth ()+u] += reflected * lmArea;
#else
        lm->GetData ()[v*lm->GetWidth ()+u].Set (norm.x*0.5+0.5, 
          norm.y*0.5+0.5, norm.z*0.5+0.5);
#endif

        // If we later do radiosity, collect the reflected energy
        if (globalConfig.GetLighterProperties ().doRadiosity)
        {
          uint patchIndex = (v-minV)/globalConfig.GetRadProperties ().vPatchResolution * prim.GetuPatches ()+(u-minU)/globalConfig.GetRadProperties ().uPatchResolution;
          RadPatch &patch = prim.GetPatches ()[patchIndex];
          patch.energy += reflected;
        }

      }
      elementCenter += prim.GetvFormVector ();
    }
  }

}
