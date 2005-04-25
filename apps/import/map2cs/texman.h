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

#ifndef __TEXMAN_H__
#define __TEXMAN_H__

#include "contain.h"
#include "csutil/ref.h"

class CMapFile;
class CZipFile;
class CTextureFile;
struct iVFS;

/**
  *
  */
class CTextureManager
{
public:
  /**
    *
    */
  CTextureManager();

  /// The destructor, will do some cleanup, as usual.
  ~CTextureManager();

  /// Load all the texture archive files, that are specified in the config file.
  void LoadTextureArchives(CMapFile* pMap);

  /// Load all the texture archive files, that are specified in the config file.
  void LoadArchive (const char* filename);

  /// Find the required Texture, and return a pointer to it.
  CTextureFile* GetTexture(const char* TextureName);

  /// Adds all textures to the given Zipfile
  bool AddAllTexturesToVFS(csRef<iVFS> VFS, const char* path);

  /// returns the Number of known textures
  size_t GetTextureCount() { return m_StoredTextures.Length(); }

  /// returns the n-th texture (from 0 to GetTextureCount()-1)
  CTextureFile* GetTexture (size_t index) {return m_StoredTextures[index];}

protected:
  void CleanupTexturename (csString& Name);

  /// A pointer to the global map.
  CMapFile* m_pMap;

  /**
    * Here are all textures stored, that have already been extracted.
    */
  CTextureFileVector      m_StoredTextures;

  /**
    * Here are all Texture Archives being stored.
    */
  CTextureArchiveVector   m_TextureArchives;
};

#endif // __TEXMAN_H__

