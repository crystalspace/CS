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

#ifndef __CS_AWS_PANEL_H__
#define __CS_AWS_PANEL_H__

#include "awscomp.h"
#include "csgeom/csrectrg.h"
#include "aws3dfrm.h"

/**
 * This class serves as a generic component which supports many of the basic
 * graphic options that components use such as background drawing and frame
 * styles. It also clips to children so that drawing is optimized.
 */

class awsPanel : public awsComponent  
{
protected:
  /// Style.
  int style;

  /// Child exclusion region.
  csRectRegion todraw;

  /// 3D frame drawer.
  aws3DFrame frame_drawer;

  /// True if the child_exclude region should be recalculated.
  bool todraw_dirty;

  /// Textures for background and overlay.
  iTextureHandle *bkg, *ovl;

  /// Alpha levels for background and overlay.
  int bkg_alpha, ovl_alpha;

  /// Subrects of the background and overlay textures to use.
  csRect bm_bkgsub, bm_ovlsub;
public:
  awsPanel ();
  virtual ~awsPanel ();

  bool Setup (iAws *_wmgr, iAwsComponentNode *settings);

  void OnDraw (csRect clip);

  void AddChild (iAwsComponent *comp);
  void RemoveChild (iAwsComponent *comp);

  void Move (int delta_x, int delta_y);

  virtual csRect getInsets ();

  virtual void OnChildMoved ();
  virtual void OnResized ();
  virtual void OnChildShow ();
  virtual void OnChildHide ();

  /// Frame styles.
  static const int fsBump;
  static const int fsSimple;
  static const int fsRaised;
  static const int fsSunken;
  static const int fsFlat;
  static const int fsNone;
  static const int fsBevel;
  static const int fsThick;
  static const int fsBitmap;
  static const int fsMask;
  static const int fsNormal;
  static const int fsToolbar;
};

#endif // __CS_AWS_PANEL_H__
