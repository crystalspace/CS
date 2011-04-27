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

#ifndef __CS_CSGFX_SHADER_LIGHTSVCACHE_H__
#define __CS_CSGFX_SHADER_LIGHTSVCACHE_H__

#include "csextern.h"

#include "csutil/array.h"
#include "csutil/ref.h"
#include "iutil/strset.h"
#include "ivideo/shader/shader.h"

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
    /// Position (world space)
    lightPositionWorld,
    /// Transform (from light to world space)
    lightTransformWorld,
    /// Inverse transform (from world to light space)
    lightTransformWorldInverse,
    /// Attenuation vector
    lightAttenuation,
    /**
     * Attenuation texture
     * \deprecated Deprecated in 1.3.
     */
    lightAttenuationTex,
    /// Direction (world space)
    lightDirectionWorld,
    /// Spot inner falloff
    lightInnerFalloff,
    /// Spot outer falloff
    lightOuterFalloff,
    /// Light type (csLightType casted to int)
    lightType,
    /// Attenuation mode (csLightAttenuationMode casted to int)
    lightAttenuationMode,
    /// Shadow map projection matrix
    lightShadowMapProjection,
    /// Shadow map pixel sizes + dimensions
    lightShadowMapPixelSize,

    /// Number of properties
    _lightCount
  };

  /// Other generally useful shader variables that can be obtained
  enum DefaultSV
  {
    /// "light ambient"
    varAmbient = 0,
    /// "light count"
    varLightCount,
    
    /// Number of other variables
    _varCount
  };
private:
  csRef<iShaderVarStringSet> strings;
  CS::ShaderVarStringID defaultVars[_varCount];
  
  CS::ShaderVarStringID lightSVIdCache_unnumbered[_lightCount];
  
  void ClearDefVars ();
public:
  /**
   * Construct an instance.
   * \remarks The instance is only useable after a call to SetStrings().
   */
  csLightShaderVarCache () { ClearDefVars (); }
  
  /**
   * Set the string set to query the name identifiers from.
   * \remarks Changing the string set will clear the cached names.
   */
  void SetStrings (iShaderVarStringSet* strings);
  
  /**
   * Obtain the name for for a lighting-relevant shader variable.
   * A caching scheme is used to avoid having to re-request a name from the
   * given string set every time one is needed.
   * \param prop Light property for which a variable name is to be retrieved.
   * \return Name of the relevant variable, csInvalidStringID in case of an
   *   error.
   */
  CS::ShaderVarStringID GetLightSVId (LightProperty prop);
    
  /** 
   * Obtain the name for for a lighting-relevant shader variable whose name
   * does not depend on the number of a light.
   * A caching scheme is used to avoid having to re-request a name from the
   * given string set every time one is needed.
   * \param var Variable for which a name is to be retrieved.
   * \return Name of the relevant variable, csInvalidStringID in case of an
   *   error.
   */
  CS::ShaderVarStringID GetDefaultSVId (DefaultSV var);
};

#endif // __CS_CSGFX_SHADER_LIGHTSVCACHE_H__
