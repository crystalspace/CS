/*
    Copyright (C) 2006 by Michael D. Adams

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

#include "cssysdef.h"

#include "sft3dcom.h"
#include "clipper.h"
#include "clip_znear.h"
#include "clip_iclipper.h"
#include "scan_pix.h"
#include "tridraw.h"

CS_PLUGIN_NAMESPACE_BEGIN(Soft3D)
{
  template<typename Pix>
  inline void TriDrawMatrixFiller_Fill(csSoftwareGraphics3DCommon* g3d,
    iTriangleDrawer** matrix)
  {
    TriDrawMatrixFiller<Pix>::Fill (g3d, matrix);
  }

#define SPECIALIZE_FILL(PIX, NAME) \
  extern void NAME(csSoftwareGraphics3DCommon*, iTriangleDrawer**); \
  template<> \
  inline void TriDrawMatrixFiller_Fill<PIX>(csSoftwareGraphics3DCommon* g3d, \
    iTriangleDrawer** matrix) \
  { NAME (g3d, matrix); }

#define DEFINE_FILL(PIX, NAME) \
  void NAME(csSoftwareGraphics3DCommon* g3d, \
    iTriangleDrawer** matrix) \
  { TriDrawMatrixFiller<PIX>::Fill (g3d, matrix); }

#define PIX Pix_Generic<uint32>
SPECIALIZE_FILL(PIX, TriDrawMatrixFiller_Fill_Generic_uint32)
#undef PIX

#define PIX Pix_Generic<uint16>
SPECIALIZE_FILL(PIX, TriDrawMatrixFiller_Fill_Generic_uint16)
#undef PIX

#define PIX Pix_Fix<uint32, 24, 0xff, 16, 0xff, 8,  0xff, 0, 0xff>
SPECIALIZE_FILL(PIX, TriDrawMatrixFiller_Fill_Fix_uint32)
#undef PIX

#define PIX Pix_Fix<uint16, 0,  0, 8,  0xf8, 3,  0xfc, -3, 0xf8>
SPECIALIZE_FILL(PIX, TriDrawMatrixFiller_Fill_Fix_uint16_a)
#undef PIX

#define PIX Pix_Fix<uint16, 0,  0, 7,  0xf8, 2,  0xf8, -3, 0xf8>
SPECIALIZE_FILL(PIX, TriDrawMatrixFiller_Fill_Fix_uint16_b)
#undef PIX

#undef FILL
  
}
CS_PLUGIN_NAMESPACE_END(Soft3D)
