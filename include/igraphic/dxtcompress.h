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
 * DXT compressor/decompressor interface
 */

#ifndef __CS_IGRAPHIC_DXTCOMPRESS_H__
#define __CS_IGRAPHIC_DXTCOMPRESS_H__

#include "csgfx/rgbpixel.h"
#include "csutil/scf_interface.h"

/**
 * \addtogroup gfx2d
 * @{
 */

namespace CS
{
  namespace Graphics
  {
    /**
     * Description on how pixel data before compression resp. after
     * decompression is arranged in blocks.
     * DXT (and related) compression methods work on 4x4 blocks.
     * This structure contains the necessary information to locate the
     * individual pixels in a block as well as the next block to work on.
     */
    struct UncompressedDXTDataLayout
    {
      /**
       * Distance (in bytes) to the next pixel in a row.
       * If 0, the effective pixel distance is the size of the uncompressed
       * input/output data type (which depends on the context).
       */
      size_t bytesToNextPixel;
      /**
       * Distance (in bytes) to the next row of pixels, from the beginning of
       * the last row.
       * If 0, the effective row distance is 4 times the effective pixel distance.
       */
      size_t bytesToNextRow;
      /**
       * Distance (in bytes) to the next block of pixels, from the beginning of
       * the last block.
       * If 0, the effective block distance is 4 times the effective pixel distance.
       */
      size_t bytesToNextBlock;
      /**
       * Number of blocks per row. After that number of blocks was decompressed
       * the output pointer is advanced to the next row of destination blocks,
       * which is computed from the start of the last row of destination blocks
       * plus four times bytesToNextRow.
       */
      size_t blocksPerRow;

      UncompressedDXTDataLayout()
        : bytesToNextPixel (0), bytesToNextRow (0), bytesToNextBlock (0), blocksPerRow (1) {}

      /**
       * Set members with value 0 to default values based on the given size of
       * the uncompressed data type.
       */
      void Fix (size_t uncompressedSize)
      {
        if (!bytesToNextPixel) bytesToNextPixel = uncompressedSize;
        if (!bytesToNextRow) bytesToNextRow = 4*bytesToNextPixel;
        if (!bytesToNextBlock) bytesToNextBlock = 4*bytesToNextPixel;
      }
    };

    /**\def CS_DXTDECOMPRESSOR_DEFAULT
     * Class ID of the default DXT decompressor implementation
     */
    #define CS_DXTDECOMPRESSOR_DEFAULT "crystalspace.graphic.dxt.decompress.default"

    struct iDXTDecompressor : public virtual iBase
    {
      SCF_INTERFACE(CS::Graphics::iDXTDecompressor, 0, 0, 1);

      /**
       * Decompress a number of DXT1 (aka BC1) color blocks to the RGB
       * components of a csRGBpixel array.
       * Notably, the alpha components of the destination csRGBpixel array
       * will *not* be modified!
       * \param inBlockPtr Pointer to input DXT block data, starting at the
       *    first color block.
       * \param blockDistance Distance between color blocks, in bytes.
       * \param numBlocks Number of blocks to decode.
       * \param outDataPtr Destination csRGBpixel array.
       * \param outDataLayout Layout of the destination array.
       * \returns Pointer to the would-be next input block.
       */
      virtual const void* DecompressDXT1RGB (const void* inBlockPtr,
                                             size_t blockDistance,
                                             size_t numBlocks,
                                             csRGBpixel* outDataPtr,
                                             const UncompressedDXTDataLayout& outDataLayout) = 0;
      /**
       * Decompress a number of DXT1 (aka BC1) color blocks to the RGBA
       * components of a csRGBpixel array.
       * The alpha components of the destination csRGBpixel array are set to
       * the binary alpha value extracted from the color block values.
       * \param inBlockPtr Pointer to input DXT block data, starting at the
       *    first color block.
       * \param blockDistance Distance between color blocks, in bytes.
       * \param numBlocks Number of blocks to decode.
       * \param outDataPtr Destination csRGBpixel array.
       * \param outDataLayout Layout of the destination array.
       * \returns Pointer to the would-be next input block.
       */
      virtual const void* DecompressDXT1RGBA (const void* inBlockPtr,
                                              size_t blockDistance,
                                              size_t numBlocks,
                                              csRGBpixel* outDataPtr,
                                              const UncompressedDXTDataLayout& outDataLayout) = 0;

      /**
       * Decompress a number of DXT3 (aka BC2) alpha blocks to an array of
       * unsigned 8-bit values.
       * \param inBlockPtr Pointer to input DXT block data, starting at the
       *    first alpha block.
       * \param blockDistance Distance between alpha blocks, in bytes.
       * \param numBlocks Number of blocks to decode.
       * \param outDataPtr Destination unsigned 8-bit array.
       * \param outDataLayout Layout of the destination array.
       * \returns Pointer to the would-be next input block.
       */
      virtual const void* DecompressDXT3Alpha (const void* inBlockPtr,
                                               size_t blockDistance,
                                               size_t numBlocks,
                                               uint8* outDataPtr,
                                               const UncompressedDXTDataLayout& outDataLayout) = 0;

      /**
       * Decompress a number of DXT UNORM blocks to an array of
       * unsigned 8-bit values.
       * These blocks serve as the alpha blocks in the DXT5 (aka BC3) format
       * as well as the component blocks in the BC4_UNORM, BC5_UNORM and
       * 3Dc formats.
       * \param inBlockPtr Pointer to input DXT block data, starting at the
       *    first alpha block.
       * \param blockDistance Distance between alpha blocks, in bytes.
       * \param numBlocks Number of blocks to decode.
       * \param outDataPtr Destination unsigned 8-bit array.
       * \param outDataLayout Layout of the destination array.
       * \returns Pointer to the would-be next input block.
       */
      virtual const void* DecompressDXTUNormToUI8 (const void* inBlockPtr,
                                                   size_t blockDistance,
                                                   size_t numBlocks,
                                                   uint8* outDataPtr,
                                                   const UncompressedDXTDataLayout& outDataLayout) = 0;
      /**
       * Decompress a number of DXT UNORM blocks to an array of
       * signed 8-bit values.
       * These blocks serve as the component blocks in the BC4_SNORM and
       * BC5_SNORM formats.
       * \param inBlockPtr Pointer to input DXT block data, starting at the
       *    first alpha block.
       * \param blockDistance Distance between alpha blocks, in bytes.
       * \param numBlocks Number of blocks to decode.
       * \param outDataPtr Destination unsigned 8-bit array.
       * \param outDataLayout Layout of the destination array.
       * \returns Pointer to the would-be next input block.
       */
      virtual const void* DecompressDXTSNormToI8 (const void* inBlockPtr,
                                                  size_t blockDistance,
                                                  size_t numBlocks,
                                                  int8* outDataPtr,
                                                  const UncompressedDXTDataLayout& outDataLayout) = 0;

      /**
       * Decompress a number of DXT UNORM blocks to an array of
       * unsigned 16-bit values.
       * These blocks serve as the alpha blocks in the DXT5 (aka BC3) format
       * as well as the component blocks in the BC4_UNORM, BC5_UNORM and
       * 3Dc formats.
       * The compression format can store values with a precision higher than
       * 8 bit, so decompressing to 16 bit gives higher quality results.
       * \param inBlockPtr Pointer to input DXT block data, starting at the
       *    first alpha block.
       * \param blockDistance Distance between alpha blocks, in bytes.
       * \param numBlocks Number of blocks to decode.
       * \param outDataPtr Destination unsigned 16-bit array.
       * \param outDataLayout Layout of the destination array.
       * \returns Pointer to the would-be next input block.
       */
      virtual const void* DecompressDXTUNormToUI16 (const void* inBlockPtr,
                                                    size_t blockDistance,
                                                    size_t numBlocks,
                                                    uint16* outDataPtr,
                                                    const UncompressedDXTDataLayout& outDataLayout) = 0;
      /**
       * Decompress a number of DXT UNORM blocks to an array of
       * signed 16-bit values.
       * These blocks serve as the component blocks in the BC4_SNORM and
       * BC5_SNORM formats.
       * The compression format can store values with a precision higher than
       * 8 bit, so decompressing to 16 bit gives higher quality results.
       * \param inBlockPtr Pointer to input DXT block data, starting at the
       *    first alpha block.
       * \param blockDistance Distance between alpha blocks, in bytes.
       * \param numBlocks Number of blocks to decode.
       * \param outDataPtr Destination unsigned 16-bit array.
       * \param outDataLayout Layout of the destination array.
       * \returns Pointer to the would-be next input block.
       */
      virtual const void* DecompressDXTSNormToI16 (const void* inBlockPtr,
                                                   size_t blockDistance,
                                                   size_t numBlocks,
                                                   int16* outDataPtr,
                                                   const UncompressedDXTDataLayout& outDataLayout) = 0;
    };

/*
    struct iDXTCompressor : public virtual iBase
    {
    };
*/
  } // namespace Graphics
} // namespace CS

/** @} */

#endif // __CS_IGRAPHIC_DXTCOMPRESS_H__
