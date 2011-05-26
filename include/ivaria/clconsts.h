/*
  Copyright (C) 2010 by Matthieu Kraus

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_OPENCL_CONSTS_H__
#define __CS_OPENCL_CONSTS_H__

// cl types required to write data to buffers (cl_float, cl_char, ...)
#include <CL/cl_platform.h>

namespace CS
{
namespace CL
{
  enum MemoryAccessType
  {
    MEM_READ       = 1,
    MEM_WRITE      = 2,
    MEM_READ_WRITE = MEM_READ | MEM_WRITE
  };

  enum MemoryType
  {
    MEM_BUFFER,
    MEM_IMAGE,
    MEM_SAMPLER
  };

  enum ImageChannelType
  {
    FMT_SIZE    = 3,
    FMT_SIGNED  = 1 << 2,
    FMT_NORMAL  = 1 << 3,
    FMT_PACKED  = 1 << 4,
    FMT_FLOAT   = 1 << 5,

    FMT_UINT8_N  = 0 | FMT_NORMAL,
    FMT_UINT16_N = 1 | FMT_NORMAL,
    FMT_SINT8_N  = 0 | FMT_SIGNED | FMT_NORMAL,
    FMT_SINT16_N = 1 | FMT_SIGNED | FMT_NORMAL,
    FMT_HALF     = 1 | FMT_FLOAT | FMT_SIGNED | FMT_NORMAL,
    FMT_FLOAT    = 2 | FMT_FLOAT | FMT_SIGNED | FMT_NORMAL,

    FMT_UINT16_N_565    = 0 | FMT_PACKED,
    FMT_UINT16_N_555    = 1 | FMT_PACKED,
    FMT_UINT32_N_101010 = 2 | FMT_PACKED,

    FMT_UINT8  = 0,
    FMT_UINT16 = 1,
    FMT_UINT32 = 2,
    FMT_SINT8  = 0 | FMG_SIGNED,
    FMT_SINT16 = 1 | FMT_SIGNED,
    FMT_SINT32 = 2 | FMT_SIGNED
  };

  enum ImageChannelOrder
  {
    FMT_R    = 0x1 << 8,
    FMT_Rx   = 0x2 << 8,
    FMT_A    = 0x3 << 8,
    FMT_RG   = 0x4 << 8,
    FMT_RGx  = 0x5 << 8,
    FMT_RA   = 0x6 << 8,
    FMT_RGBA = 0x7 << 8,

    // only valid with FMT_{U,S}INT8{_N,}
    // (FMT_SIZE = 0 && !(FMT_PACKED|FMT_FLOAT))
    FMT_ARGB = 0x10 << 8,
    FMT_BGRA = 0x11 << 8,

    // only valid with FMT_PACKED
    FMT_RGB  = 0x20 << 8,
    FMT_RGBx = 0x21 << 8,

    // only valid with FMT_NORMAL
    FMT_INT  = 0x30 << 8,
    FMT_LUM  = 0x31 << 8
  };

  enum SamplerFilterMode
  {
    FILTER_LINEAR,
    FILTER_NEAREST
  };

  enum SamplerAddressMode
  {
    ADDR_MIRRORED_REPEAT,
    ADDR_REPEAT,
    ADDR_CLAMP_TO_EDGE,
    ADDR_CLAMP,
    ADDR_NONE
  };
} // namespace CL
} // namespace CS

#endif // __CS_OPENCL_CONSTS_H__

