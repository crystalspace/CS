/*
    Copyright (C) 2000 by Samuel Humphreys

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

#ifndef __PROEX2D_H__
#define __PROTEX2D_H__

#include "csutil/scf.h"
#include "graph2d.h"

struct iObjectRegistry;

class csProcTextureSoft2D : public csGraphics2D
{
public:
  // For 16bit procedural textures sharing texture manager
  csRGBpixel *image_buffer;

  // Under certain circumstances, this class allocates its own memory, this
  // flag indicates whether or not this is the case and hence our
  // responsibility.
  bool destroy_memory;

  csProcTextureSoft2D (iObjectRegistry *object_reg);
  virtual ~csProcTextureSoft2D ();

  virtual void Close ();

  virtual bool BeginDraw () { return (Memory != NULL); }

  virtual void Print (csRect *area = NULL);

  virtual iGraphics2D *CreateOffScreenCanvas
  (int width, int height, void *buffer, bool use8bit,
   csPixelFormat *ipfmt, csRGBpixel *palette = NULL, int pal_size = 0);
};

#endif // __PROTEX2D_H__
