/*
  Crystal Space Windowing System: 2D sprites
  Copyright (C) 1998 by Jorrit Tyberghein
  Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CSSPR2D_H__
#define __CSSPR2D_H__

#include "igraph2d.h"
#include "csutil/csbase.h"
#include "csengine/texture.h"

/**
 * This class is a abstraction for simple 2D sprites. Sprites
 * can be drawn with a transparent key color as well as without
 * transparent color. These sprites are slower relatively to
 * run-length encoded sprites (see csSprite2D_RLE) but has the
 * advantage that these sprites can be dynamically-scaled,
 * while RLE-encoded cannot.
 */
class csSprite2D : public csBase
{
private:
  csTextureHandle *hTex;
  int tx, ty, tw, th;

public:
  /// Initialize the sprite from a texture.
  csSprite2D (csTextureHandle *hTexture, int x, int y, int w, int h);

  /// Deinitialize the sprite
  virtual ~csSprite2D ();

  /// Return true if sprite has been initialized okay
  bool ok () { return hTex != NULL; }

  /// Set new location of sprite image on texture
  void SetTextureRectangle (int x, int y, int w, int h)
  { tx = x; ty = y; tw = w; th = h; }

  /// Draw the sprite given the screen position and size
  virtual void Draw (IGraphics2D* g2d, int sx, int sy, int sw, int sh);
  /// Draw the sprite without rescale
  void Draw (IGraphics2D* g2d, int sx, int sy) { Draw (g2d, sx, sy, tw, th); }

  /// Return sprite width
  int Width () { return tw; }
  /// Return sprite height
  int Height () { return th; }

  csTextureHandle *GetTextureHandle () { return hTex; }
};

#endif // __CSSPR2D_H__
