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

#include "csutil/csvector.h"
#include "csutil/parray.h"

class csPolygon3D;
class csPolygon3DStatic;
class csThingObjectType;

/**
 * An dynamic array of csPolygon3DStatic objects.
 * This class is used in polygon set class and thing template class
 * for storing the polygons that the model consists of.
 */
class csPolygonStaticArray : public csVector
{
private:
  csThingObjectType* thing_type;

public:
  /// Create the polygon array object
  csPolygonStaticArray (int iLimit, int iDelta) : csVector (iLimit, iDelta)
  {
    thing_type = 0;
  }

  /// Destroy the polygon array and all inserted polygons
  virtual ~csPolygonStaticArray ();

  /// Set the thing type used to destroy polygons.
  void SetThingType (csThingObjectType* thing_type)
  {
    csPolygonStaticArray::thing_type = thing_type;
  }

  /// Delete a particular array element
  virtual bool FreeItem (void* Item);

  /// Find a polygon by name
  virtual int CompareKey (void* Item, const void* Key, int Mode) const;

  /// Get a polygon given its index in the array
  csPolygon3DStatic *Get (int iIndex) const;

  /// Get the entire array of polygons as an array of pointers
  csPolygon3DStatic **GetArray ()
  { return (csPolygon3DStatic **)root; }
};

/**
 * An dynamic array of csPolygon3D objects.
 * This class is used in polygon set class and thing template class
 * for storing the polygons that the model consists of.
 */
class csPolygonArray : public csVector
{
private:
  csThingObjectType* thing_type;

public:
  /// Create the polygon array object
  csPolygonArray (int iLimit, int iDelta) : csVector (iLimit, iDelta)
  {
    thing_type = 0;
  }

  /// Destroy the polygon array and all inserted polygons
  virtual ~csPolygonArray ();

  /// Set the thing type used to destroy polygons.
  void SetThingType (csThingObjectType* thing_type)
  {
    csPolygonArray::thing_type = thing_type;
  }

  /// Delete a particular array element
  virtual bool FreeItem (void* Item);

  /// Find a polygon by name
  virtual int CompareKey (void* Item, const void* Key, int Mode) const;

  /// Get a polygon given its index in the array
  csPolygon3D *Get (int iIndex) const;

  /// Get the entire array of polygons as an array of pointers
  csPolygon3D **GetArray ()
  { return (csPolygon3D **)root; }
};

#endif // __CS_PARRAYS_H__

