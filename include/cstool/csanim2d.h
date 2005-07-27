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

#ifndef __CS_CSANIM2D_H__
#define __CS_CSANIM2D_H__

#include "csextern.h"

#include "cstool/cspixmap.h"
#include "csutil/array.h"
#include "csutil/parray.h"

struct iGraphics3D;
struct iTextureHandle;

class csPixmap;
class csAnimatedPixmap;

/// A 2d animation template. This class is used to create animated pixmaps.
class CS_CRYSTALSPACE_EXPORT csAnimationTemplate
{
private:
  /// sprite frames
  csPDelArray<csPixmap> Frames;
  /**
   * absolute time to finish a frame
   * (the time from start to finish of the frame)
   */
  csArray<csTicks> FinishTimes;

public:
  /// build a new animation
  csAnimationTemplate();
  /// destructor
  ~csAnimationTemplate();

  /// get number of frames
  inline size_t GetFrameCount() const
  {return Frames.Length();}
  /// get total length of animation (all delays added together)
  inline csTicks GetLength() const
  { 
    if (GetFrameCount() == 0)
      return 0;
    else
      return (csTicks)(FinishTimes.Get (GetFrameCount()-1)); 
  }
  /// add a frame. (giving the length of this frame)
  inline void AddFrame(csTicks Delay, csPixmap *s)
  {FinishTimes.Push(GetLength() + Delay); Frames.Push (s);}
  /// add a frame (giving the length of this frame)
  inline void AddFrame(csTicks Delay, iTextureHandle *Tex)
  {AddFrame(Delay, new csSimplePixmap(Tex));}
  /// add a frame (giving the length of this frame)
  inline void AddFrame(csTicks Delay, iTextureHandle *Tex, int x, int y,
	int w, int h)
  {AddFrame(Delay, new csSimplePixmap(Tex, x, y, w, h));}

  /// get a frame by number
  inline csPixmap *GetFrame(size_t n) const
  {return Frames.Get(n);}
  /// get a frame by time
  csPixmap *GetFrameByTime(csTicks Time);

  /// create an instance of this animation
  csAnimatedPixmap *CreateInstance();
};


/// A pixmap with a 2d animation.
class CS_CRYSTALSPACE_EXPORT csAnimatedPixmap : public csPixmap
{
public:
  /// Create an animated pixmap.
  csAnimatedPixmap(csAnimationTemplate *tpl);
  /// Destroy this object.
  virtual ~csAnimatedPixmap();

  // Implementation of csPixmap.
  virtual int Width();
  virtual int Height();
  virtual iTextureHandle *GetTextureHandle();
  virtual void DrawScaled (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
    uint8 Alpha = 0);
  virtual void DrawTiled (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
    int orgx, int orgy, uint8 Alpha = 0);
  virtual void Advance(csTicks ElapsedTime);

private:
  csAnimationTemplate *Template;
  csTicks CurrentTime;
  csPixmap *CurrentFrame;
};

#endif // __CS_CSANIM2D_H__
