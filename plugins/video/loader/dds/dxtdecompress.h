/*
    Copyright (C) 2011 by Frank Richter

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

/**\file
 * CS::Graphics::iDXTDecompressor implementation
 */

#ifndef __DXTDECOMPRESS_H__
#define __DXTDECOMPRESS_H__

#include "igraphic/dxtcompress.h"
#include "iutil/comp.h"

#include "csutil/scf_implementation.h"

CS_PLUGIN_NAMESPACE_BEGIN(DDSImageIO)
{
  class DXTDecompressor :
    public scfImplementation2<DXTDecompressor,
                              iComponent,
                              CS::Graphics::iDXTDecompressor>
  {
  public:
    DXTDecompressor (iBase* parent);

    /**\name iComponent implementation
     * @{ */
    bool Initialize (iObjectRegistry* object_reg);
    /** @} */

    /**\name CS::Graphics::iDXTDecompressor implementation
     * @{ */
    const void* DecompressDXT1RGB (const void* inBlockPtr,
                                   size_t blockDistance,
                                   size_t numBlocks,
                                   csRGBpixel* outDataPtr,
                                   const CS::Graphics::UncompressedDXTDataLayout& outDataLayout);
    const void* DecompressDXT1RGBA (const void* inBlockPtr,
                                    size_t blockDistance,
                                    size_t numBlocks,
                                    csRGBpixel* outDataPtr,
                                    const CS::Graphics::UncompressedDXTDataLayout& outDataLayout);

    const void* DecompressDXT3Alpha (const void* inBlockPtr,
                                     size_t blockDistance,
                                     size_t numBlocks,
                                     uint8* outDataPtr,
                                     const CS::Graphics::UncompressedDXTDataLayout& outDataLayout);

    const void* DecompressDXTUNormToUI8 (const void* inBlockPtr,
                                         size_t blockDistance,
                                         size_t numBlocks,
                                         uint8* outDataPtr,
                                         const CS::Graphics::UncompressedDXTDataLayout& outDataLayout);
    const void* DecompressDXTSNormToI8 (const void* inBlockPtr,
                                        size_t blockDistance,
                                        size_t numBlocks,
                                        int8* outDataPtr,
                                        const CS::Graphics::UncompressedDXTDataLayout& outDataLayout);

    const void* DecompressDXTUNormToUI16 (const void* inBlockPtr,
                                          size_t blockDistance,
                                          size_t numBlocks,
                                          uint16* outDataPtr,
                                          const CS::Graphics::UncompressedDXTDataLayout& outDataLayout);
    const void* DecompressDXTSNormToI16 (const void* inBlockPtr,
                                         size_t blockDistance,
                                         size_t numBlocks,
                                         int16* outDataPtr,
                                         const CS::Graphics::UncompressedDXTDataLayout& outDataLayout);
    /** @} */
  };
}
CS_PLUGIN_NAMESPACE_END(DDSImageIO)

#endif // __DXTDECOMPRESS_H__
