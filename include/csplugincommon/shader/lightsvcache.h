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

#ifndef __CS_CSPLUGINCOMMON_SHADER_LIGHTSVCACHE_H__
#define __CS_CSPLUGINCOMMON_SHADER_LIGHTSVCACHE_H__

#include "csextern.h"

#include "iutil/strset.h"

/**\file
 * Helper to cache names of shader variables relevant to lighting.
 */
 
/**
 * Helper to cache names of shader variables relevant to lighting.
 */
class CS_CRYSTALSPACE_EXPORT csLightShaderVarCache
{
public:
  /// Properties of lights for which shader variables are known
  enum LightProperty
  {
    /// Diffuse color
    lightDiffuse = 0,
    /// Specular color
    lightSpecular,
    /// Position (object space)
    lightPosition,
    /// Position (world space)
    lightPositionWorld,
    /// Attenuation vector
    lightAttenuation,
    /// Attenuation texture
    lightAttenuationTex,

    /// Number of properties
    lightCount
  };

  /// Other generally useful shader variables that can be obtained
  enum DefaultSV
  {
    /// "light ambient"
    varAmbient = 0,
    /// "light count"
    varLightCount,
    
    /// Number of other variables
    varCount
  };
private:
  struct LightSvIdCacheEntry
  {
    csStringID ids[lightCount];
  };
  csArray<LightSvIdCacheEntry> lightSVIdCache;
  csRef<iStringSet> strings;
  csStringID defaultVars[varCount];
  
  void ClearDefVars ();
public:
  /**
   * Construct an instance.
   * \remarks The instance is only useable after a call to SetStrings().
   */
  csLightShaderVarCache () { ClearDefVars(); }
  
  /**
   * Set the string set to query the name identifiers from.
   * \remarks Changing the string set will clear the cached names.
   */
  void SetStrings (iStringSet* strings);
  
  /**
   * Obtain the name for for a lighting-relevant shader variable whose name
   * also depends on the number of a light.
   * A caching scheme is used to avoid having to re-request a name from the
   * given string set every time one is needed.
   * \param num Number of the light that appears in the name
   * \param prop Light property for which a variable name is to be retrieved.
   * \return Name of the relevant variable, csInvalidStringID in case of an
   *   error.
   */
  csStringID GetLightSVId (size_t num, LightProperty prop);
    
  /** 
   * Obtain the name for for a lighting-relevant shader variable whose name
   * does not depend on the number of a light.
   * A caching scheme is used to avoid having to re-request a name from the
   * given string set every time one is needed.
   * \param var Variable for which a name is to be retrieved.
   * \return Name of the relevant variable, csInvalidStringID in case of an
   *   error.
   */
  csStringID GetDefaultSVId (DefaultSV var);
};

#endif // __CS_CSPLUGINCOMMON_SHADER_LIGHTSVCACHE_H__
