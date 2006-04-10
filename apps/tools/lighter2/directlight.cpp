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

#include "directlight.h"
#include "scene.h"
#include "statistics.h"
#include "raytracer.h"

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
      float primProgressStep = lightProgressStep / prims.GetSize ();
      for (unsigned i = 0; i < prims.GetSize (); i++)
      {
        globalStats.IncTaskProgress (primProgressStep);
        ShadeRadPrimitive (rayTracer, *prims[i], radLight);
        globalTUI.Redraw (TUI::TUI_DRAW_RAYCORE | TUI::TUI_DRAW_PROGRESS);
      }

    }
  }

  void DirectLighting::ShadeRadPrimitive(Raytracer &tracer, RadPrimitive &prim, 
    Light *light)
  {
    // Compute shading for each point on the primitive, using normal Phong shading
    // for diffuse surfaces

    csVector3 elementCenter = prim.GetMinCoord () + prim.GetuFormVector () * 0.5f + prim.GetvFormVector () * 0.5f;

    int minU, minV, maxU, maxV;
    prim.ComputeMinMaxUV (minU, maxU, minV, maxV);

    uint findex = 0;

    for (int v = minV; v <= maxV; v++)
    {
      csVector3 ec = elementCenter;
      for (int u = minU; u <= maxU; u++, findex++, ec += prim.GetuFormVector ())
      {
        const float elemArea = prim.GetElementAreas ()[findex];
        if (elemArea <= 0.0f) continue; //need an area

        csVector3 jiVec = light->position - ec;

        float distSq = jiVec.SquaredNorm ();
        jiVec.Normalize ();

        float cosTheta_j = - (prim.GetPlane ().GetNormal () * jiVec);

        if (cosTheta_j <= 0.0f) continue; //backface culling

        // Do a 5 ray visibility test
        float visFact = RaytraceFunctions::Vistest5 (tracer, 
          ec, prim.GetuFormVector (), prim.GetvFormVector (), light->position);
        //float visFact = 1.0f;
        
        // refl = reflectance * lightsource * cos(theta) / distance^2 * visfact
        float phongConst = cosTheta_j / distSq;

        // energy
        csColor energy = light->freeEnergy * phongConst * visFact;
        csColor reflected = energy * prim.GetOriginalPrimitive ()->GetReflectanceColor();

        // Store the reflected color
        Lightmap * lm = prim.GetRadObject ()->GetLightmaps ()[prim.GetLightmapID ()];
        lm->GetData ()[v*lm->GetWidth ()+u] += reflected;
        

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
