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


#include "crystalspace.h"

#include "common.h"
#include "light.h"
#include "raytracer.h"

// Attenuation functions
static float LightAttnNone (float, const csVector3&);
static float LightAttnLinear (float, const csVector3&);
static float LightAttnInverse (float, const csVector3&);
static float LightAttnRealistic (float, const csVector3&);
static float LightAttnCLQ (float, const csVector3&);

static lighter::LightAttenuationFunc attnFuncTable[] = 
{
  LightAttnNone,
  LightAttnLinear,
  LightAttnInverse,
  LightAttnRealistic,
  LightAttnCLQ
};


namespace lighter
{

  Light::Light (bool deltaDistribution)
    : deltaDistribution (deltaDistribution)
  {
  }

  Light::~Light ()
  {

  }

  void Light::SetAttenuation (csLightAttenuationMode mode, const csVector3& constants)
  {
    attenuationMode = mode;
    attenuationConsts = constants;
    attenuationFunc = attnFuncTable[mode];
  }


  bool VisibilityTester::Unoccluded (Raytracer& rt)
  {
    return true;
  }

}


// Attenuation functions
static float LightAttnNone (float, const csVector3&)
{
  /// no attenuation: *1
  return 1;
}

static float LightAttnLinear (float squaredDistance, const csVector3& c)
{
  /// linear attenuation:  * (1 - distance / radius)
  return csMax (0.0f, 1.0f - (sqrtf (squaredDistance) / c.x));
}

static float LightAttnInverse (float squaredDistance, const csVector3&)
{
  /// inverse attenuation:  * 1 / distance
  return 1.0f / sqrtf(squaredDistance);
}

static float LightAttnRealistic (float squaredDistance, const csVector3&)
{
  /// realistic attenuation: * 1 / distance^2
  return 1.0f / squaredDistance;
}

static float LightAttnCLQ (float squaredDistance, const csVector3& c)
{
  /** 
   * CLQ, Constant Linear Quadratic: 
   * * 1 / (constant1 + constant2*distance + constant3*distance^2)
   */
  return c * csVector3 (1, sqrtf(squaredDistance), squaredDistance);
}