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

#ifndef __CWORLD_H__
#define __CWORLD_H__

#include "dochelp.h"
#include "iworld.h"
#include "csutil/ref.h"

class CVertexBuffer;
class CCSSector;
struct iDocumentNode;

/**
  * this class encapsulates the worldin Crystal Space terminology. This means
  * it is an array of Sectors. This special class is able to store the world
  * in the standard ASCII CS world format
  */
class CCSWorld : public CIWorld
{
public:
  /// The constructor (as usual)
  CCSWorld();

  /// The destuctor
  ~CCSWorld();

  /**
    * Writes the sector and all contained things into the Crystal
    * Space worldfile.
    */
  bool Write(csRef<iDocumentNode> root, CMapFile* pMap, const char * sourcename);

  /// Get the filedescriptor of the file currently open for exporting
  //FILE*     GetFile() {return m_fd;}

  /// Writes all the Key Value pairs of the given entity.
  static bool WriteKeys(csRef<iDocumentNode> node, CIWorld* pWorld, 
    CMapEntity* pEntity);

  /**
    * Write a given polygon to the Worldfile. If SectorPolygon is true,
    * then this is a polygon for a sector wall, which means for example
    * reverse vertex order to make to polygon visible. You also need to
    * hand over a pointer to the sector, this polygon is going to be
    * part of, and the current VertexBuffer, so the polygon knows which
    * index numbers to use.
    */
  bool WritePolygon(csRef<iDocumentNode> node, CMapPolygon* pPolygon, 
    CCSSector* pSector, bool SectorPolygon, const CVertexBuffer& V,
    bool &Sky);

  /// return a new CCSThing
  virtual CIThing* CreateNewThing(CMapEntity* pEntity);

  /// return a new CCSSector
  virtual CISector* CreateNewSector(CMapBrush* pBrush);

  /// Return true, if a skysector is needed.
  bool NeedSkysector();

  /**
   * Checks is a sky is needed and writes a dome or a box without the sector
   * heading
   */
  void WriteSky(csRef<iDocumentNode> node);

  /**
   * Return the worldspawn entity. (All general map settings are stored there)
   * returns 0 if there is no such entity. (I would consider this to be
   * a map bug...)
   */
  CMapEntity* GetWorldspawn();

  /**
    * Write a vector to the worldfile. This will be everything, including
    * opening and closing bracket and doing the proper scaling
    */
  bool WriteVector(csRef<iDocumentNode> node, const char* name, const CdVector3& v);

protected:
  /// Write a vector as vertex (Including the VERTEX() statement)
  bool WriteVertex(csRef<iDocumentNode> node, double x, double y, double z);

  /// Write a vector (including brackets and scaling)
  bool WriteVector(csRef<iDocumentNode> node, const char* name, double x, double y, double z);

  /**
    * Search a map for manual sectors. All other entities are stored in an
    * other array called m_Entities, so every entity in the map is stored here.
    * Either as sector or as entity
    */
  void FindSectors();

  /// Generate a default sector, that contains everything.
  void GenerateDefaultsector();

  /**
   * look for models and 3D sprites that need additional textures in the
   * resulting map
   */
  void FindAdditionalTextures();

  /// Request all needed textures for display of sky from the texture manager
  void AllocateSkytextures();

  /// Write a complete skysector
  void WriteSkysector(csRef<iDocumentNode> node);

  /// Write a skydome sector with name "cs_skysector"
  void WriteSkydome(csRef<iDocumentNode> node);

  /// Write a skybox sector with name "cs_skysector"
  void WriteSkybox(csRef<iDocumentNode> node);

  /// Writes Sprites
  void WriteSpritesTemplate(csRef<iDocumentNode> node);

  /// Writes Scripts
  void WriteScriptsTemplate(csRef<iDocumentNode> node);

  /**
    * Write the textures section to the worldfile.
    */
  bool WriteTextures(csRef<iDocumentNode> node);

  /// Write the plugins section to the worldfile
  bool WritePlugins(csRef<iDocumentNode> node);

  /// Write the sounds section to the worldfile
  void WriteSounds(csRef<iDocumentNode> node);

  /// Writes all the sector and everything inside of it.
  bool WriteSectors(csRef<iDocumentNode> node);

  /// Writes the settings block
  bool WriteSettings(csRef<iDocumentNode> node);

  /// Write the player start point
  bool WritePlayerStart(csRef<iDocumentNode> node);

  /// Write Templates for all curves
  bool WriteCurvetemplates(csRef<iDocumentNode> node);

  bool WriteLibs (csRef<iDocumentNode> node, const char* type = 0);
public:
  /// Write a simple 'texmap' section for a polygon based on plane.
  bool WriteTexMap (CMapTexturedPlane* plane, DocNode& poly);

protected:
}; //CCSWorld

#endif // __CWORLD_H__

