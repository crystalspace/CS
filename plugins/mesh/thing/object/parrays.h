/*
    Dynamic arrays of engine objects
    Copyright (C) 1999 by Andrew Zabolotny
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

#ifndef __CS_PARRAYS_H__
#define __CS_PARRAYS_H__

#include "csutil/array.h"
#include "polygon.h"

class csThingObjectType;

/**
 * An dynamic array of csPolygon3DStatic objects.
 * This class is used in polygon set class and thing template class
 * for storing the polygons that the model consists of.
 */
class csPolygonStaticArray : public csArray<csPolygon3DStatic*>
{
private:
  csThingObjectType* thing_type;

public:
  /// Create the polygon array object
  csPolygonStaticArray (int iLimit, int iDelta)
  	: csArray<csPolygon3DStatic*> (iLimit, iDelta)
  {
    thing_type = 0;
  }

  /// Destroy the polygon array and all inserted polygons
  ~csPolygonStaticArray ();

  /// Set the thing type used to destroy polygons.
  void SetThingType (csThingObjectType* thing_type)
  {
    csPolygonStaticArray::thing_type = thing_type;
  }

  /// Free all polygons.
  void FreeAll ();

  /// Delete a particular array element
  void FreeItem (csPolygon3DStatic* Item);

  /// Find a polygon by name
  static int CompareKey (csPolygon3DStatic* const&, char const* const& Key);

  /// Return a functor wrapping CompareKey for a given name.
  static csArrayCmp<csPolygon3DStatic*,char const*> KeyCmp(char const* k)
  {
    return csArrayCmp<csPolygon3DStatic*,char const*>(k, CompareKey);
  }
};

/**
 * An dynamic array of csPolygon3D objects.
 * This class is used in polygon set class and thing template class
 * for storing the polygons that the model consists of.
 */
class csPolygonArray : public csArray<csPolygon3D>
{
private:
  csThingObjectType* thing_type;

public:
  /// Create the polygon array object
  csPolygonArray (int iLimit, int iDelta)
  	: csArray<csPolygon3D> (iLimit, iDelta)
  {
    thing_type = 0;
  }

  /// Set the thing type used to destroy polygons.
  void SetThingType (csThingObjectType* thing_type)
  {
    csPolygonArray::thing_type = thing_type;
  }
};

#endif // __CS_PARRAYS_H__

