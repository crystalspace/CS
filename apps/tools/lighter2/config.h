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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "csutil/stringarray.h"

namespace lighter
{

  // Object holding global and part-local config
  class Configuration
  {
  public:
    Configuration ();

    // Initialize configuration
    void Initialize (iConfigFile* cfgFile = 0);
   
    // Settings of what to do
    struct LighterProperties
    {
      // Direct lighting from light sources
      bool doDirectLight;
      // HL2-style directional LMs
      bool directionalLMs;
      // Whether to generate maps containing light directions for specular
      bool specularDirectionMaps;
      // Indirect Illumination through photon mapping
      bool indirectLMs;
      // Number of threads to use for multicore parts
      uint numThreads;
      // Save buffers as binary
      bool saveBinaryBuffers;
      // Check for duplicate objects when loading map data.
      bool checkDupes;
    };

    // Lightmap and lightmap layout properties
    struct LightmapProperties
    {
      // Density in u and v direction (in lumels per world unit).
      float lmDensity;

      // Max lightmap sizes
      uint maxLightmapU, maxLightmapV;

      // Black threshold
      float blackThreshold;

      /* When the dot product of two normals is bigger than 1-tolerance
         they're considered as equal. */
      float normalsTolerance;

      // Whether to store PD light maps as grayscale maps.
      bool grayPDMaps;
    };
    
    // Terrain lighting properties
    struct TerrainProperties
    {
      // Max lightmap sizes
      uint maxLightmapU, maxLightmapV;
    };

    // Direct light (direct illumination) calculation settings
    struct DIProperties
    {
      // Light multiplier for point light sources. A point light in this context
      // is just the opposite to an area light (so it can be point, spot,)
      float pointLightMultiplier;

      // Light multiplier for area light sources.
      float areaLightMultiplier;

      // Lighting routine to use for elements in lightmap sampling
      enum
      {
        LM_LIGHT_ALL1,
        LM_LIGHT_ALL4,
        LM_LIGHT_RND1,
        LM_LIGHT_RND4
      } lmElementLighting;

      // Lighting routine to use for per vertex lighting sampling
      enum
      {
        PVL_LIGHT_ALL,
        PVL_LIGHT_RND1
      } pvlElementLighting;
      
    };

    // Indirect Light calculations and settings
    struct INDIProperties
    {
      // Number of photons to emit
      int numPhotons;
      // Number of photons per sample
      int numPerSample;
      // Maximum photon recursion depth
      int maxRecursionDepth;
      // Maximum number of neighbors to sample
      int maxNumNeighbors;
      // The sample distance for sampling photons
      float sampleDistance;
      // Flag for Final Gather
      bool finalGather;
      // Number of final gather rays
      int numFinalGatherRays;
      // Save photon map
      bool savePhotonMap;
    };

    struct DebugProperties
    {
      /* Regular expression for meshes for which to generate "debug occlusion"
         visualiuzation. */
      csString rayDebugRE;
    };


    // Public accessible (readable) properties
    const LighterProperties& GetLighterProperties () const
    {
      return lighterProperties;
    }

    const LightmapProperties& GetLMProperties () const
    {
      return lmProperties;
    }

    const TerrainProperties& GetTerrainProperties () const
    {
      return terrainProperties;
    }

    const DIProperties& GetDIProperties () const
    {
      return diProperties;
    }

    const DebugProperties& GetDebugProperties() const
    {
      return debugProperties;
    }
		
    const INDIProperties& GetIndirectProperties() const
    {
      return indtLightProperties;
    }

  protected:
    // Properties
    LighterProperties     lighterProperties;
    LightmapProperties    lmProperties;
    TerrainProperties     terrainProperties;
    DIProperties          diProperties;
    DebugProperties       debugProperties;
    INDIProperties        indtLightProperties;
  };

}

#endif
