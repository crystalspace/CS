/*
    Copyright (C) 2000 by Jorrit Tyberghein
  
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

#ifndef __CSANIM2D_H__
#define __CSANIM2D_H__

#include "csutil/csvector.h"
#include "csengine/cspixmap.h"

struct iGraphics3D;
class csPixmap;

/**
 * A 2d animation. This class works similar to csPixmap, except that it
 * keeps a list of textures and displays one of them depending on time.
 * Like csPixmap, you can use PIXMAP_* macros to align the animation
 * when drawing.
 */
class csAnimation2D {
private:
  /// sprite frames
  csVector Frames;
  /// time per frame (msec)
  unsigned long FrameTime;

  /// add a frame.
  void AddFrame(csPixmap *s) {Frames.Push(s);}

public:
  /// build a new animation
  csAnimation2D() {FrameTime=100;}
  /// destructor
  ~csAnimation2D();

  /// add a frame
  void AddFrame(iTextureHandle *Tex) {AddFrame(new csPixmap(Tex));}
  /// add a frame
  void AddFrame(iTextureHandle *Tex, int x, int y, int w, int h)
        {AddFrame(new csPixmap(Tex, x, y, w, h));}
  /// get a frame by time
  csPixmap *GetFrameByTime(unsigned long Time);
  /// get a frame by number
  csPixmap *GetFrame(int n) {return (csPixmap*)(Frames.Get(n));}
  /// get number of frames
  int NumFrames() {return Frames.Length();}

  /// get time per frame
  unsigned long GetFrameTime() {return FrameTime;}
  /// set time per frame
  void SetFrameTime(unsigned long t) {FrameTime=t;}
  /// get total time
  unsigned long GetTotalTime() {return FrameTime*NumFrames();}

  /// draw the sprite
  void Draw(iGraphics3D*, int x, int y, unsigned long time);
  /// draw the sprite (aligned)
  void DrawAlign(iGraphics3D*,int x,int y, int alnx, int alny,
        unsigned long time);
  /// rescale the sprite
  void DrawScaled(iGraphics3D*, int x, int y, int w2, int h2,
        unsigned long time);
  /// rescale the sprite (aligned)
  void DrawScaledAlign(iGraphics3D*, int x, int y, int w2, int h2,
        int alnx, int alny, unsigned long time);
};

#endif
