/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005 by Frank Richter

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

#include "csgfx/lightsvcache.h"

void csLightShaderVarCache::ClearDefVars ()
{
  for (size_t n = 0; n < _varCount; n++) defaultVars[n] = csInvalidStringID;
}

void csLightShaderVarCache::SetStrings (iStringSet* strings)
{
  lightSVIdCache.DeleteAll ();
  ClearDefVars();
  this->strings = strings;
}
  
csStringID csLightShaderVarCache::GetLightSVId (size_t num, LightProperty prop)
{
  static const char* svSuffixes[_lightCount] = {
    "diffuse",
    "specular",
    "position object",
    "position",
    "position world",
    "attenuation",
    "attenuationtex",
    "direction object",
    "direction",
    "direction world",
    "inner falloff",
    "outer falloff",
    "type",
    "attenuation mode"
  };

  if (!strings.IsValid()) return csInvalidStringID;
  
  if (num >= lightSVIdCache.GetSize())
  {
    csString str;
    for (size_t n = lightSVIdCache.GetSize(); n <= num; n++)
    {
      for (int p = 0; p < _lightCount; p++)
      {
	CS_ASSERT_MSG (
	  "You added stuff to csLightShaderVarCache::LightProperty "
	  "but didn't update " __FILE__,
	  svSuffixes[p] != 0);
	str.Format ("light %zu %s", n, svSuffixes[p]);
	lightSVIdCache.GetExtend (num).ids[p] = strings->Request (str);
      }
    }
  }
  return lightSVIdCache[num].ids[prop];
}

csStringID csLightShaderVarCache::GetDefaultSVId (DefaultSV var)
{
  static const char* svNames[_varCount] = {
    "light ambient",
    "light count"
  };
  
  if (!strings.IsValid()) return csInvalidStringID;

  if (defaultVars[var] == csInvalidStringID)
  {
    CS_ASSERT_MSG (
      "You added stuff to csLightShaderVarCache::DefaultSV "
      "but didn't update " __FILE__,
      defaultVars[var] != 0);
    defaultVars[var] = strings->Request (svNames[var]);
  }
  return defaultVars[var];
}
