/*
    Copyright (C) 2001 by W.C.A. Wijngaards
  
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

#ifndef __ISOVIEW_H__
#define __ISOVIEW_H__

#include "ivaria/iso.h"

class csIsoRenderView;
class csBoxClipper;

/**
 *  isometric view
 */
class csIsoView : public iIsoView {
private:
  /// the isoengine to use
  iIsoEngine *engine;
  /// the world shown
  iIsoWorld *world;
  /// a renderview - kept allocated for speed
  csIsoRenderView *rview;

  /// rectangle on screen to fill
  csRect rect;
  /// scroll of the view (in screen space, the point where (0,0,0) lands)
  csVector2 scroll;
  /** data for world to screen transformation
   *  the x,y,z axes projected onto the screen.
   *  Each vector should have the length and direction for 1 unit.
   *  Note these cannot be changed freely, the axes must be approximately
   *  aligned as depicted. (y being up on the screen image, z being
   *  right and up on the screen image, x being right and down.)
   *  also, y_axis.x == 0.0, x_axis.y != 0.0.
   */
  csVector2 x_axis, y_axis, z_axis;
  /// 1/x_axis.y -- kept for speed reasons
  float invx_axis_y;

public:
  DECLARE_IBASE;

  ///
  csIsoView(iBase *iParent, iIsoEngine *eng, iIsoWorld *world);
  ///
  virtual ~csIsoView();

  /// do drawing precalcs
  void PreCalc();

  //------- iIsoView ----------------------------------------------
  virtual void SetWorld(iIsoWorld* world) {csIsoView::world=world;}
  virtual iIsoWorld* GetWorld() const {return world;}
  virtual void W2S(const csVector3& world, csVector2& screen);
  virtual void W2S(const csVector3& world, csVector3& screen);
  virtual void S2W(const csVector2& screen, csVector3& world);
  virtual void Draw();
  virtual const csRect& GetRect() const {return rect;}
  virtual void SetRect(const csRect& rect) {csIsoView::rect = rect;}
  virtual const csVector2& GetScroll() const {return scroll;}
  virtual void SetScroll(const csVector3& worldpos, const csVector2& coord);
  virtual void MoveScroll(const csVector3& delta);
};


#endif
