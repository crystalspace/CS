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

#ifndef __CS_ITEXTURE_ITEXLOADERCTX_H__
#define __CS_ITEXTURE_ITEXLOADERCTX_H__

/**\file
 * Texture loader context.
 */

/**
 * \addtogroup loadsave
 * @{ */
 
#include "csutil/scf.h"

struct iImage;

SCF_VERSION (iTextureLoaderContext, 0, 0, 2);

/**
 * Interface passed to a texture loader, holding some common texture 
 * properties.
 */
struct iTextureLoaderContext : public iBase
{
  /// Have any flags been specified?
  virtual bool HasFlags () = 0;
  /// Get the specified flags
  virtual int GetFlags () = 0;
  
  /// Has an image been specified?
  virtual bool HasImage () = 0;
  /// Get the image
  virtual iImage* GetImage() = 0;
  
  /// Has a size been specified?
  virtual bool HasSize () = 0;
  /// Get the size
  virtual void GetSize (int& w, int& h) = 0;
  
  /// Get the texture's name
  virtual const char* GetName () = 0;
};

/** @} */

#endif
