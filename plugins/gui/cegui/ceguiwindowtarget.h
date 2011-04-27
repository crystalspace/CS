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

#ifndef _CEGUIWINDOWTARGET_H_
#define _CEGUIWINDOWTARGET_H_

#include "ceguiimports.h"
#include "ceguirendertarget.h"

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  /// CEGUI::RenderTarget
  class WindowTarget : public RenderTarget
  {
  public:
    /// Constructor
    WindowTarget(Renderer& owner, iObjectRegistry* reg);

    /// Destructor
    virtual ~WindowTarget();

    // implement parts of CEGUI::RenderTarget interface
    bool isImageryCache() const;
  };

} CS_PLUGIN_NAMESPACE_END(cegui)

#endif  // end of guard _CEGUIWINDOWTARGET_H_
