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

#ifndef __MPOLY_H__
#define __MPOLY_H__

#include "contain.h"

class CMapPolygon
{
public:
  /**
    * The constructor. Creates an empty polygn
    */
  CMapPolygon();

  /**
    * The copy constructor. Creates a copy of the given polygn
    */
  CMapPolygon(const CMapPolygon& poly);

  /**
    * The destructor. Cleans up things. Will free the vertices, but will
    * _not_ free the planes this is suppoesd to be done somewhere else!
    */
  ~CMapPolygon();

  /**
    * Creates a polygon, that sits on the given baseplane. It is
    * defined, by that plane, after carving away all the other planes
    * in the given planes. (The baseplane may be part of "planes", it
    * is being ignored then)
    * Create can be called multiple times. It will remove any previous
    * definition at first, so the old values will be replaced.
    */
  void Create(CMapTexturedPlane*             pBaseplane,
              const CMapTexturedPlaneVector& planes,
              CMapBrush*                     pBrush);

  /**
    * Remove all Contents from this Polygon, free the memory, and make it
    * an empty polygon again.
    */
  void Clear();

  /**
    * returns true, if this Polygon is empty. This is the case, after
    * construction, if the last Create did not produce a closed
    * polygon or if it didn't produce a polygon at all.
    */
  bool IsEmpty();

  /**
    * returns the Area of the Polygon (currently broken...)
    */
  double GetArea();

  /**
    * Splits this polygon using the given plane. The result of the split
    * is returned in pTargetPoly. The original poly is not modified.
    * The routine returns the part of the polygon that is on the "inside"
    * part of the plane. In quake notation, the inside part of a plane is
    * the part, where csPlane::Qualify() will return > 0.
    */
  void Split(CMapTexturedPlane* pSplitplane, CMapPolygon* pTargetPoly);

  /// Assign an existing polygon.
  void SetPolygon(CMapPolygon* pPoly);

  /**
    * Change the order of vertices and planes , as well as the side of
    * the baseplanes, so that the other side of this polygon set is
    * created. (Concerning backface culling)
    */
  void FlipSide();

  /// Give some info to the polygon so better errors can be generated.
  void SetErrorInfo(int BrushLineNumber, int PlaneNumber);

  /// Access all vertices from outside
  size_t             GetVertexCount() const     {return m_Vertices.Length();}
  CdVector3          GetVertex(size_t index) const {return *(m_Vertices.Get(index));}
  CMapTexturedPlane* GetPlane (size_t index) const {return m_Planes.Get(index);}

  /// returns the Baseplane for this polygon
  CMapTexturedPlane* GetBaseplane() {return m_pBaseplane;}

  /// returns the pointer to the defining entity
  CMapEntity*        GetEntity();

protected:
  /**
    * Finds two planes, who will create a valid point of the polygon. If there
    * are no planes, plane1 and plane2 will be set to 0.
    */
  void GetStartplanes(const CMapTexturedPlaneVector& planes,
                      CMapTexturedPlane*&            pPlane1,
                      CMapTexturedPlane*&            pPlane2,
                      CdVector3&                     point);

  /**
    * Returns the number of valid vertices, the given plane will produce.
    * Any regular plane will produce two vertices (an edge). However, there
    * are some planes, that will produce none or, just a single vertex.
    * We need to ignore these, when we try to determine the edges of the
    * final polygon
    */
  int  GetNumberOfValidVertices(CMapTexturedPlane*             pPlane,
                                const CMapTexturedPlaneVector& planes);

  /**
    * Check in the given point is inside the volume formed by "planes"
    * To avoid rounding problems, you can hand over 3 ignore planes,
    * that will not be handled. Normally, you will give the 3 planes
    * that define the point to be ignored here.
    * return true, if the point is inside the planes.
    */
  bool CheckIfInside(const CdVector3&               point,
                     const CMapTexturedPlaneVector& planes,
                     CMapTexturedPlane const*       pIgnorePlane1,
                     CMapTexturedPlane const*       pIgnorePlane2,
                     CMapTexturedPlane const*       pIgnorePlane3);

  /// For debugging purposes: Dump all info to stdout.
  void DumpPolyinfo(CMapTexturedPlane*             pBaseplane,
                    const CMapTexturedPlaneVector& planes);

  /**
    * A pointer to the baseplane
    */
  CMapTexturedPlane*      m_pBaseplane;

  /**
    * An array containing all the vetices. Note that this info is derived
    * from m_pBaseplane and m_pPlanes, so it is only a convenience to store
    * them here.
    */
  CVector3Vector          m_Vertices;

  /**
    * An ordered array of the planes, defining this Polygon. Each vertex is
    * the common point of three planes: The baseplane, the plane with the same
    * position in the array, and the plane at the next position in the array.
    * (The last vertex will be combined with the first plane...)
    */
  CMapTexturedPlaneVector m_Planes;

  /// info of the polygon about the line number of the brush
  int m_BrushLineNumber;

  /// info on the plane in the brush
  int m_PlaneNumber;

  /// pointer to the brush, that this polyon was originally part of.
  CMapBrush* m_pBrush;
};

#endif // __MPOLY_H__

