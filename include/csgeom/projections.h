/*
    Copyright (C) 2008 by Frank Richter

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

#ifndef __CS_CSGEOM_PROJECTIONS_H__
#define __CS_CSGEOM_PROJECTIONS_H__

/**\file 
 * Create standard projections
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csgeom/matrix4.h"

namespace CS
{
  namespace Math
  {
    /**
     * Helpers to create standard projections
     */
    struct Projections
    {
      /**
       * Create an orthographic projection. \a left is mapped to x=0, \a right
       * to x=1, \a bottom to y=0 and \a top to y=1. \a near specfies the near
       * clipping distance, \a far the far clipping distance.
       * \sa \link http://www.opengl.org/sdk/docs/man/xhtml/glOrtho.xml glOrtho \endlink
       */
      static Matrix4 Ortho (float left, float right, float bottom, float top,
        float near, float far)
      {
	return Matrix4 (2.0f/(right-left), 0, 0, -(right+left)/(right-left),
			0, 2.0f/(top-bottom), 0, -(top+bottom)/(top-bottom),
			0, 0, -2.0f/(far-near), -(far+near)/(far-near),
			0, 0, 0, 1);
      }
      
      /**
       * Create a perspective projection as used by CS.
       */
      static Matrix4 CSPerspective (int viewWidth, int viewHeight, 
				    float shiftX, float shiftY,
				    float invAspect)
      {
	Matrix4 Mortho (Ortho (0, viewWidth, 0, viewHeight, -1.0f, 10.0f));

	CS::Math::Matrix4 Mtranslate (
	  1, 0, 0, shiftX,
	  0, 1, 0, shiftY,
          0, 0, 1,      0,
          0, 0, 0,      1);

	CS::Math::Matrix4 Mprojection (
	  1, 0, 0, 0,
	  0, 1, 0, 0,
          0, 0, 0, -invAspect,
          0, 0, invAspect, 0);
	
	return (Mortho * Mtranslate) * Mprojection;
      }
    };

  } // namespace Math
} // namespace CS

/** @} */

#endif // __CS_CSGEOM_PROJECTIONS_H__
