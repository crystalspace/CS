/*
   csanim2d.cpp
   A 2d animation for a gui.
   Copyright (C) 2001 by W.C.A. Wijngaards
*/

#include "cssysdef.h"
#include "csfx/anim2d.h"
#include "igraph2d.h"
#include "igraph3d.h"
#include "itexture.h"
#include "isystem.h"

csAnim2D::csAnim2D(iSystem* sys, int num)
  : csPixmap(0)
{
  texture_rect_ok = false;
  tx=0; ty=0; 
  system = sys;
  nr = num;
  if(nr <= 0) nr = 1;
  imgs = new iTextureHandle* [nr];
  delays = new int [nr];
  lasttime = system->GetTime();
  lastnr = 0;
  for(int i=0; i<nr; i++)
  {
    delays[i] = 100;
    imgs[i] = NULL;
  }
}

csAnim2D::~csAnim2D()
{
  hTex = NULL; /// is already in imgs list
  for(int i=0; i<nr; i++)
    if(imgs[i]) imgs[i]->DecRef();
  delete[] imgs;
  delete[] delays;
}


iTextureHandle *csAnim2D::GetTextureHandle()
{
  cs_time now = system->GetTime();
  if(now<=lasttime) return imgs[lastnr]; //sanity check
  int next = (lastnr+1)%nr;
  int after = now - lasttime;
  if(after < delays[next])
    return imgs[lastnr];
  // find the next frame
  // if delay is <=0 show now (as fast as possible)
  // cycle through frames to find  it
  while((delays[next] > 0) && (after >= delays[next]))
  {
    after -= delays[next];
    next = (next+1)%nr;
  }
  lastnr = next;
  lasttime = now;
  return imgs[next];
}


void csAnim2D::SetImage(int n, iTextureHandle *tex)
{
  if(imgs[n])imgs[n]->DecRef(); 
  imgs[n]=tex; 
  if(tex)
  {
    tex->IncRef();
    int w,h;
    tex->GetMipMapDimensions(0,w,h);
    if(!texture_rect_ok) 
    {
      SetTextureRectangle(0,0,w,h);
      texture_rect_ok = true;
    }
    else
    {
      //// fit rectangle inside each frame
      if(tx+tw>w) tw = w-tx;
      if(ty+th>h) th = h-ty;
      if(tw<0) tw=0;
      if(th<0) th=0;
    }
  }
}


bool csAnim2D::ok()
{
  if(!texture_rect_ok) return false; /// 0 frames in image
  for(int i=0; i<nr; i++)
    if(imgs[i]==0) return false;
  return true;
}


void csAnim2D::DrawScaled (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
        uint8 Alpha)
{
  hTex = GetTextureHandle();
  csPixmap::DrawScaled(g3d, sx, sy, sw, sh, Alpha);
}


void csAnim2D::DrawTiled (iGraphics3D* g3d, int sx, int sy, int sw, int sh,
    int orgx, int orgy, uint8 Alpha)
{
  hTex = GetTextureHandle();
  csPixmap::DrawTiled(g3d, sx, sy, sw, sh, orgx, orgy, Alpha);
}

