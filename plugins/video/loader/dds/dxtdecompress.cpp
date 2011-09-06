/*
    Copyright (C) 2006-2011 by Frank Richter
              (C) 2003 by Matze Braun <matze@braunis.de>

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

#include "dxtdecompress.h"

#include "csutil/csendian.h"
#include "csutil/scf.h"

CS_PLUGIN_NAMESPACE_BEGIN(DDSImageIO)
{
  SCF_IMPLEMENT_FACTORY(DXTDecompressor)

  DXTDecompressor::DXTDecompressor (iBase* parent)
    : scfImplementationType (this, parent)
  { }

  bool DXTDecompressor::Initialize (iObjectRegistry* object_reg)
  {
    return true;
  }

  static inline int ColorComponent (int x, int bitcount, int shift)
  {
    int val = ((x >> shift) & ((1 << bitcount) - 1));
    return (val << (8 - bitcount)) + (val >> (2 * bitcount - 8));
  }

#define COLOR565_RED(x)     ColorComponent(x, 5, 11)
#define COLOR565_GREEN(x)   ColorComponent(x, 6, 5)
#define COLOR565_BLUE(x)    ColorComponent(x, 5, 0)

  template <bool withAlpha>
  static inline void DecodeDXT1Color (const uint8* block, uint8* outColor,
                                      const CS::Graphics::UncompressedDXTDataLayout& outColorLayout)
  {
    uint16       color_0, color_1;
    csRGBpixel   colours[4];
    uint32       bitmask;

    color_0 = csLittleEndian::Convert (*((uint16*)block));
    color_1 = csLittleEndian::Convert (*((uint16*)block+1));
    bitmask = csLittleEndian::Convert (((uint32*)block)[1]);

    colours[0].red   = COLOR565_RED(color_0);
    colours[0].green = COLOR565_GREEN(color_0);
    colours[0].blue  = COLOR565_BLUE(color_0);
    colours[0].alpha = 0xFF;

    colours[1].red   = COLOR565_RED(color_1);
    colours[1].green = COLOR565_GREEN(color_1);
    colours[1].blue  = COLOR565_BLUE(color_1);
    colours[1].alpha = 0xFF;

    if (color_0 > color_1)
    {
      // Four-color block: derive the other two colors.
      // 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
      // These 2-bit codes correspond to the 2-bit fields
      // stored in the 64-bit block.
      colours[2].blue  = (2 * colours[0].blue  + colours[1].blue  + 1) / 3;
      colours[2].green = (2 * colours[0].green + colours[1].green + 1) / 3;
      colours[2].red   = (2 * colours[0].red   + colours[1].red   + 1) / 3;
      colours[2].alpha = 0xFF;

      colours[3].blue  = (colours[0].blue  + 2 * colours[1].blue  + 1) / 3;
      colours[3].green = (colours[0].green + 2 * colours[1].green + 1) / 3;
      colours[3].red   = (colours[0].red   + 2 * colours[1].red   + 1) / 3;
      colours[3].alpha = 0xFF;
    }
    else
    {
      // Three-color block: derive the other color.
      // 00 = color_0,  01 = color_1,  10 = color_2,
      // 11 = transparent.
      // These 2-bit codes correspond to the 2-bit fields
      // stored in the 64-bit block.
      colours[2].blue  = (colours[0].blue  + colours[1].blue ) / 2;
      colours[2].green = (colours[0].green + colours[1].green) / 2;
      colours[2].red   = (colours[0].red   + colours[1].red  ) / 2;
      colours[2].alpha = 0xFF;

      colours[3].blue  = 0;
      colours[3].green = 0;
      colours[3].red   = 0;
      colours[3].alpha = 0x00;
    }

    uint8* outRow (outColor);
    for (int j = 0, k = 0; j < 4; j++)
    {
      uint8* outPixel (outRow);
      for (int i = 0; i < 4; i++, k++)
      {
        int Select = (bitmask & (0x03 << k*2)) >> k*2;
        const csRGBpixel& col = colours[Select];
        csRGBpixel* outPixelRGBA (reinterpret_cast<csRGBpixel*> (outPixel));
        if (withAlpha)
          *outPixelRGBA = col;
        else
        {
          outPixelRGBA->red   = col.red;
          outPixelRGBA->green = col.green;
          outPixelRGBA->blue  = col.blue;
        }

        outPixel += outColorLayout.bytesToNextPixel;
      }
      outRow += outColorLayout.bytesToNextRow;
    }
  }

  const void* DXTDecompressor::DecompressDXT1RGB (const void* inBlockPtr,
                                                  size_t blockDistance,
                                                  size_t numBlocks,
                                                  csRGBpixel* outDataPtr,
                                                  const CS::Graphics::UncompressedDXTDataLayout& outDataLayout)
  {
    CS::Graphics::UncompressedDXTDataLayout outLayout (outDataLayout);
    outLayout.Fix (sizeof (csRGBpixel));

    const uint8* inBlockUI8 (reinterpret_cast<const uint8*> (inBlockPtr));
    uint8* outRow (reinterpret_cast<uint8*> (outDataPtr));
    uint8* outDataBlock (outRow);
    size_t rowBlocks (outDataLayout.blocksPerRow);
    while (numBlocks-- > 0)
    {
      DecodeDXT1Color<false> (inBlockUI8, outDataBlock, outLayout);
      inBlockUI8 += blockDistance;
      if (--rowBlocks == 0)
      {
        rowBlocks = outDataLayout.blocksPerRow;
        outRow += 4*outDataLayout.bytesToNextRow;
        outDataBlock = outRow;
      }
      else
        outDataBlock += outLayout.bytesToNextBlock;
    }
    return inBlockUI8;
  }

  const void* DXTDecompressor::DecompressDXT1RGBA (const void* inBlockPtr,
                                                   size_t blockDistance,
                                                   size_t numBlocks,
                                                   csRGBpixel* outDataPtr,
                                                   const CS::Graphics::UncompressedDXTDataLayout& outDataLayout)
  {
    CS::Graphics::UncompressedDXTDataLayout outLayout (outDataLayout);
    outLayout.Fix (sizeof (csRGBpixel));

    const uint8* inBlockUI8 (reinterpret_cast<const uint8*> (inBlockPtr));
    uint8* outRow (reinterpret_cast<uint8*> (outDataPtr));
    uint8* outDataBlock (outRow);
    size_t rowBlocks (outDataLayout.blocksPerRow);
    while (numBlocks-- > 0)
    {
      DecodeDXT1Color<true> (inBlockUI8, outDataBlock, outLayout);
      inBlockUI8 += blockDistance;
      if (--rowBlocks == 0)
      {
        rowBlocks = outDataLayout.blocksPerRow;
        outRow += 4*outDataLayout.bytesToNextRow;
        outDataBlock = outRow;
      }
      else
        outDataBlock += outLayout.bytesToNextBlock;
    }
    return inBlockUI8;
  }

  const void* DXTDecompressor::DecompressDXT3Alpha (const void* inBlockPtr,
                                                    size_t blockDistance,
                                                    size_t numBlocks,
                                                    uint8* outDataPtr,
                                                    const CS::Graphics::UncompressedDXTDataLayout& outDataLayout)
  {
    CS::Graphics::UncompressedDXTDataLayout outLayout (outDataLayout);
    outLayout.Fix (sizeof (uint8));

    const uint8* inBlockUI8 (reinterpret_cast<const uint8*> (inBlockPtr));
    uint8* outRow (reinterpret_cast<uint8*> (outDataPtr));
    uint8* outDataBlock (outRow);
    size_t rowBlocks (outDataLayout.blocksPerRow);
    while (numBlocks-- > 0)
    {
      uint16 alpha_row[4];

      alpha_row[0] = csLittleEndian::Convert (csGetFromAddress::UInt16 (inBlockUI8));
      alpha_row[1] = csLittleEndian::Convert (csGetFromAddress::UInt16 (inBlockUI8 + sizeof(uint16)));
      alpha_row[2] = csLittleEndian::Convert (csGetFromAddress::UInt16 (inBlockUI8 + sizeof(uint16) * 2));
      alpha_row[3] = csLittleEndian::Convert (csGetFromAddress::UInt16 (inBlockUI8 + sizeof(uint16) * 3));

      uint8* outDataRow (outDataBlock);
      for (int j = 0; j < 4; j++)
      {
        uint8* outDataPixel (outDataRow);
        uint16 word = alpha_row[j];
        for (int i = 0; i < 4; i++)
        {
          const int a = (word & 0x0F) * 0x11;
          *outDataPixel = a;
          word >>= 4;

          outDataPixel += outDataLayout.bytesToNextPixel;
        }
        outDataRow += outDataLayout.bytesToNextRow;
      }
      inBlockUI8 += blockDistance;
      if (--rowBlocks == 0)
      {
        rowBlocks = outDataLayout.blocksPerRow;
        outRow += 4*outDataLayout.bytesToNextRow;
        outDataBlock = outRow;
      }
      else
        outDataBlock += outLayout.bytesToNextBlock;
    }
    return inBlockUI8;
  }

  namespace
  {
    template<typename T>
    struct OutputTypeTraits {};
    template<> struct OutputTypeTraits<uint8>
    {
      typedef uint IntType;
      static inline IntType Min() { return 0; }
      static inline IntType Max() { return 255; }
      static inline IntType FromStorage (uint8 v) { return v; }

      static inline void Store (void* p, uint8 v)
      { *(reinterpret_cast<uint8*> (p)) = v; }
    };
    template<> struct OutputTypeTraits<int8>
    {
      typedef int IntType;
      static inline IntType Min() { return -128; }
      static inline IntType Max() { return 127; }
      static inline IntType FromStorage (int8 v) { return v; }

      static inline void Store (void* p, int8 v)
      { *(reinterpret_cast<int8*> (p)) = v; }
    };
    template<> struct OutputTypeTraits<uint16>
    {
      typedef uint IntType;
      static inline IntType Min() { return 0; }
      static inline IntType Max() { return 65535; }
      static inline IntType FromStorage (uint8 v) { return v * 0x101; }

      static inline void Store (void* p, uint16 v)
      { csSetToAddress::UInt16 (p, v); }
    };
    template<> struct OutputTypeTraits<int16>
    {
      typedef int IntType;
      static inline IntType Min() { return -32768; }
      static inline IntType Max() { return 32767; }
      static inline IntType FromStorage (int8 v)
      { return v > 0 ? v * 0x101
                     : v * 0x100; /* @@@ FIXME: correct? RGTC compression maps -127 to -1.0 ... */ }

      static inline void Store (void* p, int16 v)
      { csSetToAddress::Int16 (p, v); }
    };
  }

  template<typename StoredType, typename OutputType>
  static inline void DecodeDXTSingleComponent (const uint8* block, uint8* outAlpha,
                                               const CS::Graphics::UncompressedDXTDataLayout& outDataLayout)
  {
    typedef typename OutputTypeTraits<OutputType>::IntType Intermediate;
    Intermediate    a0, a1;
    OutputType      alphas[8];
    const uint8*    alphamask;

    a0 = alphas[0] = OutputTypeTraits<OutputType>::FromStorage (reinterpret_cast<const StoredType*> (block)[0]);
    a1 = alphas[1] = OutputTypeTraits<OutputType>::FromStorage (reinterpret_cast<const StoredType*> (block)[1]);
    alphamask = block + 2;

    // 8-alpha or 6-alpha block?
    if (a0 > a1)
    {
      // 8-alpha block:  derive the other six alphas.
      // Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
      alphas[2] = (6 * a0 + 1 * a1 + 3) / 7; // bit code 010
      alphas[3] = (5 * a0 + 2 * a1 + 3) / 7; // bit code 011
      alphas[4] = (4 * a0 + 3 * a1 + 3) / 7; // bit code 100
      alphas[5] = (3 * a0 + 4 * a1 + 3) / 7; // bit code 101
      alphas[6] = (2 * a0 + 5 * a1 + 3) / 7; // bit code 110
      alphas[7] = (1 * a0 + 6 * a1 + 3) / 7; // bit code 111
    }
    else
    {
      // 6-alpha block.
      // Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
      alphas[2] = (4 * a0 + 1 * a1 + 2) / 5; // Bit code 010
      alphas[3] = (3 * a0 + 2 * a1 + 2) / 5; // Bit code 011
      alphas[4] = (2 * a0 + 3 * a1 + 2) / 5; // Bit code 100
      alphas[5] = (1 * a0 + 4 * a1 + 2) / 5; // Bit code 101
      alphas[6] = OutputTypeTraits<OutputType>::Min (); // Bit code 110
      alphas[7] = OutputTypeTraits<OutputType>::Max (); // Bit code 111
    }

    // Upper and lower 4x2 pixel block are both decoded in the inner loop
    uint32 bits = csLittleEndian::Convert (*((uint32*)alphamask));
    uint32 bits2 = csLittleEndian::Convert (*((uint32*)&alphamask[3]));
    uint8* outAlphaRow (outAlpha);
    uint8* outAlphaRow2 (outAlpha + 2 * outDataLayout.bytesToNextRow);
    for (uint j = 0; j < 2; j++)
    {
      uint8* outAlphaPixel (outAlphaRow);
      uint8* outAlphaPixel2 (outAlphaRow2);
      for (uint i = 0; i < 4; i++)
      {
        OutputTypeTraits<OutputType>::Store (outAlphaPixel, alphas[bits & 0x07]);
        bits >>= 3;
        OutputTypeTraits<OutputType>::Store (outAlphaPixel2, alphas[bits2 & 0x07]);
        bits2 >>= 3;
        outAlphaPixel += outDataLayout.bytesToNextPixel;
        outAlphaPixel2 += outDataLayout.bytesToNextPixel;
      }
      outAlphaRow += outDataLayout.bytesToNextRow;
      outAlphaRow2 += outDataLayout.bytesToNextRow;
    }
  }

  const void* DXTDecompressor::DecompressDXTUNormToUI8 (const void* inBlockPtr,
                                                        size_t blockDistance,
                                                        size_t numBlocks,
                                                        uint8* outDataPtr,
                                                        const CS::Graphics::UncompressedDXTDataLayout& outDataLayout)
  {
    CS::Graphics::UncompressedDXTDataLayout outLayout (outDataLayout);
    outLayout.Fix (sizeof (uint8));

    const uint8* inBlockUI8 (reinterpret_cast<const uint8*> (inBlockPtr));
    uint8* outRow (reinterpret_cast<uint8*> (outDataPtr));
    uint8* outDataBlock (outRow);
    size_t rowBlocks (outDataLayout.blocksPerRow);
    while (numBlocks-- > 0)
    {
      DecodeDXTSingleComponent<uint8, uint8> (inBlockUI8, outDataBlock, outLayout);
      inBlockUI8 += blockDistance;
      if (--rowBlocks == 0)
      {
        rowBlocks = outDataLayout.blocksPerRow;
        outRow += 4*outDataLayout.bytesToNextRow;
        outDataBlock = outRow;
      }
      else
        outDataBlock += outLayout.bytesToNextBlock;
    }
    return inBlockUI8;
  }

  const void* DXTDecompressor::DecompressDXTSNormToI8 (const void* inBlockPtr,
                                                       size_t blockDistance,
                                                       size_t numBlocks,
                                                       int8* outDataPtr,
                                                       const CS::Graphics::UncompressedDXTDataLayout& outDataLayout)
  {
    CS::Graphics::UncompressedDXTDataLayout outLayout (outDataLayout);
    outLayout.Fix (sizeof (uint8));

    const uint8* inBlockUI8 (reinterpret_cast<const uint8*> (inBlockPtr));
    uint8* outRow (reinterpret_cast<uint8*> (outDataPtr));
    uint8* outDataBlock (outRow);
    size_t rowBlocks (outDataLayout.blocksPerRow);
    while (numBlocks-- > 0)
    {
      DecodeDXTSingleComponent<int8, int8> (inBlockUI8, outDataBlock, outLayout);
      inBlockUI8 += blockDistance;
      if (--rowBlocks == 0)
      {
        rowBlocks = outDataLayout.blocksPerRow;
        outRow += 4*outDataLayout.bytesToNextRow;
        outDataBlock = outRow;
      }
      else
        outDataBlock += outLayout.bytesToNextBlock;
    }
    return inBlockUI8;
  }

  const void* DXTDecompressor::DecompressDXTUNormToUI16 (const void* inBlockPtr,
                                                         size_t blockDistance,
                                                         size_t numBlocks,
                                                         uint16* outDataPtr,
                                                         const CS::Graphics::UncompressedDXTDataLayout& outDataLayout)
  {
    CS::Graphics::UncompressedDXTDataLayout outLayout (outDataLayout);
    outLayout.Fix (sizeof (uint8));

    const uint8* inBlockUI8 (reinterpret_cast<const uint8*> (inBlockPtr));
    uint8* outRow (reinterpret_cast<uint8*> (outDataPtr));
    uint8* outDataBlock (outRow);
    size_t rowBlocks (outDataLayout.blocksPerRow);
    while (numBlocks-- > 0)
    {
      DecodeDXTSingleComponent<uint8, uint16> (inBlockUI8, outDataBlock, outLayout);
      inBlockUI8 += blockDistance;
      if (--rowBlocks == 0)
      {
        rowBlocks = outDataLayout.blocksPerRow;
        outRow += 4*outDataLayout.bytesToNextRow;
        outDataBlock = outRow;
      }
      else
        outDataBlock += outLayout.bytesToNextBlock;
    }
    return inBlockUI8;
  }

  const void* DXTDecompressor::DecompressDXTSNormToI16 (const void* inBlockPtr,
                                                        size_t blockDistance,
                                                        size_t numBlocks,
                                                        int16* outDataPtr,
                                                        const CS::Graphics::UncompressedDXTDataLayout& outDataLayout)
  {
    CS::Graphics::UncompressedDXTDataLayout outLayout (outDataLayout);
    outLayout.Fix (sizeof (uint8));

    const uint8* inBlockUI8 (reinterpret_cast<const uint8*> (inBlockPtr));
    uint8* outRow (reinterpret_cast<uint8*> (outDataPtr));
    uint8* outDataBlock (outRow);
    size_t rowBlocks (outDataLayout.blocksPerRow);
    while (numBlocks-- > 0)
    {
      DecodeDXTSingleComponent<int8, int16> (inBlockUI8, outDataBlock, outLayout);
      inBlockUI8 += blockDistance;
      if (--rowBlocks == 0)
      {
        rowBlocks = outDataLayout.blocksPerRow;
        outRow += 4*outDataLayout.bytesToNextRow;
        outDataBlock = outRow;
      }
      else
        outDataBlock += outLayout.bytesToNextBlock;
    }
    return inBlockUI8;
  }
}
CS_PLUGIN_NAMESPACE_END(DDSImageIO)
