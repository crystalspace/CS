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

#ifndef ITXTMGR_H
#define ITXTMGR_H

#include "cscom/com.h"
#include "csengine/cscolor.h"	//@@@BAD?

class Vector2;
class csMatrix3;
class csVector3;
class csRect;

interface IImageFile;
interface ITextureHandle;

extern const GUID IID_ITextureManager;

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
interface ITextureManager : public IUnknown
{
public:
  /**
   * Initialize the texture system. This function must be called
   * at least once and everytime we want to start all over using the textures.
   */
  COM_METHOD_DECL Initialize () PURE;

  /**
   * After all textures have been added, this function does all
   * needed calculations (palette, lookup tables, mipmaps, ...).
   * Prepare() must be able to handle being called twice or more
   * without ill effects.
   */
  COM_METHOD_DECL Prepare () PURE;

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
  COM_METHOD_DECL RegisterTexture (IImageFile* image, ITextureHandle** handle,
  	bool for3d, bool for2d) PURE;

  /**
   * Unregister a texture. Note that this will have no effect on the
   * possible palette and lookup tables until after Prepare() is called
   * again.
   */
  COM_METHOD_DECL UnregisterTexture (ITextureHandle* handle) PURE;

  /**
   * Merge a texture. If you just registered a texture with RegisterTexture()
   * then you can use this function to merge it to the current palette and
   * lookup tables. After calling this function you can use the texture as
   * if Prepare() has been called. This function will not actually recalculate
   * the palette and lookup tables but instead convert the given texture so
   * to the current palette.
   */
  COM_METHOD_DECL MergeTexture (ITextureHandle* handle) PURE;

  /**
   * Call this function if you want to release all ImageFile's as
   * given to this texture manager. After FreeImages() has been called
   * it is no longer allowed to call Prepare() again. So the advantage
   * of calling FreeImages() is that you gain memory (may be a lot)
   * but the disadvantage is that when you want to add textures later
   * you have to reload them all and start all over.
   */
  COM_METHOD_DECL FreeImages () PURE;

  /**
   * Reserve RGB. Call this function to reserve a color
   * from the palette (if any). This function only takes effect after
   * the next call to Prepare(). If the 'privcolor' parameter is true
   * then this color cannot be used by the textures. Otherwise it
   * can. Note that black and white are already allocated as private
   * colors.
   */
  COM_METHOD_DECL ReserveColor (int r, int g, int b, bool privcolor) PURE;

  /**
   * After calling Prepare() you can call this function to allocate
   * the palette to the 2D driver. @@@ Is this the right place for this function?
   */
  COM_METHOD_DECL AllocPalette () PURE;

  /**
   * Return a color.
   */
  COM_METHOD_DECL FindRGB (int r, int g, int b, int& color) PURE;

  /**
   * Return true if VERYNICE mipmap mode is used. This is an
   * ugly way to get this value. We need better user-config capabilities for this.@@@
   */
  COM_METHOD_DECL GetVeryNice (bool& result) PURE;

  /**
   * Set verbose mode on/off.
   */
  COM_METHOD_DECL SetVerbose (bool vb) PURE;
};

#endif      // ITXTMGR_H
