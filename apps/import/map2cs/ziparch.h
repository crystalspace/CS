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

#ifndef __ZIPARCH_H__
#define __ZIPARCH_H__

#include "contain.h"
#include "texarch.h"

class CZipFile;

/**
  *
  */
class CZipArchive : public CTextureArchive
{
public:
  /**
    *
    */
  CZipArchive();

  /**
    * Destructor, will do cleanup.
    */
  virtual ~CZipArchive();

  /**
    * Opens a Zip Archive
    */
  virtual bool Open(const char* filename);

  /**
    * Extracts a texture from a wad file to a newly created TextureFile
    */
  virtual CTextureFile* CreateTexture(const char* texturename);

protected:
  CTextureFile* ExtractTexture(const char* texturename, const char* texfilename);

  CZipFile* m_pZipFile;
};

#endif // __ZIPARCH_H__
