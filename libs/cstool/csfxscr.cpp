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
#include "cstool/csfxscr.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/material.h"
#include "csutil/cscolor.h"
#include "csgeom/vector4.h"

void csfxInterference(iGraphics2D *g2d, float amount, float anim, float length)
{
#define SEMIRAND anim = (anim + 0.137564f) - int(anim + 0.137564f);
  if (amount == 0) amount = 0.000001f;
  float skipwidth = length * (1.0f / amount);
  float xpos = 0.0f;
  float ypos = 0.0f;
  int width = g2d->GetWidth();
  int height = g2d->GetHeight();
  while(ypos < height)
  {
    float skipnow = skipwidth * anim; SEMIRAND;
    float donow = length * anim; SEMIRAND;
    int col = 255 - int(255.0f * anim); SEMIRAND;
    int colour = g2d->FindRGB(col, col, col);
    while(donow+xpos >= width)
    {
      g2d->DrawLine(xpos, ypos, width-1, ypos, colour);
      ypos++;
      if(ypos >= height) return;
      donow -= width-xpos;
      xpos = 0.0f;
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
  float fade = 1.0 - fadevalue;
  csfxScreenDPFX(g3d, 0, CS_FX_COPY, color.red, color.green, color.blue, fade);
}

void csfxFadeOut(iGraphics3D *g3d, float fadevalue)
{
  float multval = 1.f - fadevalue;
  csfxScreenDPFX(g3d, 0, CS_FX_MULTIPLY, multval, multval, multval, 1.0f);
}

void csfxFadeTo(iGraphics3D *g3d, iTextureHandle *tex, float fadevalue)
{
  float fade = 1.0f - fadevalue;
  csfxScreenDPFX(g3d, tex, CS_FX_SETALPHA(fade), 1.f, 1.f, 1.f, 1.f);
}

void csfxGreenScreen(iGraphics3D *g3d, float fadevalue)
{
  float multval = 1.0f - fadevalue;
  csfxScreenDPFX(g3d, 0, CS_FX_MULTIPLY, multval, 1.0f, multval, 1.0f);
}

void csfxRedScreen(iGraphics3D *g3d, float fadevalue)
{
  float multval = 1.0f - fadevalue;
  csfxScreenDPFX(g3d, 0, CS_FX_MULTIPLY, 1.0f, multval, multval, 1.0f);
}

void csfxBlueScreen(iGraphics3D *g3d, float fadevalue)
{
  float multval = 1.0f - fadevalue;
  csfxScreenDPFX(g3d, 0, CS_FX_MULTIPLY, multval, multval, 1.0f, 1.0f);
}

void csfxWhiteOut(iGraphics3D *g3d, float fadevalue)
{
  csfxScreenDPFX(g3d, 0, CS_FX_ADD, fadevalue, fadevalue, fadevalue, 1.0f);
}

void csfxShadeVert(iGraphics3D *g3d, const csColor& topcolor,
  const csColor& bottomcolor, uint mixmode)
{
  float sx = 0;
  float sy = 0;
  float sw = g3d->GetWidth ();
  float sh = g3d->GetHeight ();
  float smx = sx + sw;
  float smy = sy + sh;

  csSimpleRenderMesh mesh;
  csVector3 verts[4];
  csVector2 texels[4];
  csVector4 colors[4];
  float fa = 1.0f;

  mesh.meshtype = CS_MESHTYPE_QUADS;
  mesh.vertexCount = 4;
  mesh.vertices = verts;
  mesh.texcoords = texels;
  mesh.colors = colors;
  mesh.texture = 0;
  // @@@ Bit of a hack
  if ((mixmode & CS_FX_MASK_MIXMODE) == CS_FX_ALPHA)
  {
    fa = float (mixmode & CS_FX_MASK_ALPHA) / 255.0f;
    mesh.mixmode = CS_FX_COPY;
    mesh.alphaType.autoAlphaMode = false;
    mesh.alphaType.alphaType = csAlphaMode::alphaSmooth;
  }
  else
    mesh.mixmode = mixmode;

  verts[0].Set (sx, sy, 0.0f);
  texels[0].Set (0.0f, 1.0f);
  colors[0].Set (topcolor.red, topcolor.green, topcolor.blue, fa);
  
  verts[1].Set (smx, sy, 0.0f);
  texels[1].Set (0.0f, 0.0f);
  colors[1].Set (topcolor.red, topcolor.green, topcolor.blue, fa);

  verts[2].Set (smx, smy, 0.0f);
  texels[2].Set (1.0f, 0.0f);
  colors[2].Set (bottomcolor.red, bottomcolor.green, bottomcolor.blue, fa);

  verts[3].Set (sx, smy, 0.0f);
  texels[3].Set (1.0f, 1.0f);
  colors[3].Set (bottomcolor.red, bottomcolor.green, bottomcolor.blue, fa);

  g3d->DrawSimpleMesh (mesh, csSimpleMeshScreenspace);
}


void csfxScreenDPFX(iGraphics3D *g3d, iTextureHandle *tex, uint mixmode,
  float r, float g, float b, float a)
{
  csfxScreenDPFXPartial (g3d, 0, 0, g3d->GetWidth (), g3d->GetHeight (),
    tex, mixmode, r, g, b, a);
}

void csfxScreenDPFXPartial(iGraphics3D *g3d, int x, int y, int w, int h,
  iTextureHandle *tex, uint mixmode, float fr, float fg, float fb, float fa)
{
  csSimpleRenderMesh mesh;
  csVector3 verts[4];
  csVector2 texels[4];
  csVector4 colors[4];

  mesh.meshtype = CS_MESHTYPE_QUADS;
  mesh.vertexCount = 4;
  mesh.vertices = verts;
  mesh.texcoords = texels;
  mesh.colors = colors;
  mesh.texture = tex;
  // @@@ Bit of a hack
  if ((mixmode & CS_FX_MASK_MIXMODE) == CS_FX_ALPHA)
  {
    fa = float (mixmode & CS_FX_MASK_ALPHA) / 255.0f;
    mesh.mixmode = CS_FX_COPY;
    mesh.alphaType.autoAlphaMode = false;
    mesh.alphaType.alphaType = csAlphaMode::alphaSmooth;
  }
  else if(fa < 1.0f)
  {
    mesh.mixmode = mixmode;
    mesh.alphaType.autoAlphaMode = false;
    mesh.alphaType.alphaType = csAlphaMode::alphaSmooth;
  }
  else
    mesh.mixmode = mixmode;

  verts[0].Set (x, y, 0);
  texels[0].Set (0.0f, 0.0f);
  colors[0].Set (fr, fg, fb, fa);
  
  verts[1].Set (x + w, y, 0);
  texels[1].Set (1.0f, 0.0f);
  colors[1].Set (fr, fg, fb, fa);

  verts[2].Set (x + w, y + h, 0);
  texels[2].Set (1.0f, 1.0f);
  colors[2].Set (fr, fg, fb, fa);

  verts[3].Set (x, y + h, 0);
  texels[3].Set (0.0f, 1.0f);
  colors[3].Set (fr, fg, fb, fa);

  g3d->DrawSimpleMesh (mesh, csSimpleMeshScreenspace);
}

