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

#ifndef TXTMGR_OPENGL_H
#define TXTMGR_OPENGL_H

#include "csutil/scf.h"
#include "cs3d/common/txtmgr.h"
#include "itexture.h"

struct iImageFile;

// Colors are encoded in a 16-bit short using the following
// distribution (only for 8-bit mode):
#define BITS_RED	6
#define BITS_GREEN	6
#define BITS_BLUE	4
#define MASK_RED	((1 << BITS_RED)   - 1)
#define MASK_GREEN	((1 << BITS_GREEN) - 1)
#define MASK_BLUE	((1 << BITS_BLUE)  - 1)
#define NUM_RED		(1 << BITS_RED)
#define NUM_GREEN	(1 << BITS_GREEN)
#define NUM_BLUE	(1 << BITS_BLUE)

#define TABLE_WHITE	0
#define TABLE_RED	1
#define TABLE_GREEN	2
#define TABLE_BLUE	3
#define TABLE_WHITE_HI	4
#define TABLE_RED_HI	5
#define TABLE_GREEN_HI	6
#define TABLE_BLUE_HI	7

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

class csTextureMMOpenGL : public csHardwareAcceleratedTextureMM
{
public:
  csTextureMMOpenGL(iImageFile* image) 
   : csHardwareAcceleratedTextureMM (image) {}
  ///
  virtual void remap_texture(csTextureManager *new_palette) 
    { remap_palette_24bit(new_palette); }
};

/**
 * OpenGL version of the texture manager. This
 */
class csTextureManagerOpenGL : public csTextureManager
{
private:
  int num_red, num_green, num_blue;
  /// The configuration file (duplicate! should not be freed)
  csIniFile* config;

  /// Configuration values for color matching.
  int prefered_dist;
  /// Configuration values for color matching.
  int prefered_col_dist;

  /// Read configuration values from config file.
  void read_config ();

  /**
   * Encode RGB values to a 16-bit word (for 16-bit mode).
   */
  ULong encode_rgb (int r, int g, int b);

  ///
  csTexture* get_texture (int idx, int lev);

  /**
   * Find rgb for a specific map type and apply an intensity.
   * 'map_type' is one of TABLE_....
   */
  int find_rgb_map (int r, int g, int b, int map_type, int l);

public:
  ///
  csTextureManagerOpenGL (iSystem* iSys, iGraphics2D* iG2D);
  ///
  virtual ~csTextureManagerOpenGL ();
  ///
  virtual void Initialize ();

  ///
  virtual void clear ();

  ///
  virtual void Prepare ();
  ///
  virtual iTextureHandle *RegisterTexture (iImageFile* image, bool for3d, bool for2d);
  ///
  virtual void UnregisterTexture (iTextureHandle* handle);
  ///
  virtual void MergeTexture (iTextureHandle* handle);
  ///
  virtual void FreeImages ();
  ///
  virtual void ReserveColor (int r, int g, int b);
  ///
  virtual void AllocPalette ();

  /// Create a new texture.
  csTextureMMOpenGL* new_texture (iImageFile* image);

  ///
  bool force_mixing (char* mix);

  /**
   * Return the index for some color. This works in 8-bit
   * (returns an index in the 256-color table) and in 15/16-bit
   * (returns a 15/16-bit encoded RGB value).
   */
  virtual int find_color (int r, int g, int b);

  /**
   * This version of find_rgb finds some r,g,b value AFTER gamma
   * correction is applied. This is useful for console messages
   * that always need the same color regardless of the gamma
   * correction.
   */
  int find_rgb_real (int r, int g, int b);

  /**
   * Remap all textures.
   */
  void remap_textures ();

  /// Set configuration file for use inside Initialize () call
  void SetConfig (csIniFile* newconfig)
  {
    config = newconfig;
  }
};

#endif // TXTMGR_OPENGL_H
