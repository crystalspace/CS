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

#ifndef __CS_PROTEX3D_H__
#define __CS_PROTEX3D_H__

#include "sft3dcom.h"
#include "ivideo/isprotex.h"

class csTextureHandleSoftware;
class csTextureCacheSoftware;

class csSoftProcTexture3D : public csGraphics3DSoftwareCommon, 
			    public iSoftProcTexture
{
  /// Whether sharing resources
  bool sharing;
  /// True when it is necessary to reprepare a texture each update.
  bool reprepare;
  /// If reprepare is true soft_tex_mm is reprepared
  csTextureHandleSoftware *soft_tex_mm;
  /// The parent procedural texture as registered with the main texture manager
  csTextureHandleSoftware *parent_tex_mm;
  /// The main gfx contexts texture cache
  csTextureCacheSoftware *main_tcache;

public:
  DECLARE_IBASE;

  csSoftProcTexture3D (iBase *iParent);
  virtual ~csSoftProcTexture3D ();

  bool Prepare (csTextureManagerSoftware *main_texman, 
		csTextureHandleSoftware *tex_mm, 
		void *buffer, uint8 *bitmap);

  virtual bool Initialize (iSystem *iSys);

  virtual void Print (csRect *area);

  //----------------------------------------------------------------------------
  /// The entry interface for other than software drivers..
  /// implementation of iSoftProcTexture
  virtual iTextureHandle *CreateOffScreenRenderer 
    (iGraphics3D *parent_g3d, iGraphics3D *partner_g3d, int width, int height, 
     void *buffer, csPixelFormat *ipfmt, int flags);
  /// Converts mode from alone to shared.
  virtual void ConvertMode ();

private:
  csGraphics3DSoftwareCommon* partner;
};

#endif // __CS_PROTEX3D_H__
