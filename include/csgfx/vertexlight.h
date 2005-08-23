/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#ifndef __CS_CSGFX_VERTEXLIGHT_H__
#define __CS_CSGFX_VERTEXLIGHT_H__

#include "csqsqrt.h"
#include "csgeom/math.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "csgfx/lightsvcache.h"
#include "csgfx/vertexlistwalker.h"
#include "csutil/cscolor.h"

#include "iengine/light.h"
#include "iengine/movable.h"
#include "ivideo/shader/shader.h"

/**\file 
 * Attenuation functors
 */

/**
 * Light properties, as needed by the attenuation and lighting functors.
 */
struct CS_CRYSTALSPACE_EXPORT csLightProperties
{
  /// Attenuation coefficients (for CLQ attenuation)
  csVector3 attenuationConsts;
  /// Light position (object space)
  csVector3 posObject;
  /// Light direction (object space)
  csVector3 dirObject;
  /// Light diffuse color
  csColor color;
  /// Spotlight inner falloff
  float spotFalloffInner;
  /// Spotlight outer falloff
  float spotFalloffOuter;
  /// Light type
  csLightType type;
  /// Light attenuation mode
  csLightAttenuationMode attenuationMode;

  csLightProperties () : spotFalloffInner(0.0f), spotFalloffOuter(0.0f),
    type(CS_LIGHT_POINTLIGHT) {}
  /**
   * Convenience constructor to fill the structure from a set of shader
   * variables.
   */
  csLightProperties (size_t lightNum, csLightShaderVarCache& svcache,
    const csShaderVarStack &stacks)
  {
    csStringID id;

    id = svcache.GetLightSVId (lightNum, 
      csLightShaderVarCache::lightAttenuation);
    if ((stacks.Length() > id) && (stacks[id] != 0))
      stacks[id]->GetValue (attenuationConsts);

    id = svcache.GetLightSVId (lightNum, 
      csLightShaderVarCache::lightPosition);
    if ((stacks.Length() > id) && (stacks[id] != 0))
      stacks[id]->GetValue (posObject);

    id = svcache.GetLightSVId (lightNum, 
      csLightShaderVarCache::lightDirection);
    if ((stacks.Length() > id) && (stacks[id] != 0))
      stacks[id]->GetValue (dirObject);

    id = svcache.GetLightSVId (lightNum, 
      csLightShaderVarCache::lightDiffuse);
    if ((stacks.Length() > id) && (stacks[id] != 0))
      stacks[id]->GetValue (color);

    id = svcache.GetLightSVId (lightNum, 
      csLightShaderVarCache::lightInnerFalloff);
    if ((stacks.Length() > id) && (stacks[id] != 0))
      stacks[id]->GetValue (spotFalloffInner);

    id = svcache.GetLightSVId (lightNum, 
      csLightShaderVarCache::lightOuterFalloff);
    if ((stacks.Length() > id) && (stacks[id] != 0))
      stacks[id]->GetValue (spotFalloffOuter);

    int t = CS_LIGHT_POINTLIGHT;
    id = svcache.GetLightSVId (lightNum, 
      csLightShaderVarCache::lightType);
    if ((stacks.Length() > id) && (stacks[id] != 0))
      stacks[id]->GetValue (t);
    type = (csLightType)t;

    t = CS_ATTN_NONE;
    id = svcache.GetLightSVId (lightNum, 
      csLightShaderVarCache::lightAttenuationMode);
    if ((stacks.Length() > id) && (stacks[id] != 0))
      stacks[id]->GetValue (t);
    attenuationMode = (csLightAttenuationMode)t;
  }
};

/**
 * No attenuation. 
 */
struct CS_CRYSTALSPACE_EXPORT csNoAttenuation
{
  csNoAttenuation (const csLightProperties& /*light*/)
  {}

  CS_FORCEINLINE void operator() (float /*distance*/, float & /*dp*/) const
  {}
};

/**
 * Linear attenuation.
 * Out = in * (1 - distance/radius)
 */
struct CS_CRYSTALSPACE_EXPORT csLinearAttenuation
{
  csLinearAttenuation (const csLightProperties& light)
  {
    invrad = 1/light.attenuationConsts.x;
  }

  CS_FORCEINLINE void operator() (float distance, float& dp) const
  {
    dp = csMax (dp * (1 - distance * invrad), 0.0f);
  }

  float invrad;
};

/**
 * Inverse linear attenuation.
 * Out = in * / distance
 */
struct CS_CRYSTALSPACE_EXPORT csInverseAttenuation
{
  csInverseAttenuation (const csLightProperties& /*light*/)
  {}

  CS_FORCEINLINE void operator() (float distance, float& dp) const
  {
    dp = dp / distance;
  }
};


/**
 * Inverse quadratic attenuation.
 * Out = in * / distance^2
 */
struct CS_CRYSTALSPACE_EXPORT csRealisticAttenuation
{
  csRealisticAttenuation (const csLightProperties& /*light*/)
  {}

  CS_FORCEINLINE void operator() (float distance, float& dp) const
  {
    dp = dp / (distance*distance);
  }
};

/**
 * Constant, Linear, Quadratic attenuation
 * Out = in /(const + distance*lin + distance^2*quad)
 */
struct CS_CRYSTALSPACE_EXPORT csCLQAttenuation
{
  csCLQAttenuation (const csLightProperties& light)
    : attnVec (light.attenuationConsts)
  {}

  CS_FORCEINLINE void operator() (float distance, float& dp) const
  {
    dp = dp/(csVector3 (1.0, distance, distance*distance)*attnVec);
  }

  csVector3 attnVec;
};


/**
 * Preform pointlight lighting calculation without shadowing.
 * Template parameters:
 *   AttenuationProc - Functor for attenuation
 */
template<class AttenuationProc>
class csPointLightProc
{
public:
  csPointLightProc (const csLightProperties& light, const csReversibleTransform &objT,
    float blackLimit = 0.0001f)
    : attn (light), nullColor (0.0f, 0.0f, 0.0f), blackLimit (blackLimit)
  {    
    lightPos = light.posObject;
    lightCol = light.color;
  }

  CS_FORCEINLINE
  csColor ProcessVertex (const csVector3 &v,const csVector3 &n) const
  {
    //compute gouraud shading..
    csVector3 direction = lightPos-v;
    float distance = csQsqrt(direction.SquaredNorm ());
    float dp = (direction*n)/distance;
    if (dp > blackLimit)
    {
      attn (distance, dp);
      return lightCol*dp;
    }
    return nullColor;
  }

private:
  AttenuationProc attn;
  csVector3 lightPos; //localspace
  csColor lightCol;
  csColor nullColor;
  float blackLimit;
};

/**
 * Preform directional light lighting calculation without shadowing.
 * Template parameters:
 *   AttenuationProc - Functor for attenuation
 */
template<class AttenuationProc>
class csDirectionalLightProc
{
public:
  csDirectionalLightProc (const csLightProperties& light, const csReversibleTransform &objT,
    float blackLimit = 0.0001f)
    : attn (light), nullColor (0.0f, 0.0f, 0.0f), blackLimit (blackLimit)
  {
    //csReversibleTransform lightT = light->GetMovable ()->GetFullTransform ();
    lightPos = light.posObject;//objT.Other2This (lightT.GetOrigin ());
    /*lightDir = objT.Other2ThisRelative (lightT.This2OtherRelative (
      light->GetDirection ()));
    lightDir = lightDir.Unit ();*/
    lightDir = light.dirObject;
    lightCol = light.color;
  }

  CS_FORCEINLINE
  csColor ProcessVertex (const csVector3 &v,const csVector3 &n) const
  {
    //compute gouraud shading..
    float dp = lightDir*n;
    if (dp > blackLimit)
    {
      csVector3 direction = lightPos-v;
      float distance = csQsqrt(direction.SquaredNorm ());
      attn (distance, dp);
      return lightCol*dp;
    }
    return nullColor;
  }

private:
  AttenuationProc attn;
  csVector3 lightPos; //localspace
  csVector3 lightDir; //localspace
  csColor lightCol;
  csColor nullColor;
  float blackLimit;
};

/**
* Preform spotlight lighting calculation without shadowing.
* Template parameters:
*   AttenuationProc - Functor for attenuation
*/
template<class AttenuationProc>
class csSpotLightProc
{
public:
  csSpotLightProc (const csLightProperties& light, const csReversibleTransform &objT,
    float blackLimit = 0.0001f)
    : attn (light), nullColor (0.0f, 0.0f, 0.0f), blackLimit (blackLimit)
  {
    //csReversibleTransform lightT = light->GetMovable ()->GetFullTransform ();
    lightPos = light.posObject;
    lightDir = light.dirObject;

    lightCol = light.color;
    falloffInner = light.spotFalloffInner;
    falloffOuter = light.spotFalloffOuter;
    //light->GetSpotLightFalloff (falloffInner, falloffOuter);
  }

  CS_FORCEINLINE
  csColor ProcessVertex (const csVector3 &v,const csVector3 &n) const
  {
    csVector3 direction = (lightPos-v).Unit ();

    //compute gouraud shading..
    float dp = direction*n;
    if (dp > blackLimit)
    {
      float cosfact =
	csSmoothStep (-(direction*lightDir), falloffOuter, falloffInner);
      float distance = csQsqrt(direction.SquaredNorm ());
      if (cosfact > 0)
      {
        attn (distance, dp);
        return lightCol*dp*cosfact;
      }
    }
    return nullColor;
  }

private:
  AttenuationProc attn;
  csVector3 lightPos; //localspace
  csVector3 lightDir; //localspace
  csColor lightCol;
  csColor nullColor;
  float blackLimit;
  float falloffInner, falloffOuter;
};

struct iVertexLightCalculator
{
public:
  virtual void CalculateLighting (const csLightProperties& light, 
    const csReversibleTransform &objtransform,
    size_t numvert, csVertexListWalker<csVector3> vb, 
    csVertexListWalker<csVector3> nb, csColor *litColor) const = 0;

  virtual void CalculateLightingAdd (const csLightProperties& light, 
    const csReversibleTransform &objtransform,
    size_t numvert, csVertexListWalker<csVector3> vb, 
    csVertexListWalker<csVector3> nb, csColor *litColor) const = 0;

  virtual void CalculateLightingMul (const csLightProperties& light, 
    const csReversibleTransform &objtransform,
    size_t numvert, csVertexListWalker<csVector3> vb, 
    csVertexListWalker<csVector3> nb, csColor *litColor) const = 0;
};

template<class LightProc>
class csVertexLightCalculator : public iVertexLightCalculator
{
public:
  csVertexLightCalculator ()
  {
  }

  virtual void CalculateLighting (const csLightProperties& light, 
    const csReversibleTransform &objtransform,
    size_t numvert, csVertexListWalker<csVector3> vb, 
    csVertexListWalker<csVector3> nb, csColor *litColor) const
  {
    // setup the light calculator
    LightProc lighter (light, objtransform);

    for (size_t n = 0; n < numvert; n++)
    {
      litColor[n] = lighter.ProcessVertex (vb[n], nb[n]);
    }
  }

  virtual void CalculateLightingAdd (const csLightProperties& light, 
    const csReversibleTransform &objtransform,
    size_t numvert, csVertexListWalker<csVector3> vb, 
    csVertexListWalker<csVector3> nb, csColor *litColor) const
  {
    // setup the light calculator
    LightProc lighter (light, objtransform);

    for (size_t n = 0; n < numvert; n++)
    {
      litColor[n] += lighter.ProcessVertex (vb[n], nb[n]);
    }
  }

  virtual void CalculateLightingMul (const csLightProperties& light, 
    const csReversibleTransform &objtransform,
    size_t numvert, csVertexListWalker<csVector3> vb, 
    csVertexListWalker<csVector3> nb, csColor *litColor) const
  {
    // setup the light calculator
    LightProc lighter (light, objtransform);

    for (size_t n = 0; n < numvert; n++)
    {
      litColor[n] *= lighter.ProcessVertex (vb[n], nb[n]);
    }
  }
};

#endif //__CS_VERTEXLIGHT_H__
