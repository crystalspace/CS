/*
    The Crystal Space geometry loader interface
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __IMAP_PARSER_H__
#define __IMAP_PARSER_H__

#include "isys/plugin.h"
#include "ivideo/txtmgr.h"

struct iImage;
struct iTextureHandle;
struct iTextureWrapper;
struct iSoundData;
struct iSoundHandle;
class csSoundDataObject;
class csMeshFactoryWrapper;
class csMeshWrapper;

SCF_VERSION (iLoader, 0, 0, 1);

/**
 * This is a common interface for geometry loaders.
 * It provides methods for loading entire maps as well as separate models.
 */
struct iLoader : public iPlugIn
{
  /**
   * Load the given file (from VFS) into engine.
   * This does not clear anything that could be already loaded there; call
   * ClearAll () if you need it.
   */
  virtual bool Load (const char *iName) = 0;

  /**
   * Parse a string and load the model/s.
   * It is not adviced to make heavy use of this feature; some loaders (e.g.
   * the standard one) can speed up loading by preparsing files and storing the
   * pre-tokenized data somewhere near the original file; if you use this
   * method you will always go through the tokenizer first.  The caller should
   * be prepared for iData to be modified.  This is not always the case (for
   * example the standard loader just temporarily modifies it and restores
   * back) but in any case for performance reasons this memory could be used.
   */
  virtual bool Parse (char *iData) = 0;
};

SCF_VERSION (iLoaderNew, 0, 0, 1);
/**
 * This interface will replace iLoader somewhere in the future.
 */
struct iLoaderNew : public iPlugIn
{
  /// Set loader mode (see CS_LOADER_XXX flags above)
  virtual void SetMode (int iFlags) = 0;

  /// Load an image file
  virtual iImage *LoadImage (const char* Filename) = 0;
  /**
   * Load an image and create a texture handle from it. The 'Flags' parameter
   * accepts the flags described in ivideo/txtmgr.h.
   */
  virtual iTextureHandle *LoadTexture (const char* Filename,
    int Flags = CS_TEXTURE_3D) = 0;
  /**
   * Load a texture and register it with the engine. 'Name' is the name that
   * the engine will use for the wrapper. The 'Flags' parameter accepts the
   * flags described in ivideo/txtmgr.h. This function also creates a
   * material for the texture.
   */
  virtual iTextureWrapper *LoadTexture (const char *Name,
    const char *FileName, int Flags = CS_TEXTURE_3D) = 0;

  /// Load a sound file and return an iSoundData object
  virtual iSoundData *LoadSoundData (const char *fname) = 0;
  /// Load a sound file and register the sound
  virtual iSoundHandle *LoadSound (const char *fname) = 0;
  /// Load a sound file, register the sound and create a wrapper object for it
  virtual csSoundDataObject *LoadSound (const char *name, const char *fname) = 0;

  /// Load map file into engine.
  virtual bool LoadMapFile (const char* filename) = 0;
  /**
   * Merge map file into engine (i.e. don't clear the current engine
   * contents first). If 'onlyRegion' is true then portals will only
   * connect to the sectors in the current region, things will only use
   * thing templates defined in the current region and meshes will
   * only use mesh factories defined in the current region.
   */
  virtual bool AppendMapFile (const char* filename, bool onlyRegion = true) = 0;
  /// Load library from a VFS file
  virtual bool LoadLibraryFile (const char* filename) = 0;

  /// Load a Mesh Object Factory from the map file.
  virtual csMeshFactoryWrapper* LoadMeshObjectFactory (const char* fname) = 0;
  /**
   * Load a mesh object from a file.
   * The mesh object is not automatically added to the engine and sector.
   */
  virtual csMeshWrapper* LoadMeshObject (const char* fname) = 0;
};

#endif // __IMAP_PARSER_H__
