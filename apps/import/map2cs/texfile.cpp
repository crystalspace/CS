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
#include "mapstd.h"
#include "texfile.h"
#include "zipfile.h"
#include "isys/plugin.h"
#include "igraphic/imageio.h"

static const char* InvisibleTextures[] = 
{
 //"common/areaportal", 
 "common/caulk", 
 "common/clip", 
 //"common/clusterportal", 
 "common/cushion", 
 "common/donotenter", 
 "common/fullclip", 
 "common/hint", 
 "common/invisible", 
 "common/missileclip", 
 "common/nodraw", 
 "common/nodrawnonsolid", 
 "common/nodrop", 
 "common/noimpact", 
 "common/nolightmap", 
 "common/origin", 
 "common/portal", 
 "common/qer_portal", 
 "common/skip", 
 "common/slick", 
 //"common/teleporter", 
 "common/trigger", 
 "common/weapclip"
};

CTextureFile::CTextureFile()
{
  m_pImage         = NULL;
  m_OriginalWidth  = 1;
  m_OriginalHeight = 1;
  m_Visible        = false;
  m_ColorKeyed     = false;
  m_Mipmapped      = true;
  m_R              = 0.0f;
  m_G              = 0.0f;
  m_B              = 0.0f;
}

CTextureFile::~CTextureFile()
{
  delete m_pImage;
}

void CTextureFile::SetOriginalData(char* Data, int Size)
{
  // create the image loader. @@@ This is a quick hack that only works
  // because the image loader doesn't ever use the system driver!
  // If this changes then map2cs needs a system driver too!
  static bool IL_Loaded = false;
  static iImageIO *ImageLoader = NULL;
  if (!IL_Loaded)
  {
    IL_Loaded = true;

    ImageLoader = SCF_CREATE_INSTANCE("crystalspace.graphic.image.io.multiplex", iImageIO);
    if (ImageLoader)
    {
      iPlugin *Plugin = SCF_QUERY_INTERFACE (ImageLoader, iPlugin);
      if (!Plugin || !Plugin->Initialize(NULL))
      {
        ImageLoader->DecRef();
	ImageLoader = NULL;
      }
      Plugin->DecRef ();
    }
  }

  m_OriginalData.SetData(Data, Size);
  if (Data && Size && ImageLoader)
  {
    iImage* ifile = ImageLoader->Load ((UByte *) Data, Size, CS_IMGFMT_TRUECOLOR);
    if (ifile)
    {
      m_OriginalWidth  = ifile->GetWidth();
      m_OriginalHeight = ifile->GetHeight();
      ifile->DecRef();
    }
  }
}

bool CTextureFile::AddToZip(CZipFile* pZipfile)
{
  if (m_OriginalData.GetSize() == 0) return true;
  return pZipfile->AddData(&m_OriginalData, m_Filename);
}

void CTextureFile::SetTexturename(const char* name)
{
  m_Texturename = name;

  m_Visible = true;
  int i;
  for (i=0; i<int(sizeof(InvisibleTextures)/sizeof(InvisibleTextures[0])); i++)
  {
    if (strcmp(m_Texturename, InvisibleTextures[i])==0)
    {
      m_Visible = false;
    }
  }
}

bool CTextureFile::IsVisible()
{
  return m_Visible;
}


