/*
    Crystal Space 3D engine: 2D pixmaps
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_CSPIXMAP_H__
#define __CS_CSPIXMAP_H__

#include "csextern.h"

#include "csutil/ref.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"

/// alignment settings for drawing pixmaps
#define PIXMAP_TOP      0
#define PIXMAP_LEFT     0
#define PIXMAP_CENTER   1
#define PIXMAP_BOTTOM   2
#define PIXMAP_RIGHT    2

/**
 * This class is an simple set of inline routines good as an abstraction
 * for simple 2D sprites. Pixmaps can be drawn with a transparent key color
 * as well as without transparent color: all this depends on texture handle.
 * Note that this class is only used as the interface to several
 * implementations of pixmaps, for example csSimplePixmap.
 */
class CS_CRYSTALSPACE_EXPORT csPixmap
{
public:
  /// Destructor
  virtual ~csPixmap() {}

  /// return current width of this pixmap
  virtual int Width() = 0;
  /// return current height of this pixmap
  virtual int Height() = 0;
  /// advance in time
  virtual void Advance(csTicks ElapsedTime) = 0;
  /// return current texture handle
  virtual iTextureHandle *GetTextureHandle() = 0;

  /// Draw the pixmap given the screen position and new size
  virtual void DrawScaled (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
    uint8 Alpha = 0) = 0;
  /// Draw the pixmap given the screen position and new size (aligned)
  inline void DrawScaledAlign (iGraphics3D* g3d, int sx, int sy, int sw,
      int sh, int alnx, int alny, uint8 Alpha = 0)
    { DrawScaled (g3d, sx - alnx * sw / 2, sy - alny * sh / 2, sw, sh, Alpha); }
  /// Draw the pixmap without rescale
  inline void Draw (iGraphics3D* g3d, int sx, int sy, uint8 Alpha = 0)
    { DrawScaled (g3d, sx, sy, Width(), Height(), Alpha); }
  /// Draw the pixmap without rescale (aligned)
  inline void DrawAlign (iGraphics3D* g3d, int sx, int sy, int alnx, int alny,
      uint8 Alpha = 0)
    { DrawScaledAlign (g3d, sx, sy, Width(), Height(), alnx, alny, Alpha); }

  /**
   * Draw the pixmap tiled over an area. multiple draw commands with the
   * same texture and same origin values will align properly.
   * The orgx and orgy point to a pixel (perhaps offscreen) where the
   * (0,0) pixel of this pixmap would be drawn.
   */
  virtual void DrawTiled (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
    int orgx, int orgy, uint8 Alpha = 0) = 0;
  /// Fill a rectangle with the pixmap, tiled.
  inline void DrawTiled (iGraphics3D* g3d, int sx, int sy, int w, int h,
      uint8 Alpha = 0)
  { DrawTiled (g3d, sx, sy, w, h, sx, sy, Alpha); }
};

/// This is the simple implementation of csPixmap that uses a single texture
class CS_CRYSTALSPACE_EXPORT csSimplePixmap : public csPixmap
{
protected:
  csRef<iTextureHandle> hTex;
  int tx, ty, tw, th;

public:
  /// Initialize the pixmap from a texture.
  csSimplePixmap (iTextureHandle *hTexture);
  /// Initialize the pixmap from a texture with given rectangle
  csSimplePixmap (iTextureHandle *hTexture, int x, int y, int w, int h);
  /// Deinitialize the pixmap
  virtual ~csSimplePixmap ();

  /// Change pixmap texture handle
  void SetTextureHandle (iTextureHandle *hTexture);
  /// Set new location of pixmap image on texture
  inline void SetTextureRectangle (int x, int y, int w, int h)
  { tx = x; ty = y; tw = w; th = h; }

  // implementation of csPixmap methods
  virtual void DrawScaled (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
    uint8 Alpha = 0);
  virtual void DrawTiled (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
    int orgx, int orgy, uint8 Alpha = 0);
  virtual int Width ();
  virtual int Height ();
  virtual void Advance(csTicks /*ElapsedTime*/) {}
  virtual iTextureHandle *GetTextureHandle ();
};

#endif // __CS_CSPIXMAP_H__
