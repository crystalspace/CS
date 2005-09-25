/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005 by Frank Richter

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

#ifndef __CS_SOFT3D_SCANLINE_H__
#define __CS_SOFT3D_SCANLINE_H__

#include "types.h"

namespace cspluginSoft3d
{

  class ScanlineRendererBase
  {
  public:
    typedef void (ScanlineRendererBase::*ScanlineProc) (
      InterpolateScanline& ipol);

    virtual ScanlineProc Init () = 0;
  };

  class ScanlineRenderer : public ScanlineRendererBase
  {
    void Scan (InterpolateScanline& ipol)
    {
    }
  public:
    ScanlineProc Init ()
    {
      return (ScanlineProc)&ScanlineRenderer::Scan;
    }
  };

} // namespace cspluginSoft3d

#endif // __CS_SOFT3D_SCANLINE_H__
