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

#ifndef __CS_ISORVIEW_H__
#define __CS_ISORVIEW_H__

#include "ivaria/iso.h"

class csIsoRenderBucket;

/**
 *  isometric rendering view, used during rendering
*/
class csIsoRenderView : public iIsoRenderView 
{
private:
  /// view to use
  iIsoView *view;
  /// efficiency shortcut
  iGraphics3D *g3d;
  /// the render pass
  int renderpass;
  /// precalced minmaxes
  int startx, starty, scanw, scanh;
  /// the precalced nr of cells per 1.0 world y
  float celpery;
  /// the clipper
  iClipper2D *clipper;
  /// lower bound on screen z
  float minz;
  /// buckets to fill with rendering info
  csIsoRenderBucket **buckets;
  /// maxbuckets;
  int maxbuckets;
  /// a list of prealloced buckets
  csIsoRenderBucket *prebuck;

public:
  SCF_DECLARE_IBASE;

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
  void SetPrecalcGrid(int sx, int sy, int sw, int sh, float cpy)
  { startx = sx; starty = sy; scanw = sw; scanh = sh; celpery = cpy; }
  /// get num buckets
  int GetNumBuckets() const {return maxbuckets;}
  /// create new buckets for later drawing
  void CreateBuckets(int num);
  /// draw all the buckets
  void DrawBuckets();

  //-------- iIsoRenderView -----------------------------------
  virtual iIsoView* GetView() const {return view;}
  virtual iGraphics3D* GetG3D() const {return g3d;}
  virtual int GetRenderPass() const {return renderpass;}
  virtual iClipper2D* GetClipper() const {return clipper;}
  virtual void GetPrecalcGrid(int& sx, int& sy, int& sw, int& sh,
    float& cpy) const
  { sx = startx; sy = starty; sw = scanw; sh = scanh; cpy = celpery; }
  virtual float GetMinZ() const {return minz;}
  virtual void SetMinZ(float val) {minz = val;}
  virtual void AddPolyFX(int materialindex, G3DPolygonDPFX *g3dpolyfx,
    uint mixmode);
};

/** keeps render info
*/
class csIsoRenderBucket
{
public:
  /// ptr to alloced g3dpolyfx to draw
  G3DPolygonDPFX *g3dpolyfx;
  /// next to draw;
  csIsoRenderBucket *next;
};

#endif // __CS_ISORVIEW_H__
