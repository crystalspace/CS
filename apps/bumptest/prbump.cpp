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
#include "qint.h"
#include "prbump.h"

#include "cssys/system.h"
#include "csutil/cscolor.h"
#include "csutil/util.h"
#include "csgfx/rgbpixel.h"

#include "ivideo/txtmgr.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "igraphic/image.h"
#include "iengine/light.h"


csProcBump::csProcBump () : csProcTexture()
{
  bumpmap = NULL;
  palsize = 0;
  palette = NULL;
  mat_w = 256;
  mat_h = 256;
  fastdhdx = fastdhdy = NULL;

  texFlags = CS_TEXTURE_3D | CS_TEXTURE_PROC | CS_TEXTURE_NOMIPMAPS
    ;//| CS_TEXTURE_PROC_ALONE_HINT;
}

csProcBump::csProcBump (iImage *map) : csProcTexture()
{
  bumpmap = map;
  palsize = 0;
  palette = NULL;
  mat_w = map->GetWidth();
  mat_h = map->GetHeight();
  fastdhdx = fastdhdy = NULL;

  texFlags = CS_TEXTURE_3D | CS_TEXTURE_PROC | CS_TEXTURE_NOMIPMAPS
    ;//| CS_TEXTURE_PROC_ALONE_HINT;
}

csProcBump::~csProcBump ()
{
  delete[] palette;
  delete[] fastdhdx;
  delete[] fastdhdy;
}

bool csProcBump::PrepareAnim ()
{
  if (anim_prepared) return true;
  if (!csProcTexture::PrepareAnim ()) return false;
  MakePalette (256);
  return true;
}

void csProcBump::SetHSI (csColor& col, float H, float S, float I)
{
  /// from Hue Saturation Intensity to Red Green Blue
  float Temp=H;
  col.red = 1.0 + S * sin(Temp - 2.0*PI/3.0);
  col.green = 1.0 + S * sin(Temp);
  col.blue = 1.0 + S * sin(Temp + 2.0*PI/3.0);
  Temp = 63.999 * I / 512.0;
  col *= Temp;
}

void csProcBump::MakePalette (int max)
{
  int i;
  if (palette) delete[] palette;
  palsize = max;
  palette = new int[palsize];
  palette[0] = ptTxtMgr->FindRGB(0,0,0);
  for (i=0 ; i<palsize ; i++)
    palette[i] = palette[0];
  /// fill the palette
  int maxcolours = palsize;
  //csColor col;
  int r,g,b;
  for (i=0; i<maxcolours; i++)
  {
    /*
    float H = 1.0-1.5*float(i)/float(maxcolours);
    float S = 1.0-float(i)/float(maxcolours);
    float I = float(i)/float(maxcolours);
    I = 8.0 * I;
    SetHSI (col, H, S, I);
    col *= 255.0;
    r = (int) col.red;
    g = (int) col.green;
    b = (int) col.blue;
    */
    r = g = b = i;
    palette[i] = ptTxtMgr->FindRGB (r,g,b);
  }
}


void csProcBump::Animate (csTicks current_time)
{
  (void)current_time;
  // nothing to do
}

void csProcBump::Recalc(const csVector3& center, const csVector3& normal, 
    const csVector3& xdir, const csVector3& ydir,
    int numlight, iLight **lights)
{
  int i, x,y;
  if (!ptG3D->BeginDraw (CSDRAW_2DGRAPHICS))
    return;
  /// calc diff values
  int w = bumpmap->GetWidth();
  //int h = bumpmap->GetHeight();
  csVector3 xax = xdir.Unit();
  csVector3 yax = ydir.Unit();
  int size = mat_w*mat_h;
  float *vals = new float[size];
  for(i=0; i<size; i++) 
    vals[i] = 0.0;

  for(i=0; i<numlight; i++)
  {
    csVector3 lightdir = lights[i]->GetCenter() - center;
    lightdir.Normalize();
    // the amount of light that is displayed using regular lighting
    // this amount must be perturbed.
    float lightnow = lightdir * normal;
    //printf("Lightdir %g %g %g , giving lightnow %g \n", 
      //lightdir.x, lightdir.y, lightdir.z, lightnow);

    for (y=0; y<mat_h; y++)
      for (x=0; x<mat_w;x++)
      {
        csVector3 pixnormal = GetNormal(x, y, normal, xax, yax);
        vals[ y*w+x ] += (lightdir * pixnormal) - lightnow;
        vals[ y*w+x ] += (lightdir * pixnormal) - lightnow;
      }
  }

  /// draw texture
  for (y=0; y<mat_h; y++)
    for (x=0; x<mat_w;x++)
    {
      /// scale -1 .. +1 to 0 .. 256 and clip to edges.
      int col = QInt(vals[y*w+x] * 128) + 128;
      //col = x % 256;
      //col = GetHeight(x%w,y%h);
      if(col < 0) col=0;
      else if(col > 255) col=255;

      ptG2D->DrawPixel (x, y, palette[col*palsize/256] );
    }
  ptG3D->FinishDraw ();
  ptG3D->Print (NULL);
  delete[] vals;
}
	  
int csProcBump::GetHeight(int x, int y)
{
  int r, g, b;
  if(bumpmap->GetFormat() & CS_IMGFMT_PALETTED8)
  {
    int pal = ((unsigned char*)bumpmap->GetImageData())[
      y*bumpmap->GetWidth() + x];
    r = bumpmap->GetPalette()[pal].red;
    g = bumpmap->GetPalette()[pal].green;
    b = bumpmap->GetPalette()[pal].blue;
  }
  else
  {
    // truecolor image
    r = ((csRGBpixel*)bumpmap->GetImageData())
      [y*bumpmap->GetWidth() + x].red;
    g = ((csRGBpixel*)bumpmap->GetImageData())
      [y*bumpmap->GetWidth() + x].green;
    b = ((csRGBpixel*)bumpmap->GetImageData())
      [y*bumpmap->GetWidth() + x].blue;
  }
  return (r+g+b)/3;
}

csVector3 csProcBump::GetNormal(int x, int y, const csVector3& mainnormal,
  const csVector3& xdir, const csVector3& ydir)
{
  float dhdx = 0; // delta height / delta x
  float dhdy = 0; // delta height / delta y
  int w = bumpmap->GetWidth();
  int h = bumpmap->GetHeight();
  dhdx = float(GetHeight((x+1)%w,y%h) - GetHeight((x+w-1)%w,y%h)) * 0.5;
  dhdy = float(GetHeight(x%w,(y+1)%h) - GetHeight(x%w,(y+h-1)%h)) * 0.5;
  /// both now have values in the range -128 .. +128
  csVector3 res = mainnormal;
  res -= xdir * dhdx * (1./64.);
  res -= ydir * dhdy * (1./64.);
  return res.Unit();
}


void csProcBump::SetupFast()
{
  /// this texture must be the same size as the bumpmap
  int x, y;
  int w = mat_w, h = mat_h;
  int size = mat_w*mat_h;
  fastdhdx = new unsigned char [size];
  fastdhdy = new unsigned char [size];
  unsigned char *dhdx = fastdhdx;
  unsigned char *dhdy = fastdhdy;
  for (y=0; y<mat_h; y++)
    for (x=0; x<mat_w;x++)
    {
      *dhdx++ = (GetHeight((x+1)%w,y) - GetHeight((x+w-1)%w,y))/2 + 128;
      *dhdy++ = (GetHeight(x,(y+1)%h) - GetHeight(x,(y+h-1)%h))/2 + 128;
    }
}

void csProcBump::RecalcFast(const csVector3& center, const csVector3& normal, 
    const csVector3& xdir, const csVector3& ydir,
    int numlight, iLight **lights)
{
  int l,i,x,y;
  // precalc fast lookup tables for slopes
  if(!fastdhdx) SetupFast();
  // precalc some tables for the current situation
  // perturbation for the dhdx+128 values
  short dxval[256];
  short dyval[256];
  // the maximum number of lights that will affect a bumpmapoverlay.
#define MAXBUMPLIGHTS 10
  if(numlight > MAXBUMPLIGHTS) numlight = MAXBUMPLIGHTS;
  csVector3 lightdirs[MAXBUMPLIGHTS];
  float totallightat = 0.0;

  for(l=0; l<numlight; l++)
  {
    lightdirs[l] = (lights[l]->GetCenter() - center).Unit();
    totallightat +=  lightdirs[l] * normal;
  }

  csVector3 add = xdir.Unit() * (1./64.);
  csVector3 pixnormal;
  float resval;
  for(i=0; i<256; i++)
  {
    pixnormal = normal - add * (i-128);
    pixnormal.Normalize();
    resval = 0.0;
    for(l=0; l<numlight; l++)
        resval += (lightdirs[l] * pixnormal);
    dxval[i] = QInt((resval-totallightat)*128.);
  }
  add = ydir.Unit() * (1./64.);
  for(i=0; i<256; i++)
  {
    pixnormal = normal - add * (i-128);
    pixnormal.Normalize();
    resval = 0.0;
    for(l=0; l<numlight; l++)
        resval += (lightdirs[l] * pixnormal);
    dyval[i] = QInt((resval-totallightat)*128.) + 128;
    // 128 is added to dyval, but not to dxval.
    // so that the result of dxval+dyval is a dx + dy + 128 value.
  }

  /// draw texture
  if (!ptG3D->BeginDraw (CSDRAW_2DGRAPHICS))
    return;
  i=0;
  for (y=0; y<mat_h; y++)
    for (x=0; x<mat_w;x++)
    {
      /// scale -1 .. +1 to 0 .. 256 and clip to edges.
      //int col = QInt((dxval[fastdhdx[i]] + dyval[fastdhdy[i]]) * 128.) + 128;
      short col = dxval[fastdhdx[i]] + dyval[fastdhdy[i]];
      if(col < 0) col=0;
      else if(col > 255) col=255;
      /// palettesize must be 256 (FindRGB(col,col,col) should work too)
      ptG2D->DrawPixel (x, y, palette[col] );
      i++;
    }
  ptG3D->FinishDraw ();
  ptG3D->Print (NULL);
}

