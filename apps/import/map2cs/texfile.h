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

#ifndef __TEXFILE_H__
#define __TEXFILE_H__

#include "bindata.h"
#include "csgfx/csimage.h"
#include "csutil/ref.h"

class CMapFile;
class CZipFile;
struct iVFS;

/**
  *
  */
class CTextureFile
{
public:
  /**
    *
    */
  CTextureFile();

  /// The destructor, will do some cleanup, as usual.
  ~CTextureFile();

  /// Set the name of the texture
  void SetTexturename(const char* name);

  /// Get the name of the texture
  const char* GetTexturename() {return m_Texturename;}

  /// Set the filename of the texture (including extension)
  void SetFilename(const char* name) {m_Filename = name;}

  /// Get the filename of the texture (including extension)
  const char* GetFilename() {return m_Filename;}

  void SetOriginalSize(int w, int h)
    {m_OriginalWidth = w; m_OriginalHeight = h;}

  /// Get the original width of the texture (before scaling)
  int GetOriginalWidth()  {return m_OriginalWidth;}

  /// Get the original height of the texture (before scaling)
  int GetOriginalHeight() {return m_OriginalHeight;}

  /// Adds this texture to the given Zip-File.
  bool AddToVFS(csRef<iVFS> VFS, const char* path);

  /// Sets information about the original file
  void SetOriginalData(char* Data, int Size);

  /**
    * Returns false, if this Texture is some internal texture for Quake.
    * this is the case for clip or hint brushes for example
    */
  bool IsVisible();

  /// return true, if this texture is color keyed
  bool IsColorKeyed() {return m_ColorKeyed;}

  /// return the Keycolor
  void GetKeyColor(float& r, float& g, float& b) {r=m_R;g=m_G;b=m_B;}

  /// set the Keycolor
  void SetKeyColor(float r, float g, float b) {m_R=r;m_G=g;m_B=b;m_ColorKeyed=true;}

  /// return true, if this texture will be mipmapped
  bool IsMipmapped() {return m_Mipmapped;}

  /// decide, if this texture will be mipmapped
  void SetMipmapped(bool Mipmapped) {m_Mipmapped = Mipmapped;}

  /// set visible flag
  void SetVisible (bool visible) { m_Visible = visible; }

  /// store this texture in zip?
  void SetStored (bool stored) { m_Stored = stored; }

  /// return true if this texture will be stored in zip
  bool IsStored() {return m_Stored;}

protected:
  /**
    * The name of the texture, without filetype extension or references to
    * a path on the local HD. It _can_ contain relative paths like in Q3A,
    * where a texture name could be "gothic_base/wall_012_fragment"
    */
  csString m_Texturename;

  /**
    * The name, that will be used in the map file. It contains the filetype
    * as extension, but it will not contain paths, because we are going to use
    * a flat structure in CS map files. This name can be related to the original
    * Texture name, but it could well be a name like "tex00001.jpg"
    */
  csString m_Filename;

  /**
    * Here is the original Data being stored. This is a simple copy of the file
    * on Disk. It can be added to the Worldfile, if no further processing like
    * scaling is required.
    */
  CBinaryData m_OriginalData;

  /**
    * The texture as a useable image.
    */
  csImageFile* m_pImage;

  /// the original width of the texture (before scaling)
  int m_OriginalWidth;

  /// the original height of the texture (before scaling)
  int m_OriginalHeight;

  /// false, if this texture is not to be displayed
  bool m_Visible;

  /// true, if this texture has a keycolor
  bool m_ColorKeyed;

  /// true, if this texture is mipmapped
  bool m_Mipmapped;

  /// The Keycolor if m_ColorKeyed is true;
  float m_R, m_G, m_B;

  /// store on disk?
  bool m_Stored;
};

#endif // __TEXFILE_H__

