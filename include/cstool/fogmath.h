/*
    Copyright (C) 2004 by Anders Stenberg

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

#ifndef __CS_CSTOOL_FOGMATH_H__
#define __CS_CSTOOL_FOGMATH_H__

/**\file
 * Fog math utilities.
 */

/**
 * Calculates fog density for a certain depth in the range [0..1]
 */
inline float csFogRamp (float depth)
{
  return 1.0 - exp (-depth*7);
  //return depth;
}

#endif // __CS_CSTOOL_FOGMATH_H__

