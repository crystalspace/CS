/*
    Copyright (C) 2001 by Christopher Nelson

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

#ifndef __CS_AWS_3DFRM_H__
#define __CS_AWS_3DFRM_H__

#include "csgeom/csrectrg.h"
#include "ivideo/texture.h"

struct iGraphics2D;
struct iGraphics3D;
struct iAws;
struct iAwsComponent;

/// These are frame styles supported by the 3d frame class and thus globally
const int _3dfsBump = 0;
const int _3dfsSimple = 1;
const int _3dfsRaised = 2;
const int _3dfsSunken = 3;
const int _3dfsFlat = 4;
const int _3dfsNone = 5;
const int _3dfsBevel = 6;
const int _3dfsThick = 7;
const int _3dfsBitmap = 8;
const int _3dfsSmallRaised = 9;
const int _3dfsSmallSunken = 10;
const int _3dfsMask  = 0x1f;

/**
 * This class draws several different 3d frame types to avoid code duplication.
 */

class aws3DFrame
{
protected:
  /// A ref to the current 2d drawing context.
  iGraphics2D* g2d;

  /// A ref to the current 3d drawing context.
  iGraphics3D* g3d;

  /// The basic colors.
  int hi, hi2, lo, lo2, fill, dfill, black, bfill;

  /// Background/Overlay textures.
  iTextureHandle *bkg, *ovl;

  /// Background/Overlay alpha levels.
  int bkg_alpha, ovl_alpha;

  void DrawTexturedBackground (
    csRectRegion* todraw,
    iTextureHandle* txt,
    int alpha_level,
    csRect txt_align);

  void DrawFlatBackground (csRectRegion* todraw, int color);

  void DrawRaisedFrame (csRect frame);

  void DrawSmallRaisedFrame (csRect frame);

  void DrawSunkenFrame (csRect frame);

  void DrawSmallSunkenFrame (csRect frame);

  void DrawBumpFrame (csRect frame);

  void DrawBevelFrame (csRect frame);

  void DrawThickFrame (csRect frame);
public:
  enum {
    fsBump = _3dfsBump,
    fsSimple = _3dfsSimple,
    fsRaised = _3dfsRaised,
    fsSunken = _3dfsSunken,
    fsFlat = _3dfsFlat,
    fsNone = _3dfsNone,
    fsBevel = _3dfsBevel,
    fsThick = _3dfsThick,
    fsBitmap = _3dfsBitmap,
    fsSmallRaised = _3dfsSmallRaised,
    fsSmallSunken = _3dfsSmallSunken,
    fsMask = _3dfsMask
  };

  /// Creates a frame drawer.
  aws3DFrame ();

  /// Destroys a frame drawer.
  ~aws3DFrame ();

  /**
   * Retrieve the default pallete along with textures and alpha levels
   */
  void Setup (
    iAws *wmgr,
    iTextureHandle* bkg = 0,
    int bkg_alpha = 0,
    iTextureHandle* ovl = 0,
    int ovl_alpha = 0);

  /**
   * Draws a frame of the given type. Three different versions are presented
   * depending on whether you need offsets on the background/overlay textures
   */
  void Draw (csRect frame, int frame_style, csRectRegion* rgn = 0);

  void Draw (csRect frame, int frame_style, csRect bkg_align,
    csRectRegion* rgn = 0);
  
  void Draw (csRect frame, int frame_style, csRect bkg_align,
    csRect ovl_align, csRectRegion* rgn = 0);

  csRect SubRectToAlign (csRect comp_frame, csRect txt_sub_rect);

  void SetBackgroundTexture (iTextureHandle* bkg);
  void SetOverlayTexture (iTextureHandle* ovl);
  void SetBackgroundAlpha (int bkg_alpha);
  void SetOverlayAlpha (int ovl_alpha);
  void SetBackgroundColor (int color);
  csRect GetInsets (int style);
};

#endif // __CS_AWS_3DFRM_H__
