/*
    Copyright (C) 2002 by Marten Svanfeldt
    Written by Marten Svanfeldt

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

#ifndef __CS_IVIDEO_EFVECTOR4_H__
#define __CS_IVIDEO_EFVECTOR4_H__

/**\file
 * Effect 4-component vector
 */
 
/**
 * \addtogroup gfx3d
 * @{ */

#include "csgeom/vector3.h"
#include "csgfx/rgbpixel.h"

/**
 * A simple 4-component vector, used by the effect system.
 */
struct csEffectVector4
{
  /// X component
  float x;
  /// Y component
  float y;
  /// Z component
  float z;
  /// W component
  float w;

  /// Initialize X, Y, Z to 0, W to 1
  csEffectVector4 ()
  { x=0.0f;y=0.0f;z=0.0f;w=1.0f; }

  /// Initialize X, Y, Z to <i>value</i>, W to 1
  csEffectVector4 (float value)
  { x=value; y=value; z=value; w=1.0f; }

  /// Initialize X, Y, Z to with the given values, W to 1
  csEffectVector4 (float _x, float _y,float _z)
  { x=_x; y=_y; z=_z; w=1.0f; }

  /// Initialize X, Y, Z, W with the given values
  csEffectVector4 (float _x, float _y,float _z,float _w)
  { x=_x; y=_y; z=_z; w=_w; }

  /// Copy X, Y, Z from the vector, set W to 1
  csEffectVector4 (const csVector3 &vec)
  { x=vec.x; y = vec.y; z = vec.z; w=1.0f; }

  /// Copy from the color: X = R, Y = G, Z = B, W = A
  csEffectVector4 (const csRGBpixel &color)
  { x=color.red; y=color.green; z=color.blue; w=color.alpha; }

  /// Copy components from given effect vector
  csEffectVector4 (const csEffectVector4 &vec)
  { x=vec.x; y = vec.y; z = vec.z; w=vec.w; }
};

/** @} */

#endif // __CS_IVIDEO_EFVECTOR4_H__
