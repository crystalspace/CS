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

#ifndef __CS_VERTEXLIGHT_H__
#define __CS_VERTEXLIGHT_H__

#include "csgeom/math.h"
#include "csgeom/vector3.h"
#include "iengine/movable.h"
#include "iengine/light.h"

// Attenuation functors

/**
 * No attenuation. 
 */
struct CS_CSGFX_EXPORT csNoAttenuation
{
  csNoAttenuation (iLight *light)
  {}

  CS_FORCEINLINE void operator() (float distance, float &dp) const
  {}
};

/**
 * Linear attenuation.
 * Out = in * (1 - distance/radius)
 */
struct CS_CSGFX_EXPORT csLinearAttenuation
{
  csLinearAttenuation (iLight *light)
  {
    invrad = 1/light->GetAttenuationConstants ().x;
  }

  CS_FORCEINLINE void operator() (float distance, float& dp) const
  {
    dp = dp * (1 - distance * invrad);
  }

  float invrad;
};

/**
 * Inverse linear attenuation.
 * Out = in * / distance
 */
struct CS_CSGFX_EXPORT csInverseAttenuation
{
  csInverseAttenuation (iLight *light)
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
struct CS_CSGFX_EXPORT csRealisticAttenuation
{
  csRealisticAttenuation (iLight *light)
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
struct CS_CSGFX_EXPORT csCLQAttenuation
{
  csCLQAttenuation (iLight *light)
    : attnVec (light->GetAttenuationConstants ())
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
  csPointLightProc (iLight *light, iMovable *objectMovable,
    float blackLimit = 0.0001f)
    : attn (light), nullColor (0.0f, 0.0f, 0.0f), blackLimit (blackLimit)
  {
    csReversibleTransform objT = objectMovable->GetFullTransform ();
    lightPos = objT.Other2This (light->GetMovable ()->GetFullPosition ());
    lightCol = light->GetColor ();
  }

  CS_FORCEINLINE
  csColor ProcessVertex (const csVector3 &v,const csVector3 &n) const
  {
    //compute gouraud shading..
    csVector3 direction = v-lightPos;
    float distance = csQsqrt(direction.SquaredNorm ());
    float dp = direction*n/distance;
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
  csDirectionalLightProc (iLight *light, iMovable *objectMovable,
    float blackLimit = 0.0001f)
    : attn (light), nullColor (0.0f, 0.0f, 0.0f), blackLimit (blackLimit)
  {
    csReversibleTransform objT = objectMovable->GetFullTransform ();
    csReversibleTransform lightT = light->GetMovable ()->GetFullTransform ();
    lightPos = objT.Other2This (lightT.GetOrigin ());
    lightDir = objT.Other2ThisRelative (lightT.This2OtherRelative (
      light->GetDirection ()));
    lightDir = lightDir.Unit ();
    lightCol = light->GetColor ();
  }

  CS_FORCEINLINE
  csColor ProcessVertex (const csVector3 &v,const csVector3 &n) const
  {
    //compute gouraud shading..
    float dp = lightDir*n;
    if (dp > blackLimit)
    {
      csVector3 direction = v-lightPos;
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
  csSpotLightProc (iLight *light, iMovable *objectMovable,
    float blackLimit = 0.0001f)
    : attn (light), nullColor (0.0f, 0.0f, 0.0f), blackLimit (blackLimit)
  {
    csReversibleTransform objT = objectMovable->GetFullTransform ();
    csReversibleTransform lightT = light->GetMovable ()->GetFullTransform ();
    lightPos = objT.Other2This (lightT.GetOrigin ());
    lightDir = objT.Other2ThisRelative (lightT.This2OtherRelative (
      light->GetDirection ()));
    lightDir = lightDir.Unit ();

    lightCol = light->GetColor ();
    light->GetSpotLightFalloff (falloffInner, falloffOuter);
  }

  CS_FORCEINLINE
  csColor ProcessVertex (const csVector3 &v,const csVector3 &n) const
  {
    csVector3 direction = (v-lightPos).Unit ();

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

template<class LightProc, class VertexType = csVector3,
  class ColorType = csColor>
class csVertexLightCalculator
{
public:
  csVertexLightCalculator ()
  {
  }

  void CalculateLighting (iLight *light, iMovable *objectMovable,
    size_t numvert, VertexType *vb, csVector3 *nb, ColorType *litColor) const
  {
    // setup the light calculator
    LightProc lighter (light, objectMovable);

    for (size_t n = 0; n < numvert; n++)
    {
      litColor[n] = lighter.ProcessVertex (vb[n], nb[n]);
    }
  }

  void CalculateLightingAdd (iLight *light, iMovable *objectMovable,
    size_t numvert, VertexType *vb, csVector3 *nb, ColorType *litColor) const
  {
    // setup the light calculator
    LightProc lighter (light, objectMovable);

    for (size_t n = 0; n < numvert; n++)
    {
      litColor[n] += lighter.ProcessVertex (vb[n], nb[n]);
    }
  }

  void CalculateLightingMul (iLight *light, iMovable *objectMovable,
    size_t numvert, VertexType *vb, csVector3 *nb, ColorType *litColor) const
  {
    // setup the light calculator
    LightProc lighter (light, objectMovable);

    for (size_t n = 0; n < numvert; n++)
    {
      litColor[n] *= lighter.ProcessVertex (vb[n], nb[n]);
    }
  }
};

#endif //__CS_VERTEXLIGHT_H__
