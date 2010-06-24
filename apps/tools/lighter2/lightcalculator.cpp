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

#include "lightcalculator.h"
#include "lightcomponent.h"

#include "scene.h"
#include "lightmap.h"

namespace lighter
{
  //--------------------------------------------------------------------------
  LightCalculator::LightCalculator (const csVector3& tangentSpaceNorm, 
    size_t subLightmapNum) : tangentSpaceNorm (tangentSpaceNorm),
    fancyTangentSpaceNorm (!(tangentSpaceNorm - csVector3 (0, 0, 1)).IsZero ()),
    subLightmapNum (subLightmapNum)
  {}

  LightCalculator::~LightCalculator() {}

  void LightCalculator::addComponent(LightComponent* newComponent, float coefficient, float offset)
  {
    component.push_back(newComponent);
    componentCoefficient.push_back(coefficient);
    componentOffset.push_back(offset);
  }

  void LightCalculator::ComputeSectorStaticLighting (
    Sector* sector, Statistics::Progress& progress)
  {
    // Set task progress to 0%
    progress.SetProgress (0);
    size_t totalElements = 0;
    
    // Resize the effecting light list in each component
    for(size_t i=0; i<component.size(); i++)
      component[i]->resizeAffectingLights( sector->allNonPDLights.GetSize ());

    // Sum up total amount of elements for progress display purposes
    ObjectHash::GlobalIterator giter = sector->allObjects.GetIterator ();
    while (giter.HasNext ())
    {
      // Get the next object and skip unlight objects
      csRef<Object> obj = giter.Next ();
      if (obj->GetFlags ().Check (OBJECT_FLAG_NOLIGHT)) continue;

      // Count elements (vertices or primitives depending on global settings)
      if (obj->lightPerVertex)
        totalElements += obj->GetVertexData().positions.GetSize();

      else
      {
        // Loop through submesses to get a count all primitives
        csArray<PrimitiveArray>& submeshArray = obj->GetPrimitives ();
        for (size_t submesh = 0; submesh < submeshArray.GetSize (); ++submesh)
        {
          PrimitiveArray& primArray = submeshArray[submesh];

          for (size_t pidx = 0; pidx < primArray.GetSize (); ++pidx)
          {
            Primitive& prim = primArray[pidx];
            totalElements += prim.GetElementCount();
          }
        }
      }

    }

    // Initialize the sampler
    SamplerSequence<2> masterSampler;

    // Create a progressState to easily increment progress on this task
    Statistics::ProgressState progressState(progress, totalElements);

    // Rest the object looper and loop though all object again
    giter.Reset();
    while (giter.HasNext ())
    {
      // Get reference to next object
      csRef<Object> obj = giter.Next ();

      // Skip unlight objects
      if (obj->GetFlags ().Check (OBJECT_FLAG_NOLIGHT))
        continue;

      // Create the list of affecting lights in each component
      ComputeAffectingLights (obj);

      // Either light per vertex or add light to the lightmap
      if (obj->lightPerVertex)
      {
        ComputeObjectStaticLightingForVertex (
          sector, obj, masterSampler, progressState);
      }
      else
      {
        ComputeObjectStaticLightingForLightmap (
          sector, obj, masterSampler, progressState);
      }

    }

    // Set task progress to 100%
    progress.SetProgress (1);
    return;
  }

  void LightCalculator::ComputeObjectStaticLightingForLightmap (
    Sector* sector, Object* obj, 
    SamplerSequence<2>& masterSampler,
    Statistics::ProgressState& progress)
  {
    // Get submesh list for looping through elements
    csArray<PrimitiveArray>& submeshArray = obj->GetPrimitives ();

    // Get list of pseudo-dynamic lights in this sector
    const LightRefArray& allPDLights = sector->allPDLights;

    // Local lists of pseudo-dynamic lights affecting this object
    // and the lightmaps associated with them
    LightRefArray PDLights;
    csArray<Lightmap*> pdLightLMs;

    // Create list of pseudo-dynamic lights that will affect object
    for (size_t pdli = 0; pdli < allPDLights.GetSize (); ++pdli)
    {
      Light* pdl = allPDLights[pdli];
      if (pdl->GetBoundingSphere().TestIntersect (obj->GetBoundingSphere()))
      {
        PDLights.Push (pdl);
      }
    }

    // Loop through all submesh elements
    for (size_t submesh = 0; submesh < submeshArray.GetSize (); ++submesh)
    {
      // Reset area/pixel ratio
      float area2pixel = 1.0f;

      // Loop through all element primitives    
      PrimitiveArray& primArray = submeshArray[submesh];
      for (size_t pidx = 0; pidx < primArray.GetSize (); ++pidx)
      {
        // Get next primitive
        Primitive& prim = primArray[pidx];

        // Compute area/pixel ratio
        area2pixel = 
          1.0f / (prim.GetuFormVector () % prim.GetvFormVector ()).Norm();

        // Get reference to this primitive's lightmap (non pseudo-dynamic)
        size_t numElements = prim.GetElementCount ();        
        Lightmap* normalLM = sector->scene->GetLightmap (
          prim.GetGlobalLightmapID (), subLightmapNum, (Light*)0);

        // Lock access to this lightmap (for thread syncronization)
        ScopedSwapLock<Lightmap> lightLock (*normalLM);

        // This seems to have something to do with specular maps but I'm unsure ??
        bool recordInfluence =
          globalConfig.GetLighterProperties().specularDirectionMaps
          && (subLightmapNum == 0);

        // Rebuild the list of pseudo-dynamic lightmaps
        pdLightLMs.Empty ();
        for (size_t pdli = 0; pdli < PDLights.GetSize (); ++pdli)
        {
          // Ger reference to this light's lightmap
          Lightmap* lm = sector->scene->GetLightmap (prim.GetGlobalLightmapID (),
            subLightmapNum, PDLights[pdli]);

          // Lock the lightmap for local use only and add it to the list
          lm->Lock ();
          pdLightLMs.Push (lm);
        }

        // Compute some object space metrics (minimums and offsets)
        csVector2 minUV = prim.GetMinUV ();
        const size_t uOffs = size_t (floorf (minUV.x));
        const size_t vOffs = size_t (floorf (minUV.y));

        // Iterate all primitive elements
        for (size_t eidx = 0; eidx < numElements; ++eidx)
        {
          // Skip empty elements
          Primitive::ElementType elemType = prim.GetElementType (eidx);
          if (elemType == Primitive::ELEMENT_EMPTY)
          {                        
            progress.Advance ();
            continue;
          }

          // Some stuff (Not sure of the purpose)
          ElementProxy ep = prim.GetElement (eidx);
          size_t u, v;
          prim.GetElementUV (eidx, u, v);
          u += uOffs;
          v += vOffs;

          // Ditto
          const float pixelAreaPart = 
            elemType == Primitive::ELEMENT_BORDER ? prim.ComputeElementFraction (eidx) : 
                                                    1.0f;

          // Compute lighting for non pseudo-dynamic lights
          csColor c(0, 0, 0);
          for(size_t i=0; i<component.size(); i++)
          {
            csColor value = 
                  component[i]->ComputeElementLightingComponent(sector,
                                    ep, masterSampler, recordInfluence);
            
            if(!value.IsBlack())
            {
              c += componentCoefficient[i] * value + componentOffset[i];
            }
          }

          // Update the normal lightmap
          normalLM->SetAddPixel (u, v, c * pixelAreaPart);

          // Loop through pseudo-dynamic lights
          for (size_t pdli = 0; pdli < PDLights.GetSize (); ++pdli)
          {    
            Lightmap* lm = pdLightLMs[pdli];
            Light* pdl = PDLights[pdli];

            // Compute lighting for one pseudo-dynamic light
            csColor c(0, 0, 0);
            for(size_t i=0; i<component.size(); i++)
            {
              if(component[i]->SupportsPDLights())
              {
                csColor value =
                  component[i]->ComputeElementLightingComponent(sector, ep,
                                        masterSampler, recordInfluence, pdl);

                if(!value.IsBlack())
                {
                  c += componentCoefficient[i] * value + componentOffset[i];
                }
              }
            }

            // Update this light's light map
            lm->SetAddPixel (u, v, c * pixelAreaPart);
          }

          // Done with one primitive element
          // Advance the task progress indicator
          progress.Advance ();
        }

        // Release the locks on the pseudo-dynamic light maps
        // for this object
        for (size_t pdli = 0; pdli < PDLights.GetSize (); ++pdli)
        {
          pdLightLMs[pdli]->Unlock();
        }

      }   // End primitive loop
    }     // End submesh element loop
  }       // End function

  void LightCalculator::ComputeObjectStaticLightingForVertex (
    Sector* sector, Object* obj, 
    SamplerSequence<2>& masterSampler,
    Statistics::ProgressState& progress)
  {
    const LightRefArray& allPDLights = sector->allPDLights;
    LightRefArray PDLights;

    Object::LitColorArray* litColors = obj->GetLitColors (subLightmapNum);
    const ObjectVertexData& vdata = obj->GetVertexData ();

    for (size_t pdli = 0; pdli < allPDLights.GetSize (); ++pdli)
    {
      Light* pdl = allPDLights[pdli];
      if (pdl->GetBoundingSphere().TestIntersect (obj->GetBoundingSphere()))
      {
        PDLights.Push (pdl);
      }
    }

    for (size_t i = 0; i < vdata.positions.GetSize (); ++i)
    {
      csColor& c = litColors->Get (i);
      const csVector3& normal = ComputeVertexNormal (obj, i);
#ifdef DUMP_NORMALS
      const csVector3 normalBiased = normal*0.5f + csVector3 (0.5f);
      c = csColor (normalBiased.x, normalBiased.y, normalBiased.z);
#else
      const csVector3& pos = vdata.positions[i];
      c.Set(0, 0, 0);
      for(size_t j=0; j<component.size(); j++)
      {
        csColor value =
          component[j]->ComputePointLightingComponent(sector, obj, pos,
                              normal, masterSampler);

        if(!value.IsBlack())
        {
          c += componentCoefficient[i] * value + componentOffset[i];
        }
      }

      // Shade PD lights
      for (size_t pdli = 0; pdli < PDLights.GetSize (); ++pdli)
      {
        Light* pdl = PDLights[pdli];
        Object::LitColorArray* pdlColors = obj->GetLitColorsPD (pdl, subLightmapNum);
        csColor& c = pdlColors->Get (i);
        for(size_t j=0; j<component.size(); j++)
        {
          csColor value =
            component[j]->ComputePointLightingComponent(sector, obj, pos,
                                normal, masterSampler, pdl);

          if(!value.IsBlack())
          {
            c += componentCoefficient[i] * value + componentOffset[i];
          }
        }
      }
#endif
      progress.Advance ();
    }
  }

  void LightCalculator::ComputeAffectingLights (Object* obj)
  {
    Sector* sector = obj->GetSector();
    
    for (size_t i = 0; i < sector->allNonPDLights.GetSize(); i++)
    {
      Light* light = sector->allNonPDLights[i];
      bool effect = light->GetBoundingSphere().TestIntersect (obj->GetBoundingSphere());
      for(size_t j=0; j<component.size(); j++)
        component[j]->setAffectingLight( i, effect );
    }
  }

  csVector3 LightCalculator::ComputeVertexNormal (Object* obj, 
                                                 size_t index) const
  {
    if (fancyTangentSpaceNorm)
    {
      csMatrix3 ts = obj->GetTangentSpace (index);
      csVector3 v = ts * tangentSpaceNorm;
      v.Normalize();
      return v;
    }
    else
      return obj->GetVertexData().normals[index];
  }
};