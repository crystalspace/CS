/*
    Copyright (C) 2005 Dan Hardfeldt and Seth Yastrov

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _CS_CEGUITEXTURE_H_
#define _CS_CEGUITEXTURE_H_

/**\file 
*/
/**
* \addtogroup CEGUI
* @{ */

#include "csutil/ref.h"
#include "ivideo/texture.h"

#include "CEGUI.h"
#include "ceguirenderer.h"

struct iObjectRegistry;

/// CS implementation of CEGUI::Texture.
class csCEGUITexture : public CEGUI::Texture
{
  friend class csCEGUIRenderer;
public:
  /// Constructor.
  csCEGUITexture (CEGUI::Renderer*, iObjectRegistry*);

  /// Destructor.
  virtual ~csCEGUITexture ();

  /// Get the width of the texture.
  virtual CEGUI::ushort getWidth (void) const;

  /// Get the height of the texture.
  virtual CEGUI::ushort getHeight (void) const;

  /// Load a texture from a file.
  virtual void loadFromFile (const CEGUI::String &filename, 
    const CEGUI::String &resourceGroup);

  /**
   * Load a texture directly from memory. This is called from CEGUI,
   * for example, when a font should be loaded.
   */
  virtual void loadFromMemory (const void *buffPtr, 
    CEGUI::uint buffWidth, CEGUI::uint buffHeight);

  /// Get a handle to the texture.
  iTextureHandle* GetTexHandle();

private:
  /// Returns the renderer
  CEGUI::Renderer* getRenderer (void) const;
  CEGUI::Renderer* renderer;
  csRef<iTextureHandle> hTxt;
  iObjectRegistry* obj_reg;

};

#endif 
