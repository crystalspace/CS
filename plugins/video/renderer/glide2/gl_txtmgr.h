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

#ifndef TXTMGR_GLIDE_H
#define TXTMGR_GLIDE_H

#include "csutil/scf.h"
#include "cs3d/common/txtmgr.h"
#include "itexture.h"

class csTextureMMGlide;
class csTextureManagerGlide;
struct iImage;

// Colors are encoded in a 16-bit short using the following
// distribution (only for 8-bit mode):
#define BITS_RED 6
#define BITS_GREEN 6
#define BITS_BLUE 4
#define MASK_RED ((1<<BITS_RED)-1)
#define MASK_GREEN ((1<<BITS_GREEN)-1)
#define MASK_BLUE ((1<<BITS_BLUE)-1)
#define NUM_RED (1<<BITS_RED)
#define NUM_GREEN (1<<BITS_GREEN)
#define NUM_BLUE (1<<BITS_BLUE)

#define TABLE_RED	0
#define TABLE_GREEN	1
#define TABLE_BLUE	2
#define TABLE_RED_HI	3
#define TABLE_GREEN_HI	4
#define TABLE_BLUE_HI	5

typedef UShort RGB16map[256];
typedef unsigned char RGB8map[256];

/// The prefered distances to use for the color matching.
#define PREFERED_DIST 16333
#define PREFERED_COL_DIST 133333

#define R24(rgb) (((rgb)>>16)&0xff)
#define G24(rgb) (((rgb)>>8)&0xff)
#define B24(rgb) ((rgb)&0xff)

/**
 * Lookup table entry corresponding to one palette entry.
 * 'red', 'green', and 'blue' are tables giving the red, green,
 * and blue components for all light levels of that palette index.
 */
struct PalIdxLookup
{
  RGB16map red;
  RGB16map green;
  RGB16map blue;
};

#define R24(rgb) (((rgb)>>16)&0xff)
#define G24(rgb) (((rgb)>>8)&0xff)
#define B24(rgb) ((rgb)&0xff)

/**
 * csTextureMMGlide represents a texture and all its mipmapped
 * variants.
 */
class csTextureMMGlide : public csTextureMM
{
private:
  /// Convert ImageFile to internal format.
  virtual void convert_to_internal (csTextureManager* texman, iImage* imfile, unsigned char* bm);

public:
  ///
  csTextureMMGlide (iImage* image, int flags);
  ///
  virtual ~csTextureMMGlide ();

  /**
   * This function does not really remap but it converts
   * the format to an ULong format suitable for 24-bit
   * internal texture format.
   */
  virtual void remap_palette_24bit (csTextureManager* texman);
};

/**
 * Glide version of the texture manager. This
 */
class csTextureManagerGlide : public csTextureManager
{
private:
  /**
   * Encode RGB values to a 16-bit word (for 16-bit mode).
   */
  ULong encode_rgb (int r, int g, int b);

public:
  ///
  csTextureManagerGlide (iSystem* iSys, iGraphics2D* iG2D, csIniFile *config);
  ///
  virtual ~csTextureManagerGlide ();

  ///
  virtual void PrepareTextures ();
  ///
  virtual iTextureHandle *RegisterTexture (iImage* image, int flags);
  ///
  virtual void PrepareTexture (iTextureHandle *handle);
  ///
  virtual void UnregisterTexture (iTextureHandle* handle);

  /**
   * Remap all textures.
   */
  void remap_textures ();
};


#endif // TXTMGR_OPENGL_H

