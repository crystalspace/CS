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

#ifndef __LIGHTPROP_H__
#define __LIGHTPROP_H__

#include "scene.h"

namespace lighter
{

  /**
   * No attenuation. 
   */
  struct NoAttenuation
  {
    NoAttenuation (const Light_old& /*light*/)
    {}

    CS_FORCEINLINE_TEMPLATEMETHOD 
    float operator() (float dp, float /*distSquared*/) const
    {
      return dp;
    }
  };

  /**
   * Linear attenuation.
   * Out = in * (1 - distance/radius)
   */
  struct LinearAttenuation
  {
    LinearAttenuation (const Light_old& light)
    {
      invrad = 1/light.attenuationConsts.x;
    }

    CS_FORCEINLINE_TEMPLATEMETHOD 
    float operator() (float dp, float distSquared) const
    {
      return csMax (dp * (1 - sqrtf (distSquared) * invrad), 0.0f);
    }
  private:
    float invrad;
  };

  /**
   * Inverse linear attenuation.
   * Out = in * / distance
   */
  struct InverseAttenuation
  {
    InverseAttenuation (const Light_old& /*light*/)
    {}

    CS_FORCEINLINE_TEMPLATEMETHOD
    float operator() (float dp, float distSquared) const
    {
      return dp / sqrtf (distSquared);
    }
  };


  /**
   * Inverse quadratic attenuation.
   * Out = in * / distance^2
   */
  struct RealisticAttenuation
  {
    RealisticAttenuation (const Light_old& /*light*/)
    {}

    CS_FORCEINLINE_TEMPLATEMETHOD
    float operator() (float dp, float distSquared) const
    {
      return dp / distSquared;
    }
  };

  /**
   * Constant, Linear, Quadratic attenuation
   * Out = in /(const + distance*lin + distance^2*quad)
   */
  struct CLQAttenuation
  {
    CLQAttenuation (const Light_old& light)
      : attnVec (light.attenuationConsts)
    {}

    CS_FORCEINLINE_TEMPLATEMETHOD
    float operator() (float dp, float distSquared) const
    {
      return dp / 
        (csVector3 (1.0, sqrtf (distSquared), distSquared) * attnVec);
    }
  private:
    csVector3 attnVec;
  };

} // namespace lighter

#endif // __LIGHTPROP_H__
