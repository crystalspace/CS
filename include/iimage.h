/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __IIMAGE_H__
#define __IIMAGE_H__

#include "csutil/scf.h"

struct RGBPixel;
struct Filter3x3;
struct Filter5x5;

SCF_INTERFACE (iImageFile, 0, 0, 1) : public iBase
{
  ///
  virtual RGBPixel *GetImageData () = 0;
  ///
  virtual int GetWidth () = 0;
  ///
  virtual int GetHeight () = 0;
  ///
  virtual int GetSize () = 0;
  ///
  virtual iImageFile *MipMap (int steps, Filter3x3* filt1, Filter5x5* filt2) = 0;
  ///
  virtual iImageFile *MipMap (int steps) = 0;
  ///
  virtual iImageFile *Blend (Filter3x3* filter) = 0;
  /// Set image file name
  virtual void SetName (const char *iName) = 0;
  /// Get image file name
  virtual const char *GetName () = 0;
};

#endif // __IIMAGE_H__
