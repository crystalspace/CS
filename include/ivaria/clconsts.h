/*
  Copyright (C) 2011 by Matthieu Kraus

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
#include CS_HEADER_GLOBAL(CS_OPENCL_PATH,cl.h)

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
    // logarithm dualis of the
    // channel size in bytes
    FMT_SIZE    = 0x03,
    FMT_SIGNED  = 0x04,
    FMT_NORMAL  = 0x08,
    FMT_PACKED  = 0x10,
    FMT_FP      = 0x20,

    // only used to differentiate those where all
    // other flags and the size are equal
    FMT_SPECIAL = 0x40,

    // flag to be used to get the channel
    // type from a format mask
    FMT_TYPE    = 0xFF,

    // normalized formats
    FMT_UINT8_N  = 0 | FMT_NORMAL,
    FMT_UINT16_N = 1 | FMT_NORMAL,
    FMT_SINT8_N  = 0 | FMT_SIGNED | FMT_NORMAL,
    FMT_SINT16_N = 1 | FMT_SIGNED | FMT_NORMAL,
    FMT_HALF     = 1 | FMT_FP | FMT_SIGNED | FMT_NORMAL,
    FMT_FLOAT    = 2 | FMT_FP | FMT_SIGNED | FMT_NORMAL,

    // packed formats
    FMT_UINT16_N_565    = 1 | FMT_PACKED | FMT_NORMAL | FMT_SPECIAL,
    FMT_UINT16_N_555    = 1 | FMT_PACKED | FMT_NORMAL,
    FMT_UINT32_N_101010 = 2 | FMT_PACKED | FMT_NORMAL,

    // unnormalized formats
    FMT_UINT8  = 0,
    FMT_UINT16 = 1,
    FMT_UINT32 = 2,
    FMT_SINT8  = 0 | FMT_SIGNED,
    FMT_SINT16 = 1 | FMT_SIGNED,
    FMT_SINT32 = 2 | FMT_SIGNED
  };

  enum ImageChannelOrder
  {
    // flag to be used to get the channel
    // order from a format mask
    FMT_ORDER = 0xFF00,

    // flag to be used to get the channel
    // count from a format mask
    // (similiar to FMT_SIZE)
    FMT_COUNT = 0x0300,

    // not valid with FMT_PACKED
    // single channel
    FMT_R    = 0x0400,
    FMT_Rx   = 0x0800,
    FMT_A    = 0x0C00,

    // not valid with FMT_PACKED
    // dual channel
    FMT_RG   = 0x0500,
    FMT_RGx  = 0x0900,
    FMT_RA   = 0x0D00,

    // not valid with FMT_PACKED
    // 4 channels
    FMT_RGBA = 0x0700,

    // only valid with FMT_{U,S}INT8{_N,}
    // 4 channels
    FMT_ARGB = 0x1300,
    FMT_BGRA = 0x1700,

    // only valid with FMT_PACKED
    // "single" channel (packed)
    FMT_RGB  = 0x2000,
    FMT_RGBx = 0x2400,

    // only valid with FMT_NORMAL
    // not valid with FMT_PACKED
    // single channel
    FMT_INT  = 0x3000,
    FMT_LUM  = 0x3400
  };

  enum SamplerFilterMode
  {
    FILTER_LINEAR = CL_FILTER_LINEAR,
    FILTER_NEAREST = CL_FILTER_NEAREST
  };

  enum SamplerAddressMode
  {
    ADDR_MIRRORED_REPEAT = CL_ADDRESS_MIRRORED_REPEAT,
    ADDR_REPEAT          = CL_ADDRESS_REPEAT,
    ADDR_CLAMP_TO_EDGE   = CL_ADDRESS_CLAMP_TO_EDGE,
    ADDR_CLAMP           = CL_ADDRESS_CLAMP,
    ADDR_NONE            = CL_ADDRESS_NONE
  };
} // namespace CL
} // namespace CS

#endif // __CS_OPENCL_CONSTS_H__

