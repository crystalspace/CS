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
    lighterProperties.doDirectLight = true;
    lighterProperties.indirectLMs = false;
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
    indtLightProperties.numPerSample = 50;
    indtLightProperties.sampleDistance = 1.0f;

    indtLightProperties.finalGather = true;
    indtLightProperties.numFinalGatherRays = 30;
  }

  void Configuration::Initialize (iConfigFile* _cfgFile)
  {
    csRef<iConfigFile> cfgFile = _cfgFile;
    if (!cfgFile.IsValid())
      cfgFile = scfQueryInterface<iConfigFile> (globalLighter->configMgr);
    
    lighterProperties.doDirectLight = cfgFile->GetBool ("lighter2.DirectLight", 
      lighterProperties.doDirectLight);
    lighterProperties.directionalLMs = cfgFile->GetBool ("lighter2.BumpLMs", 
      lighterProperties.directionalLMs);
    lighterProperties.specularDirectionMaps = cfgFile->GetBool ("lighter2.SpecMaps", 
      true) && lighterProperties.directionalLMs;
    lighterProperties.indirectLMs = cfgFile->GetBool("lighter2.IndirectLight",
      lighterProperties.indirectLMs);
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
    indtLightProperties.numPerSample = cfgFile->GetInt("lighter2.photonsPerSample",
      indtLightProperties.numPerSample);
    indtLightProperties.sampleDistance = cfgFile->GetFloat("lighter2.sampleDistance",
      indtLightProperties.sampleDistance);

    indtLightProperties.finalGather = cfgFile->GetBool("lighter2.finalGather",
      indtLightProperties.finalGather);
    indtLightProperties.numFinalGatherRays = cfgFile->GetInt("lighter2.numFGRays",
      indtLightProperties.numFinalGatherRays);
  }
}
