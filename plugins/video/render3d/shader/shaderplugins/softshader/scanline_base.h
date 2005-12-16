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
    public scfImplementation2<ScanlineRendererBase, 
			      iScanlineRenderer,
			      iDefaultScanlineRenderer>
  {
  public:
    Pixel flat_col;
    uint32* bitmap;
    int v_shift_r;
    int and_w;
    int and_h;
    int colorShift;
    int alphaShift;

    ScanlineRendererBase() : scfImplementationType (this),
      flat_col (255, 255, 255, 255),
      colorShift(16), alphaShift (16) {}
    virtual ~ScanlineRendererBase() {}

    void SetFlatColor (const csVector4& v)
    {
      flat_col.c.r = csClamp ((int)(v.x * 255.99f), 255, 0);
      flat_col.c.g = csClamp ((int)(v.y * 255.99f), 255, 0);
      flat_col.c.b = csClamp ((int)(v.z * 255.99f), 255, 0);
      flat_col.c.a = csClamp ((int)(v.w * 255.99f), 255, 0);
    }
    void SetShift (int c, int a) 
    { 
      colorShift = 16-c;
      alphaShift = 16-a;
    }
  };
} // namespace cspluginSoftshader

#endif // __CS_SOFTSHADER_SCANLINE_BASE_H__
