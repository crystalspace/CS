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
    void Initialize ();
   
    // Settings of what to do
    struct LighterProperties
    {
      // Direct lighting from light sources
      bool doDirectLight;
    };

    // Lightmap and lightmap layout properties
    struct LightmapProperties
    {
      // Density in u and v direction. u = uTexelPerUnit*x etc.. 
      float uTexelPerUnit, vTexelPerUnit;

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


    // Public accessible (readable) properties
    const LighterProperties& GetLighterProperties () const
    {
      return lighterProperties;
    }

    const LightmapProperties& GetLMProperties () const
    {
      return lmProperties;
    }

    const DIProperties& GetDIProperties () const
    {
      return diProperties;
    }

  protected:
    // Properties
    LighterProperties     lighterProperties;
    LightmapProperties    lmProperties;
    DIProperties          diProperties;
  };

}

#endif
