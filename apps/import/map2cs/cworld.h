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

#ifndef CWORLD_H
#define CWORLD_H

#include "iworld.h"

class CVertexBuffer;
class CCSSector;

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
  bool Write(const char* filename, CMapFile* pMap, const char * sourcename);

  /// Get the filedescriptor of the file currently open for exporting
  FILE*     GetFile() {return m_fd;}

  /// Writes all the Key Value pairs of the given entity.
  static bool WriteKeys(CIWorld* pWorld, CMapEntity* pEntity);

  /**
    * Write a given polygon to the Worldfile. If SectorPolygon is true,
    * then this is a polygon for a sector wall, which means for example
    * reverse vertex order to make to polygon visible. You also need to
    * hand over a pointer to the sector, this polygon is going to be
    * part of, and the current VertexBuffer, so the polygon knows which
    * index numbers to use.
    */
  bool WritePolygon(CMapPolygon* pPolygon, CCSSector* pSector,
                    bool SectorPolygon, const CVertexBuffer& Vb);

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
  void WriteSky();

protected:
  /**
    * Write a vector to the worldfile. This will be everything, including
    * opening and closing bracket and doing the proper scaling
    */
  bool WriteVector(const CdVector3& v);

  /// Write a vector as vertex (Including the VERTEX() statement)
  bool WriteVertex(double x, double y, double z);

  /// Write a vector (including brackets and scaling)
  bool WriteVector(double x, double y, double z);

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

  /**
   * Return the worldspawn entity. (All general map settings are stored there)
   * returns NULL if there is no such entity. (I would consider this to be
   * a map bug...)
   */
  CMapEntity* GetWorldspawn();

  /// Request all needed textures for display of sky from the texture manager
  void AllocateSkytextures();

  /// Write a complete skysector
  void WriteSkysector();

  /// Write a skydome sector with name "cs_skysector"
  void WriteSkydome();

  /// Write a skybox sector with name "cs_skysector"
  void WriteSkybox();

  /// Writes Sprites 
  void WriteSpritesTemplate();

  /// Writes Scripts 
  void WriteScriptsTemplate();

  /**
    * Write the textures section to the worldfile.
    */
  bool WriteTextures();

  /// Write the plugins section to the worldfile 
  bool WritePlugins();

  /// Write the sounds section to the worldfile 
  void WriteSounds();

  /// Write the planes section to the worldfile
  bool WritePlanes();

  /// Writes all the sector and everything inside of it.
  bool WriteSectors();

  /// Write the player start point
  bool WritePlayerStart();

  /// Write Templates for all curves
  bool WriteCurvetemplates();

protected:
}; //CCSWorld

#endif

