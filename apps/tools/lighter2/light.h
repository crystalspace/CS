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

#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "lighter.h"
#include "raytracer.h"

namespace lighter
{
  class Light;
  class Raytracer;
  class Sector;
  struct HitIgnoreCallback;
  class Primitive;
  class PrimitiveBase;
  class KDTree;
  
  class VisibilityTester
  {
    static size_t rayID;
  public:
    VisibilityTester (Light* light, Object* obj);

    enum OcclusionState
    {
      occlOccluded,
      occlUnoccluded,
      occlPartial
    };
    OcclusionState Occlusion (const Object* ignoreObject,
      const Primitive* ignorePrim = 0);
    OcclusionState Occlusion (const Object* ignoreObject,
      HitIgnoreCallback* ignoreCB);

    
    csColor GetFilterColor ();

    //void CollectHits (HitPointCallback* hitCB, HitIgnoreCallback* ignoreCB);

    void AddSegment (KDTree* tree, const csVector3& start, const csVector3& end);
    void AddSegment (KDTree* tree, const csVector3& start, const csVector3& dir, float maxL);
    
    void Clear() { allSegments.DeleteAll(); }

  private:
    // for RayDebugHelper
    Light* light;
    Object* obj;

    struct Segment
    {
      KDTree* tree;
      Ray ray;
    };
    csArray<Segment> allSegments;

    csArray<HitPoint> transparentHits;
    struct HitCallback : public HitPointCallback
    {
      VisibilityTester& parent;
      HitCallback (VisibilityTester& parent) : parent (parent) {}
  
      bool RegisterHit (const Ray &ray, const HitPoint &hit)
      {
        globalLighter->rayDebug.RegisterHit (parent.light, parent.obj, 
          ray, hit);
        if (hit.kdFlags & KDPRIM_FLAG_TRANSPARENT)
        {
          parent.transparentHits.Push (hit);
          return true;
        }
        return false;
      }
    };
  };

  typedef float(*LightAttenuationFunc)(float squaredDistance, 
    const csVector4& constants);

  /// Baseclass for lights
  class Light : public csRefCount
  {
  public:
    /// Destructor
    virtual ~Light ();

    /// Does light have delta distribution (0 extent)
    inline bool IsDeltaLight () const
    {
      return deltaDistribution;
    }

    /**
     * Sample light given a sample position, normal and random variables.
     * Returns color, outgoing vector, pdf and a functor to test visibility
     */
    virtual csColor SampleLight (const csVector3& point, const csVector3& n,
      float u1, float u2, csVector3& lightVec, float& pdf, VisibilityTester& vistest,
      const csPlane3* visLimitPlane = 0) = 0;

    /**
     * Return light power over S2 sphere
     */
    virtual csColor GetPower () const = 0;

    /// Compute the light position from given sampling values
    virtual csVector3 GetLightSamplePosition (float u1, float u2) = 0;

    // Properties
    void SetAttenuation (csLightAttenuationMode mode, const csVector4& constants);

    // Getters/setters
    
    inline bool IsPDLight () const 
    { 
      return pseudoDynamic;
    }

    inline void SetPDLight (bool pd)
    {
      pseudoDynamic = pd;
    }

    inline bool IsRealLight () const
    {
      return realLight;
    }

    inline Sector* GetSector () const
    {
      return ownerSector;
    }

    inline const csVector3& GetPosition () const
    {
      return position;
    }

    inline void SetPosition (const csVector3& p)
    {
      position = p;
      boundingSphere.SetCenter (p);
      lightFrustum.SetOrigin (p);
    }

    inline const csColor& GetColor () const
    {
      return color;
    }

    inline void SetColor (const csColor& c)
    {
      color = c;
    }

    inline const csMD5::Digest& GetLightID () const
    {
      return lightId;
    }

    inline void SetLightID (const char* id)
    {
      memcpy (lightId.data, id, csMD5::Digest::DigestLen);
    }

    inline const csSphere& GetBoundingSphere () const 
    {
      return boundingSphere;
    }

    inline const csFrustum& GetFrustum () const
    {
      return lightFrustum;
    }

    inline const csString& GetName () const
    {
      return name;
    }

    inline void SetName (const char* name)
    {
      this->name = name;
    }

    /**
     * If the light is a proxy, returns the pointer to the "original" light.
     * If the light is not a proxy simply returns pointer to itself.
     */
    virtual Light* GetOriginalLight () { return this; }

     /// Helper methods to attenuate light
    inline float ComputeAttenuation (float sqD)
    {
      return attenuationFunc (sqD, attenuationConsts);
    }

 protected:
    /// Constructor
    Light (Sector* owner, bool deltaDistribution);
 
    // Data

    // Atteunation related
    csLightAttenuationMode attenuationMode;
    csVector4 attenuationConsts;
    LightAttenuationFunc attenuationFunc;

    // Common properties
    Sector* ownerSector;
    csVector3 position;
    csColor color;
    csMD5::Digest lightId;
    csSphere boundingSphere;
    csString name;

    csFrustum lightFrustum;

    // Type
    bool deltaDistribution;
    bool pseudoDynamic;
    bool realLight;
  };
  typedef csRefArray<Light> LightRefArray;



  // Point light source
  class PointLight : public Light
  {
  public:
    PointLight (Sector* owner);

    virtual ~PointLight();

    virtual csColor SampleLight (const csVector3& point, const csVector3& n,
      float u1, float u2, csVector3& lightVec, float& pdf, VisibilityTester& vistest,
      const csPlane3* visLimitPlane = 0);

    /**
     * Return light power over S2 sphere
    */
    virtual csColor GetPower () const;

    inline float GetRadius () const
    {
      return radius;
    }

    void SetRadius (float radius);

  protected:
    /// Compute the light position from given sampling values
    virtual csVector3 GetLightSamplePosition (float u1, float u2)
    {
      return GetPosition ();
    }

    float radius;
  };
  
  // Spot light source
  class SpotLight : public Light
  {
  public:
    SpotLight (Sector* owner);

    virtual ~SpotLight();

    virtual csColor SampleLight (const csVector3& point, const csVector3& n,
      float u1, float u2, csVector3& lightVec, float& pdf, VisibilityTester& vistest,
      const csPlane3* visLimitPlane = 0);

    /**
     * Return light power over S2 sphere
    */
    virtual csColor GetPower () const;

    inline float GetRadius () const
    {
      return radius;
    }

    void SetRadius (float radius);

    inline void GetFalloff (float& in, float &out) const
    {
      in = inner;
      out = outer;
    }

    void SetFalloff (float inner, float outer);

    inline csVector3 GetDirection () const
    {
      return dir;
    }

    void SetDirection (csVector3 direction);

  protected:
    /// Compute the light position from given sampling values
    virtual csVector3 GetLightSamplePosition (float u1, float u2)
    {
      return GetPosition ();
    }

    float radius;
    float inner;
    float outer;
    csVector3 dir;
  };
  
  // Directional light source
  class DirectionalLight : public Light
  {
  public:
    DirectionalLight (Sector* owner);

    virtual ~DirectionalLight();

    virtual csColor SampleLight (const csVector3& point, const csVector3& n,
      float u1, float u2, csVector3& lightVec, float& pdf, VisibilityTester& vistest,
      const csPlane3* visLimitPlane = 0);

    /**
     * Return light power over S2 sphere
    */
    virtual csColor GetPower () const;

    inline float GetRadius () const
    {
      return radius;
    }

    void SetRadius (float radius);

    inline float GetLength () const
    {
      return length;
    }

    void SetLength (float length);

    inline csVector3 GetDirection () const
    {
      return dir;
    }

    void SetDirection (csVector3 direction);

  protected:
    /// Compute the light position from given sampling values
    virtual csVector3 GetLightSamplePosition (float u1, float u2)
    {
      return GetPosition ();
    }

    float radius;
    float length;
    csVector3 dir;
  };
  
  // Proxy light, used when light source is in different sector
  class ProxyLight : public Light
  {
  public:
    ProxyLight (Sector* owner, Light* parentLight, const csFrustum& frustum,
      const csReversibleTransform& transform, const csPlane3& portalPlane);
    virtual ~ProxyLight();

    virtual csColor SampleLight (const csVector3& point, const csVector3& n,
      float u1, float u2, csVector3& lightVec, float& pdf, VisibilityTester& vistest,
      const csPlane3* visLimitPlane = 0);

    /**
     * Return light power over S2 sphere
     */
    virtual csColor GetPower () const;

    virtual Light* GetOriginalLight () { return parent->GetOriginalLight(); }
  private:
    /// Compute the light position from given sampling values
    virtual csVector3 GetLightSamplePosition (float u1, float u2);

    Light* parent;
    csReversibleTransform proxyTransform;
    csPlane3 portalPlane;
  };
} // namespace lighter

#endif
