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

#ifndef __CS_SOFTSHADER_SCANLINE_BASE_H__
#define __CS_SOFTSHADER_SCANLINE_BASE_H__

#include "csgeom/math.h"
#include "csplugincommon/softshader/scanline.h"
#include "csutil/scf_implementation.h"

namespace cspluginSoftshader
{
  using namespace CrystalSpace::SoftShader;

  class ScanlineRendererBase : 
    public scfImplementation1<ScanlineRendererBase, iScanlineRenderer>
  {
  public:
    uint8 flat_r, flat_g, flat_b, flat_a;
    uint32* bitmap;
    int v_shift_r;
    int and_w;
    int and_h;
    int colorShift;

    ScanlineRendererBase() : scfImplementationType (this),
      flat_r(255), flat_g(255), flat_b(255), flat_a(255),
      colorShift(16) {}
    virtual ~ScanlineRendererBase() {}

    void SetFlatColor (const csVector4& v)
    {
      flat_r = csClamp ((int)(v.x * 255.99f), 255, 0);
      flat_g = csClamp ((int)(v.y * 255.99f), 255, 0);
      flat_b = csClamp ((int)(v.z * 255.99f), 255, 0);
      flat_a = csClamp ((int)(v.w * 255.99f), 255, 0);
    }
    void SetShift (int x) { colorShift = 16-x; }
  };
} // namespace cspluginSoftshader

#endif // __CS_SOFTSHADER_SCANLINE_BASE_H__
