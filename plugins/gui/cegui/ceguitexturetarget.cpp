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

#include "ceguitexturetarget.h"
#include "ceguitexture.h"


CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  //----------------------------------------------------------------------------//
  const float TextureTarget::DEFAULT_SIZE = 128.0f;

  //----------------------------------------------------------------------------//
  TextureTarget::TextureTarget(Renderer& owner, iObjectRegistry* reg) :
    RenderTarget(owner, reg),
    d_CEGUITexture(0)
  {
  }

  //----------------------------------------------------------------------------//
  TextureTarget::~TextureTarget()
  {
    d_owner.destroyTexture(*d_CEGUITexture);
  }

  //----------------------------------------------------------------------------//
  bool TextureTarget::isImageryCache() const
  {
    return true;
  }

  //----------------------------------------------------------------------------//
  void TextureTarget::clear()
  {
    if (!d_viewportValid)
      updateViewport();
  }

  //----------------------------------------------------------------------------//
  CEGUI::Texture& TextureTarget::getTexture() const
  {
    return *d_CEGUITexture;
  }

  //----------------------------------------------------------------------------//
  void TextureTarget::declareRenderSize(const CEGUI::Size& sz)
  {
  }

  //----------------------------------------------------------------------------//
  bool TextureTarget::isRenderingInverted() const
  {
    return false;
  }

//----------------------------------------------------------------------------//

} CS_PLUGIN_NAMESPACE_END(cegui)
