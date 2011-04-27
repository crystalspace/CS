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

#include "ceguiimports.h"
#include "ceguirendertarget.h"
#include "ceguigeometrybuffer.h"

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  //----------------------------------------------------------------------------//
  RenderTarget::RenderTarget(Renderer& owner, iObjectRegistry* reg) :
    d_owner(owner),
    d_area(0, 0, 0, 0),
    //d_renderTarget(0),
    //d_viewport(0),
    d_matrix(),
    d_matrixValid(false),
    d_viewportValid(false)
  {
  }

  //----------------------------------------------------------------------------//
  RenderTarget::~RenderTarget()
  {
    //delete d_viewport;
  }

  //----------------------------------------------------------------------------//
  void RenderTarget::draw(const CEGUI::GeometryBuffer& buffer)
  {
    buffer.draw();
  }

  //----------------------------------------------------------------------------//
  void RenderTarget::draw(const CEGUI::RenderQueue& queue)
  {
    queue.draw();
  }

  //----------------------------------------------------------------------------//
  void RenderTarget::setArea(const CEGUI::Rect& area)
  {
    d_area = area;
    d_matrixValid = false;
    d_viewportValid = false;
  }

  //----------------------------------------------------------------------------//
  const CEGUI::Rect& RenderTarget::getArea() const
  {
    return d_area;
  }

  //----------------------------------------------------------------------------//
  void RenderTarget::activate()
  {
    if (!d_matrixValid)
      updateMatrix();

    if (!d_viewportValid)
      updateViewport();

    //d_renderSystem._setViewport(d_viewport);
    //d_renderSystem._setProjectionMatrix(d_matrix);
    //d_renderSystem._setViewMatrix(CS::Matrix4::IDENTITY);
  }

  //----------------------------------------------------------------------------//
  void RenderTarget::deactivate()
  {
    // currently nothing to do in the basic case
  }

  //----------------------------------------------------------------------------//
  void RenderTarget::unprojectPoint(const CEGUI::GeometryBuffer& buff,
                                    const CEGUI::Vector2& p_in, CEGUI::Vector2& p_out) const
  {
  }

  //----------------------------------------------------------------------------//
  void RenderTarget::updateMatrix() const
  {
  }

  //----------------------------------------------------------------------------//
  void RenderTarget::updateViewport()
  {
  }

  //----------------------------------------------------------------------------//

} CS_PLUGIN_NAMESPACE_END(cegui)
