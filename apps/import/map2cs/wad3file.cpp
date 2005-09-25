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
#include "csutil/csendian.h"
#include "csgfx/memimage.h"
#include "csgfx/rgbpixel.h"
#include "igraphic/imageio.h"
#include "csutil/csstring.h"
#include "iutil/databuff.h"
#include "mapstd.h"
#include "wad3file.h"
#include "texfile.h"

struct S_Lumpinfo
{
  int  filepos;
  int  disksize;
  int  size;          // uncompressed
  char type;
  char compression;
  char pad1, pad2;
  char name[16];      // must be null terminated
};

int LittleEndian(int l)
{
  unsigned char* Buffer = (unsigned char*)(&l);
  return Buffer[0] | (Buffer[1] << 8) | (Buffer[2] << 16) | (Buffer[3] << 24);
}

void WriteByte(char*& p, unsigned char c)
{
  *((unsigned char*)p) = c;
  p+=1;
}

void WriteWord(char*& p, unsigned short s)
{
  csSetToAddress::UInt16 (p, csLittleEndian::UInt16 (s));
  p+=2;
}

void WriteDword(char*& p, unsigned long l)
{
  csSetToAddress::UInt32 (p, csLittleEndian::UInt32 (l));
  p+=4;
}

int CreateBitmap(char*&      p,
		 int&	     Size,
                 char*       pBits,
                 int         Width,
                 int         Height,
                 char*       pPalette)
{
  int i;

  int OutputWidth = ((Width + 3) & ~3);
  int NumBmpBytes = OutputWidth * Height;
  int NumPalBytes = 256 * 4 /* sizeof RGBQUAD*/;

  Size = 54 + NumBmpBytes + NumPalBytes;

  p = new char[Size];
  char* pBitmap = p;

  //BITMAPFILEHEADER
  WriteWord (pBitmap, 'B' + ('M' << 8));               //bfType
  WriteDword(pBitmap, Size);                           //bfSize
  WriteWord (pBitmap, 0);                              //bfReserved1
  WriteWord (pBitmap, 0);                              //bfReserved2
  WriteDword(pBitmap, 54 + NumPalBytes);               //bfOffBits

  //BITMAPINFOHEADER
  WriteDword(pBitmap, 40);                             //biSize
  WriteDword(pBitmap, OutputWidth);                    //biWidth
  WriteDword(pBitmap, Height);                         //biHeight
  WriteWord (pBitmap, 1);                              //biPlanes
  WriteWord (pBitmap, 8);                              //biBitCount
  WriteDword(pBitmap, 0);                              //biCompression
  WriteDword(pBitmap, 0);                              //biSizeImage
  WriteDword(pBitmap, 0);                              //biXPelsPerMeter
  WriteDword(pBitmap, 0);                              //biYPelsPerMeter
  WriteDword(pBitmap, 256);                            //biClrUsed
  WriteDword(pBitmap, 256);                            //biClrImportant

  //Palette (256*RGBQUAD)
  for (i = 0; i < 256; i++)
  {
    char r = pPalette[3*i+0];
    char g = pPalette[3*i+1];
    char b = pPalette[3*i+2];
    WriteByte(pBitmap, b);
    WriteByte(pBitmap, g);
    WriteByte(pBitmap, r);
    WriteByte(pBitmap, 0);
  }

  //Bitmap data
  for(i = 0; i < Height; i++)
  {
    //swap top and bottom.
    memcpy(pBitmap, &pBits[Width * (Height-i-1)], OutputWidth);
    pBitmap += OutputWidth;
  }

  return Size;
}

bool WriteBitmap(const char* BmpFilename,
                 char*       pBits,
                 int         Width,
                 int         Height,
                 char*       pPalette)
{
  //Open Outfile
  FILE* fd = fopen(BmpFilename, "wb");
  if (!fd) return false;

  char* p;
  int Size = CreateBitmap(p, Size, pBits, Width, Height, pPalette);

  bool ok = (fwrite(p, Size, 1, fd) != 1);

  fclose(fd);
  delete[] p;
  return ok;
}

CWad3File::CWad3File()
{
  m_Filehandle = 0;
  m_Lumpinfo   = 0;
  m_Numlumps   = 0;
}

CWad3File::~CWad3File()
{
  if (m_Filehandle)
  {
    fclose(m_Filehandle);
  }
  delete []m_Lumpinfo;
}

extern iImageIO* ImageLoader;

bool CWad3File::Extract(const char* Texture, char*& Data, int& Size, csString& fn)
{
  miptex_t TexInfo;
  int      LumpNr;
  if (GetQtexInfo(Texture, &TexInfo, &LumpNr))
  {
    Seek(m_Lumpinfo[LumpNr].filepos);
    char* Buffer = new char[m_Lumpinfo[LumpNr].size];

    Read(Buffer, m_Lumpinfo[LumpNr].size);

    uint8 *wadpal = (uint8*)Buffer + TexInfo.offsets[3] + TexInfo.width * TexInfo.height / 64 + 2;
    csRGBpixel pal[256];

    int i;
    for (i=0;i<256;i++)
    {
      pal[i].red = *wadpal++;
      pal[i].green = *wadpal++;
      pal[i].blue = *wadpal++;
    }

    // try to compress the texture into a PNG first
    csImageMemory mi (TexInfo.width, TexInfo.height, 
      (void*)((uint8*)Buffer + TexInfo.offsets[0]),
      false, CS_IMGFMT_PALETTED8, pal);
    // set the keycolor
    if (*Texture == '{')
    {
      // arg. some textures around specify their transparency color
      // in pal index 255, some others uses blue, but it isn't on
      // index 255. IIRC HL wants blue AND index 255.
      // Anyway, we seek the pal for bright blue. If found: our 
      // transparency color. If not: take index 255.
      has_keycolor = true;

      bool found_blue = false;
      for (i = 0; !found_blue && (i < 256); i++)
      {
	if ((pal[i].red == 0) && (pal[i].green == 0) && 
	  (pal[i].blue == 255))
	{
	  found_blue = true;
	}
      }

      if (found_blue)
      {
	keycolor_r = 0;
	keycolor_g = 0;
	keycolor_b = 255;
      }
      else
      {
        keycolor_r = pal[255].red;
        keycolor_g = pal[255].green;
        keycolor_b = pal[255].blue;
      }
      mi.SetKeycolor (keycolor_r, keycolor_g, keycolor_b);
    }
    else
      has_keycolor = false;
    csRef<iDataBuffer> db = ImageLoader->Save (&mi, "image/png", "compress=100");
    if (db)
    {
      Size = (int)db->GetSize();
      Data = new char[Size];
      memcpy (Data, db->GetData(), Size);
      db = 0;
      fn.Format ("%s.png", Texture);
    }
    else
    {
      // if this fails, fall back to BMP
      char* pBitmap = Buffer + TexInfo.offsets[0];

      Size = CreateBitmap(Data, Size, pBitmap,
                         TexInfo.width, TexInfo.height,
                         Buffer + TexInfo.offsets[3] + TexInfo.width * TexInfo.height / 64 + 2 );

      fn.Format ("%s.bmp", Texture);
    }
    delete [] Buffer;
   
    return true;
  }
  else
  {
    return false;
  }
}

bool CWad3File::ExtractToFile(const char* texturename)
{
  char* pData;
  int   Size;
  csString fn;
  if (Extract(texturename, pData, Size, fn))
  {
    FILE* fd = fopen(fn, "wb");
    if (!fd) return false;

    bool ok = (fwrite(pData, Size, 1, fd) != 1);

    fclose(fd);
    delete[] pData;
    return ok;
  }
  else
  {
    return false;
  }
}

CTextureFile* CWad3File::CreateTexture(const char* texturename)
{
  CTextureFile* pTexture = 0;
  char*         pData    = 0;
  int           Size     = 0;
  csString fn;

  if (Extract(texturename, pData, Size, fn))
  {
    pTexture = new CTextureFile;

    pTexture->SetTexturename (texturename);
    pTexture->SetFilename    (fn);
    pTexture->SetOriginalData(pData, Size);
    if (has_keycolor)
    {
      pTexture->SetKeyColor (keycolor_r / 255.0f, keycolor_g / 255.0f, 
	keycolor_b / 255.0f);
    }

    miptex_t Info;
    if (GetQtexInfo(texturename, &Info))
    {
      pTexture->SetOriginalSize(Info.width, Info.height);
    }
  }

  return pTexture;
}

bool CWad3File::Open(const char *filename)
{
  m_Filehandle = fopen(filename, "rb");
  if (!m_Filehandle)
  {
    csPrintf("Can't open %s!\n", filename);
    return false;
  }

  // First data in the Wad is a magic token, that identifies the WAD3 format
  int FileID;
  if (!Read(FileID)) return false;
  if (FileID != 0x33444157)
  {
    csPrintf("%s is not a valid WAD3 file!\n", filename);
    return false;
  }

  //The next info ist the number of lumps in the WAD3 file.
  if (!Read(m_Numlumps)) return false;

  //Allocate all needed lumps
  m_Lumpinfo = new S_Lumpinfo[m_Numlumps];

  //The next Info is the fileoffset, where the Infotable for the
  //lumps can be found.
  int InfotableOffset;
  if (!Read(InfotableOffset)) return false;

  //Seek to this table
  Seek(InfotableOffset);

  //Read all lumpinfo
  Read(m_Lumpinfo, m_Numlumps*sizeof(S_Lumpinfo));

  // Convert pos and size to proper endian format.
  int i;
  for (i=0 ; i<m_Numlumps ; i++)
  {
    m_Lumpinfo[i].filepos = LittleEndian(m_Lumpinfo[i].filepos);
    m_Lumpinfo[i].size    = LittleEndian(m_Lumpinfo[i].size);
  }

  return true;
}

bool CWad3File::GetQtexInfo(const char* Texture, miptex_t* pInfo, int* LumpNr)
{
  if (!m_Filehandle) return false;

  int i, level;
  for (i = 0; i < m_Numlumps; i++)
  {
    if (strcasecmp( m_Lumpinfo[i].name, Texture) == 0)
    {
      Seek(m_Lumpinfo[i].filepos);
      if (LumpNr)
      {
        *LumpNr = i;
      }

      Read(pInfo, sizeof (miptex_t));

      pInfo->width   = LittleEndian(pInfo->width);
      pInfo->height  = LittleEndian(pInfo->height);

      for (level = 0; level<MIPLEVELS; level++)
      {
        pInfo->offsets[level] = LittleEndian(pInfo->offsets[level]);
      }
      return true;
    }
  }

  return false;
}

bool CWad3File::Seek(int Pos)
{
  if (!m_Filehandle)
  {
    return false;
  }

  if (fseek (m_Filehandle, Pos, SEEK_SET) == 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool CWad3File::Read(void* Buffer, int count)
{
  if (!m_Filehandle)
  {
    return false;
  }

  if ((int)fread(Buffer, 1, count, m_Filehandle) != count)
  {
    return false;
  }

  return true;
}

bool CWad3File::Read(int& Val)
{
  if (Read(&Val, 4))
  {
    Val = LittleEndian(Val);
    return true;
  }
  return false;
}

