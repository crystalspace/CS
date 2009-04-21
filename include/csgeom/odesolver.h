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

/**\file
 * ODE solvers
 */

#ifndef __CS_CSGEOM_ODESOLVER_H__
#define __CS_CSGEOM_ODESOLVER_H__

#include "csgeom/vector3.h"

namespace CS
{
namespace Math
{

  /**
   * Embedded Runge-Kutta 4/5th order ODE solver for non-stiff ODEs.
   *
   * Solve the system 
   *
   * y' = f(t, y) 
   *
   * where y (and y') are scalar or vector.
   *
   * For reference, see:
   * "Ordinary and partial differential equation routines in C, C++, Fortran,
   *  Java, Maple and MATLAB" by H.J. Lee & W.E. Schiesser
   */
  class Ode45
  {
  public:

    /**
     * Step system a single step with step length h.
     *
     * \param f Function in y' = f(t, y)
     * \param h Step length
     * \param t0 Initial time
     * \param y0 Initial y value
     * \param yout Resulting y value
     * \param size Number of elements in y0 and yout
     * \return Error estimate
     */
    template<typename FuncType, typename ArgType>
    static ArgType Step (FuncType& f, ArgType h, ArgType t0, ArgType* y0, 
      ArgType* yout, size_t size)
    {
      // We need k1-k6
      CS_ALLOC_STACK_ARRAY(ArgType, k1, size);
      CS_ALLOC_STACK_ARRAY(ArgType, k2, size);
      CS_ALLOC_STACK_ARRAY(ArgType, k3, size);
      CS_ALLOC_STACK_ARRAY(ArgType, k4, size);
      CS_ALLOC_STACK_ARRAY(ArgType, k5, size);
      CS_ALLOC_STACK_ARRAY(ArgType, k6, size);
      CS_ALLOC_STACK_ARRAY(ArgType, tmp, size);
        
      // k1
      f (t0, y0, k1, size);

      // prepare for k2
      for (size_t i = 0; i < size; ++i)
      {        
        k1[i] *= h;
        tmp[i] = y0[i] + 0.25*k1[i];
      }

      // k2
      f (t0 + 0.25f*h, tmp, k2, size);

      // prepare for k3
      for (size_t i = 0; i < size; ++i)
      {        
        k2[i] *= h;
        tmp[i] = y0[i] + (3.0/32)*k1[i] 
                       + (9.0/32)*k2[i];
      }

      // k3
      f (t0 + (3.0f/8)*h, tmp, k3, size);

      // prepare for k4
      for (size_t i = 0; i < size; ++i)
      {        
        k3[i] *= h;
        tmp[i] = y0[i] + (1932.0/2197)*k1[i]
                       - (7200.0/2197)*k2[i]
                       + (7296.0/2197)*k3[i];
      }

      // k4
      f (t0 + (12.0f/13)*h, tmp, k4, size);

      // prepare for k5
      for (size_t i = 0; i < size; ++i)
      {        
        k4[i] *= h;
        tmp[i] = y0[i] + (439.0/216)*k1[i]
                       - (8.0)*k2[i]
                       + (3680.0/513)*k3[i]
                       - (845.0/4104)*k4[i];
      }

      // k5
      f (t0 + h, tmp, k5, size);

      // prepare for k6
      for (size_t i = 0; i < size; ++i)
      {        
        k5[i] *= h;
        tmp[i] = y0[i] - (8.0/27)*k1[i]
                       + (2.0)*k2[i]
                       - (3544.0/2565)*k3[i]
                       + (1859.0/4104)*k4[i]
                       - (11.0/40)*k5[i];
      }

      // k6
      f (t0 + 0.5f*h, tmp, k6, size);

      ArgType errMag = 0;
      // Finally calculate 4th and 5th order result, error term and final result
      for (size_t i = 0; i < size; ++i)
      {        

        k6[i] *= h;

        ArgType y4 = y0[i] + (25.0/216)*k1[i]
                           + (1408.0/2565)*k3[i]
                           + (2197.0/4104)*k4[i]
                           - (1.0/5)*k5[i];
        
        ArgType y5 = y0[i] + (16.0/315)*k1[i]
                           + (6656.0/12825)*k3[i]
                           + (28561.0/56430)*k4[i]
                           - (9.0f/50)*k5[i]
                           + (2.0/55)*k6[i];

        ArgType yErr = y4 - y5;

        yout[i] = y5 + yErr;
        
        errMag += yErr*yErr;
      } 

      return sqrtf (errMag);
    }


    /**
     * Step system a single step with step length h.
     *
     * \param f Function in y' = f(t, y)
     * \param h Step length
     * \param t0 Initial time
     * \param y0 Initial y value
     * \param yout Resulting y value
     * \return Error estimate
     */
    template<typename FuncType, typename ArgType>
    static float Step (FuncType& f, ArgType h, ArgType t0, csVector3 y0, 
      csVector3& yout)
    {
      // We need k1-k6
      csVector3 k1, k2, k3, k4, k5, k6;
      
      // k1
      k1 = h * f (t0, y0);

      // k2
      k2 = h * f (t0 + 0.25f*h, y0 + 0.25f*k1);

      // k3
      k3 = h * f (t0 + (3.0f/8)*h, y0 + (3.0f/32)*k1
                                      + (9.0f/32)*k2);

      // k4
      k4 = h * f (t0 + (12.0f/13)*h, y0 + (1932.0f/2197)*k1
                                        - (7200.0f/2197)*k2
                                        + (7296.0f/2197)*k3);

      // k5
      k5 = h * f (t0 + h, y0 + (439.0f/216)*k1
                             - 8.0f*k2
                             + (3680.0f/513)*k3
                             - (845.0f/4104)*k4);

      // k6
      k6 = h * f (t0 + 0.5f*h, y0 - (8.0f/27)*k1
                                  + (2.0f)*k2
                                  - (3544.0f/2565)*k3
                                  + (1859.0f/4104)*k4
                                  - (11.0f/40)*k5);

      // Finally calculate 4th and 5th order result, error term and final result
      csVector3 y4 = y0 + (25.0f/216)*k1
                        + (1408.0f/2565)*k3
                        + (2197.0f/4104)*k4
                        - (1.0f/5)*k5;

      csVector3 y5 = y0 + (16.0f/315)*k1
                        + (6656.0f/12825)*k3
                        + (28561.0f/56430)*k4
                        - (9.0f/50)*k5
                        + (2.0f/55)*k6;

      csVector3 yErr = y4 - y5;

      yout = y5 + yErr;      

      return yErr.Norm ();
    }

    /**
     * Step system a single step with step length h.
     *
     * \param f Function in y' = f(t, y)
     * \param h Step length
     * \param t0 Initial time
     * \param y0 Initial y value
     * \param yout Resulting y value
     * \return Error estimate
     */
    template<typename FuncType, typename ArgType>
    static ArgType Step (FuncType& f, ArgType h, ArgType t0, ArgType y0, 
      ArgType& yout)
    {
      // We need k1-k6
      ArgType k1, k2, k3, k4, k5, k6;
      
      // k1
      k1 = h * f (t0, y0);

      // k2
      k2 = h * f (t0 + 0.25f*h, y0 + 0.25f*k1);

      // k3
      k3 = h * f (t0 + (3.0f/8)*h, y0 + (3.0f/32)*k1
                                      + (9.0f/32)*k2);

      // k4
      k4 = h * f (t0 + (12.0f/13)*h, y0 + (1932.0f/2197)*k1
                                        - (7200.0f/2197)*k2
                                        + (7296.0f/2197)*k3);

      // k5
      k5 = h * f (t0 + h, y0 + (439.0f/216)*k1
                             - 8.0f*k2
                             + (3680.0f/513)*k3
                             - (845.0f/4104)*k4);

      // k6
      k6 = h * f (t0 + 0.5f*h, y0 - (8.0f/27)*k1
                                  + (2.0f)*k2
                                  - (3544.0f/2565)*k3
                                  + (1859.0f/4104)*k4
                                  - (11.0f/40)*k5);

      // Finally calculate 4th and 5th order result, error term and final result
      ArgType y4 = y0 + (25.0f/216)*k1
                      + (1408.0f/2565)*k3
                      + (2197.0f/4104)*k4
                      - (1.0f/5)*k5;

      ArgType y5 = y0 + (16.0f/315)*k1
                      + (6656.0f/12825)*k3
                      + (28561.0f/56430)*k4
                      - (9.0f/50)*k5
                      + (2.0f/55)*k6;

      ArgType yErr = y4 - y5;

      yout = y5 + yErr;      

      return yErr;
    }

  };


}
}

#endif
