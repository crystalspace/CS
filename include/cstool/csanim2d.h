/*
    Copyright (C) 2000 by Jorrit Tyberghein
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

#ifndef __CSANIM2D_H__
#define __CSANIM2D_H__

#include "csutil/csvector.h"
#include "cstool/cspixmap.h"

struct iGraphics3D;
class csPixmap;
class csAnimatedPixmap;

/// A 2d animation template. This class is used to create animated pixmaps.
class csAnimationTemplate {
private:
  /// sprite frames
  csVector Frames;
  /** 
   * absolute time to finish a frame 
   * (the time from start to finish of the frame)
   */
  csVector FinishTimes;

public:
  /// build a new animation
  csAnimationTemplate();
  /// destructor
  ~csAnimationTemplate();

  /// get number of frames
  inline int GetFrameCount() const
    {return Frames.Length();}
  /// get total length of animation (all delays added together)
  inline csTime GetLength() const
    {return (GetFrameCount()==0)?0:(csTime)FinishTimes.Get(GetFrameCount()-1);}
  /// add a frame. (giving the length of this frame)
  inline void AddFrame(csTime Delay, csPixmap *s)
    {FinishTimes.Push((csSome)(GetLength() + Delay)); Frames.Push(s);}
  /// add a frame (giving the length of this frame)
  inline void AddFrame(csTime Delay, iTextureHandle *Tex)
    {AddFrame(Delay, new csSimplePixmap(Tex));}
  /// add a frame (giving the length of this frame)
  inline void AddFrame(csTime Delay, iTextureHandle *Tex, int x, int y, int w, int h)
    {AddFrame(Delay, new csSimplePixmap(Tex, x, y, w, h));}

  /// get a frame by number
  inline csPixmap *GetFrame(int n) const
    {return (csPixmap*)(Frames.Get(n));}
  /// get a frame by time
  csPixmap *GetFrameByTime(csTime Time);

  /// create an instance of this animation
  csAnimatedPixmap *CreateInstance();
};


/// a pixmap with a 2d animation 
class csAnimatedPixmap : public csPixmap {
public:
  /// create an animated pixmap
  csAnimatedPixmap(csAnimationTemplate *tpl);
  /// destroy this object
  virtual ~csAnimatedPixmap();

  // implementation of csPixmap
  virtual int Width();
  virtual int Height();
  virtual iTextureHandle *GetTextureHandle();
  virtual void DrawScaled (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
    uint8 Alpha = 0);
  virtual void DrawTiled (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
    int orgx, int orgy, uint8 Alpha = 0);
  virtual void Advance(csTime ElapsedTime);

private:
  csAnimationTemplate *Template;
  csTime CurrentTime;
  csPixmap *CurrentFrame;
};

#endif
