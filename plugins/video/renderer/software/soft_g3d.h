/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#ifndef __SOFT_G3D_H__
#define __SOFT_G3D_H__

// GRAPH3D.H
// csGraphics3DSoftware software rasterizer class.

#include "csutil/scf.h"
#include "iconfig.h"
#include "video/renderer/software/sft3dcom.h"


class csGraphics3DSoftware : public csGraphics3DSoftwareCommon
{

public:
  DECLARE_IBASE;
  ///
  csGraphics3DSoftware (iBase *iParent);
  ///
  virtual ~csGraphics3DSoftware ();

  ///
  virtual bool Initialize (iSystem *iSys);
  ///
  virtual bool Open (const char *Title);

  /// Create an off screen renderer
  virtual iGraphics3D *CreateOffScreenRenderer (int width, int height, 
     csPixelFormat *pfmt, void *buffer, RGBPixel *palette, int pal_size);

  ///------------------- iConfig interface implementation -------------------
  struct csSoftConfig : public iConfig
  {
    DECLARE_EMBEDDED_IBASE (csGraphics3DSoftware);
    virtual bool GetOptionDescription (int idx, csOptionDescription *option);
    virtual bool SetOption (int id, csVariant* value);
    virtual bool GetOption (int id, csVariant* value);
  } scfiConfig;
  friend struct csSoftConfig;


};

#endif // __SOFT_G3D_H__
