/*
   csanim2d.h
   A 2d animation pixmap subclass.
   Copyright (C) 2001 by W.C.A. Wijngaards
*/

#ifndef CSANIM2D_H
#define CSANIM2D_H

#include "csengine/cspixmap.h"
struct iTextureHandle;
struct iSystem;
struct iGraphics3D;

/**
 * A class that displays a 2d animation.
 * simply initialize, then set all the image frames with texture.
 * By default the smallest size frame is used for display (if
 * all frames are the same size, that size is set).
 * Then you can use all the csPixmap commands for drawing,
 * and they draw the animation frame at that moment.
 * Thus, if you simply redraw each frame, it will animate correctly.
*/
class csAnim2D: public csPixmap
{
  /// have an image rectangle yet
  bool texture_rect_ok;
  /// the system (for getting time)
  iSystem *system;
  /// nr of animation frames.
  int nr;
  /// array of the animation images.
  iTextureHandle **imgs;
  /// the delay that should be before each frame.
  int *delays;
  /// time when last frame was started.
  cs_time lasttime;
  /// last frame nr.
  int lastnr;

public:
  /// create a N frame animation. Size of 1st frame is used for display.
  csAnim2D(iSystem *sys, int num);
  ///
  virtual ~csAnim2D();

  /// set image for frame N (sets image rectangle to 'correct' size)
  void SetImage(int n, iTextureHandle *tex);
  /// get image N
  iTextureHandle *GetImage(int n) const {return imgs[n];}
  /// set delay before frame N is shown.
  void SetDelay(int n, int delay) {delays[n]=delay<0?0:delay;}
  /// get a delay before a frame 
  int GetDelay(int n) const {return delays[n];}

  /// Get the current image that should be displayed at time of call (*now*).
  virtual iTextureHandle *GetTextureHandle();

  /// Return true if animation is inited OK.
  virtual bool ok();
  /// Draw the pixmap given the screen position and new size
  virtual void DrawScaled (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
        uint8 Alpha = 0);
  /**
   * Draw the pixmap tiled over an area. multiple draw commands with the
   * same texture and same origin values will align properly.
   * The orgx and orgy point to a pixel (perhaps offscreen) where the
   * (0,0) pixel of this pixmap would be drawn.
   */
  virtual void DrawTiled (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
    int orgx, int orgy, uint8 Alpha = 0);
};

#endif // CSANIM2D_H
