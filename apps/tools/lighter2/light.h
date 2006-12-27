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

#include "raytracer.h"

namespace lighter
{
  class Raytracer;
  
  class VisibilityTester
  {
  public:
    VisibilityTester ();

    /// Test if we have visibility within given raytracer
    bool Unoccluded (Raytracer& rt, const Primitive* ignorePrim = 0);

    void SetSegment (const csVector3& start, const csVector3& end);
    void SetSegment (const csVector3& start, const csVector3& dir, float maxL);

  private:
    Ray testRay;
  };

  typedef float(*LightAttenuationFunc)(float squaredDistance, 
    const csVector3& constants);

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
      float u1, float u2, csVector3& lightVec, float& pdf, VisibilityTester& vistest) = 0;

    /**
     * Return light power over S2 sphere
     */
    virtual csColor GetPower () const = 0;

    // Properties
    void SetAttenuation (csLightAttenuationMode mode, const csVector3& constants);

    // Getters/setters
    
    inline bool IsPDLight () const 
    { 
      return pseudoDynamic;
    }

    inline void SetPDLight (bool pd)
    {
      pseudoDynamic = pd;
    }

    inline const csVector3& GetPosition () const
    {
      return position;
    }

    inline void SetPosition (const csVector3& p)
    {
      position = p;
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

  protected:
    /// Constructor
    Light (bool deltaDistribution);

    /// Helper methods to attenuate light
    inline float ComputeAttenuation (float sqD)
    {
      return attenuationFunc (sqD, attenuationConsts);
    }

    // Data

    // Atteunation related
    csLightAttenuationMode attenuationMode;
    csVector3 attenuationConsts;
    LightAttenuationFunc attenuationFunc;

    // Common properties
    csVector3 position;
    csColor color;
    csMD5::Digest lightId;

    // Type
    bool deltaDistribution;
    bool pseudoDynamic;
  };
  typedef csRefArray<Light> LightRefArray;


  // Point light source
  class PointLight : public Light
  {
  public:
    PointLight ();

    virtual ~PointLight();

    virtual csColor SampleLight (const csVector3& point, const csVector3& n,
      float u1, float u2, csVector3& lightVec, float& pdf, VisibilityTester& vistest);

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
    float radius;

  };
} // namespace lighter

#endif // __LIGHT_H__
