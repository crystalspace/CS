/*
    Map2cs a convertor to convert the frequently used MAP format, into
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

#ifndef __BRUSH_H__
#define __BRUSH_H__

#include "contain.h"

//Some forward declarations needed in class declaration.
class CMapParser;
class CMapTexturedPlane;
class CMapFile;
class CdVector3;

/**
  * This bounding box is to manage the bounding box of a brush, or any other
  * set of map polygons. It supports a fast intersection test of boxes,
  * to check if there is any interaction possible.
  */
class CMapBrushBoundingBox
{
public:
  CMapBrushBoundingBox()
  {
    m_Defined = false;
  }

  void Extend(CMapPolygon* pPoly);
  bool Intersects(CMapBrushBoundingBox* pOtherBox);

private:
  bool m_Defined;
  double m_x1, m_x2, m_y1, m_y2, m_z1, m_z2;
};

/**
  * This class represents a Brush. A brush is the only form of geometry
  * conatined within a map file. A brush is always part of an entity, which
  * can contain any number of brushes (or none at all).
  * A brush is defined by a set of planes, which will be used to delimt a
  * convex polyhedron. The most common case is a block, that is delimited
  * by 6 planes. (one for every side.)
  * All planes will also conatin texture info. Planes can be shared among
  * bushed, so it will only store references to planes here.
  */
class CMapBrush
{
public:
  /**
    * The constructor as usual
    */
  CMapBrush(CMapEntity* pEntity);

  /**
    * The destuctor as usual. Note, that this will _not_ destroy the Planes,
    * because they are only stored as reference here. The will be destroyed
    * from the container inside CMapFile
    */
  ~CMapBrush();

  /**
    * Reads an entire Brush, until the terminating "}" or until an erro
    * occurs. returns false if an error occured. Parsing of the file should
    * then stop. The error message has then aready been generated.
    */
  bool Read      (CMapParser* pParser, CMapFile* pMap);

  /**
    * Reads three integer values from the parser and stores them inside a
    * vector3. Will also do error handling and error messages.
    * returns false, if there was an error. Parsing of the file should then
    * stop.
    */
  bool ReadVector(CMapParser* pParser, CdVector3& v);

  /// Create all polygons for this brush.
  void CreatePolygons();

  /// Check, if the given vector is inside this brush
  bool IsInside(CdVector3& v);

  /// Get the resulting polygon, if the plane cuts through the brush
  void IntersectWithPlane(CMapTexturedPlane* pIntersectplane,
                          CMapPolygon& Poly);

  /// Access all polygons from outside
  size_t GetPolygonCount()      {return m_Polygons.Length();}
  CMapPolygon* GetPolygon(size_t index) {return m_Polygons.Get(index);}

  /// Add a plane. (Useful, when creating a brush from the outside)
  void AddPlane(CMapTexturedPlane* pPlane) {m_Planes.Push(pPlane);}

  /**
    * Returns false, if this Bursh is some internal object for Quake.
    * this is the case for clip or hint brushes for example
    */
  bool IsVisible();

  /// returns a pointer to the entity that defined this brush (or 0)
  CMapEntity* GetEntity() {return m_pEntity;}

  /// returns a pointer to the bounding box
  CMapBrushBoundingBox* GetBoundingBox() {return &m_BoundingBox;}

protected:

  /**
    * Here, the brush will store references to all planes that form that
    * brush.
    */
  CMapTexturedPlaneVector m_Planes;

  /**
    * Here are all polgons stored, that are defined by the planes in
    * m_Planes. (Note that this info is derived from m_Planes, so it is
    * only a secondary data structure for convenience)
    */
  CMapPolygonVector       m_Polygons;

  /**
    * Pointer to the entity that defined this brush (or 0)
    */
  CMapEntity*             m_pEntity;

  /**
    * The number of the line, where the brush definition starts. (for error
    * messages)
    */
  int m_Line;

  /**
    * A bounding box, to speed up some calculations.
    */
  CMapBrushBoundingBox    m_BoundingBox;
};

#endif // __BRUSH_H__

