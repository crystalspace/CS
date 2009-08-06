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

#include "scene.h"

namespace lighter
{
  PhotonmapperLighting::PhotonmapperLighting ()
  {
    // Cache global photon map settings
    searchRadius =         globalConfig.GetIndirectProperties() .sampleDistance;
    numPhotonsPerSector =  globalConfig.GetIndirectProperties() .numPhotons;
    numSamplesForDensity = globalConfig.GetIndirectProperties() .maxDensitySamples;

    // Cache global final gather settings
    finalGather =             globalConfig.GetIndirectProperties() .finalGather;
    numFinalGatherMSubdivs  = globalConfig.GetIndirectProperties() .numFinalGatherRays;
    numFinalGatherNSubdivs  = globalConfig.GetIndirectProperties() .numFinalGatherRays;

    // Determine which bounces should be stored
    directLightEnabled = 
      (globalConfig.GetLighterProperties ().directLightEngine ==
          LIGHT_ENGINE_PHOTONMAPPER);

    indirectLightEnabled = 
      (globalConfig.GetLighterProperties ().indirectLightEngine ==
          LIGHT_ENGINE_PHOTONMAPPER);

    // Initialize random number generators
    randVect.Initialize();
    randFloat.Initialize();
  }

  PhotonmapperLighting::~PhotonmapperLighting () {}

  void PhotonmapperLighting::SetPhotonStorage(bool storeDirectPhotons,
          bool storeIndirectPhotons)
  {
    directLightEnabled = storeDirectPhotons;
    indirectLightEnabled = storeIndirectPhotons;
  }

  csColor PhotonmapperLighting::ComputeElementLightingComponent(Sector* sector,
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

  csColor PhotonmapperLighting::ComputePointLightingComponent(Sector* sector, 
        Object* obj, const csVector3& point, const csVector3& normal, 
        SamplerSequence<2>& lightSampler)
  {
    // Color to return
    csColor c(0.0f, 0.0f, 0.0f);

    // Are we doing a final gather ?
    if (finalGather)
    {
      // Accumulate the energy from the FG rays in 'final'
      csColor final(0.0f, 0.0f, 0.0f);

      // Loop through an M by N grid of sample rays
      for (size_t j = 1; j <= numFinalGatherMSubdivs; j++)
      {
        for (size_t i = 1; i <= numFinalGatherNSubdivs; i++)
        {
          // Use stratified sampling to sample the hemisphere above our point
          csVector3 sampleDir = StratifiedSample(normal, i, j,
                numFinalGatherMSubdivs, numFinalGatherNSubdivs);

          // Build a hit and ray structure to use for Final Gather Rays
          lighter::HitPoint hit;
          hit.distance = FLT_MAX*0.9f;

          lighter::Ray ray;
          ray.type = RAY_TYPE_OTHER2;   // Special type for Final Gather rays
          ray.direction = sampleDir;
          ray.origin = point;
          ray.minLength = 0.01f;

          // Trace the final gather ray
          if (lighter::Raytracer::TraceClosestHit(sector->kdTree, ray, hit)
              && hit.primitive)
          {
            // Compute the direction to the source point
            csVector3 dirToSource = point - hit.hitPoint;
            dirToSource.Normalize();

            // Calculate the normal at the hit point
            csVector3 hNorm = hit.primitive->ComputeNormal(hit.hitPoint);

            // Make sure normal is facing towards source point
            if(dirToSource*hNorm < 0.0) hNorm -= hNorm;

            // Sample the photon map at the hit point and accumulate the energy
            final += sector->SamplePhoton(hit.hitPoint, hNorm, searchRadius);
          }
          //else
          //{
          //  final += sector->SamplePhoton(point, normal, searchRadius);
          //}
        }
      }

      // Normalize the accumulated energy
      c = final * (PI / (numFinalGatherMSubdivs*numFinalGatherNSubdivs));
    }
    else
    {
      c = sector->SamplePhoton(point, normal, searchRadius);
    }

    return c;
  }

  void PhotonmapperLighting::BalancePhotons(Sector *sect, Statistics::Progress& progress)
  {
    // Balance the PhotonMap KD-Trees
    progress.SetProgress(0.0f);
    Statistics::ProgressState progressState(progress, (int)(0.5*sect->GetPhotonCount()));

    sect->BalancePhotons(progressState);

    progress.SetProgress(0.0f);

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

  void PhotonmapperLighting::EmitPhotons(Sector *sect, Statistics::Progress& progress)
  {
    progress.SetProgress(0.0f);

    // Determine maximum allowed photon recursion
    size_t maxDepth = 0;
    if(indirectLightEnabled)
    {
      maxDepth =
        (size_t)globalConfig.GetIndirectProperties ().maxRecursionDepth;
    }

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
      bool warning = false, stop = false;
      for (size_t num = 0; num < photonsForCurLight && !stop; ++num)
      {
        // Get direction to emit the photon
        csVector3 dir;

        switch (curLightType) {

          // directional light
          case CS_LIGHT_DIRECTIONAL:
            {
              if(!warning)
              {
                globalLighter->Report (
                    "Directional lights are ignored for indirect light calculation");
                warning = true; stop = true; continue;
              }
            }
            break;

          // spotlight
          case CS_LIGHT_SPOTLIGHT:
            {
              if(!warning)
              {
                globalLighter->Report (
                  "Spotlight falloff is ignored for indirect light calculation");
                warning = true;
              }

              // Get spotlight properties
              float innerFalloff, outterFalloff;
              csVector3 lightDir = ((SpotLight*)curLight)->GetDirection();
              ((SpotLight*)curLight)->GetFalloff(innerFalloff, outterFalloff);

              // Generate a random direction within the spotlight cone
              dir = SpotlightDir(lightDir, outterFalloff);
            }
            break;

          // Default behavior is to treat a light like a point light
          case CS_LIGHT_POINTLIGHT:
          default:
            {
              // Genrate a random direction vector for uniform light source sampling
              dir = randVect.Get();
            }
            break;
        }

        // Emit a single photon into the sector containing this light
        const PhotonRay newPhoton = { pos, dir, color, power, RAY_TYPE_OTHER1 };
        EmitPhoton(sect, newPhoton, maxDepth, 0, !directLightEnabled);
        progressState.Advance();
      }

      // Scale the photons for this light
      sect->ScalePhotons(1.0/photonsForCurLight);
    }

    progress.SetProgress(1.0f);
  }

  void PhotonmapperLighting::EmitPhoton(Sector* &sect, const PhotonRay &photon,
    const size_t &maxDepth, const size_t &depth, const bool &ignoreDirect)
  {    
    // Check recursion depth
    if(depth > maxDepth)
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

      // Record photon based on enabled options
      if((depth == 0 && !ignoreDirect) || depth > 0)
      {
        sect->AddPhoton(reflColor, hit.hitPoint, dir);
      }

      // If indirect lighting is enabled, scatter the photon
      if(maxDepth > 0)
      {
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
          EmitPhoton(sect, newPhoton, maxDepth, depth+1, ignoreDirect);
        }
      }
    }
  }

  csVector3 PhotonmapperLighting::SpotlightDir(const csVector3 &dir, const float cosTheta)
  {
    // Local, static random number generator
    static csRandomFloatGen randGen;

    // Get two uniformly distributed random numbers between 0 and 1
    double e1 = randGen.Get();
    double e2 = randGen.Get();

    // Compute the angles of rotation around the optical axis
    double theta = acos(cosTheta)*e1;
    double phi = 2.0*PI*e2;

    return RotateAroundN(dir, theta, phi);
  }

  csVector3 PhotonmapperLighting::DiffuseScatter(const csVector3 &n)
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

    return RotateAroundN(n, theta, phi);
  }

  csVector3 PhotonmapperLighting::StratifiedSample(const csVector3 &n, const size_t i,
                                      const size_t j, const size_t M, const size_t N)
  {
    // Local, static random number generator
    static csRandomFloatGen randGen;

    // Get two uniformly distributed random numbers between 0 and 1
    double e1 = randGen.Get();
    double e2 = randGen.Get();

    // Generate rotation angles around n.  Here we are generating
    // jittered samples in a grid across the hemisphere weighted
    // by the cos function again (lambert's law).
    double theta = acos(sqrt((j-e1)/M));
    double phi = 2.0*PI*( (i-e2)/N );

    return RotateAroundN(n, theta, phi);
  }

  csVector3 PhotonmapperLighting::RotateAroundN(const csVector3 &n,
    const double theta, const double phi)
  {
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
