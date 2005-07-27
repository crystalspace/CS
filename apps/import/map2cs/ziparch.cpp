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

#include "cssysdef.h"

#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "csutil/databuf.h"

#include "mapstd.h"
#include "texfile.h"
#include "ziparch.h"
#include "zipfile.h"

CZipArchive::CZipArchive()
{
  m_pZipFile = 0;
}

CZipArchive::~CZipArchive()
{
  delete m_pZipFile;
}


CTextureFile* CZipArchive::CreateTexture(const char* texturename)
{
  csString texfilename;
  CTextureFile* pTexture = 0;

  const char* Extensions[] = {"tga", "jpg", "jpeg", "bmp", "wal", "gif", 
    "png", "dds"};

  int i;
  for (i=0; i<int(sizeof(Extensions)/sizeof(Extensions[0])); i++)
  {
    texfilename.Format ("%s.%s", texturename, Extensions[i]);
    pTexture = ExtractTexture(texturename, texfilename);
    if (pTexture) return pTexture;
  }

  return 0;
}

extern iImageIO* ImageLoader;

CTextureFile* CZipArchive::ExtractTexture(const char* texturename, const char* texfilename)
{
  CTextureFile* pTexture = 0;
  CBinaryData   TextureData;

  if (m_pZipFile->ExtractData(&TextureData, texfilename))
  {
    pTexture = new CTextureFile;

    pTexture->SetTexturename (texturename);
    pTexture->SetFilename    (texfilename);
    pTexture->SetOriginalData(TextureData.GetData(), TextureData.GetSize());

    csRef<iDataBuffer> buf;
    buf.AttachNew (new csDataBuffer (TextureData.GetData(), 
      TextureData.GetSize(), false));
    csRef<iImage> img (ImageLoader->Load (buf, CS_IMGFMT_ANY));
    if (img)
    {
      pTexture->SetOriginalSize (img->GetWidth(), img->GetHeight());
    }
    else
      pTexture->SetOriginalSize(256, 256);
  }

  return pTexture;
}


bool CZipArchive::Open(const char *filename)
{
  m_pZipFile = new CZipFile(filename);
  return true;
}


