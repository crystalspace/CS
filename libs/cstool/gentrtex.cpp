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
#include "cstool/gentrtex.h"
#include "csutil/cscolor.h"
#include "igraphic/image.h"
#include "csgfx/rgbpixel.h"
#include "csgfx/memimage.h"
#include "csqint.h"



//-- csGenerateImageTextureBlend ----------------------------------
csGenerateImageTextureBlend::csGenerateImageTextureBlend()
{
  layers = 0;
  valuefunc = 0;
}

csGenerateImageTextureBlend::~csGenerateImageTextureBlend()
{
  csGenerateImageLayer *p = layers, *np =0;
  while(p)
  {
    np = p->next;
    delete p->tex;
    delete p;
    p = np;
  }
}

void csGenerateImageTextureBlend::AddLayer(float value,
  csGenerateImageTexture *tex)
{
  /// find a spot
  csGenerateImageLayer *p = layers, *prevp = 0;
  while(p && (p->value < value))
  {
    prevp = p;
    p = p->next;
  }
  /// check value
  if(p && (p->value == value))
  {
    printf("csGenerateImageTextureBlend Error: "
      "encountered duplicate value %g. Ignoring.\n", value);
    return;
  }
  /// create new
  csGenerateImageLayer *part = new csGenerateImageLayer;
  part->next = 0;
  part->value = value;
  part->tex = tex;
  /// insert in list
  if(p==0)
  {
    // append
    if(prevp) prevp->next = part;
    else layers = part;
  }
  else if(prevp == 0)
  {
    // prepend
    part->next = layers;
    layers = part;
  }
  else
  {
    // insert into linked list
    part->next = p;
    prevp->next = part;
  }
}

void csGenerateImageTextureBlend::GetColor(csColor &col,
  float x, float y)
{
  /// get height
  float value = valuefunc->GetValue(x, y);
  /// find closest layers (below <), (above >=)
  csGenerateImageLayer *below = 0, *above = 0;
  above = layers;
  while(above && (above->value < value))
  {
    below = above;
    above = above->next;
  }
  /// compute color
  float belowfactor = 0.0f;
  float abovefactor = 0.0f;
  csColor abovecol, belowcol;
  if(!below && !above)
  {
    col.Set(0.5f, 0.5f, 0.5f);
    return;
  }
  if(!below)
  {
    abovefactor = 1.0f;
    above->tex->GetColor(abovecol, x,y);
  }
  else if(!above)
  {
    belowfactor = 1.0f;
    below->tex->GetColor(belowcol, x,y);
  }
  else { // both an above and below - blend
    float dist = above->value - below->value;
    belowfactor = (above->value - value) / dist;
    abovefactor = 1.0f - belowfactor;
    above->tex->GetColor(abovecol, x,y);
    below->tex->GetColor(belowcol, x,y);
  }

  col.Set(0.0f, 0.0f, 0.0f);
  col += abovecol * abovefactor;
  //printf("col = %g %g %g\n", col.red, col.green, col.blue);
  col += belowcol * belowfactor;
  //printf("col = %g %g %g\n", col.red, col.green, col.blue);
}

//---- csGenerateImageTextureSingle ----------------------------
csGenerateImageTextureSingle::~csGenerateImageTextureSingle()
{
}


void csGenerateImageTextureSingle::SetImage(iImage *im)
{
  image = im;
}

void csGenerateImageTextureSingle::GetImagePixel(
  iImage *image, int x, int y, csRGBpixel& res)
{
  int r, g, b;
  x %= image->GetWidth();
  y %= image->GetHeight();
  if(image->GetFormat() & CS_IMGFMT_PALETTED8)
  {
    int pal = ((unsigned char*)image->GetImageData())[
      y*image->GetWidth() + x];
    r = image->GetPalette()[pal].red;
    g = image->GetPalette()[pal].green;
    b = image->GetPalette()[pal].blue;
  }
  else
  {
    // truecolor image
    r = ((csRGBpixel*)image->GetImageData())
      [y*image->GetWidth() + x].red;
    g = ((csRGBpixel*)image->GetImageData())
      [y*image->GetWidth() + x].green;
    b = ((csRGBpixel*)image->GetImageData())
      [y*image->GetWidth() + x].blue;
  }
  //printf("Return image pixel %d,%d  %d %d %d\n", x, y, r, g, b);
  res.Set(r,g,b);
}


void csGenerateImageTextureSingle::ComputeLayerColor(
  const csVector2& pos, csColor& col)
{
  csVector2 imagepos = pos - offset;
  imagepos.x *= scale.x;
  imagepos.y *= scale.y;
  imagepos.x *= image->GetWidth();
  imagepos.y *= image->GetHeight();
  /// imagepos is now a pixel, floating valued.
  csRGBpixel pix;
  csColor col1, col2; /// left & right linear interpolation
  int x = csQint(imagepos.x);
  int y = csQint(imagepos.y);

  //GetImagePixel(layer->image, x, y, pix);
  //col1.Set(pix.red, pix.green, pix.blue);
  //return col1;

  float blendy = imagepos.y - float(y);
  float invblendy = 1.f - blendy;
  GetImagePixel(image, x, y+1, pix);
  col1.red = float(pix.red) * blendy;
  col1.green = float(pix.green) * blendy;
  col1.blue = float(pix.blue) * blendy;
  GetImagePixel(image, x, y, pix);
  col1.red += float(pix.red) * invblendy;
  col1.green += float(pix.green) * invblendy;
  col1.blue += float(pix.blue) * invblendy;

  GetImagePixel(image, x+1, y+1, pix);
  col2.red = float(pix.red) * blendy;
  col2.green = float(pix.green) * blendy;
  col2.blue = float(pix.blue) * blendy;
  GetImagePixel(image, x+1, y, pix);
  col2.red += float(pix.red) * invblendy;
  col2.green += float(pix.green) * invblendy;
  col2.blue += float(pix.blue) * invblendy;

  /// now linearly interpolate col1 and col2
  float blendx = imagepos.x - float(x);
  float invblendx = 1.f - blendx;
  col.red = invblendx * col1.red + blendx * col2.red;
  col.green = invblendx * col1.green + blendx * col2.green;
  col.blue = invblendx * col1.blue + blendx * col2.blue;

  // return trilinear interpolated value
  //printf("layercol = %g %g %g\n", col.red, col.green, col.blue);
  col *= 1.0f / 255.0f;
}

void csGenerateImageTextureSingle::GetColor(csColor &col,
  float x, float y)
{
  ComputeLayerColor(csVector2(x,y), col);
}




//----------- csGenerateImage --------------------------------------

csGenerateImage::csGenerateImage()
{
  //baselist = 0;
  //heightfunc = 0;
  tex = 0;
}

csGenerateImage::~csGenerateImage()
{
  delete tex;
}

iImage *csGenerateImage::Generate(int totalw, int totalh,
  int startx, int starty, int partw, int parth)
{
  csImageMemory *csim = new csImageMemory(partw, parth);
  csim->Clear(csRGBpixel (128, 128, 128));
  iImage *result = csim;

  csVector2 pixelsize (1.0f / float(totalw), 1.0f / float(totalh));
  csVector2 startpos(float(startx) * pixelsize.x, float(starty)*pixelsize.y);
  csVector2 pos;
  /// memory image is always truecolor
  csRGBpixel *destpix = (csRGBpixel*)result->GetImageData();
  csRGBpixel pix;
  csColor col;
  int y, x;
  for(y=0; y< parth; y++)
  {
    pos.y = startpos.y + pixelsize.y * float(y);
    pos.x = startpos.x;
    for(x=0; x< partw; x++)
    {
      /// compute color
      tex->GetColor(col, pos.x, pos.y);
      pix.Set(csQint(col.red*255.),csQint(col.green*255.),csQint(col.blue*255.));
      //if(x==0)printf("Set pixel %3d, %3d to %3g %3g %3g\n", x, y,
        //col.red, col.green, col.blue);
      /// set pixel
      *destpix = pix;
      destpix++;
      pos.x += pixelsize.x;
    }
  }

  return result;
}


//------------------ csGenerateImageValueFunc -----------------
csGenerateImageValueFuncTex::~csGenerateImageValueFuncTex()
{
  delete tex;
}

float csGenerateImageValueFuncTex::GetValue(float x, float y)
{
  csColor col;
  tex->GetColor(col, x, y);
  return (col.red + col.green + col.blue) * (1.0f / 3.0f);
}

