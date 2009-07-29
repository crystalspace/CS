/*
  Copyright (C) 2008 by Greg Hoffman
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

#include "common.h"
#include "indirectlight.h"
//#include "jensenphotonmap.h"

#include "scene.h"

namespace lighter
{
  IndirectLight::IndirectLight ()
  {
    // Cache global settings
    finalGather =           globalConfig.GetIndirectProperties() .finalGather;
    numFinalGatherRays  =   globalConfig.GetIndirectProperties() .numFinalGatherRays;
    searchRadius =          globalConfig.GetIndirectProperties() .sampleDistance;
    numPhotonsPerSector =   globalConfig.GetIndirectProperties() .numPhotons;
    numSamplesPerPhoton =   globalConfig.GetIndirectProperties() .numPerSample;

    // Initialize random number generators
    randVect.Initialize();
    randFloat.Initialize();
  }

  IndirectLight::~IndirectLight () {}

  csColor IndirectLight::ComputeElementLightingComponent(Sector* sector,
      ElementProxy element, SamplerSequence<2>& lightSampler,
      bool recordInfluence)
  {
    // Get the position, normal and containing object for this element
    csVector3 pos = element.primitive.ComputeElementCenter(element.element);
    csVector3 normal = element.primitive.ComputeNormal(pos);
    Object* obj = element.primitive.GetObject();

    // Now compute like we would a vertex
    return ComputePointLightingComponent(sector, obj, pos, normal, lightSampler);
  }

  csColor IndirectLight::ComputePointLightingComponent(Sector* sector, 
        Object* obj, const csVector3& point, const csVector3& normal, 
        SamplerSequence<2>& lightSampler)
  {
    // color to return
    csColor c;

    // check to see if we are supposed to do a final gather
    if (finalGather)
    {
      // average over the number of FG rays
      csColor final(0,0,0);
      for (size_t num = 0; num < (size_t)numFinalGatherRays; ++num)
      {
        lighter::HitPoint hit;
        hit.distance = FLT_MAX*0.9f;
        lighter::Ray ray;
        ray.type = RAY_TYPE_OTHER2;

        // create a new directional vector that faces towards the
        // direction of the normal
        csVector3 newDir = randVect.Get();
        if (newDir*normal < 0.0)
        {
          newDir = newDir*-1.0;
        }
        ray.direction = newDir;
        ray.origin = point;
        ray.minLength = 0.01f;

        // raytrace another ray
        if (lighter::Raytracer::TraceClosestHit(sector->kdTree, ray, hit))
        {
          if (hit.primitive)
          {
            csVector3 dirToSrc = csVector3(-ray.direction.x, 
              -ray.direction.y, -ray.direction.z);
            csVector3 hNorm = 
              hit.primitive->ComputeNormal(hit.hitPoint);

//            final += 
//              sector->photonMap->SampleColor(hit.hitPoint, 
//                    searchRadius, hNorm, dirToSrc);
          }
        }
        else
        {
//          final += sector->photonMap->SampleColor(point, searchRadius, normal);
        }
      }
      // average the color
      c = final * (1.0 / numFinalGatherRays);
    }
    else
    {
      c = sector->SamplePhoton(point, normal, searchRadius);
    }

    return c;
  }

  void IndirectLight::EmitPhotons(Sector *sect, Statistics::Progress& progress)
  {
    progress.SetProgress(0.0f);

    // Iterate through all the non 'Pseudo Dynamic' light sources
    const LightRefArray& allNonPDLights = sect->allNonPDLights;
    Statistics::ProgressState progressState(progress, numPhotonsPerSector);

    // Iterate over the lights to determine the total lumen power in the sector
    double sectorLumenPower = 0;
    for(size_t lightIdx = 0; lightIdx < allNonPDLights.GetSize(); ++lightIdx)
    {
      Light* curLight = allNonPDLights[lightIdx];
      csColor pow = curLight->GetColor()*curLight->GetPower();
      sectorLumenPower += (pow.red + pow.green + pow.blue)/3.0;
    }

    for (size_t lightIdx = 0; lightIdx < allNonPDLights.GetSize(); ++lightIdx)
    {
      // Get the position, color and power for this light source
      Light* curLight = allNonPDLights[lightIdx];
      const csVector3& pos = curLight->GetPosition();
      const csColor& color = curLight->GetColor();
      const csColor& power = curLight->GetPower();

      // Determine type of light source
      csLightType curLightType = CS_LIGHT_POINTLIGHT;
      if(dynamic_cast<SpotLight*>(curLight) != NULL) curLightType = CS_LIGHT_SPOTLIGHT;
      else if(dynamic_cast<DirectionalLight*>(curLight) != NULL) curLightType = CS_LIGHT_DIRECTIONAL;

      // How many photons does this light get (proportional to the fraction
      // of power this light contributes to the sector)?
      csColor pow = curLight->GetColor()*curLight->GetPower();
      double powerScale = ((pow.red + pow.green + pow.blue)/3.0)/sectorLumenPower;
      size_t photonsForCurLight = floor(powerScale*numPhotonsPerSector + 0.5);

      // Loop to generate the requested number of photons for this light source
      for (size_t num = 0; num < photonsForCurLight; ++num)
      {
        switch (curLightType) {

          // directional light
          case CS_LIGHT_DIRECTIONAL:
            {
              globalLighter->Report (
                  "Directional lights are ignored for indirect light calculation");
            }
            break;

          // spotlight
          case CS_LIGHT_SPOTLIGHT:
            {
              globalLighter->Report (
                  "Spotlights are ignored for indirect light calculation");
            }
            break;

          // Default behavior is to treat a light like a point light
          case CS_LIGHT_POINTLIGHT:
          default:
            {
              // Genrate a random direction vector for uniform light source sampling
              csVector3 dir = randVect.Get();

              // Emit a single photon into the sector containing this light
              const PhotonRay newPhoton = { pos, dir, color, powerScale*power, RAY_TYPE_OTHER1 };
              EmitPhoton(sect, newPhoton, 0);
            }
            break;
        }

        progressState.Advance();
      }
    }

    // Balance the PhotonMap KD-Tree
    sect->BalancePhotons();

    progress.SetProgress(1.0f);

    // Save the photon map if requested
    if(globalConfig.GetIndirectProperties ().savePhotonMap)
    {
      static int secCount = 0;
      char filename[30];
      sprintf(filename, "photonmap%d.dat", secCount);
      sect->SavePhotonMap(filename);
      secCount++;
    }
  }

  void IndirectLight::EmitPhoton(Sector* &sect, const PhotonRay &photon, const size_t &depth)
  {    
    // Check recursion depth
    if(globalConfig.GetIndirectProperties ().maxRecursionDepth > 0 &&
        depth > (size_t)globalConfig.GetIndirectProperties ().maxRecursionDepth)
    {
      return;
    }

    // Trace this photon using the standard raytracer
    lighter::HitPoint hit;
    hit.distance = FLT_MAX*0.9f;
    lighter::Ray ray = photon.getRay();    
    if (lighter::Raytracer::TraceClosestHit(sect->kdTree, ray, hit))
    {
      // TODO: Why would a hit be returned and 'hit.primitive' be NULL?
      if (!hit.primitive) { return; }

      // Compute reflection direction
      csVector3 normal = hit.primitive->ComputeNormal(hit.hitPoint);
      csVector3 dir = photon.direction;
      float dot = normal*dir;
      csVector3 reflDir = dir;
      reflDir -= normal*2*dot;

      // Compute reflected color
      csColor reflColor = photon.color*photon.power;

      // Record photon (initializing photon map object if needed)
      sect->AddPhoton(reflColor, hit.hitPoint, dir);

      // Russian roulette to cut off recursion depth early (but only if we are past first recursion level)
      float pd = (reflColor.red + reflColor.green + reflColor.blue) / 3.0;
      csRandomFloatGen randGen;
      float rand = randGen.Get();

      if(depth == 0 || rand <= pd)
      {
        // Determine color and power of scattered light
        // TODO: attenuate color and/or power by material properties
        csColor newColor = photon.color;
        csColor newPower = photon.power;
        
        // Generate a scattering direction in the hemisphere around the normal
        csVector3 scatterDir = DiffuseScatter(normal);

        // Emit a new Photon
        const PhotonRay newPhoton = { hit.hitPoint, scatterDir, newColor, newPower, RAY_TYPE_REFLECT };
        EmitPhoton(sect, newPhoton, depth+1);
      }
    }
  }

  csVector3 IndirectLight::DiffuseScatter(const csVector3 &n)
  {
    // Local, static random number generator
    static csRandomFloatGen randGen;

    // Get two uniformly distributed random numbers between 0 and 1
    double e1 = randGen.Get();
    double e2 = randGen.Get();

    // Compute the angles of rotation around the normal
    // Note: altitude is weighted by cosine just like Lambert's law
    double theta = acos(sqrt(e1));
    double phi = 2.0*PI*e2;

    // Find orthogonal axis (must avoid the dominant axis)
    csVector3 orthoN, rotAxis;
    switch(n.DominantAxis())
    {
      case CS_AXIS_X: orthoN.Set(0, 1, 0); break;
      case CS_AXIS_Y: orthoN.Set(0, 0, 1); break;
      case CS_AXIS_Z: orthoN.Set(1, 0, 0); break;
    }

    rotAxis.Cross(n, orthoN);
    rotAxis.Normalize();

    // Create a vector to hold the results and a
    // quaternion for rotations
    csVector3 result = n;
    csQuaternion rotater;

    // Rotate n theta radians around 'rotAxis'
    rotater.SetAxisAngle(rotAxis, theta);
    result = rotater.Rotate(result);

    // Rotate again, this time phi radians around 'n'
    rotater.SetAxisAngle(n, phi);
    result = rotater.Rotate(result);

    return result;
  }
}