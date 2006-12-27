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
#include "kdtree.h"

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


  //--
  VisibilityTester::VisibilityTester ()
  {
  }

  void VisibilityTester::SetSegment (const csVector3& start, const csVector3& end)
  {
    csVector3 dir = end-start;
    float d = dir.Norm ();

    SetSegment (start, dir / d, d);
  }

  void VisibilityTester::SetSegment (const csVector3& start, const csVector3& dir,
    float maxL)
  {
    testRay.origin = start;
    testRay.direction = dir;
    //testRay.minLength = FLT_EPSILON;
    //testRay.maxLength = maxL * (1.0f - FLT_EPSILON);
    testRay.maxLength = maxL - FLT_EPSILON*10.0f;
    testRay.ignoreFlags = KDPRIM_FLAG_NOSHADOW; // Ignore primitives that don't cast shadows
  }

  bool VisibilityTester::Unoccluded (Raytracer& rt, const Primitive* ignorePrim)
  {
    HitPoint hp;
    testRay.ignorePrimitive = ignorePrim;
    return !rt.TraceAnyHit (testRay, hp);
  }



  //--
  PointLight::PointLight ()
    : Light (true)
  {

  }

  PointLight::~PointLight ()
  {

  }

  csColor PointLight::SampleLight (const csVector3& point, const csVector3& n,
    float u1, float u2, csVector3& lightVec, float& pdf, VisibilityTester& vistest)
  {
    lightVec = position - point;
    float sqD = lightVec.SquaredNorm ();
    float d = sqrtf (sqD);
    lightVec /= d;

    pdf = 1;

    vistest.SetSegment (position, -lightVec, d);

    csColor res = color * ComputeAttenuation (sqD);

    return res;
  }

  csColor PointLight::GetPower () const
  {
    return color;
  }

  void PointLight::SetRadius (float r)
  {
    radius = r;
    //Update bb
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
