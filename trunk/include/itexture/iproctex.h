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

#ifndef __CS_ITEXTURE_IPROCTEX_H__
#define __CS_ITEXTURE_IPROCTEX_H__

/**\file
 * Interface with properties common to all procedural textures.
 */

/**
 * \addtogroup gfx3d
 * @{ */
 
#include "csutil/scf_interface.h"

struct iTextureFactory;

/**
 * Interface to a texture factory.
 */
struct iProcTexture : public virtual iBase
{
  SCF_INTERFACE(iProcTexture, 2,0,0);
  /// Get the 'always animate' flag.
  virtual bool GetAlwaysAnimate () const = 0;
  /// Set the 'always animate' flag.
  virtual void SetAlwaysAnimate (bool enable) = 0;
  /// Returns the texturefactor that created this proctexture.
  virtual iTextureFactory* GetFactory() = 0;
};

/** @} */

#endif
