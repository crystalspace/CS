/*
    Copyright (C) 1998, 2000mak by Jorrit Tyberghein
  
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

#include "csutil/scf.h"
#include "cstypes.h"

#ifndef ITEXTURE_H
#define ITEXTURE_H

SCF_VERSION (iTextureHandle, 2, 0, 0);

struct iGraphics3D;

/** 
 * A texture handle as returned by iTextureManager.
 */
struct iTextureHandle : public iBase
{
  /// Enable transparent color
  virtual void SetTransparent (bool Enable) = 0;

  /// Set the transparent color.
  virtual void SetTransparent (UByte red, UByte green, UByte blue) = 0;

  /// Get the transparent status (false if no transparency, true if transparency).
  virtual bool GetTransparent () = 0;

  /// Get the transparent color
  virtual void GetTransparent (UByte &red, UByte &green, UByte &blue) = 0;

  /**
   * Get the dimensions for a given mipmap level (0 to 3).
   * If the texture was registered just for 2D usage, mipmap levels above
   * 0 will return false.
   */
  virtual bool GetMipMapDimensions (int mipmap, int &mw, int &mh) = 0;

  /// Get the mean color.
  virtual void GetMeanColor (UByte &red, UByte &green, UByte &blue) = 0;

  /// Get data associated internally with this texture by texture cache
  virtual void *GetCacheData () = 0;

  /// Set data associated internally with this texture by texture cache
  virtual void SetCacheData (void *d) = 0;

  /**
   * Query the private object associated with this handle.
   * For internal usage by the 3D driver.
   */
  virtual void *GetPrivateObject () = 0;

  /**
   * If the texture handle was created with as a procedural texture, this
   * function returns an iGraphics3D interface to a texture buffer which 
   * can be used in the  same way as a frame buffer based iGraphics3D.
   * This interface only becomes available once the texture has been
   * prepared by the texture manager.
   */ 
  virtual iGraphics3D *GetProcTextureInterface () = 0;

  /**
   * If this is a procedural texture with mip-mapping enabled, call this 
   * function to update its mip maps after a change. 
   * (currently unimplemented)
   */
  virtual void ProcTextureSync () = 0;
};

#endif //ITEXTURE_H
