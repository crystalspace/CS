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

#ifndef __CS_MATH_H__
#define __CS_MATH_H__

/**\file 
*/
/**
* \addtogroup geom_utils
* @{ */

/**
 * Returns bigger of a and b. If they are equal, a or b can be returned.
 */
template<class T>
const T& csMax (const T& a, const T& b)
{
  if (a >= b) return a;
  return b;
}

/**
 * Returns smaller of a and b. If they are equal, a or b can be returned.
 */
template<class T>
const T& csMin (const T& a, const T& b)
{
  if (a < b) return a;
  return b;
}


/**
 * Clamp a between max and min.
 */
template<class T>
T csClamp (const T& a, T max, T min)
{
  return csMin (csMax (a, min), max);
}

/**
 * Preforms a smooth interpolation of a on range min to max.
 * \return Smooth interporlated value if \a min \< \a a \< \a max, 
 *  and 0 resp. 1 if \a a is smaller than \a min resp. larger than \a max.
 */
template<class T>
T csSmoothStep (const T& a, T max, T min)
{
  T tmp, tmp2;
  if (a <= min)
    tmp = 0.0f;
  else if (a >= max)
    tmp = 1.0f;
  else
  {
    tmp2 = (a - min) / (max-min);
    tmp = tmp2*tmp2 * (3.0 - 2.0*tmp2);
  }
  return tmp;
}

/**
 * Returns the square of the argument
 */
template<class T>
T csSquare (const T& x)
{
  return x * x;
}

/** @} */

#endif //__CS_MATH_H__
