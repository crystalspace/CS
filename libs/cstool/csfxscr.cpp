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
  uint8 red = int (255*color.red);
  uint8 green = int (255*color.green);
  uint8 blue = int (255*color.blue);
  float fade = 1.0 - fadevalue;
  csfxScreenDPFX(g3d, 0, CS_FX_SETALPHA(fade), red, green, blue);
}

void csfxFadeOut(iGraphics3D *g3d, float fadevalue)
{
  uint8 multval = 255 - int(255.*fadevalue);
  csfxScreenDPFX(g3d, 0, CS_FX_MULTIPLY, multval, multval, multval);
}

void csfxFadeTo(iGraphics3D *g3d, iTextureHandle *tex, float fadevalue)
{
  float fade = 1.0f - fadevalue;
  csfxScreenDPFX(g3d, tex, CS_FX_SETALPHA(fade), 255, 255, 255);
}

void csfxGreenScreen(iGraphics3D *g3d, float fadevalue)
{
  uint8 multval = 255 - int(255.*fadevalue);
  csfxScreenDPFX(g3d, 0, CS_FX_MULTIPLY, multval, 255, multval);
}

void csfxRedScreen(iGraphics3D *g3d, float fadevalue)
{
  uint8 multval = 255 - int(255.*fadevalue);
  csfxScreenDPFX(g3d, 0, CS_FX_MULTIPLY, 255, multval, multval);
}

void csfxBlueScreen(iGraphics3D *g3d, float fadevalue)
{
  uint8 multval = 255 - int(255.*fadevalue);
  csfxScreenDPFX(g3d, 0, CS_FX_MULTIPLY, multval, multval, 255);
}


void csfxWhiteOut(iGraphics3D *g3d, float fadevalue)
{
  uint8 multval = int(255.*fadevalue);
  csfxScreenDPFX(g3d, 0, CS_FX_ADD, multval, multval, multval);
}

void csfxShadeVert(iGraphics3D *g3d, const csColor& topcolor,
  const csColor& bottomcolor, uint mixmode)
{
  float hw = float (g3d->GetWidth () / 2);
  float hh = float (g3d->GetHeight () / 2);
  float asp = hw / hh;

  float sx = -asp;
  float sy = -1.0f;
  float smx = asp;
  float smy = 1.0f;

  csSimpleRenderMesh mesh;
  static uint indices[4] = {0, 1, 2, 3};
  csVector3 verts[4];
  csVector2 texels[4];
  csVector4 colors[4];
  float fa = 1.0f;

  mesh.meshtype = CS_MESHTYPE_QUADS;
  mesh.indexCount = 4;
  mesh.indices = indices;
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

  verts[0].Set (sx, sy, 2.0f);
  texels[0].Set (0.0f, 1.0f);
  colors[0].Set (topcolor.red, topcolor.green, topcolor.blue, fa);
  
  verts[1].Set (sx, smy, 2.0f);
  texels[1].Set (0.0f, 0.0f);
  colors[1].Set (bottomcolor.red, bottomcolor.green, bottomcolor.blue, 
    fa);

  verts[2].Set (smx, smy, 2.0f);
  texels[2].Set (1.0f, 0.0f);
  colors[2].Set (bottomcolor.red, bottomcolor.green, bottomcolor.blue, 
    fa);

  verts[3].Set (smx, sy, 2.0f);
  texels[3].Set (1.0f, 1.0f);
  colors[3].Set (topcolor.red, topcolor.green, topcolor.blue, fa);

  g3d->DrawSimpleMesh (mesh);
}


void csfxScreenDPFX(iGraphics3D *g3d, iTextureHandle *tex, uint mixmode,
  uint8 r, uint8 g, uint8 b)
{
  csfxScreenDPFXPartial (g3d, 0, 0, g3d->GetWidth (), g3d->GetHeight (),
    tex, mixmode, r, g, b);
}

void csfxScreenDPFXPartial(iGraphics3D *g3d, int x, int y, int w, int h,
  iTextureHandle *tex, uint mixmode, uint8 r, uint8 g, uint8 b)
{
  csSimpleRenderMesh mesh;
  static uint indices[4] = {0, 1, 2, 3};
  csVector3 verts[4];
  csVector2 texels[4];
  csVector4 colors[4];
  float fr = float (r) / 255.0f;
  float fg = float (g) / 255.0f;
  float fb = float (b) / 255.0f;
  float fa = 1.0f;

  mesh.meshtype = CS_MESHTYPE_QUADS;
  mesh.indexCount = 4;
  mesh.indices = indices;
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

