/*
    Dynamic arrays of engine objects
    Copyright (C) 1999 by Andrew Zabolotny

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

#ifndef __CS_PARRAYS_H__
#define __CS_PARRAYS_H__

#include "csutil/typedvec.h"

class csCurve;
class csPolygon3D;

CS_DECLARE_TYPED_VECTOR_NODELETE (csCurvesArrayHelper, csCurve);

/// A dynamic array of csCurve objects
class csCurvesArray : public csCurvesArrayHelper {
public:
  virtual bool FreeItem (csSome item);
  virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const;
  csCurvesArray (int l=8, int t=8) : csCurvesArrayHelper (l, t) {}
};

/**
 * An dynamic array of csPolygon3D objects.
 * This class is used in polygon set class and thing template class
 * for storing the polygons that the model consists of.
 */
class csPolygonArray : public csVector
{
public:
  /// Create the polygon array object
  csPolygonArray (int iLimit, int iDelta) : csVector (iLimit, iDelta)
  { }

  /// Destroy the polygon array and all inserted polygons
  virtual ~csPolygonArray ();

  /// Delete a particular array element
  virtual bool FreeItem (csSome Item);

  /// Find a polygon by name
  virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const;

  /// Get a polygon given its index in the array
  csPolygon3D *Get (int iIndex) const;

  /// Get the entire array of polygons as an array of pointers
  csPolygon3D **GetArray ()
  { return (csPolygon3D **)root; }
};

#endif // __CS_PARRAYS_H__

