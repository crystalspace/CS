/*
    Map2cs: a convertor to convert the frequently used MAP format, into
    something, that can be directly understood by Crystal Space.

    Copyright (C) 1999 Thomas Hieber (thieber@gmx.net)
    modified by Petr Kocmid (pkocmid@atlas.cz)

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

#ifndef __IWORLD_H__
#define __IWORLD_H__

#include "contain.h"
#include "csutil/ref.h"

struct iDocumentNode;

class CMapFile;

/**
  * this class encapsulates the worldin Crystal Space terminology. This means
  * it is an array of Sectors. This is also an abstract baseclass, that will
  * be overridden for other output formats.
  */
class CIWorld
{
public:
  /// The constructor (as usual)
  CIWorld();

  /// The destuctor
  virtual ~CIWorld();

  /**
    * Writes the sector and all contained things into the Crystal
    * Space worldfile.
    */
  virtual bool Write(csRef<iDocumentNode> root, CMapFile* pMap, const char * sourcename) = 0;

  /// Get a pointer to the map to convert.
  CMapFile* GetMap()  {return m_pMap;}

  /// Get the scalefactor, that was specified by the user
  double    GetScalefactor() {return m_ScaleFactor;}

  /**
    * Returns a pointer to the sector, which contains the given Point, or
    * returns a 0 pointer, if there is no such sector.
    */
  CISector* FindSectorForPoint(CdVector3& v);

  /// Get the number of "non-sector" entities
  size_t GetNumEntities() {return m_Entities.Length();}

  /// Return the n-th "non-sector" entity
  CMapEntity* GetEntity(size_t n) {return m_Entities[n];}

  /// Get the number of sectors
  size_t GetNumSectors() {return m_Sectors.Length();}

  /// Return the n-th sector
  CISector* GetSector(size_t n) {return m_Sectors[n];}

  /**
    * derived export classes override this and return an instance of the
    * proper type
    */
  virtual CIThing* CreateNewThing(CMapEntity* pEntity) = 0;

  /**
    * derived export classes override this and return an instance of the
    * proper type
    */
  virtual CISector* CreateNewSector(CMapBrush* pBrush) = 0;

protected:
  /**
    * Search a map for manual sectors. All other entities are stored in an
    * other array called m_Entities, so every entity in the map is stored here.
    * Either as sector or as entity
    */
  void FindSectors();

  // Generate a default sector, that contains everything.
  void GenerateDefaultsector();

  /**
    * Search all manual sectors for common polygons, that are to be turned
    * into portals.
    */
  void FindPortals();

  /**
    * Insert all brush based entities as things into the sectors
    */
  void InsertThings();

  /**
    * Initialise m_TextureFileNames with a list of all textures
    */
  void BuildTexturelist();

  /**
    * Prepare all internal data for export. returns true, if ok
    */
  bool PrepareData(const char* filename, CMapFile* pMap);

protected:
  /// Array containing all Sectors
  CISectorVector   m_Sectors;

  /// Array containing all non-sector Entities
  CMapEntityVector m_Entities;

  /// Array containing all used texture filenames
  CCharVector      m_TextureFileNames;

  /// A Pointer to the map containing the data to be written
  CMapFile*        m_pMap;

  /// The factor to be used while converting Quake coordinates
  double           m_ScaleFactor;
}; //CCSWorld

#endif // __IWORLD_H__

