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

#ifndef __CS_ISOVIEW_H__
#define __CS_ISOVIEW_H__

#include "ivaria/iso.h"
#include "iengine/camera.h"

class csIsoRenderView;
class csBoxClipper;
class csIsoFakeCamera;
class csPlane3;

struct iMeshWrapper;

/**
 *  isometric view
 */
class csIsoView : public iIsoView
{
private:
  /// the isoengine to use
  iIsoEngine *engine;
  /// the world shown
  iIsoWorld *world;
  /// a renderview - kept allocated for speed
  csIsoRenderView *rview;
  /// a fake 3d perspective camera
  csIsoFakeCamera *fakecam;

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
  SCF_DECLARE_IBASE;

  ///
  csIsoView(iBase *iParent, iIsoEngine *eng, iIsoWorld *world);
  ///
  virtual ~csIsoView();

  /// do drawing precalcs
  void PreCalc();

  //------- iIsoView ----------------------------------------------
  virtual iIsoEngine *GetEngine() const {return engine;}
  virtual void SetWorld(iIsoWorld* world) {csIsoView::world=world;}
  virtual iIsoWorld* GetWorld() const {return world;}
  virtual void W2S(const csVector3& world, csVector2& screen) const;
  virtual void W2S(const csVector3& world, csVector3& screen) const;
  virtual void S2W(const csVector2& screen, csVector3& world) const;
  virtual void Draw();
  virtual const csRect& GetRect() const {return rect;}
  virtual void SetRect(const csRect& rect) {csIsoView::rect = rect;}
  virtual const csVector2& GetScroll() const {return scroll;}
  virtual csVector3 GetViewScroll() const;
  virtual void SetScroll(const csVector3& worldpos, const csVector2& coord);
  virtual void MoveScroll(const csVector3& delta);
  virtual void SetAxes(float xscale, float yscale, float zscale,
    float zskew, float xskew);
  virtual iCamera* GetFakeCamera(const csVector3& center,
    iIsoRenderView *rview);
};


/// fake a perspective camera for iso
class csIsoFakeCamera : public iCamera
{
  // view we are based on
  iIsoView *view;
  // fake fov
  int fov;
  float invfov;
  // fov angle
  float fovangle;
  // shift
  float shiftx, shifty;
  // camera transform
  csOrthoTransform trans;
  // camera mirrored?
  bool mirror;
  // camera number for keep track of changes
  long camnum;
  /// the scale of the view
  float scale;
public:
  SCF_DECLARE_IBASE;
  csIsoFakeCamera();
  virtual ~csIsoFakeCamera();

  /// precalc values for this view
  void SetIsoView(const csVector2& scroll, const csVector2& x_axis,
    const csVector2& y_axis, const csVector2& z_axis);

  /// ready for a particular mesh (approximate for that mesh)
  void IsoReady(const csVector3& position, iIsoRenderView *rview);

  //----------- iCamera --------------------------
  virtual csCamera* GetPrivateObject () {return 0;}
  virtual iCamera* Clone () const
  { return new csIsoFakeCamera (*this); }
  virtual int GetFOV () const {return fov;}
  virtual float GetInvFOV () const {return invfov;}
  virtual float GetFOVAngle () const {return fovangle;}
  virtual void SetFOV (int, int) {};
  virtual void SetFOVAngle(float, int) {};
  virtual float GetShiftX () const {return shiftx;}
  virtual float GetShiftY () const {return shifty;}
  virtual void SetPerspectiveCenter(float, float) {}
  virtual csOrthoTransform& GetTransform ()
  { return trans; }
  virtual const csOrthoTransform& GetTransform () const
  { return trans; }
  virtual void SetTransform (const csOrthoTransform& tr)
  { trans = tr; }
  virtual void MoveWorld (const csVector3&, bool ) {}
  virtual void Move (const csVector3&, bool ) {}
  virtual void MoveWorldUnrestricted (const csVector3& ) {}
  virtual void MoveUnrestricted (const csVector3& ) {}
  virtual iSector *GetSector() const {return 0;}
  virtual void SetSector(struct iSector *) {}
  virtual void Correct(int) {}
  virtual bool IsMirrored() const {return mirror;}
  virtual void SetMirrored(bool m) {mirror = m;}
  virtual void SetFarPlane(csPlane3*) { }
  virtual csPlane3* GetFarPlane() const {return 0;}
  virtual long GetCameraNumber() const {return camnum;}

  /// but in isometric space :-)
  virtual void Perspective (const csVector3& v, csVector2& p) const
  {
    view->W2S(v, p);
  }

  virtual void InvPerspective (const csVector2& p, float z, csVector3& v) const
  {
    view->S2W(p, v);
    /// @@@ not done yet.... use the screen Z value to compute z,x.
    (void)z;
  }
  virtual void OnlyPortals (bool) { }
  virtual bool GetOnlyPortals () { return true; }
};


#endif // __CS_ISOVIEW_H__
