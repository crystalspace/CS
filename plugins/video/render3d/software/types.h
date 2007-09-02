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

#ifndef __CS_SOFT3D_TYPES_H__
#define __CS_SOFT3D_TYPES_H__

#include "ivideo/graph3d.h"

CS_PLUGIN_NAMESPACE_BEGIN(Soft3D)
{
  #define VATTR_SPEC(x)           (CS_VATTRIB_ ## x - CS_VATTRIB_SPECIFIC_FIRST)
  #define VATTR_GEN(x)							      \
    ((CS_VATTRIB_ ## x - CS_VATTRIB_GENERIC_FIRST) + CS_VATTRIB_SPECIFIC_LAST + 1)

  static const size_t activeBufferCount = CS_VATTRIB_SPECIFIC_LAST - 
    CS_VATTRIB_SPECIFIC_FIRST + 1;
  static const size_t activeTextureCount = 4;

  /// A buffer used to pass vertex data around in the renderer
  struct VertexBuffer
  {
    uint8* data;
    size_t comp;
  };
}
CS_PLUGIN_NAMESPACE_END(Soft3D)

#endif // __CS_SOFT3D_TYPES_H__
