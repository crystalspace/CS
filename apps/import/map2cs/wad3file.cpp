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
#include "cssys/csendian.h"
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
  set_le_short ((void*)p, s);
  p+=2;
}

void WriteDword(char*& p, unsigned long l)
{
  set_le_long ((void*)p, l);
  p+=4;
}

int CreateBitmap(char*&      p,
                 char*       pBits, 
                 int         Width, 
                 int         Height, 
                 char*       pPalette)
{
  int i;

  int OutputWidth = ((Width + 3) & ~3);
  int NumBmpBytes = OutputWidth * Height;
  int NumPalBytes = 256 * 4 /* sizeof RGBQUAD*/;

  int Size = 54 + NumBmpBytes + NumPalBytes;

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
  int Size = CreateBitmap(p, pBits, Width, Height, pPalette);

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

bool CWad3File::Extract(const char* Texture, char*& Data, int& Size)
{
  miptex_t TexInfo;
  int      LumpNr;
  if (GetQtexInfo(Texture, &TexInfo, &LumpNr))
  {
    Seek(m_Lumpinfo[LumpNr].filepos);
    char* Buffer = new char[m_Lumpinfo[LumpNr].size];

    Read(Buffer, m_Lumpinfo[LumpNr].size);

    char* pBitmap = Buffer + TexInfo.offsets[0];

    Size = CreateBitmap(Data, pBitmap, 
                         TexInfo.width, TexInfo.height, 
                         Buffer + TexInfo.offsets[3] + TexInfo.width * TexInfo.height / 64 + 2 );

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
  if (Extract(texturename, pData, Size))
  {
    char texfilename[256];
    sprintf (texfilename, "%s.bmp", texturename);

    FILE* fd = fopen(texfilename, "wb");
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
  CTextureFile* pTexture = NULL;
  char*         pData    = NULL;
  int           Size     = 0;

  if (Extract(texturename, pData, Size))
  {
    pTexture = new CTextureFile;

    char texfilename[256];
    sprintf (texfilename, "%s.bmp", texturename);

    pTexture->SetTexturename (texturename);
    pTexture->SetFilename    (texfilename);
    pTexture->SetOriginalData(pData, Size);

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
    printf("Can't open %s!\n", filename);
    return false;
  }
  
  // First data in the Wad is a magic token, that identifies the WAD3 format
  int FileID;
  if (!Read(FileID)) return false;
  if (FileID != 0x33444157)
  {
    printf("%s is not a valid WAD3 file!\n", filename);
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

