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

#ifndef __RAYGENERATOR_H__
#define __RAYGENERATOR_H__

#include "raytracer.h"

namespace lighter
{

  /**
   * Generate pseudo-random rays.
   */
  struct PseudoRandomRaygenerator
  {
    PseudoRandomRaygenerator ()
    {
      randGen.Initialize ();
    }

    Ray operator () (const csVector3& origin)
    {
      bool haveNrs = false;
      // Use Marsaglias method to get spherical coordinates randomly distributed
      float x1, x2, x1sq, x2sq;
      while (!haveNrs)
      {
        x1 = randGen.Get (-1, 1);
        x2 = randGen.Get (-1, 1);
        x1sq = x1*x1;
        x2sq = x2*x2;
        if ( (x1sq)+(x2sq) < 1) haveNrs = true;
      }

      Ray ret;
      ret.origin = origin;
      ret.direction.x = 2*x1*sqrtf(1-x1sq-x2sq);
      ret.direction.y = 2*x2*sqrtf(1-x1sq-x2sq);
      ret.direction.z = 1-2*(x1sq+x2sq);
      ret.direction.Normalize (); //shouldn't be needed;
      return ret;
    }

    csRandomFloatGen randGen;
  };

  /**
   * Generate a list of rays using supplied generator. 
   */
  template<class T>
  struct RandomRayListGenerator
  {
    csArray<Ray> operator () (unsigned int count, const csVector3& origin)
    {
      csArray<Ray> rays; rays.SetCapacity (count);
      for (unsigned int i = 0; i < count; i++)
      {
        rays.Push (t (origin));
      }

      return rays;
    }

    T t;
  };
}

#endif
