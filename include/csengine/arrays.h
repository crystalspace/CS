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

#ifndef __ARRAYS_H__
#define __ARRAYS_H__

#include "csutil/csvector.h"

class csCurve;

/**
 * A dynamic array of curve surfaces.
 * This class is used in polygon set and in thing template classes
 * for storing the curves that are part of respective objects.
 */
class csCurvesArray : public csVector
{
public:
  /// Initialize the vector of curve surfaces
  csCurvesArray (int iLimit, int iDelta) : csVector (iLimit, iDelta)
  { }

  /// Destroy the array and all curve surfaces inserted into this array
  virtual ~csCurvesArray ();

  /// Free a single element of the array
  virtual bool FreeItem (csSome Item);

  /// Search for a curve surface object by name
  virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const;

  /// Get a curve surface by index
  csCurve *Get (int iIndex) const
  { return (csCurve *)csVector::Get (iIndex); }

  /// Get the entire array of curve surfaces
  csCurve **GetArray ()
  { return (csCurve **)root; }
};

class csPolygonInt;
class csPolygon3D;

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
  csPolygonInt **GetArray ()
  { return (csPolygonInt **)root; }
};

class csLightHalo;

// Private class for keeping an array of halos
class csHaloArray : public csVector
{
public:
  // Constructor
  csHaloArray () : csVector (16, 16) { }

  // Destructor
  virtual ~csHaloArray ();

  // Free an item from array
  virtual bool FreeItem (csSome Item);

  // Find a halo by referenced light
  virtual int CompareKey (csSome Item, csConstSome Key, int /*Mode*/) const;

  // Return an reference to Nth halo info
  inline csLightHalo *Get (int n) const
  { return (csLightHalo *)csVector::Get (n); }
};

#endif // __ARRAYS_H__
