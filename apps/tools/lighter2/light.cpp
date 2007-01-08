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
#include "scene.h"

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

  Light::Light (Sector* o, bool deltaDistribution)
    : ownerSector (o), lightFrustum (csVector3 (0,0,0)),
    deltaDistribution (deltaDistribution),  pseudoDynamic (false), realLight (true)
  {
    lightFrustum.MakeInfinite ();
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

  void VisibilityTester::AddSegment (KDTree* tree, const csVector3& start, const csVector3& end)
  {
    csVector3 dir = end-start;
    float d = dir.Norm ();

    AddSegment (tree, start, dir / d, d);
  }

  void VisibilityTester::AddSegment (KDTree* tree, const csVector3& start, const csVector3& dir,
    float maxL)
  {
    Segment s;

    s.ray.origin = start;
    s.ray.direction = dir;
    s.ray.maxLength = maxL - FLT_EPSILON*10.0f;
    s.ray.ignoreFlags = KDPRIM_FLAG_NOSHADOW; // Ignore primitives that don't cast shadows
    s.tree = tree;

    allSegments.Push (s);
  }

  bool VisibilityTester::Unoccluded (const Primitive* ignorePrim)
  {
    HitPoint hp;
    for (size_t i = 0; i < allSegments.GetSize (); ++i)
    {
      Segment& s = allSegments[i];
      s.ray.ignorePrimitive = ignorePrim;

      if (Raytracer::TraceAnyHit (s.tree, s.ray, hp))
        return false;
    }

    return true;    
  }

  bool VisibilityTester::Unoccluded (HitIgnoreCallback* ignoreCB)
  {
    HitPoint hp;
    for (size_t i = 0; i < allSegments.GetSize (); ++i)
    {
      Segment& s = allSegments[i];

      if (Raytracer::TraceAnyHit (s.tree, s.ray, hp, ignoreCB))
        return false;
    }

    return true;
  }

  void VisibilityTester::CollectHits (csSet<csConstPtrKey<Primitive> > &primitives, 
    HitIgnoreCallback* ignoreCB)
  {
    struct HitCB : public HitPointCallback
    {
      HitCB (csSet<csConstPtrKey<Primitive> > &primitives)
        : primitives (primitives)
      {
      }
      
      virtual void RegisterHit (const Ray &ray, const HitPoint &hit)
      {
        primitives.Add (hit.primitive);
      }

      csSet<csConstPtrKey<Primitive> > &primitives;
    } hitCB (primitives);

    for (size_t i = 0; i < allSegments.GetSize (); ++i)
    {
      Segment& s = allSegments[i];

      Raytracer::TraceAllHits (s.tree, s.ray, &hitCB, ignoreCB);
    }
  }


  //--
  PointLight::PointLight (Sector* o)
    : Light (o, true)
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

    vistest.AddSegment (ownerSector->kdTree, position, -lightVec, d);

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
    boundingBox.SetSize (csVector3 (r));
  }


  ProxyLight::ProxyLight (Sector* owner, Light* parentLight)
    : Light (owner, parentLight->IsDeltaLight ()), parent (parentLight)
  {
    realLight = false;

    SetPDLight (parent->IsPDLight ());
    SetColor (parent->GetColor ());
    SetLightID ((const char*)parent->GetLightID ().data);
  }

  ProxyLight::~ProxyLight ()
  {
  }

  csColor ProxyLight::SampleLight (const csVector3& point, const csVector3& n,
    float u1, float u2, csVector3& lightVec, float& pdf, VisibilityTester& vistest)
  {
    return parent->SampleLight (point, n, u1, u2, lightVec, pdf, vistest);
  }

  csColor ProxyLight::GetPower () const
  {
    return parent->GetPower ();
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
