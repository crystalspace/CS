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
#include "csfx/cspixmap.h"

struct iGraphics3D;
class csPixmap;
class csAnimatedPixmap;

/// A 2d animation template. This class is used to create animated pixmaps.
class csAnimationTemplate {
private:
  // sprite frames
  csVector Frames;
  // absolute time to finish a frame
  csVector FinishTimes;

public:
  /// build a new animation
  csAnimationTemplate();
  /// destructor
  ~csAnimationTemplate();

  // add a frame.
  inline void AddFrame(cs_time Delay, csPixmap *s)
    {Frames.Push(s); FinishTimes.Push((csSome)(GetLength() + Delay));}
  /// add a frame
  inline void AddFrame(cs_time Delay, iTextureHandle *Tex)
    {AddFrame(Delay, new csSimplePixmap(Tex));}
  /// add a frame
  inline void AddFrame(cs_time Delay, iTextureHandle *Tex, int x, int y, int w, int h)
    {AddFrame(Delay, new csSimplePixmap(Tex, x, y, w, h));}

  /// get a frame by number
  inline csPixmap *GetFrame(int n) const
    {return (csPixmap*)(Frames.Get(n));}
  /// get number of frames
  inline int GetNumFrames() const
    {return Frames.Length();}
  /// get total length of animation (all delays added together)
  inline cs_time GetLength() const
    {return (cs_time)FinishTimes.Get(GetNumFrames()-1);}
  /// get a frame by time
  csPixmap *GetFrameByTime(cs_time Time);

  /// create an instance of this animation
  csAnimatedPixmap *CreateInstance();
};

class csAnimatedPixmap : public csPixmap {
public:
  /// create an animated pixmap
  csAnimatedPixmap(csAnimationTemplate *tpl);
  /// destroy this object
  ~csAnimatedPixmap();

  // implementation of csPixmap
  virtual int Width();
  virtual int Height();
  virtual iTextureHandle *GetTextureHandle();
  virtual void DrawScaled (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
    uint8 Alpha);
  virtual void DrawTiled (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
    int orgx, int orgy, uint8 Alpha);
  virtual void Advance(cs_time ElapsedTime);

private:
  csAnimationTemplate *Template;
  cs_time CurrentTime;
  csPixmap *CurrentFrame;
};

#endif
