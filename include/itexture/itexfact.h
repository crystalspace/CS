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

#ifndef __CS_ITEXTURE_ITEXFACT_H__
#define __CS_ITEXTURE_ITEXFACT_H__

/**\file
 * Texture factory.
 */

/**
 * \addtogroup gfx3d
 * @{ */
 
#include "csutil/scf.h"

struct iTextureWrapper;
struct iTextureType;

/**
 * Interface to a texture factory.
 * Textures need to know their size at creation time; this information can be
 * taken from an image or explicitly specified.
 */
struct iTextureFactory : public virtual iBase
{
  SCF_INTERFACE(iTextureFactory, 2,0,0);
  /**
   * Generate a new texture with the selected parameters.
   */
  virtual csPtr<iTextureWrapper> Generate () = 0;

  /**
   * Set the size of the texture to be created.
   */
  virtual void SetSize (int w, int h) = 0;
  /**
   * Get the size of the texture to be created.
   */
  virtual void GetSize (int& w, int& h) = 0;
  /**
   * Get the TextureType for this texture factory.
   */
  virtual iTextureType* GetTextureType () const = 0;
};

SCF_VERSION (iTextureType, 0, 0, 1);

/**
 * Texture type.
 * Interface used to create instances of iTextureFactory.
 */
struct iTextureType : public virtual iBase
{
  /**
   * Create a new instance of a texture factory.
   */
  virtual csPtr<iTextureFactory> NewFactory() = 0;
};

/** @} */

#endif
