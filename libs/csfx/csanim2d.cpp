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

#include "cssysdef.h"
#include "csfx/csanim2d.h"
#include "igraph3d.h"

csAnimationTemplate::csAnimationTemplate() {
}

csAnimationTemplate::~csAnimationTemplate() {
  for (int i=0;i<Frames.Length();i++) {
    csPixmap *p = GetFrame(i);
    delete p;
  }
}

csPixmap *csAnimationTemplate::GetFrameByTime(cs_time Time) {
  // test for empty animation
  if (GetNumFrames() == 0) return NULL;
  // wrap time
  Time %= GetLength();
  // search for frame (@@@ optimize this!)
  long i;
  for (i=0; i<GetNumFrames(); i++) {
    if (Time < (cs_time)FinishTimes.Get(i))
      return GetFrame(i);
  }
  // this should never happen because it means that this class is buggy
  CS_ASSERT(false);
  return NULL;
}

/***************************************************************************/

csAnimatedPixmap::csAnimatedPixmap(csAnimationTemplate *tpl)
{
  Template = tpl;
  CurrentTime = 0;
  CurrentFrame = (tpl->GetNumFrames()>0) ? tpl->GetFrame(0) : NULL;
}

csAnimatedPixmap::~csAnimatedPixmap()
{
}

int csAnimatedPixmap::Width()
{
  return CurrentFrame ? CurrentFrame->Width() : 0;
}

int csAnimatedPixmap::Height()
{
  return CurrentFrame ? CurrentFrame->Height() : 0;
}

void csAnimatedPixmap::DrawScaled (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
  uint8 Alpha)
{
  if (CurrentFrame) CurrentFrame->DrawScaled(g3d, sx, sy, sw, sh, Alpha);
}

void csAnimatedPixmap::DrawTiled (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
  int orgx, int orgy, uint8 Alpha)
{
  if (CurrentFrame) CurrentFrame->DrawTiled(g3d, sx, sy, sw, sh, orgx, orgy, Alpha);
}

void csAnimatedPixmap::Advance(cs_time ElapsedTime)
{
  CurrentTime += ElapsedTime;
  CurrentFrame = Template->GetFrameByTime(CurrentTime);
}

iTextureHandle *csAnimatedPixmap::GetTextureHandle() {
  return CurrentFrame ? CurrentFrame->GetTextureHandle() : NULL;
}
