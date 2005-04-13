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

#include "csgfx/gradient.h"
#include "ivideo/txtmgr.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "csutil/cscolor.h"
#include "csutil/util.h"
#include "iutil/objreg.h"

#include "prfire.h"

SCF_IMPLEMENT_IBASE_EXT(csProcFire)
  SCF_IMPLEMENTS_INTERFACE(iFireTexture)
SCF_IMPLEMENT_IBASE_EXT_END

/// the implementation of the fire texture
csProcFire::csProcFire (iTextureFactory* p,int w, int h) : csProcTexture(p)
{
  palsize = 0;
  palette = 0;
  palette_idx = 0;
  image[0] = 0;
  image[1] = 0;
  blitbuf = 0;
  mat_w = w;
  mat_h = h;

  smoothing = 2;
  possburn = 100;
  addburn = 5;
  contburn = 80;
  extinguish = 3*256/mat_h;

  single_flame_mode = true;
  halfbase = mat_w/4;
  fireline = 0;

  postsmooth = 0;

  texFlags = CS_TEXTURE_3D;
}

csProcFire::~csProcFire ()
{
  delete[] palette;
  delete[] palette_idx;
  delete[] fireline;
  delete[] image[0];
  delete[] image[1];
  delete[] blitbuf;
}

bool csProcFire::PrepareAnim ()
{
  if (anim_prepared) return true;
  if (!csProcTexture::PrepareAnim ()) return false;
  if (!palette) MakePalette (256);
  fireline = new uint8[mat_w];
  image[0] = new uint8[mat_w*mat_h];
  image[1] = new uint8[mat_w*mat_h];
  blitbuf = new unsigned char [mat_w*mat_h*4];
  curimg = 0;
  memset (image[0], 0, mat_w*mat_h);
  memset (image[1], 0, mat_w*mat_h);
  memset (fireline, 0, mat_w);

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
  return fireline[x];
}

void csProcFire::SetHSI (csColor& col, float H, float S, float I)
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
  delete[] palette_idx;
  palsize = max;
  palette = new csRGBcolor [palsize];
  palette_idx = new int [palsize];
  memset (palette, 0, sizeof(csRGBcolor) * palsize);
  memset (palette_idx, 0, palsize*sizeof (int));
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
    col *= 255.99f;
    r = (int) col.red;
    g = (int) col.green;
    b = (int) col.blue;
    palette[i].red = r;
    palette[i].green = g;
    palette[i].blue = b;
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
    palette[i].red = r;
    palette[i].green = g;
    palette[i].blue = b;
  }
}


void csProcFire::Animate (csTicks /*current_time*/)
{
  int i;

  /// do nextframe of firetexture
  int x, y;
  int newimg = curimg ^ 1;
  /// put fireline at bottom
  uint8* im = image[curimg]+(mat_h-1)*mat_w;	// Point to last line.
  uint8* fl = fireline;
  memcpy (im, fl, mat_w);

  /// move all pixels
  im = image[curimg] + mat_w;	// Point after first row (y=1)
  uint8* im2 = image[newimg];	
  for (y = 1; y < mat_h; y++)
  {
    for (x = 0; x < mat_w; x++)
    {
      if (rng.Get (27) >= 8)
      {
	int nx = x + rng.Get (3) - 1;
	if (nx < 0) nx = mat_w-1;
	else if (nx >= mat_w) nx = 0;
	int part = *(im + nx - x) - rng.Get (extinguish);
	if (part<extinguish) part=0;
	*(im2) = part;
      }
      else
      {
	*im2 = *(im - mat_w);
      }
      im++; im2++;
    }
  }
  memcpy (im2, fl, mat_w);

  // smooth the whole image
  if (postsmooth > 0)
  {
    im = image[newimg];
    im2 = image[curimg];
    for (y = 0; y < mat_h; y++)
    {
      for (x = 0; x < mat_w; x++)
      {
	int j, v = 0, n = 0;
	for (i = y - postsmooth; i <= y + postsmooth; i++)
	{
	  // prevent wrap-around effects
	  if ((i < 0) || (i >= mat_h)) continue;
	  for (j = x - postsmooth; j <= x + postsmooth; j++)
	  {
	    if (single_flame_mode && ((j < 0) || (j >= mat_w))) 
	      continue;
	    int nx = j; 
	    if (nx < 0) nx += mat_w; 
	    if (nx >= mat_w) nx -= mat_w;
	    int ny = i; 
	    if (ny < 0) ny += mat_h; 
	    if (ny >= mat_h) ny-= mat_h;
	    v += *(im + ny * mat_w + nx);
	    n++;
	  }
	}
	*(im2++) = (v / n);
      }
    }
    newimg ^= 1;
  }

  if (rng.Get (100) == 0)
  {
    int start = rng.Get (mat_w);
    for (i = start; i < start+5; i++)
      GetFireLine (i) = 255;
  }

  /// burn new fireline
  for (x = 0; x < mat_w; x++)
  {
    int c = fireline [x];
    if (c < 40)
      c += rng.Get(possburn); /// 3 = paper, 90 = oil
    else c += rng.Get(contburn)-contburn/2 + addburn;
    if (c > 255)
      c = 255;
    fireline [x] = c;
  }

  if (rng.Get (100) == 0)
    for (i = 0; i < 10; i++)
      fireline [ 0+rng.Get(mat_w) ] = 0;

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

  if (visible)
  {
    /// draw firetexture
    im = image[newimg];
    unsigned char* d = blitbuf;
    for (y = 0 ; y < mat_h ; y++)
      for (x = 0 ; x < mat_w ; x++)
      {
	int col = *im++;
	col = col * palsize / 256;
	*d++ = palette[col].red;
	*d++ = palette[col].green;
	*d++ = palette[col].blue;
	*d++ = 0xff;
      }
    tex->GetTextureHandle ()->Blit (0, 0, mat_w,mat_h, blitbuf);
  }
  curimg = newimg;
}

void csProcFire::SetPossibleBurn (int possburn)
{
  csProcFire::possburn = possburn;
}

int csProcFire::GetPossibleBurn()
{
  return possburn;
}

void csProcFire::SetAdditionalBurn (int addburn)
{
  csProcFire::addburn = addburn;
}

int csProcFire::GetAdditionalBurn()
{
  return addburn;
}
  
void csProcFire::SetContinuedBurn (int contburn)
{
  csProcFire::contburn = contburn;
}

int csProcFire::GetContinuedBurn()
{
  return contburn;
}
  
void csProcFire::SetSmoothing (int smoothing)
{
  csProcFire::smoothing = smoothing;
}

int csProcFire::GetSmoothing()
{
  return smoothing;
}
  
void csProcFire::SetExtinguish (int extinguish)
{
  csProcFire::extinguish = extinguish;
}

int csProcFire::GetExtinguish()
{
  return extinguish;
}

void csProcFire::SetSingleFlameMode (bool enable)
{
  single_flame_mode = enable;
}

bool csProcFire::GetSingleFlameMode()
{
  return single_flame_mode;
}

void csProcFire::SetHalfBase (int halfbase)
{
  csProcFire::halfbase = halfbase;
}

int csProcFire::GetHalfBase()
{
  return halfbase;
}

void csProcFire::SetPostSmoothing (int amount)
{
  postsmooth = MIN (amount, MIN (mat_w - 1, mat_h - 1));
}

int csProcFire::GetPostSmoothing ()
{
  return postsmooth;
}

void csProcFire::SetPalette (const csGradient gradient)
{
  palsize = 256;
  if (!palette) palette = new csRGBcolor[palsize];
  gradient.Render (palette, palsize, -0.5f, 1.5f);
}
