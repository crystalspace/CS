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
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "igraphic/imageio.h"
#include "iutil/vfs.h"
#include "csutil/databuf.h"

static const char* InvisibleTextures[] =
{
 //"common_areaportal",
 "common_caulk",
 "common_clip",
 //"common_clusterportal",
 "common_cushion",
 "common_donotenter",
 "common_fullclip",
 "common_hint",
 "common_invisible",
 "common_missileclip",
 "common_nodraw",
 "common_nodrawnonsolid",
 "common_nodrop",
 "common_noimpact",
 "common_nolightmap",
 "common_origin",
 "common_portal",
 "common_qer_portal",
 "common_skip",
 "common_slick",
 //"common_teleporter",
 "common_trigger",
 "common_weapclip",
 "clip",
 "aaatrigger",
 "origin"
};

static const char* UnstoredTextures[] =
{
  "sky"
};

CTextureFile::CTextureFile()
{
  m_OriginalWidth  = 1;
  m_OriginalHeight = 1;
  m_Visible        = false;
  m_ColorKeyed     = false;
  m_Mipmapped      = true;
  m_R              = 0.0f;
  m_G              = 0.0f;
  m_B              = 0.0f;
  m_Stored	   = false;
}

CTextureFile::~CTextureFile()
{
}

extern iImageIO* ImageLoader;

void CTextureFile::SetOriginalData(char* Data, int Size)
{
  m_OriginalData.SetData(Data, Size);
  if (Data && Size && ImageLoader)
  {
    csRef<iDataBuffer> buf;
    buf.AttachNew (new csDataBuffer (Data, Size, false));
    csRef<iImage> ifile (ImageLoader->Load (buf, CS_IMGFMT_TRUECOLOR));
    if (ifile)
    {
      m_OriginalWidth  = ifile->GetWidth();
      m_OriginalHeight = ifile->GetHeight();
    }
  }
}

bool CTextureFile::AddToVFS(csRef<iVFS> VFS, const char* path)
{
  if (m_OriginalData.GetSize() == 0) return true;
  VFS->PushDir();
  VFS->ChDir (path);
  bool res = false;
  if (VFS->Exists (m_Filename))
  {
    size_t vfssize;
    VFS->GetFileSize (m_Filename, vfssize);
    if (vfssize == (size_t) m_OriginalData.GetSize())
    {
      res = true;
    }
    else
    {
      printf ("Texture %s: file %s already exists, but has different size (%zu != %d).\n"
	"Maybe a texture with this name exists in multiple archives?\n",
	(const char*) m_Texturename, 
	(const char*) m_Filename, vfssize, m_OriginalData.GetSize());
    }
  }
  else
  {
    VFS->WriteFile (m_Filename, (char*)m_OriginalData.GetData(),
      m_OriginalData.GetSize());
    res = true;
  }
  VFS->PopDir ();
  return res;
//  return pZipfile->AddData(&m_OriginalData, m_Filename);
}

void CTextureFile::SetTexturename(const char* name)
{
  m_Texturename = name;

  m_Visible = true;
  int i;
  for (i=0; i<int(sizeof(InvisibleTextures)/sizeof(InvisibleTextures[0])); i++)
  {
    if (strcasecmp (m_Texturename, InvisibleTextures[i])==0)
    {
      m_Visible = false;
      break;
    }
  }

  if (m_Visible)
  {
    m_Stored = true;
    int i;
    for (i=0; i<int(sizeof(UnstoredTextures)/sizeof(UnstoredTextures[0])); i++)
    {
      if (strcasecmp(m_Texturename, UnstoredTextures[i])==0)
      {
	m_Stored = false;
	break;
      }
    }
  }
  else
    m_Stored = false;
}

bool CTextureFile::IsVisible()
{
  return m_Visible;
}


