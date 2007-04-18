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
  static const float lightValueEpsilon = 2.0f/256.0f;

  Configuration::Configuration ()
  {
    //Setup defaults
    lighterProperties.doDirectLight = true;

    lmProperties.uTexelPerUnit = 2.0f;
    lmProperties.vTexelPerUnit = 2.0f;
    lmProperties.maxLightmapU = 1024;
    lmProperties.maxLightmapV = 1024;
    lmProperties.blackThreshold = lightValueEpsilon;

    diProperties.pointLightMultiplier = 1.0f;
    diProperties.areaLightMultiplier = 1.0f;
  }

  void Configuration::Initialize ()
  {
    csRef<iConfigManager> cfgMgr = globalLighter->configMgr;
    
    lighterProperties.doDirectLight = cfgMgr->GetBool ("lighter2.DirectLight", 
      lighterProperties.doDirectLight);


    lmProperties.uTexelPerUnit = cfgMgr->GetFloat ("lighter2.uTexelPerUnit", 
      lmProperties.uTexelPerUnit);
    lmProperties.vTexelPerUnit = cfgMgr->GetFloat ("lighter2.vTexelPerUnit", 
      lmProperties.vTexelPerUnit);

    lmProperties.maxLightmapU = cfgMgr->GetInt ("lighter2.maxLightmapU", 
      lmProperties.maxLightmapU);
    lmProperties.maxLightmapV = cfgMgr->GetInt ("lighter2.maxLightmapV", 
      lmProperties.maxLightmapV);
   

    lmProperties.blackThreshold = cfgMgr->GetInt ("lighter2.blackThreshold", 
      lmProperties.blackThreshold);
    lmProperties.blackThreshold = csMax (lmProperties.blackThreshold,
      lightValueEpsilon); // Values lower than the LM precision don't make sense
  }
}
