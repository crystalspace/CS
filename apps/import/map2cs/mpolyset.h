/*
    Map2cs: a convertor to convert the frequently used MAP format, into
    something, that can be directly understood by Crystal Space.

    Copyright (C) 1999 Thomas Hieber (thieber@gmx.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __MPOLYSET_H__
#define __MPOLYSET_H__

#include "contain.h"

/**
  * This class stores an open number of concave polygons, that are coplanar. The
  * polygons need not have a commom texture, but they need to have a common
  * geometric baseplane.
  * This class will allow simple geometrical operations on polygon sets. This
  * will help to generate portals, or to clip awain hidden surfaces.
  */
class CMapPolygonSet
{
public:
  /**
    * Create an empty Polygon set.
    */
  CMapPolygonSet();

  /**
    * Create a new Polygon set from a given Polygon set. All information is
    * being duplicated, so the original Polygon set can be altered any way,
    * without affecting the newly created Set.
    */
  CMapPolygonSet(const CMapPolygonSet& Set);

  /**
    * Create a new Polygon set from a single Polygon. All information is
    * being duplicated, so the original Polygon can be altered any way,
    * without affecting the newly created Set.
    */
  CMapPolygonSet(const CMapPolygon&    Poly);

  /**
    * Delete the polygon set and all contained Objects.
    */
  ~CMapPolygonSet();

  /// Assignment operator
  CMapPolygonSet& operator=(const CMapPolygonSet& Other);

  /// Returns the number of polygons, this set is made of
  size_t GetPolygonCount() {return m_Polygon.Length();}

  CMapPolygon* GetPolygon(size_t index) {return m_Polygon[index];}

  /**
    * Adds the polygons contained in the other set to this set.
    * The polygons get duplicated.
    * No additonal check for overlapping is performed.
    */
  void AddPolygons(const CMapPolygonSet& Other);

  /**
    * Change the order of vertices and planes for every polygon, as well
    * as all baseplanes, so that the other side of this polygon set is
    * created. (Concerning backface culling)
    */
  void FlipSide();

  /**
    * Removes all area of this polygon set, that is not also part
    * of the other polygon set.
    */
  void ReduceToCommonParts(const CMapPolygonSet& Other, bool optimise=true);

  /**
    * Removes all area of this polygon set, that is also part
    * of the other polygon set.
    */
  void RemoveCommonParts(const CMapPolygonSet& Other, bool optimise=true);

  /// returns true, if the polygonset is empty.
  bool IsEmpty() {return m_Polygon.Length()<=0;}

  /**
    * Gets the Basplane for this polygon set. Attention: Though all
    * Polygons share the same _geometric_ baseplane, they need not
    * share the same texture plane, so you man _not_ use this
    * baseplane to retreive texture information!
    */
  CMapTexturedPlane* GetBaseplane();

protected:
  /**
    * Removes the area of the given polygon from this polygon set
    */
  void RemovePolygon(const CMapPolygon& Other);

  /**
    * Array, containing all the convex polygons, that compose this
    * freeformed, but coplanar polygon set.
    */
  CMapPolygonVector m_Polygon;
};

#endif // __MPOLYSET_H__
