/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
  
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef TGAIMAGE_H
#define TGAIMAGE_H

#include "csgfxldr/csimage.h"

class TGAImageLoader;

/**
 * An csImageFile subclass for reading TGA files.
 */
class ImageTgaFile : public csImageFile
{
  ///
  friend class csImageFile;	// For constructor
  friend class TGAImageLoader;

private:
  /// Read the TGA file from the buffer.
  ImageTgaFile (UByte* buf, long size);

public:
  ///
  virtual ~ImageTgaFile ();
};

/**
 * The TGA Image Loader.
 */
class TGAImageLoader : public ImageLoader
{
protected:
  ///
  virtual csImageFile* LoadImage (UByte* buf, ULong size);
  virtual AlphaMapFile* LoadAlphaMap(UByte *buffer,ULong size);
public:
  ///
  virtual const char* GetName() const
  { return "TARGA"; }
  ///
  virtual const char* GetDescription() const 
  { return "TARGA image format"; }
};

/* Header definition. */
struct ImageHeader {
    unsigned char IDLength;		/* length of Identifier String */
    unsigned char CoMapType;		/* 0 = no map */
    unsigned char ImgType;		/* image type (see below for values) */
    unsigned char Index_lo, Index_hi;	/* index of first color map entry */
    unsigned char Length_lo, Length_hi;	/* number of entries in color map */
    unsigned char CoSize;		/* size of color map entry (15,16,24,32) */
    unsigned char X_org_lo, X_org_hi;	/* x origin of image */
    unsigned char Y_org_lo, Y_org_hi;	/* y origin of image */
    unsigned char Width_lo, Width_hi;	/* width of image */
    unsigned char Height_lo, Height_hi;	/* height of image */
    unsigned char PixelSize;		/* pixel size (8,16,24,32) */
    unsigned char AttBits;		/* 4 bits, number of attribute bits per pixel */
    unsigned char Rsrvd;		/* 1 bit, reserved */
    unsigned char OrgBit;		/* 1 bit, origin: 0=lower left, 1=upper left */
    unsigned char IntrLve;		/* 2 bits, interleaving flag */
    };

typedef char ImageIDField[256];

/* Definitions for image types. */
#define TGA_Null 0
#define TGA_Map 1
#define TGA_RGB 2
#define TGA_Mono 3
#define TGA_RLEMap 9
#define TGA_RLERGB 10
#define TGA_RLEMono 11
#define TGA_CompMap 32
#define TGA_CompMap4 33

/* Definitions for interleave flag. */
#define TGA_IL_None 0
#define TGA_IL_Two 1
#define TGA_IL_Four 2

#endif

