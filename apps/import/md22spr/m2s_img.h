/*
  Crystal Space Quake MDL/MD2 convertor
  Copyright (C) 2000 by Eric Sunshine <sunshine@sunshineco.com>

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

#ifndef __M2S_IMG_H__
#define __M2S_IMG_H__

#include "igraphic/image.h"
#include "csgfx/rgbpixel.h"

class SkinImage : public iImage
{
protected:
  void* Image;
  csRGBpixel Palette[256];
  int Width, Height, Size;
public:
  SCF_DECLARE_IBASE;
  SkinImage(void* Data, const void* Pal, int w, int h);
  virtual ~SkinImage () { SCF_DESTRUCT_IBASE(); }
  virtual void *GetImageData ();
  virtual int GetWidth ();
  virtual int GetHeight ();
  virtual int GetSize ();
  virtual void Rescale (int NewWidth, int NewHeight);
  virtual csPtr<iImage> MipMap (int step, csRGBpixel *transp);
  virtual void SetName (const char*);
  virtual const char *GetName ();
  virtual int GetFormat ();
  virtual csRGBpixel *GetPalette ();
  virtual uint8 *GetAlpha ();
  virtual void CheckAlpha ();
  virtual void SetFormat (int iFormat);
  virtual csPtr<iImage> Clone ();
  virtual csPtr<iImage> Crop (int x, int y, int width, int height);
  virtual bool HasKeycolor () { return 0; }
  virtual void GetKeycolor (int &r, int &g, int &b) { r=0;g=0;b=0; }
  virtual csPtr<iImage> Sharpen (csRGBpixel *transp, int strength);
  virtual int HasMipmaps() { return 0;}
};

#endif // __M2S_IMG_H__
