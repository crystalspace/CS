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
#include "iutil/objreg.h"

#include "cstool/prwater.h"

/// the implementation of the water texture
csProcWater::csProcWater (iTextureFactory* p) : csProcTexture(p)
{
  palsize = 0;
  palette = 0;
  image = 0;
  mat_w = 64;
  mat_h = 64;

  texFlags = CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS ;
}

csProcWater::~csProcWater ()
{
  delete[] palette;
  delete[] image;
}

bool csProcWater::PrepareAnim ()
{
  if (anim_prepared) return true;
  if (!csProcTexture::PrepareAnim ()) return false;
  dampening = 4;
  MakePalette (256);
  nr_images = 2; // must be 2
  cur_image = 0;
  image = new signed char[nr_images*mat_w*mat_h];
  memset (image, 0, sizeof(signed char)*nr_images*mat_w*mat_h);
  return true;
}

signed char& csProcWater::GetImage (int im, int x, int y)
{
  if (im<0) im += nr_images;
  if (x<0) x += mat_w;
  if (y<0) y += mat_h;
  x %= mat_w;
  y %= mat_h;
  im %= nr_images;
  return image[ im*mat_w*mat_h + y*mat_w + x ];
}

void csProcWater::SetHSI (csColor& col, float H, float S, float I)
{
  /// from Hue Saturation Intensity to Red Green Blue
  float Temp=H;
  col.red = 1.0f + S * sin(Temp - TWO_PI / 3.0f);
  col.green = 1.0f + S * sin(Temp);
  col.blue = 1.0f + S * sin(Temp + TWO_PI / 3.0f);
  Temp = 63.999f * I / 512.0f;
  col *= Temp;
}

void csProcWater::MakePalette (int max)
{
  int i;
  delete[] palette;
  palsize = max;
  palette = new int[palsize];
  palette[0] = g2d->FindRGB(0,0,0);
  for (i=0 ; i<palsize ; i++)
    palette[i] = palette[0];
  /// fill the palette
  int maxcolours = palsize;
  csColor col;
  int r,g,b;
  for (i=0; i<maxcolours; i++)
  {
    float H = 1.0f - 1.5f * float(i) / float(maxcolours);
    float S = 1.0f - float(i) / float(maxcolours);
    float I = float(i)/float(maxcolours);
    I = 8.0f * I;
    SetHSI (col, H, S, I);
    col *= 255.0f;
    r = (int) col.red;
    g = (int) col.green;
    b = (int) col.blue;
    palette[i] = g2d->FindRGB (r,g,b);
  }
}

void csProcWater::MakePuddle (int sx, int sy, int rad, int val)
{
  int sqrad = rad *rad;
  int y, x;
  for (y=-rad ; y<=rad ; y++)
    for (x=-rad ; x<=rad ; x++)
    {
      int d = x*x + y*y;
      if (d < sqrad)
	GetImage (cur_image, sx+x, sy+y) += val * (sqrad-d) / sqrad;
    }
}


void csProcWater::PressAt (int sx, int sy, int rad, int val)
{
  int pudval = GetImage (cur_image, sx, sy);
  pudval = (pudval + val) % 256;
  MakePuddle (sx, sy, rad, pudval);
}


void csProcWater::Animate (csTicks current_time)
{
  (void)current_time;
  int i,x,y;
  /// draw palette
  //for(i=0; i<palsize; i++)
  //{
  //  g2d->DrawBox(i*(g2d->GetWidth()/palsize), 1,
  //    (g2d->GetWidth()/palsize)+5, 10, palette[i]);
  //}

  /// flip
  cur_image ++;
  cur_image %= nr_images;

  int num = GetRandom (3);
  for (i=0; i<num; i++)
  {
    int sx = GetRandom (mat_w);
    int sy = GetRandom (mat_h);
    int rainbump = 20;
    int val = rainbump + GetRandom (rainbump);
    int rainsize = 1+(mat_w+mat_h)/64;
    int rad = rainsize+GetRandom (rainsize);
    MakePuddle (sx, sy, rad, val);
  }

  /// compute next water frame.
  for (y=0; y<mat_h; y++)
    for (x=0; x<mat_w; x++)
    {
      int wt = GetImage (cur_image-1, x, y-1) +
        GetImage (cur_image-1, x, y+1) +
        GetImage (cur_image-1, x-1, y) +
        GetImage (cur_image-1, x+1, y);
      wt >>=1;
      wt -= GetImage (cur_image, x, y);
      wt -= (wt >> dampening);
      GetImage (cur_image, x,y) = wt;
    }

  g3d->SetRenderTarget (tex->GetTextureHandle ());
  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS)) return;
  /// draw texture
  for (y=0; y<mat_h; y++)
    for (x=0; x<mat_w;x++)
    {
      int ofx = GetImage (cur_image, x, y) - GetImage (cur_image, x, y+1);
      //int ofy = GetImage(cur_image, x, y) - GetImage(cur_image, x+1, y);
      int col = (128 - ofx);
      /// now ofx/8 and ofy/8 give index in background picture to blend with
      if (col<0)col=0;
      if (col>255)col=255;
      g2d->DrawPixel (x, y, palette[col*palsize/256] );
    }
  g3d->FinishDraw ();
}

