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

#ifndef __CSPIXMAP_H__
#define __CSPIXMAP_H__

#include "igraph3d.h"
#include "itexture.h"
#include "csutil/csbase.h"

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
 */
class csPixmap : public csBase
{
private:
  iTextureHandle *hTex;
  int tx, ty, tw, th;

public:
  /// Set new location of pixmap image on texture
  void SetTextureRectangle (int x, int y, int w, int h)
  { tx = x; ty = y; tw = w; th = h; }

  /// Initialize the pixmap from a texture.
  csPixmap (iTextureHandle *hTexture)
  {
    int w,h;
    hTex = hTexture;
    hTex->GetMipMapDimensions (0,w,h);
    SetTextureRectangle (0, 0, w, h);
  }

  /// Initialize the pixmap from a texture with given rectangle
  csPixmap (iTextureHandle *hTexture, int x, int y, int w, int h)
  {
    hTex = hTexture;
    SetTextureRectangle (x, y, w, h);
  }

  /// Deinitialize the pixmap
  virtual ~csPixmap ()
  {
  }

  /// Return true if pixmap has been initialized okay
  bool ok ()
  { return hTex != NULL; }

  /// Draw the pixmap given the screen position and new size
  virtual void DrawScaled (iGraphics3D* g3d, int sx, int sy, int sw, int sh)
  {
    if (hTex)
      g3d->DrawPixmap (hTex, sx, sy, sw, sh, tx, ty, tw, th);
  }

  /// Draw the pixmap given the screen position and new size (aligned)
  void DrawScaledAlign (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
        int alnx, int alny)
  {
     DrawScaled (g3d, sx-alnx*sw/2, sy-alny*sh/2, sw, sh);
  }

  /// Draw the pixmap without rescale
  void Draw (iGraphics3D* g3d, int sx, int sy)
  { DrawScaled (g3d, sx, sy, tw, th); }

  /// Draw the pixmap without rescale (aligned)
  void DrawAlign (iGraphics3D* g3d, int sx, int sy, int alnx, int alny)
  { DrawScaledAlign (g3d, sx, sy, tw, th, alnx, alny); }

  /** Draw the pixmap tiled over an area. multiple draw commands with the
    same texture and same shift values will align properly.
    The shiftx and shifty point to a pixel (perhaps offscreen) where the
    (0,0) pixel of this pixmap would be drawn.
    */
  void DrawTiledShifted(iGraphics3D* g3d, int sx, int sy, int w, int h,
    int shiftx, int shifty)
  {
    if(!hTex) return;
    while(shiftx<0) shiftx+=tw;
    while(shifty<0) shifty+=th;
    for(int my=-shifty; my < h; my += th)
      for(int mx=-shiftx; mx < w; mx += tw)
      {
        int px = sx + ((mx < 0)?0:mx);
        int py = sy + ((my < 0)?0:my);
	int mw = tw + ((mx+tw>w)?w-mx-tw:0) + ((mx<0)?mx:0);
	int mh = th + ((my+th>h)?h-my-th:0) + ((my<0)?my:0);
        g3d->DrawPixmap (hTex, px, py, mw, mh, 
	  tx + (mx<0?-mx:0), ty + (my<0?-my:0), mw, mh);
      }
  }

  /// Fill a rectangle with the pixmap, tiled.
  void DrawTiled(iGraphics3D* g3d, int sx, int sy, int w, int h)
  { DrawTiledShifted(g3d, sx, sy, w, h, sx, sy); }

  /// Return pixmap width
  int Width ()
  { return tw; }
  /// Return pixmap height
  int Height ()
  { return th; }

  /// Change pixmap texture handle
  void SetTextureHandle (iTextureHandle *hTexture)
  { hTex = hTexture; }
  /// Query pixmap texture handle
  iTextureHandle *GetTextureHandle ()
  { return hTex; }
};

#endif // __CSPIXMAP_H__
