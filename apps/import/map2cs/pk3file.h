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

#ifndef __PK3FILE_H__
#define __PK3FILE_H__

#include "contain.h"
#include "ziparch.h"


/**
  *
  */
class CPk3File : public CZipArchive
{
public:

  /**
    * Extracts a texture from a pk3 file to a newly created TextureFile
    */
  virtual CTextureFile* CreateTexture(const char* texturename);
protected:
};

#endif // __PK3FILE_H__
