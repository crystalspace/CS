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

#include "cssysdef.h"

#include "config.h"


namespace lighter
{

  Configuration globalConfig;

  Configuration::Configuration ()
  {
    //Setup defaults
    lighterProperties.doDirectLight = true;
    lighterProperties.doRadiosity = false;

    lmProperties.uTexelPerUnit = 16.0f;
    lmProperties.vTexelPerUnit = 16.0f;
    lmProperties.maxLightmapU = 1024;
    lmProperties.maxLightmapV = 1024;

    diProperties.pointLightMultiplier = 1.0f;
    diProperties.areaLightMultiplier = 1.0f;

    radProperties.uPatchResolution = 4;
    radProperties.vPatchResolution = 4;
  }

  void Configuration::Initialize (const csStringArray& files)
  {
    
  }
}
