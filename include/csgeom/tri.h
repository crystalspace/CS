  /*
    Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_TRIANGLE_H__
#define __CS_TRIANGLE_H__

/**\file 
 * Triangle.
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

/**
 * A triangle. Note that this structure is only valid if used
 * in combination with a vertex or edge table. 'a', 'b', and 'c' are then
 * indices in that table (either vertices or edges).
 */
struct csTriangle
{
  int a, b, c;

  /// Empty default constructor
  csTriangle () {}

  /// Convenience constructor, builds a triangle with initializers
  csTriangle (int _a, int _b, int _c) : a(_a), b(_b), c(_c) {}

  /// Copy constructor.
  csTriangle (const csTriangle& t)
  {
    a = t.a;
    b = t.b;
    c = t.c;
  }

  /// Assignment.
  csTriangle& operator= (const csTriangle& t)
  {
    a = t.a;
    b = t.b;
    c = t.c;
    return *this;
  }

  /// Set the values.
  void Set (int _a, int _b, int _c)
  {
    a = _a;
    b = _b;
    c = _c;
  }
};

/** @} */

#endif // __CS_TRIANGLE_H__

