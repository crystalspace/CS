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

#ifndef __IVIDEO_SPROCTXT_H__
#define __IVIDEO_SPROCTXT_H__

#include "csutil/scf.h"

SCF_VERSION (iSoftProcTexture, 0, 0, 1);

/**
 * This interface is for internal use.
 * This interface currently is implemented in the software procedural texture
 * driver as part of renderer/software. Opengl currently supports a software
 * renderer option which utilises this. See ogl_proctexsoft.* for usage.
 */
struct iSoftProcTexture : public iBase
{
  /// Prepare a software procedural texture for use by a hardware renderer
  virtual iTextureHandle *CreateOffScreenRenderer
    (iGraphics3D *parent_g3d, iGraphics3D *partner_g3d, int width, int height,
     void *buffer, csPixelFormat *ipfmt, int flags) = 0;

  /// This converts mode once and for all to a less efficient one.
  virtual void ConvertMode () = 0;
};


#endif // __IVIDEO_SPROCTXT_H__
