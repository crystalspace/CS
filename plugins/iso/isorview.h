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

#ifndef __ISORVIEW_H__
#define __ISORVIEW_H__

#include "ivaria/iso.h"

/**
 *  isometric rendering view, used during rendering
*/
class csIsoRenderView : public iIsoRenderView {
private:
  /// view to use
  iIsoView *view;
  /// efficiency shortcut
  iGraphics3D *g3d;
  /// the render pass
  int renderpass;
  /// precalced minmaxes 
  int startx, starty, scanw, scanh;
  /// the clipper
  iClipper2D *clipper;

public:
  DECLARE_IBASE;

  ///
  csIsoRenderView (iBase *iParent);
  ///
  virtual ~csIsoRenderView ();

  /// set the rendering pass
  void SetRenderPass(int pass) {renderpass = pass;}
  /// set the view
  void SetView(iIsoView *vw) {view = vw;}
  /// set the g3d
  void SetG3D(iGraphics3D *g) {g3d = g;}
  /// set the clipper
  void SetClipper(iClipper2D *c) {clipper = c;}
  /// set precalc grid values
  void SetPrecalcGrid(int sx, int sy, int sw, int sh)
  { startx = sx; starty = sy; scanw = sw; scanh = sh; }

  //-------- iIsoRenderView -----------------------------------
  virtual iIsoView* GetView() const {return view;}
  virtual iGraphics3D* GetG3D() const {return g3d;}
  virtual int GetRenderPass() const {return renderpass;}
  virtual iClipper2D* GetClipper() const {return clipper;}
  virtual void GetPrecalcGrid(int& sx, int& sy, int& sw, int& sh) const
  { sx = startx; sy = starty; sw = scanw; sh = scanh; }

};

#endif
