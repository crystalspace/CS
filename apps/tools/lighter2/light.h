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


namespace lighter
{
  class VisibilityTester
  {};

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
  };

}

#endif