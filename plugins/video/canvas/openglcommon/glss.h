/*
    Copyright (C) 2003 by Jorrit Tyberghein
              (C) 2003 by Frank Richter

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

#ifndef __CS_CANVAS_OPENGLCOMMON_GLSS_H__
#define __CS_CANVAS_OPENGLCOMMON_GLSS_H__

#include "plugins/video/canvas/common/scrshot.h"

class csGraphics2DGLCommon;

class csGLScreenShot : public iImage
{
  csGraphics2DGLCommon* G2D;
  int Format;
  csRGBpixel* Data;
  int Width, Height;

public:
  csGLScreenShot* poolNext;

  SCF_DECLARE_IBASE;

  csGLScreenShot (csGraphics2DGLCommon* G2D);
  virtual ~csGLScreenShot ();

  virtual void *GetImageData ()
  { return Data; }
  virtual int GetWidth ()
  { return Width; }
  virtual int GetHeight ()
  { return Height; }
  virtual int GetSize () { return 0; }
  virtual void Rescale (int NewWidth, int NewHeight)
  { (void) NewWidth; (void) NewHeight; }
  virtual csPtr<iImage> MipMap (int step, csRGBpixel *transp)
  { (void)step; (void)transp; return 0; }
  virtual void SetName (const char *iName)
  { (void) iName; }
  virtual const char *GetName ()
  { return 0; }
  virtual int GetFormat ()
  { return Format; }
  virtual csRGBpixel *GetPalette ()
  { return 0; }
  virtual uint8 *GetAlpha ()
  { return 0; }
  virtual void SetFormat (int /*iFormat*/)
  { }
  virtual csPtr<iImage> Clone ()
  { return 0; }
  virtual csPtr<iImage> Crop (int , int , int , int )
  { return 0; }
  virtual void CheckAlpha ()
  { }
  virtual bool HasKeycolor ()
  { return 0; }
  virtual void GetKeycolor (int &r, int &g, int &b)
  { r=0;g=0;b=0; }
  virtual csPtr<iImage> Sharpen (csRGBpixel *transp, int strength)
  { transp = 0; strength = 0; return 0; }

  virtual int HasMipmaps ()
  { return 0; }

  void SetData (void* data);
};

#endif
