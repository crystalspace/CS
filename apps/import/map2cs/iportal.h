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

#ifndef __IPORTAL_H__
#define __IPORTAL_H__

#include "mpolyset.h"

class CISector;

/**
  * This class encapulates a special PolygonSet: A portal to an other
  * sector. This class is resonsible for connecting sectors in
  * Crystal Space
  */
class CIPortal : public CMapPolygonSet
{
public:
  /**
    * Create an empty Portal.
    */
  CIPortal();

  /**
    * Create a new Portal from a given Polyset.
    */
  CIPortal(const CMapPolygonSet& Set);

  /**
    * Create a new Portal from a given Portal. All information is
    * being duplicated, so the original Portal set can be altered any way,
    * without affecting the newly created Portal.
    */
  CIPortal(const CIPortal& Portal);

  /**
    * Delete the polygon set and all contained Objects.
    */
  ~CIPortal();

  /// Assignment operator
  CIPortal& operator=(const CIPortal& Other);

  /**
    * Sets the target sector of the portal. (Not the sector, this polygon
    * is part of!
    */
  void SetTargetSector(CISector* pSector) {m_pSector = pSector;}

  /// Gets the target sector
  CISector* GetTargetSector() {return m_pSector;}

protected:
  CISector* m_pSector;
};

#endif // __IPORTAL_H__
