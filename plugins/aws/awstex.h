/*
    Copyright (C) 2001 by Christopher Nelson
  
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

#ifndef __CS_AWS_TEX_H__
#define __CS_AWS_TEX_H__

#include "csutil/parray.h"
#include "igraphic/imageio.h"
#include "iutil/strset.h"
#include "iutil/vfs.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"

struct iObjectRegistry;

/**
 * This class embeds a normal texture manager, and keeps track of all the
 * textures currently in use by the windowing system.  This includes
 * bitmaps for buttons, etc.  When the skin changes, it unloads all the
 * skin textures currently being used.  Then it is ready to demand-load
 * new ones.
 */
 
class awsTextureManager
{
private:
  /// This contains a reference to our loader.
  csRef<iImageIO> loader;

  /// This contains a reference to our texture manager.
  csRef<iTextureManager> txtmgr;

  /// This contains a reference to the VFS plugin.
  csRef<iVFS> vfs;

  /// Contains a reference to the object registry.
  iObjectRegistry *object_reg;

  /// Shared string table.
  csRef<iStringSet> strset;

  struct awsTexture
  {
    ~awsTexture();
    csRef<iImage> img;
    csRef<iTextureHandle> tex;
    unsigned long id;
  };

  /// List of textures loaded.
  csPDelArray<awsTexture> textures;

  /// Registers all currently loaded textures with the texture manager.
  void RegisterTextures ();

  /// Unregisters all currently loaded textures with the texture manager.
  void UnregisterTextures ();

  /// Translate name to ID.
  unsigned long NameToId (const char*) const;
public:
  /// Empty constructor.
  awsTextureManager ();

  /// Destructor.
  ~awsTextureManager ();

  /// Get a reference to and iLoader.
  void Initialize (iObjectRegistry *object_reg);

  /**
   * Get a texture.  If the texture is already cached, it returns
   * the cached texture. If the texture has not been cached, and a
   * filename is specified, the file is loaded. If the file cannot be
   * found, or no file was specified, 0 is returned.
   */
  iTextureHandle *GetTexture (
    const char *name,
    const char *filename = 0,
    bool replace = false,
    unsigned char key_r = 255,
    unsigned char key_g = 0,
    unsigned char key_b = 255);

  /**
   * Get a texture.  If the texture is already cached, it returns the
   * cached texture. If the texture has not been cached, and a filename
   * is specified, the file is loaded. If the file cannot be found, or no
   * file was specified, 0 is returned. This variety uses the id directly,
   * in case you have it.  Mostly used internally by AWSPrefManager.
   */
  iTextureHandle *GetTexturebyID (
    unsigned long id,
    const char *filename = 0,
    bool replace = false,
    unsigned char key_r = 255,
    unsigned char key_g = 0,
    unsigned char key_b = 255);

  /**
   * Changes the texture manager: unregisters all current textures, and
   * then re-registers them with the new manager.
   */
  void SetTextureManager (iTextureManager *txtmgr);

  /// Retrieves the texture manager that we are currently using.
  iTextureManager *GetTextureManager () { return txtmgr; }
};

#endif // __CS_AWS_TEX_H__
