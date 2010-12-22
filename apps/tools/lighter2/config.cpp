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

#include "config.h"
#include "lighter.h"

namespace lighter
{
  Configuration globalConfig;

  // @@@ Depends on lightmap precision and scale
  static const float lightValueEpsilon = 2.0f/255.0f;

  Configuration::Configuration ()
  {
    //Setup defaults
    lighterProperties.directLightEngine = LIGHT_ENGINE_RAYTRACER;
    lighterProperties.indirectLightEngine = LIGHT_ENGINE_NONE;
    lighterProperties.globalAmbient = true;
    lighterProperties.forceRealistic = false;
    lighterProperties.lightPowerScale = 1.0;
    lighterProperties.PMLightScale = 1.0;
    lighterProperties.directionalLMs = false;
    lighterProperties.specularDirectionMaps = false;
    lighterProperties.numThreads = CS::Platform::GetProcessorCount();
    lighterProperties.saveBinaryBuffers = true;
    lighterProperties.checkDupes = true;

    lmProperties.lmDensity = 4.0f;
    lmProperties.maxLightmapU = 1024;
    lmProperties.maxLightmapV = 1024;
    lmProperties.blackThreshold = lightValueEpsilon;
    lmProperties.normalsTolerance = 1.0f * (PI / 180.0f);
    lmProperties.grayPDMaps = true;
    
    terrainProperties.maxLightmapU = lmProperties.maxLightmapU;
    terrainProperties.maxLightmapV = lmProperties.maxLightmapV;

    diProperties.pointLightMultiplier = 1.0f;
    diProperties.areaLightMultiplier = 1.0f;
		
    indtLightProperties.numPhotons = 500;
    indtLightProperties.caustics = false;
    indtLightProperties.numCausticPhotons = 0;
    indtLightProperties.maxRecursionDepth = 10;
    indtLightProperties.maxDensitySamples = 50;
    indtLightProperties.sampleDistance = 1.0f;

    indtLightProperties.finalGather = true;
    indtLightProperties.numFinalGatherRays = 30;

    indtLightProperties.savePhotonMap = false;
  }

  void Configuration::Initialize (iConfigFile* _cfgFile)
  {
    csRef<iConfigFile> cfgFile = _cfgFile;
    if (!cfgFile.IsValid())
      cfgFile = scfQueryInterface<iConfigFile> (globalLighter->configMgr);
    
    const char* DLEngineStr = cfgFile->GetStr ("lighter2.DirectLight", 
      "raytracer");

    if(strcmp(DLEngineStr, "none") == 0)
      lighterProperties.directLightEngine = LIGHT_ENGINE_NONE;
    else if(strcmp(DLEngineStr, "raytracer") == 0)
      lighterProperties.directLightEngine = LIGHT_ENGINE_RAYTRACER;
    else if(strcmp(DLEngineStr, "photonmapper") == 0)
      lighterProperties.directLightEngine = LIGHT_ENGINE_PHOTONMAPPER;
    else
    {
      csPrintf("Error: Unknown direct light engine %s.\n"
               "       Options are %s, %s or %s.\n",
               CS::Quote::Single (DLEngineStr),
	       CS::Quote::Single ("none"),
	       CS::Quote::Single ("raytracer"),
	       CS::Quote::Single ("photonmapper"));
      exit(1);
    }

    const char* ILEngineStr = cfgFile->GetStr ("lighter2.IndirectLight", 
      "none");

    if(strcmp(ILEngineStr, "none") == 0)
      lighterProperties.indirectLightEngine = LIGHT_ENGINE_NONE;
    else if(strcmp(ILEngineStr, "photonmapper") == 0)
      lighterProperties.indirectLightEngine = LIGHT_ENGINE_PHOTONMAPPER;
    else
    {
      csPrintf("Error: Unknown indirect light engine %s.\n"
               "       Options are %s or %s.\n",
               CS::Quote::Single (ILEngineStr),
	       CS::Quote::Single ("none"),
	       CS::Quote::Single ("photonmapper"));
      exit(1);
    }

    lighterProperties.globalAmbient = cfgFile->GetBool ("lighter2.GlobalAmbient", 
      true);

    lighterProperties.forceRealistic = cfgFile->GetBool ("lighter2.forceRealistic", 
      false);

    lighterProperties.lightPowerScale = cfgFile->GetFloat ("lighter2.lightPowerScale", 
      lighterProperties.lightPowerScale);

    lighterProperties.PMLightScale = cfgFile->GetFloat ("lighter2.PMLightScale", 
      lighterProperties.PMLightScale);

    lighterProperties.directionalLMs = cfgFile->GetBool ("lighter2.BumpLMs", 
      lighterProperties.directionalLMs);
    lighterProperties.specularDirectionMaps = cfgFile->GetBool ("lighter2.SpecMaps", 
      true) && lighterProperties.directionalLMs;

    lighterProperties.numThreads = cfgFile->GetInt ("lighter2.NumThreads", 
      lighterProperties.numThreads);
    lighterProperties.checkDupes = cfgFile->GetBool ("lighter2.CheckDupes",
      lighterProperties.checkDupes);

    lmProperties.lmDensity = cfgFile->GetFloat ("lighter2.lmDensity", 
      lmProperties.lmDensity);
    lmProperties.maxLightmapU = cfgFile->GetInt ("lighter2.maxLightmapU", 
      lmProperties.maxLightmapU);
    lmProperties.maxLightmapV = cfgFile->GetInt ("lighter2.maxLightmapV", 
      lmProperties.maxLightmapV);
    lighterProperties.saveBinaryBuffers = cfgFile->GetBool ("lighter2.binary",
      lighterProperties.saveBinaryBuffers);
   
    lmProperties.blackThreshold = cfgFile->GetFloat ("lighter2.blackThreshold", 
      lmProperties.blackThreshold);
    lmProperties.blackThreshold = csMax (lmProperties.blackThreshold,
      lightValueEpsilon); // Values lower than the LM precision don't make sense
      
    terrainProperties.maxLightmapU = cfgFile->GetInt ("lighter2.maxTerrainLightmapU", 
      lmProperties.maxLightmapU);
    terrainProperties.maxLightmapV = cfgFile->GetInt ("lighter2.maxTerrainLightmapV", 
      lmProperties.maxLightmapV);

    float normalsToleranceAngle = cfgFile->GetFloat ("lighter2.normalsTolerance", 
      1.0f);
    lmProperties.normalsTolerance = csMax (EPSILON, normalsToleranceAngle * 
      (PI / 180.0f));

    lmProperties.grayPDMaps = cfgFile->GetBool ("lighter2.grayPDMaps", 
      lmProperties.grayPDMaps);

    debugProperties.rayDebugRE =
      cfgFile->GetStr ("lighter2.debugOcclusionRays");
		
    indtLightProperties.numPhotons = cfgFile->GetInt("lighter2.numPhotons",
      indtLightProperties.numPhotons);
    indtLightProperties.numCausticPhotons = cfgFile->GetInt("lighter2.numCausticPhotons",
      indtLightProperties.numCausticPhotons);
    indtLightProperties.caustics = cfgFile->GetBool("lighter2.caustics",
      indtLightProperties.caustics);
    indtLightProperties.maxRecursionDepth = cfgFile->GetInt("lighter2.maxRecursionDepth",
      indtLightProperties.maxRecursionDepth);
    indtLightProperties.maxDensitySamples = cfgFile->GetInt("lighter2.maxDensitySamples",
      indtLightProperties.maxDensitySamples);
    indtLightProperties.sampleDistance = cfgFile->GetFloat("lighter2.sampleDistance",
      indtLightProperties.sampleDistance);

    indtLightProperties.finalGather = cfgFile->GetBool("lighter2.finalGather",
      indtLightProperties.finalGather);
    indtLightProperties.numFinalGatherRays = cfgFile->GetInt("lighter2.numFGRays",
      indtLightProperties.numFinalGatherRays);

    indtLightProperties.savePhotonMap = cfgFile->GetBool("lighter2.savePhotonMap",
      indtLightProperties.savePhotonMap);
  }
}
