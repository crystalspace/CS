/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Written by Samuel Humphreys

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

#ifndef __CS_SOFTEX3D_H__
#define __CS_SOFTEX3D_H__

#include "sft3dcom.h"

class csTextureMMSoftware;

class csDynamicTextureSoft3D : public csGraphics3DSoftwareCommon
{
public:
  DECLARE_IBASE;

  csTextureMMSoftware *tex_mm;

  csDynamicTextureSoft3D (iBase *iParent);
  virtual ~csDynamicTextureSoft3D () {};

  virtual bool Initialize (iSystem *iSys);
  virtual iGraphics3D *CreateOffScreenRenderer (iGraphics3D *parent_g3d, 
     int width, int height, csPixelFormat *pfmt, void *buffer, 
     RGBPixel *palette, int pal_size);

  virtual void Print (csRect *area);

  void SetTarget (csTextureMMSoftware *tex_mm);
};

#endif // __CS_SOFTEX3D_H__
