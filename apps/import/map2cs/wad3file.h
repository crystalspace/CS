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

#ifndef __WAD3FILE_H__
#define __WAD3FILE_H__

#include "contain.h"
#include "texarch.h"

struct S_Lumpinfo;

const int MIPLEVELS=4;
struct miptex_t
{
  char     name[16];
  unsigned width, height;
  unsigned offsets[MIPLEVELS]; // four mip maps stored
};


/**
  *
  */
class CWad3File : public CTextureArchive
{
public:
  /**
    *
    */
  CWad3File();

  /**
    * Destructor, will do cleanup.
    */
  virtual ~CWad3File();

  /**
    * Opens a WAD file
    */
  virtual bool Open(const char* filename);

  /**
    * Extracts a bitmap to memory. You need to free Data by calling
    * delete [] Data afterwards!
    */
  bool Extract(const char* texturename, char*& Data, int& Size, csString& fn);

  /**
    * Extracts a texture from a wad file to a bmp file with the
    * same name. (.bmp is appended automatically)
    */
  bool ExtractToFile(const char* texturename);

  /**
    * Extracts a texture from a wad file to a newly created TextureFile
    */
  virtual CTextureFile* CreateTexture(const char* texturename);

  /**
    * Get Info about the given Texture. LumpNr can be 0 if you are not
    * interested in that info. Returns false, if no info about that texture
    * can be found.
    */
  bool GetQtexInfo(const char* Texture, miptex_t* pInfo, int* LumpNr=0);

protected:
  bool Seek(int Pos);
  bool Read(void* Buffer, int count);
  bool Read(int& Val);

  FILE*       m_Filehandle;
  S_Lumpinfo* m_Lumpinfo;
  int         m_Numlumps;

  bool has_keycolor;
  int keycolor_r, keycolor_g, keycolor_b;
};

#endif // __WAD3FILE_H__
