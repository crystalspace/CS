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

#ifndef CSECTOR_H
#define CSECTOR_H

#include "isector.h"

/**
  * this class encapsulates a Sector in Crystal Space terminology. This means
  * it is a concave Sector of space that can contains a number of things and
  * is connected to other sectors by portals.
  */
class CCSSector : public CISector
{
public:
  /// The constructor. Needs a MapBrush as template for the shape.
  CCSSector(CMapBrush* pBrush);

  /// The destuctor
  ~CCSSector();

  /**
    * Writes the sector and all contained things into the Crystal
    * Space worldfile.
    */
  bool Write(CIWorld* pIWorld);

protected:
  /// Write the worldspawn things as part
  bool WriteWorldspawn(CIWorld* pWorld);

  /// Write the lights inside the sector
  bool WriteLights(CIWorld* pWorld);

  /// Write the curves inside the sector
  bool WriteCurves(CIWorld* pWorld);

  /// Write all things inside the sector
  bool WriteThings(CIWorld* pWorld);

  /// Write all nodes inside the sector
  bool WriteNodes(CIWorld* pWorld);

  /// Write all 3Dsprites inside the sector
  bool WriteSprites(CIWorld* pWorld);

  /// Write all 2Dsprites inside the sector
  bool WriteSprites2D(CIWorld* pWorld);

  /// Write all fog inside the sector
  bool WriteFog(CIWorld* pWorld);

protected:
}; //CCSSector

#endif

