/*
    Copyright (C) 2005 Dan Hardfeldt, Seth Yastrov and Jelle Hellemans

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

#ifndef _CEGUITEXTURETARGET_H_
#define _CEGUITEXTURETARGET_H_

#include "ceguiimports.h"
#include "ceguirendertarget.h"

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  /// CEGUI::TextureTarget implementation for the CS engine.
  class TextureTarget : public RenderTarget, public CEGUI::TextureTarget
  {
  public:
    /// Constructor.
    TextureTarget(Renderer& owner, iObjectRegistry* reg);
    /// Destructor.
    virtual ~TextureTarget();

    // implementation of RenderTarget interface
    bool isImageryCache() const;
    // implement CEGUI::TextureTarget interface.
    void clear();
    CEGUI::Texture& getTexture() const;
    void declareRenderSize(const CEGUI::Size& sz);
    bool isRenderingInverted() const;

  protected:
    /// default / initial size for the underlying texture.
    static const float DEFAULT_SIZE;
    /// This wraps d_texture so it can be used by the core CEGUI lib.
    Texture* d_CEGUITexture;
  };

} CS_PLUGIN_NAMESPACE_END(cegui)

#endif  // end of guard _CEGUITEXTURETARGET_H_
