/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Copyright (C) 2000 by W.C.A. Wijngaards

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
#include "ivideo/txtmgr.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "csutil/cscolor.h"
#include "csutil/util.h"

#include "cstool/prfire.h"

/// the implementation of the plasma texture
csProcFire::csProcFire () : csProcTexture()
{
  palsize = 0;
  palette = NULL;
  image = 0;
  mat_w = 128;
  mat_h = 128;

  smoothing = 2;
  possburn = 100;
  addburn = 5;
  contburn = 80;
  extinguish = 3*256/mat_h;

  single_flame_mode = true;
  halfbase = mat_w/4;
  fireline = NULL;

  texFlags = CS_TEXTURE_3D | CS_TEXTURE_PROC | CS_TEXTURE_NOMIPMAPS
    ;//|CS_TEXTURE_PROC_ALONE_HINT;
}

csProcFire::~csProcFire ()
{
  delete[] palette;
  delete[] fireline;
  delete[] image;
}

bool csProcFire::PrepareAnim ()
{
  if (anim_prepared) return true;
  if (!csProcTexture::PrepareAnim ()) return false;
  MakePalette (256);
  fireline = new uint8[mat_w];
  image = new uint8[mat_w*mat_h];
  memset(image, 0, mat_w*mat_h);
  memset(fireline, 0, mat_w);

  int start = GetRandom (mat_w);

  int i;
  for (i = start; i < start+5; i++)
    GetFireLine (i) = 255;

  return true;
}

uint8& csProcFire::GetFireLine(int x)
{
  if (x < 0)
    x += mat_w;
  x %= mat_w;
  return fireline[ x ];
}

void csProcFire::SetHSI(csColor& col, float H, float S, float I)
{
  /// from Hue Saturation Intensity to Red Green Blue
  float Temp = H;
  col.red = 1.0f + S * sin(Temp - TWO_PI / 3.0f);
  col.green = 1.0f + S * sin(Temp);
  col.blue = 1.0f + S * sin(Temp + TWO_PI / 3.0f);
  Temp = 63.999f * I / 512.0f;
  col *= Temp;
}

void csProcFire::MakePalette (int max)
{
  int i;
  delete[] palette;
  palsize = max;
  palette = new int [palsize];
  palette[0] = ptTxtMgr->FindRGB (0,0,0);
  for (i = 0; i < palsize; i++)
    palette[i] = palette[0];
  /// fill the palette
  int maxcolours = palsize/2;
  csColor col;
  int r,g,b;
  for (i = 0; i < maxcolours; i++)
  {
    float H = 4.6f - 1.5f * float(i) / float(maxcolours);
    float S = float(i) / float(maxcolours);
    float I = 4.0f * float(i) / float(maxcolours);
    if (i < palsize / 4)
      S=0.0f;
    SetHSI (col, H, S, I);
    col *= 255.0f;
    r = (int) col.red;
    g = (int) col.green;
    b = (int) col.blue;
    palette[i] = ptTxtMgr->FindRGB (r,g,b);
  }
  //// guess rest of colours
  float inc = 512.0f / float (palsize - maxcolours);
  for (i = maxcolours; i < palsize; i++)
  {
    col.red += 2.0f * inc;
    col.green += inc * 0.5f;
    col.blue += inc * 0.5f;
    col.Clamp (255.0f, 255.0f, 255.0f);
    r = (int) col.red;
    g = (int) col.green;
    b = (int) col.blue;
    palette[i] = ptTxtMgr->FindRGB(r,g,b);
  }
}


void csProcFire::Animate (csTicks /*current_time*/)
{
  int i;

  /// do nextframe of firetexture
  int x, y;
  /// put fireline at bottom
  uint8* im = image+(mat_h-1)*mat_w;	// Point to last line.
  uint8* fl = fireline;
  memcpy (im, fl, mat_w);

  /// move all pixels
  im = image + mat_w;	// Point after first row (y=1)
  for (y = 1; y < mat_h; y++)
  {
    for (x = 0; x < mat_w; x++)
    {
      int part = *im - GetRandom (extinguish);
      if (part<extinguish) part=0;
      int nx = x+GetRandom (3)-1;
      if (nx < 0) nx = mat_w-1;
      else if (nx >= mat_w) nx = 0;
      *(im-mat_w+nx-x) = part;
      im++;
    }
  }

  if (GetRandom (100) == 0)
  {
    int start = GetRandom (mat_w);
    for (i = start; i < start+5; i++)
      GetFireLine (i) = 255;
  }

  /// burn new fireline
  for (x = 0; x < mat_w; x++)
  {
    int c = fireline [x];
    if (c < 40)
      c += GetRandom(possburn); /// 3 = paper, 90 = oil
    else c += GetRandom(contburn)-contburn/2 + addburn;
    if (c > 255)
      c = 255;
    fireline [x] = c;
  }

  if (GetRandom (100) == 0)
    for (i = 0; i < 10; i++)
      fireline [ 0+GetRandom(mat_w) ] = 0;

  if (single_flame_mode)
  {
    // make a single flame.
    int basestart = mat_w/2-halfbase;
    int baseend = mat_w/2+halfbase;
    for(x=0; x<basestart; x++)
      fireline[x] = 0;
    // let basestart .. baseend burn
    for(x=baseend; x<mat_w; x++)
      fireline[x] = 0;
  }

  // smooth it a little
  int sm = smoothing;
  for (x = 0; x < mat_w; x++)
  {
    int tot=0;
    for (i = x-sm; i <= x+sm; i++)
      tot += GetFireLine (i);
    fireline[x] = tot / (2*sm+1);
  }

  if (ptG3D->BeginDraw (CSDRAW_2DGRAPHICS))
  {
    /// draw firetexture
    im = image;
    for (y = 0 ; y < mat_h ; y++)
      for (x = 0 ; x < mat_w ; x++)
      {
        int col = *im++;
        ptG2D->DrawPixel (x, y, palette[col*palsize/256]);
      }
    ptG3D->FinishDraw ();
    ptG3D->Print (NULL);
  }
}

