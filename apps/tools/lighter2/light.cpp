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

#include "light.h"
#include "raytracer.h"
#include "kdtree.h"
#include "scene.h"

// Attenuation functions
static float LightAttnNone (float, const csVector4&);
static float LightAttnLinear (float, const csVector4&);
static float LightAttnInverse (float, const csVector4&);
static float LightAttnRealistic (float, const csVector4&);
static float LightAttnCLQ (float, const csVector4&);

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

  void Light::SetAttenuation (csLightAttenuationMode mode, const csVector4& constants)
  {
    attenuationMode = mode;
    attenuationConsts = constants;
    attenuationFunc = attnFuncTable[mode];
  }


  //--
  size_t VisibilityTester::rayID;

  VisibilityTester::VisibilityTester (Light* light, Object* obj) : 
    light (light), obj (obj)
  {    
  }

  void VisibilityTester::AddSegment (KDTree* tree, const csVector3& start, 
    const csVector3& end)
  {
    csVector3 dir = end-start;
    float d = dir.Norm ();

    AddSegment (tree, start, dir / d, d);
  }

  void VisibilityTester::AddSegment (KDTree* tree, const csVector3& start, 
    const csVector3& dir, float maxL)
  {
    Segment s;

    s.ray.origin = start;
    s.ray.direction = dir;
    s.ray.minLength = FLT_EPSILON*10.0f;
    s.ray.maxLength = maxL - FLT_EPSILON*10.0f;
    s.ray.ignoreFlags = KDPRIM_FLAG_NOSHADOW; // Ignore primitives that don't cast shadows
    s.ray.rayID = ++rayID; // Give unique IDs to rays; needed for the ray debugger.
    s.tree = tree;

    allSegments.Push (s);
  }

  VisibilityTester::OcclusionState VisibilityTester::Occlusion (
    const Object* ignoreObject, const Primitive* ignorePrim)
  {
    HitCallback hitcb (*this);
    size_t lastHitCount = transparentHits.GetSize();

    bool haveAnyHit = false;

    RaytraceProfiler prof(1, RAY_TYPE_SHADOW);

    // Start by testing if we have no hits or possibly hit a non-transparent one first
    if (globalLighter->rayDebug.IsEnabled())
    {
      // Use hit call back to register ray hits for debugging
      for (size_t i = 0; i < allSegments.GetSize (); ++i)
      {
	Segment& s = allSegments[i];
	s.ray.ignoreObject = ignoreObject;
	s.ray.ignorePrimitive = ignorePrim;
  s.ray.type = RAY_TYPE_IGNORE;
  
	if (Raytracer::TraceAnyHit (s.tree,s.ray, &hitcb))
	{
	  haveAnyHit = true;
	  if (transparentHits.GetSize() == lastHitCount)
	  {
	    // Non-transparent hit
	    return occlOccluded;
	  }
	}
      }
    }
    else
    {
      // No ray hit registration needed, so no callback needed either
      for (size_t i = 0; i < allSegments.GetSize (); ++i)
      {
	Segment& s = allSegments[i];
	s.ray.ignoreObject = ignoreObject;
	s.ray.ignorePrimitive = ignorePrim;
  s.ray.type = RAY_TYPE_IGNORE;

	HitPoint hit;
	if (Raytracer::TraceAnyHit (s.tree,s.ray, hit))
	{
	  haveAnyHit = true;
	  if (!(hit.kdFlags & KDPRIM_FLAG_TRANSPARENT))
	  {
	    // Non-transparent hit
	    return occlOccluded;
	  }
	}
      }
    }

    // Didn't hit anything, cannot be anything transparent either
    if (!haveAnyHit)
    {
      return occlUnoccluded;
    }

    for (size_t i = 0; i < allSegments.GetSize (); ++i)
    {
      Segment& s = allSegments[i];
      s.ray.ignoreObject = ignoreObject;
      s.ray.ignorePrimitive = ignorePrim;

      if (Raytracer::TraceAllHits (s.tree, s.ray, &hitcb))
      {
        if (transparentHits.GetSize() == 0) return occlOccluded;
      }

      if (transparentHits.GetSize() == lastHitCount)
        globalLighter->rayDebug.RegisterUnhit (light, obj, s.ray);
      lastHitCount = transparentHits.GetSize();
    }

    return (transparentHits.GetSize() != 0) ? occlPartial : occlUnoccluded;
  }

  VisibilityTester::OcclusionState VisibilityTester::Occlusion (
    const Object* ignoreObject, HitIgnoreCallback* ignoreCB)
  {
    HitCallback hitcb (*this);
    size_t lastHitCount = transparentHits.GetSize();

    RaytraceProfiler prof(1, RAY_TYPE_SHADOW);

    bool haveAnyHit = false;

    // Start by testing if we have no hits or possibly hit a non-transparent one first
    for (size_t i = 0; i < allSegments.GetSize (); ++i)
    {
      Segment& s = allSegments[i];
      s.ray.ignoreObject = ignoreObject;
      s.ray.type = RAY_TYPE_IGNORE;

      HitPoint hit;
      if (Raytracer::TraceAnyHit (s.tree,s.ray, hit, ignoreCB))
      {
        haveAnyHit = true;
        if (!(hit.kdFlags & KDPRIM_FLAG_TRANSPARENT))
        {
          // Non-transparent hit
          return occlOccluded;
        }
      }
    }

    // Didn't hit anything, cannot be anything transparent either
    if (!haveAnyHit)
    {
      return occlUnoccluded;
    }

    for (size_t i = 0; i < allSegments.GetSize (); ++i)
    {
      Segment& s = allSegments[i];
      s.ray.ignoreObject = ignoreObject;
      s.ray.type = RAY_TYPE_IGNORE;

      if (Raytracer::TraceAllHits (s.tree, s.ray, &hitcb, ignoreCB))
      {
        if (transparentHits.GetSize() == 0) return occlOccluded;
      }

      if (transparentHits.GetSize() == lastHitCount)
        globalLighter->rayDebug.RegisterUnhit (light, obj, s.ray);
      lastHitCount = transparentHits.GetSize();
    }

    return (transparentHits.GetSize() != 0) ? occlPartial : occlUnoccluded;
  }
    
  csColor VisibilityTester::GetFilterColor ()
  {
    csColor c (1, 1, 1);
    transparentHits.Sort (); //@@TODO: Consider hits from different segments...
    for (size_t i = transparentHits.GetSize(); i-- > 0; )
    {
      const HitPoint& hit = transparentHits[i];
      csVector2 uv = hit.primitive->ComputeUV (hit.hitPoint);
      const RadMaterial* mat = hit.primitive->GetMaterial ();
      if (mat == 0) continue;
      if (!mat->filterImage.IsValid()) continue;
      ScopedSwapLock<MaterialImage<csColor> > (*(mat->filterImage));
      c *= mat->filterImage->GetInterpolated (uv);
    }
    return c;
  }
  
  //--
  PointLight::PointLight (Sector* o)
    : Light (o, true)
  {}

  PointLight::~PointLight ()
  {}

  csColor PointLight::SampleLight (const csVector3& point, const csVector3& n,
    float u1, float u2, csVector3& lightVec, float& pdf, VisibilityTester& vistest,
    const csPlane3* visLimitPlane)
  {
    csSegment3 visSegment (position, point);

    lightVec = position - point;
    float sqD = lightVec.SquaredNorm ();
    float d = sqrtf (sqD);
    lightVec /= d;

    pdf = 1;

    if (visLimitPlane)
      csIntersect3::SegmentPlane (*visLimitPlane, visSegment);

    vistest.AddSegment (ownerSector->kdTree, visSegment.Start (), visSegment.End ());

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
    //Update bs
    boundingSphere.SetRadius (r);
  }




  DirectionalLight::DirectionalLight (Sector* o)
    : Light (o, true), radius (0.f), length (0.f) // initalize to zero to avoid bogus bounding sphere sizes
  {}

  DirectionalLight::~DirectionalLight ()
  {}

  csColor DirectionalLight::SampleLight (const csVector3& point, const csVector3& n,
    float u1, float u2, csVector3& lightVec, float& pdf, VisibilityTester& vistest,
    const csPlane3* visLimitPlane)
  {
    lightVec = position - point;

    float dot = -dir * lightVec;

    // project the point onto the light plane
    csVector3 P = point - dir * dot;
    // uncomment this define to have attenuation calculated based on the distance from point
    // to light plane, as opposed to from point to light centre
//#define ATTENUATION_POINT_P
#ifdef ATTENUATION_POINT_P
    float sqD = dot * dot;
#else
    float sqD = lightVec.SquaredNorm ();
#endif // ATTENUATION_POINT_P
    lightVec = -dir;

    pdf = 1;

    csSegment3 visSegment (P, point);

    if (visLimitPlane)
      csIntersect3::SegmentPlane (*visLimitPlane, visSegment);

    vistest.AddSegment (ownerSector->kdTree, visSegment.Start (), visSegment.End ());

    csColor res = color * ComputeAttenuation (sqD);

    return res;
  }

  csColor DirectionalLight::GetPower () const
  {
    return color;
  }

  void DirectionalLight::SetRadius (float r)
  {
    radius = r;
    //Update bs
    boundingSphere.SetRadius (sqrtf (r * r + length * length));
  }

  void DirectionalLight::SetLength (float l)
  {
    length = l;
    //Update bs
    boundingSphere.SetRadius (sqrtf (radius * radius + l * l));
  }

  void DirectionalLight::SetDirection (csVector3 d)
  {
    dir = d;
  }




  SpotLight::SpotLight (Sector* o)
    : Light (o, true)
  {}

  SpotLight::~SpotLight ()
  {}

  csColor SpotLight::SampleLight (const csVector3& point, const csVector3& n,
    float u1, float u2, csVector3& lightVec, float& pdf, VisibilityTester& vistest,
    const csPlane3* visLimitPlane)
  {
    lightVec = position - point;
    float sqD = lightVec.SquaredNorm ();
    float d = sqrtf (sqD);
    lightVec /= d;
    float dot = -dir * lightVec; // negate because dir points away from the light

    // early out if we're out of the light cone
    if (dot < outer)
      return csColor (0.f);

    csSegment3 visSegment (position, point);

    pdf = 1;

    if (visLimitPlane)
      csIntersect3::SegmentPlane (*visLimitPlane, visSegment);

    vistest.AddSegment (ownerSector->kdTree, visSegment.Start (), visSegment.End ());

    float falloff;
    // early out if no edge softening
    if (fabs (outer - inner) < 0.0001)
      falloff = 1.f;
    else
      falloff = csSmoothStep(dot, inner, outer);
    csColor res = color * ComputeAttenuation (sqD) * falloff;

    return res;
  }

  csColor SpotLight::GetPower () const
  {
    return color;
  }

  void SpotLight::SetRadius (float r)
  {
    radius = r;
    //Update bs
    boundingSphere.SetRadius (r);
  }

  void SpotLight::SetFalloff (float i, float o)
  {
    inner = i;
    outer = o;
  }

  void SpotLight::SetDirection (csVector3 d)
  {
    dir = d;
  }




  ProxyLight::ProxyLight (Sector* owner, Light* parentLight, const csFrustum& frustum,
    const csReversibleTransform& transform, const csPlane3& portalPlane)
    : Light (owner, parentLight->IsDeltaLight ()), parent (parentLight),
    proxyTransform (transform), portalPlane (portalPlane)
  {
    realLight = false;

    SetPDLight (parent->IsPDLight ());
    SetColor (parent->GetColor ());
    SetLightID ((const char*)parent->GetLightID ().data);
    SetName (parent->GetName ());
    lightFrustum = frustum;
    boundingSphere = transform.Other2This (parentLight->GetBoundingSphere ());

    csPlane3 bp (portalPlane);
    bp.DD += bp.norm * lightFrustum.GetOrigin ();
    bp.Invert ();
    lightFrustum.SetBackPlane (bp);
  }

  ProxyLight::~ProxyLight ()
  {
  }

  csColor ProxyLight::SampleLight (const csVector3& point, const csVector3& n,
    float u1, float u2, csVector3& lightVec, float& pdf, VisibilityTester& vistest,
    const csPlane3* visLimitPlane)
  {
    // Setup clipped visibility ray
    const csVector3 lightPos = GetLightSamplePosition (u1, u2);
    csSegment3 visSegment (lightPos, point);
    if (!lightFrustum.Contains (point-lightFrustum.GetOrigin ()) ||
      !lightFrustum.Intersect (visSegment))
    {
      pdf = 0.0f;
      return csColor (0,0,0);
    }

    if (visLimitPlane)
      csIntersect3::SegmentPlane (*visLimitPlane, visSegment);

    vistest.AddSegment (ownerSector->kdTree, visSegment.Start (), visSegment.End ());

    // Retransform values
    const csVector3 parentPos = proxyTransform.This2Other (point);
    const csVector3 parentNormal = proxyTransform.This2OtherRelative (n);

    csVector3 parentLightVec;
    csPlane3 transformedPlane;
    transformedPlane = proxyTransform.Other2This (portalPlane);

    const csColor parentLight = parent->SampleLight (point, n, u1, u2, 
      parentLightVec, pdf, vistest, &transformedPlane);
    lightVec = proxyTransform.Other2ThisRelative (parentLightVec);

    return parentLight;
  }

  csColor ProxyLight::GetPower () const
  {
    return parent->GetPower ();
  }

  csVector3 ProxyLight::GetLightSamplePosition (float u1, float u2)
  {
    return proxyTransform.Other2This (parent->GetLightSamplePosition (u1, u2));
  }
}















// Attenuation functions
static float LightAttnNone (float, const csVector4&)
{
  /// no attenuation: *1
  return 1;
}

static float LightAttnLinear (float squaredDistance, const csVector4& c)
{
  /// linear attenuation:  * (1 - distance * inverse_radius)
  return csMax (0.0f, 1.0f - (sqrtf (squaredDistance) * c.w));
}

static float LightAttnInverse (float squaredDistance, const csVector4&)
{
  /// inverse attenuation:  * 1 / distance
  return 1.0f / sqrtf(squaredDistance);
}

static float LightAttnRealistic (float squaredDistance, const csVector4&)
{
  /// realistic attenuation: * 1 / distance^2
  return 1.0f / squaredDistance;
}

static float LightAttnCLQ (float squaredDistance, const csVector4& c)
{
  /** 
   * CLQ, Constant Linear Quadratic: 
   * * 1 / (constant1 + constant2*distance + constant3*distance^2)
   */
  return c * csVector4 (1, sqrtf(squaredDistance), squaredDistance, 0);
}
