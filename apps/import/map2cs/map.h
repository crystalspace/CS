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

#ifndef __MAP_H__
#define __MAP_H__

#include "contain.h"
#include "texman.h"

class csConfigFile;
class CZipFile;

struct TextureInfo
{
  csString FileName;
  int      SizeX;
  int      SizeY;
  bool     WadTexture;
};

/**
  * This is the main class to manage all information found within
  * a map file.
  * That file can consist of any number of entities, which can all
  * contain any number of brushes.
  * The entities will define the function of the brushes (world
  * geometry, doors, water).
  * For simplification, we will also store all possible planes here.
  * This is an optimisation, that is being done while reading, because
  * only unique planes are being created. This will allow a direct
  * realtion to the plane statement in Crystal Space worldfiles, which
  * will define the texture orientation.
  */
class CMapFile
{
public:
  /**
    * Constructs the map
    */
  CMapFile();

  /**
    * Destructor, will do cleanup. It will take care of removing all
    * entities and planes.
    */
  ~CMapFile();

  /**
    * Reads the given mapfile. It will do detailed error handling and
    * error messages by itself. It will return, true, if it was able
    * to load the complete file, and false, if there was an error, and
    * loading did not successfully finish. It will _not_ clean up the
    * already loaded part.
    * This method can currently be called only once for every instance
    * of this class. Otherwise, you will merge both map files.
    */
  bool Read(const char* filename, const char* configfile);

  /**
    * Writes all the missing textures to the configfile.
    */
  bool WriteTextureinfo();

  /**
    * This method will look for the given Plane in the Plane List.
    * If a similar Plane (Same position Texture and Texture alignment)
    * is found, it will return a pointer the the already available
    * plane otherwise it will create a new plane and return a pointer
    * to that plane.
    */
  CMapTexturedPlane* AddPlane(CdVector3 v1, CdVector3 v2, CdVector3 v3,
                              const char* TextureName,
                              double x_off, double y_off,
                              double rot_angle,
                              double x_scale, double y_scale,
			      CdVector3 v_tx_right, CdVector3 v_tx_up,
                              bool QuarkModeTexture,
                              bool QuarkMirrored,
			      bool HLTexture);

  /**
    * Adds a flatshaded plane. the given values are the color components
    */
  CMapTexturedPlane* AddPlane(CdVector3 v1, CdVector3 v2, CdVector3 v3,
                              int r, int g, int b);

  /**
    * Make all entities create their polygons.
    */
  void CreatePolygons();

  /**
    * Will traverse all the map, and get the Minimum and Maximum vertex
    * Coordinates. The map needs to be prepared by CreatePolygons()
    * before, otherwise you will get a zero size. Also note, that this
    * is a pretty expensive operation, so you should memorize the size
    * somewhere and not call this method too often.
    */
  void GetMapSize(CdVector3& Min, CdVector3& Max);

  /**
    * Get the TextureFile for the given original Texture name. If the texture
    * is not found, it will return 0.
    */
  CTextureFile* GetTexture(const char* TextureName);

  /// Get the number of all contained entities
  size_t         GetNumEntities()     {return m_Entities.Length();}

  /// Get the specified entity
  CMapEntity* GetEntity(size_t index) {return m_Entities.Get(index);}

  /// Get the number of the contained planes
  size_t         GetPlaneCount()      {return m_Planes.Length();}

  /// Get the specified plane
  CMapTexturedPlane* GetPlane(size_t index) {return m_Planes.Get(index);}

  /// Get the total number of brushes in this map
  size_t GetNumBrushes() {return m_NumBrushes;}

  /// Get a pointer to the Config File
  csConfigFile* GetConfigFile() {return m_pConfigFile;}

  CTextureManager* GetTextureManager() {return &m_TextureManager;}

  /**
    * Get an integer value from the configuration instance.
    * (Store the default if the key is not found)
    */
  int   GetConfigInt   (const char *Path, int def = 0);

  /**
    * Get a real value from the configuration instance.
    * (Store the default if the key is not found)
    */
  double GetConfigFloat (const char *Path, double def = 0.0);

  /**
    * Get a string value from the configuration instance.
    * (Store the default if the key is not found)
    */
  const char *GetConfigStr (const char *Path, const char *def = "");

protected:
  /**
    * Look for the texture with the given "name" in all given paths.
    * returns true, if the file is found, else return false. If it
    * returns true, then the fully qualified name is copied to
    * fullname
    */
  bool FindTextureFile(const char* name, char* fullname);

  /**
    * Add a CMapTexturedPlane object to the Map. (If a similar plane already exists,
    * the given plane is deleted!)
    */
  CMapTexturedPlane* AddPlane(CMapTexturedPlane* pNewPlane);
protected:
  /**
    * Here all the possible planes are stored. Note, that this will be
    * all unique planes.
    */
  CMapTexturedPlaneVector m_Planes;

  /**
    * Here all entities are being stored.
    */
  CMapEntityVector        m_Entities;

  /**
    * The texture manager takes care of texture information
    */
  CTextureManager         m_TextureManager;

  /**
    * This is the configuration file, that will control the conversion
    * process.
    */
  csConfigFile* m_pConfigFile;

  ///remember the name of the Inifile here
  char* m_IniFilename;

  /// For statistics: How many Brushes are in this map.
  size_t m_NumBrushes;
};

#endif // __MAP_H__

