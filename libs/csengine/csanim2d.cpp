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
#include "csengine/csanim2d.h"
#include "igraph3d.h"

csAnimation2D::~csAnimation2D() {
  for (int i=0;i<Frames.Length();i++) {
    csPixmap *p=GetFrame(i);
    delete p;
  }
}

csPixmap *csAnimation2D::GetFrameByTime(unsigned long time) {
  cs_time tt=GetTotalTime();
  if (tt<1) return NULL;
  return GetFrame((time%tt)/FrameTime);
}

void csAnimation2D::Draw(iGraphics3D *G3D,int x,int y,unsigned long time) {
  csPixmap *spr=GetFrameByTime(time);
  if (spr) spr->Draw(G3D,x,y);
}

void csAnimation2D::DrawAlign(iGraphics3D *G3D, int x, int y, int alnx,
        int alny, unsigned long time) {
  csPixmap *spr=GetFrameByTime(time);
  if (spr) spr->DrawAlign(G3D,x,y,alnx,alny);
}

void csAnimation2D::DrawScaled(iGraphics3D *G3D,int x,int y,int w2,int h2,
        unsigned long time) {
  csPixmap *spr=GetFrameByTime(time);
  if (spr) spr->DrawScaled(G3D,x,y,w2,h2);
}

void csAnimation2D::DrawScaledAlign(iGraphics3D *G3D, int x, int y, int w2,
        int h2, int alnx, int alny, unsigned long time) {
  csPixmap *spr=GetFrameByTime(time);
  if (spr) spr->DrawScaledAlign(G3D, x, y, w2, h2, alnx, alny);
}
