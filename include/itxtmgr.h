/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Jorrit Tyberghein.

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

#ifndef __ITXTMGR_H__
#define __ITXTMGR_H__

#include "csutil/scf.h"

class Vector2;
class csMatrix3;
class csVector3;
class csRect;

scfInterface iImageFile;
scfInterface iTextureHandle;

/**
 * This is the standard texture manager interface.
 * A 3D rasterizer will have to implement a subclass of this one and
 * return a pointer to it in Graphics3D.
 * This class is responsible for receiving all textures
 * from the 3D engine, converting them to an internal format if
 * needed, calculating a palette if needed, and calculating all
 * lookup tables related to the textures. Mipmap creation is
 * also done in this class.
 */
SCF_INTERFACE (iTextureManager, 0, 0, 1) : public iBase
{
  /**
   * Initialize the texture system. This function must be called
   * at least once and everytime we want to start all over using the textures.
   */
  virtual void Initialize () = 0;

  /**
   * After all textures have been added, this function does all
   * needed calculations (palette, lookup tables, mipmaps, ...).
   * Prepare() must be able to handle being called twice or more
   * without ill effects.
   */
  virtual void Prepare () = 0;

  /**
   * Register a texture. In this function, the texture will be converted
   * to an internal format. The given input image is AddRef'd and Released later
   * when no longer useful. If you want to keep the input image make sure you
   * have called AddRef yourselves.<p>
   *
   * This function returns a handle which should be given
   * to the 3D rasterizer or 2D driver when drawing or otherwise using
   * the texture. Note that the newly added texture will not be valid
   * until Prepare() or MergeTexture() is called.<p>
   * 
   * If 'for3d' is true then the texture is prepared for the 3D rasterizer.
   * If 'for2d' is true then the texture is prepared for the 2D driver.
   * Both can be true at the same time.
   */
  virtual iTextureHandle *RegisterTexture (iImageFile* image, bool for3d, bool for2d) = 0;

  /**
   * Unregister a texture. Note that this will have no effect on the
   * possible palette and lookup tables until after Prepare() is called
   * again.
   */
  virtual void UnregisterTexture (iTextureHandle* handle) = 0;

  /**
   * Merge a texture. If you just registered a texture with RegisterTexture()
   * then you can use this function to merge it to the current palette and
   * lookup tables. After calling this function you can use the texture as
   * if Prepare() has been called. This function will not actually recalculate
   * the palette and lookup tables but instead convert the given texture so
   * to the current palette.
   */
  virtual void MergeTexture (iTextureHandle *handle) = 0;

  /**
   * Call this function if you want to release all csImageFile's as
   * given to this texture manager. After FreeImages() has been called
   * it is no longer allowed to call Prepare() again. So the advantage
   * of calling FreeImages() is that you gain memory (may be a lot)
   * but the disadvantage is that when you want to add textures later
   * you have to reload them all and start all over.
   */
  virtual void FreeImages () = 0;

  /**
   * Reserve RGB. Call this function to reserve a color
   * from the palette (if any). This function only takes effect after
   * the next call to Prepare(). Note that black and white are already
   * preallocated colors.
   */
  virtual void ReserveColor (int r, int g, int b) = 0;

  /**
   * After calling Prepare() you can call this function to allocate
   * the palette to the 2D driver. @@@ Is this the right place for this function?
   */
  virtual void AllocPalette () = 0;

  /**
   * Return a color.
   */
  virtual int FindRGB (int r, int g, int b) = 0;

  /**
   * Return true if VERYNICE mipmap mode is used. This is an
   * ugly way to get this value. We need better user-config capabilities for this.@@@
   */
  virtual bool GetVeryNice () = 0;

  /**
   * Set verbose mode on/off.
   */
  virtual void SetVerbose (bool vb) = 0;
};

#endif // __ITXTMGR_H__
