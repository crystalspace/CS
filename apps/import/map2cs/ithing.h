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

#ifndef __ITHING_H__
#define __ITHING_H__

#include "contain.h"
#include "csutil/ref.h"

class CMapPolygon;
class CISector;
class CIWorld;
struct iDocumentNode;

/**
  * This class will encapsulate a Thing to show up in a Crystal Space
  * worldfile.
  */
class CIThing
{
public:
  /// The constructor (as usual)
  CIThing(CMapEntity* pEntity);

  /// The destuctor
  virtual ~CIThing();

  /// Store a new polygon set. (will be deleted here!)
  void InsertPolygon(CMapPolygonSet* pPolygon);

  /// Returns true, if this Thing contains no polygons.
  bool IsEmpty() {return m_Polygon.Length() <= 0;}

  /// Get the name of the thing
  const char* GetName();

  /// Get the classname of the thing (the MAP entity classname)
  const char* GetClassname();

  /// Write this thing as part of an sector
  virtual bool Write(csRef<iDocumentNode> node, CIWorld* pWorld, 
    CISector* pSector) = 0;

  /// Return true, if this thing is moveable
  bool IsMoveable();

  /// Return true, if this is a sky box/dome
  bool IsSky();

  /// Return true, if there is something to write
  bool ContainsVisiblePolygons();

protected:
  /// Array containing all polygon sets. (Duplicated...)
  CMapPolygonSetVector m_Polygon;

  /**
    * A pointer to the original entity to be able to retrieve key/ value pairs
    * later on.
    */
  CMapEntity* m_pOriginalEntity;
};

#endif // __ITHING_H__

