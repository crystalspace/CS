/*
    Crystal Space Windowing System: background class
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_CSBACKGR_H__
#define __CS_CSBACKGR_H__

/**\file
 * Crystal Space Windowing System: background class
 */

/**
 * \addtogroup csws
 * @{ */
 
#include "csextern.h"
 
#include "csgfx/rgbpixel.h"

struct iTextureHandle;
class csApp;
class csComponent;

/// Background type
enum csBackgroundType
{
  /// No background
  csbgNone = 0,
  /// Single-color background
  csbgColor,
  /// Gradient background
  csbgGradient,
  /// Textured background
  csbgTextured
};

/**
 * This class is meant for displaying textured or gradiented backgrounds.
 * This is not a component; it is mainly meant for use by skins.
 */
class CS_CRYSTALSPACE_EXPORT csBackground
{
  /// Background type
  csBackgroundType type;
  /// The background texture (if not 0)
  iTextureHandle *tex;
  /// Gradient colors (for all four corners)
  csRGBcolor colors [4];
  /// The uniform background color (if type == csbgColor)
  int color;

public:
  /// Create a black background object
  csBackground ();

  /// Destroy the object
  ~csBackground ();

  /// Free the background
  void Free ()
  { SetTexture (0); }

  /// Set background texture
  void SetTexture (iTextureHandle *iTex);

  /// Query background texture
  iTextureHandle *GetTexture ()
  { return tex; }

  /// Set Nth (0-3) color of the background
  void SetColor (int iIndex, csRGBcolor &iColor)
  { colors [iIndex] = iColor; type = csbgGradient; }

  /// Get Nth (0-3) color of the background
  csRGBcolor &GetColor (int iIndex)
  { return colors [iIndex]; }

  /// Set the flat color of this background
  void SetColor (int iColor)
  { color = iColor; type = csbgColor; }

  /// Get the flat color of this background
  int GetColor ()
  { return color; }

  /// Set background type
  void SetType (csBackgroundType iType)
  { type = iType; }

  /// Query background type
  csBackgroundType GetType ()
  { return type; }

  /// Draw the background
  void Draw (csComponent &This, int x, int y, int w, int h,
    int xorg, int yorg, uint8 iAlpha);
};

/** @} */

#endif // __CS_CSBACKGR_H__
