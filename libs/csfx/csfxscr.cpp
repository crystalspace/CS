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

#include "cssysdef.h"
#include "csfx/csfxscr.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "csutil/cscolor.h"

void csfxInterference(iGraphics2D *g2d, iTextureManager *txtmgr,
  float amount, float anim, float length)
{
#define SEMIRAND anim = (anim+0.137564) - int(anim+0.137564);
  if(amount==0) amount = 0.000001;
  float skipwidth = length * (1.0/amount);
  float xpos = 0;
  float ypos = 0;
  int width = g2d->GetWidth();
  int height = g2d->GetHeight();
  while(ypos < height)
  {
    float skipnow = skipwidth * anim; SEMIRAND;
    float donow = length * anim; SEMIRAND;
    int col = 255 - int(255.*anim); SEMIRAND;
    int colour = txtmgr->FindRGB(col, col, col);
    while(donow+xpos >= width)
    {
      g2d->DrawLine(xpos, ypos, width-1, ypos, colour);
      ypos++;
      if(ypos >= height) return;
      donow -= width-xpos;
      xpos = 0.0;
    }
    g2d->DrawLine(xpos, ypos, xpos+donow, ypos, colour);
    xpos += donow;
    xpos += skipnow;
    ypos += int(xpos) / width;
    xpos += (-int(xpos) + int(xpos)%width);
  }
#undef SEMIRAND
}

void csfxFadeToColor(iGraphics3D *g3d, float fadevalue, const csColor& color)
{
  UByte red = int(255*color.red);
  UByte green = int(255*color.green);
  UByte blue = int(255*color.blue);
  float fade = 1.0 - fadevalue;
  csfxScreenDPFX(g3d, NULL, CS_FX_SETALPHA(fade), red, green, blue);
}

void csfxFadeOut(iGraphics3D *g3d, float fadevalue)
{
  UByte multval = 255 - int(255.*fadevalue);
  csfxScreenDPFX(g3d, NULL, CS_FX_MULTIPLY, multval, multval, multval);
}

void csfxFadeTo(iGraphics3D *g3d, iMaterialHandle *mat, float fadevalue)
{
  float fade = 1.0 - fadevalue;
  csfxScreenDPFX(g3d, mat, CS_FX_SETALPHA(fade), 0, 0, 0);
}

void csfxGreenScreen(iGraphics3D *g3d, float fadevalue)
{
  UByte multval = 255 - int(255.*fadevalue);
  csfxScreenDPFX(g3d, NULL, CS_FX_MULTIPLY, multval, 255, multval);
}

void csfxRedScreen(iGraphics3D *g3d, float fadevalue)
{
  UByte multval = 255 - int(255.*fadevalue);
  csfxScreenDPFX(g3d, NULL, CS_FX_MULTIPLY, 255, multval, multval);
}

void csfxBlueScreen(iGraphics3D *g3d, float fadevalue)
{
  UByte multval = 255 - int(255.*fadevalue);
  csfxScreenDPFX(g3d, NULL, CS_FX_MULTIPLY, multval, multval, 255);
}


void csfxWhiteOut(iGraphics3D *g3d, float fadevalue)
{
  UByte multval = int(255.*fadevalue);
  csfxScreenDPFX(g3d, NULL, CS_FX_ADD, multval, multval, multval);
}

void csfxShadeVert(iGraphics3D *g3d, const csColor& topcolor,
  const csColor& bottomcolor, UInt mixmode)
{
  G3DPolygonDPFX dpfx;
  dpfx.num = 4;
  dpfx.use_fog = false;
  /// sy=0 is bottom of screen
  dpfx.vertices[0].sx = 0.;
  dpfx.vertices[0].sy = 0.;
  dpfx.vertices[0].z = 0.;
  dpfx.vertices[0].u = 0.;
  dpfx.vertices[0].v = 1.;
  dpfx.vertices[0].r = bottomcolor.red;
  dpfx.vertices[0].g = bottomcolor.green;
  dpfx.vertices[0].b = bottomcolor.blue;
  dpfx.vertices[1].sx = 0.;
  dpfx.vertices[1].sy = g3d->GetHeight();
  dpfx.vertices[1].z = 0.;
  dpfx.vertices[1].u = 0.;
  dpfx.vertices[1].v = 0.;
  dpfx.vertices[1].r = topcolor.red;
  dpfx.vertices[1].g = topcolor.green;
  dpfx.vertices[1].b = topcolor.blue;
  dpfx.vertices[2].sx = g3d->GetWidth();
  dpfx.vertices[2].sy = g3d->GetHeight();
  dpfx.vertices[2].z = 0.;
  dpfx.vertices[2].u = 1.;
  dpfx.vertices[2].v = 0.;
  dpfx.vertices[2].r = topcolor.red;
  dpfx.vertices[2].g = topcolor.green;
  dpfx.vertices[2].b = topcolor.blue;
  dpfx.vertices[3].sx = g3d->GetWidth();
  dpfx.vertices[3].sy = 0.;
  dpfx.vertices[3].z = 0.;
  dpfx.vertices[3].u = 1.;
  dpfx.vertices[3].v = 1.;
  dpfx.vertices[3].r = bottomcolor.red;
  dpfx.vertices[3].g = bottomcolor.green;
  dpfx.vertices[3].b = bottomcolor.blue;
  dpfx.mat_handle = NULL;
  dpfx.mixmode = mixmode | CS_FX_GOURAUD;
  dpfx.flat_color_r = 255;
  dpfx.flat_color_g = 255;
  dpfx.flat_color_b = 255;
  int oldzbufmode = g3d->GetRenderState(G3DRENDERSTATE_ZBUFFERMODE);
  g3d->SetRenderState(G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_NONE);
  g3d->StartPolygonFX(dpfx.mat_handle, dpfx.mixmode);
  g3d->DrawPolygonFX(dpfx);
  g3d->FinishPolygonFX();
  g3d->SetRenderState(G3DRENDERSTATE_ZBUFFERMODE, oldzbufmode);
}


void csfxScreenDPFX(iGraphics3D *g3d, iMaterialHandle *mat, UInt mixmode,
  UByte r, UByte g, UByte b)
{
  G3DPolygonDPFX dpfx;
  dpfx.num = 4;
  dpfx.use_fog = false;
  dpfx.vertices[0].sx = 0.;
  dpfx.vertices[0].sy = 0.;
  dpfx.vertices[0].z = 0.;
  dpfx.vertices[0].u = 0.;
  dpfx.vertices[0].v = 1.;
  dpfx.vertices[0].r = 1.;
  dpfx.vertices[0].g = 1.;
  dpfx.vertices[0].b = 1.;
  dpfx.vertices[1].sx = 0.;
  dpfx.vertices[1].sy = g3d->GetHeight();
  dpfx.vertices[1].z = 0.;
  dpfx.vertices[1].u = 0.;
  dpfx.vertices[1].v = 0.;
  dpfx.vertices[1].r = 1.;
  dpfx.vertices[1].g = 1.;
  dpfx.vertices[1].b = 1.;
  dpfx.vertices[2].sx = g3d->GetWidth();
  dpfx.vertices[2].sy = g3d->GetHeight();
  dpfx.vertices[2].z = 0.;
  dpfx.vertices[2].u = 1.;
  dpfx.vertices[2].v = 0.;
  dpfx.vertices[2].r = 1.;
  dpfx.vertices[2].g = 1.;
  dpfx.vertices[2].b = 1.;
  dpfx.vertices[3].sx = g3d->GetWidth();
  dpfx.vertices[3].sy = 0.;
  dpfx.vertices[3].z = 0.;
  dpfx.vertices[3].u = 1.;
  dpfx.vertices[3].v = 1.;
  dpfx.vertices[3].r = 1.;
  dpfx.vertices[3].g = 1.;
  dpfx.vertices[3].b = 1.;
  dpfx.mat_handle = mat;
  dpfx.mixmode = mixmode;
  dpfx.flat_color_r = r;
  dpfx.flat_color_g = g;
  dpfx.flat_color_b = b;
  int oldzbufmode = g3d->GetRenderState (G3DRENDERSTATE_ZBUFFERMODE);
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_NONE);
  g3d->StartPolygonFX (dpfx.mat_handle, dpfx.mixmode);
  g3d->DrawPolygonFX (dpfx);
  g3d->FinishPolygonFX ();
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, oldzbufmode);
}

