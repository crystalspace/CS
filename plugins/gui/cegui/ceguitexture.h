/*
    Copyright (C) 2005 Dan Hardfeldt and Seth Yastrov

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

#ifndef _CS_CEGUITEXTURE_H_
#define _CS_CEGUITEXTURE_H_


/**\file 
*/
/**
* \addtogroup CEGUI
* @{ */

#include "csutil/ref.h"
#include "ivideo/texture.h"
#include "ceguiimports.h"
#include "ceguirenderer.h"

struct iObjectRegistry;

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  /// CS implementation of CEGUI::Texture.
  class Texture : public CEGUI::Texture
  {
    friend class csCEGUIRenderer;
  public:
    /// Constructor.
    Texture (CEGUI::Renderer*, iObjectRegistry*);

    /// Destructor.
    virtual ~Texture ();

    /// Get the size of the texture.
    virtual const CEGUI::Size& getSize() const;

    virtual const CEGUI::Size& getOriginalDataSize() const;

    virtual const CEGUI::Vector2& getTexelScaling() const;

    /// Load a texture from a file.
    virtual void loadFromFile (const CEGUI::String &filename, 
                               const CEGUI::String &resourceGroup);

    /**
     * Load a texture directly from memory. This is called from CEGUI,
     * for example, when a font should be loaded.
     */
    virtual void loadFromMemory (const void *buffPtr,
                                 const CEGUI::Size& buffer_size, 
                                 CEGUI::Texture::PixelFormat pixFmt);

    virtual void saveToMemory(void* buffer);

    /// Get a handle to the texture.
    iTextureHandle* GetTexHandle() const;

    /// Set a handle to the texture.
    void SetTexHandle( iTextureHandle*);

  private:
    /// Returns the renderer
    CEGUI::Renderer* getRenderer (void) const;
    CEGUI::Renderer* renderer;
    void updateCachedSizeValues();
    void updateCachedScaleValues();
    csRef<iTextureHandle> hTxt;
    iObjectRegistry* obj_reg;

    mutable CEGUI::Size size;
    CEGUI::Size dataSize;
    CEGUI::Vector2 texelScaling;
  };

} CS_PLUGIN_NAMESPACE_END(cegui)

#endif 
