


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
  time%=GetTotalTime();
  int n=0;
  while (time>=FrameTime) {time-=FrameTime;n++;}
  return GetFrame(n);
}

void csAnimation2D::Draw(iGraphics3D *G3D,int x,int y,unsigned long time) {
  csPixmap *spr=GetFrameByTime(time);
  spr->Draw(G3D,x,y);
}

void csAnimation2D::DrawAlign(iGraphics3D *G3D, int x, int y, int alnx,
        int alny, unsigned long time) {
  csPixmap *spr=GetFrameByTime(time);
  spr->DrawAlign(G3D,x,y,alnx,alny);
}

void csAnimation2D::DrawScaled(iGraphics3D *G3D,int x,int y,int w2,int h2,
        unsigned long time) {
  csPixmap *spr=GetFrameByTime(time);
  spr->DrawScaled(G3D,x,y,w2,h2);
}

void csAnimation2D::DrawScaledAlign(iGraphics3D *G3D, int x, int y, int w2,
        int h2, int alnx, int alny, unsigned long time) {
  csPixmap *spr=GetFrameByTime(time);
  spr->DrawScaledAlign(G3D, x, y, w2, h2, alnx, alny);
}
