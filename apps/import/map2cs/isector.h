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

#ifndef __ISECTOR_H__
#define __ISECTOR_H__

#include "contain.h"
#include "csutil/ref.h"

class CMapBrush;
class CMapPolygonSet;
class CIWorld;
struct iDocumentNode;

/**
  * this class encapsulates a Sector in Crystal Space terminology. This means
  * it is a concave Sector of space that can contains a number of things and
  * is connected to other sectors by portals.
  */
class CISector
{
public:
  /// The constructor. Needs a MapBrush as template for the shape.
  CISector(CMapBrush* pBrush);

  /// The destuctor
  virtual ~CISector();

  /// Get the name of the sector
  const char* GetName();

  /// Check, if the given vector is inside this sector
  bool IsInside(CdVector3& v);

  /**
    * Create a portal to the given sector, if they share a common
    * polygon.
    */
  void CreatePortal(CISector* pOtherSector);

  /**
    * Look for matching brush faces and replace the wall textures by
    * the brushes faces.
    */
  void TextureWalls(CIWorld* pWorld);

  /**
    * Insert all brush based entities as things that are partially inside
    * this sector.
    */
  void InsertThings(CIWorld* pWorld);


  /**
    * Writes the sector and all contained things into the Crystal
    * Space worldfile.
    */
  virtual bool Write(csRef<iDocumentNode> node, CIWorld* pWorld) = 0;

protected:
  typedef enum {SameOrientation, MirroredOrientation, AllOrientations} WallOrientation;

  /// Remove a Polygon from the wall
  void RemoveWallPolygon(CMapPolygonSet* pRemovePoly, WallOrientation Orientation);

  /**
    * Returns a pointer to the matching wall or 0, if that plane is
    * coplanar with any wall.
    */
  CMapPolygonSet* GetCorrespondingWall(CMapTexturedPlane* pPlane, WallOrientation Orientation);

protected:
  /// Pointer to the brush, that defined this sector;
  CMapBrush*           m_pOriginalBrush;

  /// Array containing all Sector walls (guaranteed to form a concave sector)
  CMapPolygonSetVector m_Walls;

  /// Array containing all Portals to other Sectors
  CIPortalVector       m_Portals;

  /// Array containing all things within the sector.
  CIThingVector        m_Things;

  /// Array containing references to all contained nodes. (entities without brushes)
  CMapEntityVector     m_Nodes;

  /**
    * This is true, if this is no explicit, user generated sector, but a generic
    * defaultsector.
    */
  bool                 m_IsDefaultsector;
}; //CCSSector

#endif // __ISECTOR_H__

