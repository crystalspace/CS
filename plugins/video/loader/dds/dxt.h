/*
    DDS image file format support for CrystalSpace 3D library
    Copyright (C) 2003 by Matze Braun <matze@braunis.de>
              (C) 2006 by Frank Richter

    Code based on DevIL source by Denton Woods which was based on an nvidia
    example at:
    http://www.nvidia.com/view.asp?IO=dxtc_decompression_code

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

CS_PLUGIN_NAMESPACE_BEGIN(DDSImageIO)
{
namespace dds
{

  struct DXTDecompress
  {

    struct Color8888
    {
      unsigned char r,g,b,a;
    };

    static inline int ColorComponent (int x, int bitcount, int shift)
    {
      int val = ((x >> shift) & ((1 << bitcount) - 1));
      return (val << (8 - bitcount)) + (val >> (2 * bitcount - 8));
    }

  #define COLOR565_RED(x)     ColorComponent(x, 5, 11)
  #define COLOR565_GREEN(x)   ColorComponent(x, 6, 5)
  #define COLOR565_BLUE(x)    ColorComponent(x, 5, 0)

    template <bool withAlpha>
    static inline void DecodeDXT1Color (const uint8* block, Color8888* outColor)
    {
      uint16       color_0, color_1;
      Color8888    colours[4];
      uint32       bitmask;

      color_0 = csLittleEndian::Convert (*((uint16*)block)); 
      color_1 = csLittleEndian::Convert (*((uint16*)block+1));
      bitmask = csLittleEndian::Convert (((uint32*)block)[1]);

      colours[0].r = COLOR565_RED(color_0); 
      colours[0].g = COLOR565_GREEN(color_0);
      colours[0].b = COLOR565_BLUE(color_0); 
      colours[0].a = 0xFF;

      colours[1].r = COLOR565_RED(color_1); 
      colours[1].g = COLOR565_GREEN(color_1);
      colours[1].b = COLOR565_BLUE(color_1);
      colours[1].a = 0xFF;

      if (withAlpha && (color_0 > color_1))
      {
        // Four-color block: derive the other two colors.
        // 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
        // These 2-bit codes correspond to the 2-bit fields
        // stored in the 64-bit block.
        colours[2].b = (2 * colours[0].b + colours[1].b + 1) / 3;
        colours[2].g = (2 * colours[0].g + colours[1].g + 1) / 3;
        colours[2].r = (2 * colours[0].r + colours[1].r + 1) / 3;
        colours[2].a = 0xFF;

        colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
        colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
        colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
        colours[3].a = 0xFF;
      }
      else 
      {
        // Three-color block: derive the other color.
        // 00 = color_0,  01 = color_1,  10 = color_2,
        // 11 = transparent.
        // These 2-bit codes correspond to the 2-bit fields
        // stored in the 64-bit block.
        colours[2].b = (colours[0].b + colours[1].b) / 2;
        colours[2].g = (colours[0].g + colours[1].g) / 2;
        colours[2].r = (colours[0].r + colours[1].r) / 2;
        colours[2].a = 0xFF;

        colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
        colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
        colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
        colours[3].a = 0x00;
      }
      
      for (int j = 0, k = 0; j < 4; j++) 
      {
        for (int i = 0; i < 4; i++, k++) 
        {
          int Select = (bitmask & (0x03 << k*2)) >> k*2;
          const Color8888& col = colours[Select];
          *outColor++ = col;
        }
      }
    }

    static inline void DecodeDXT5Alpha (const uint8* block, uint8* outAlpha)
    {
      uint8		  alphas[8];
      const uint8*	  alphamask;

      alphas[0] = block[0];
      alphas[1] = block[1];
      alphamask = block + 2;

      // 8-alpha or 6-alpha block?
      if (alphas[0] > alphas[1]) 
      {
        // 8-alpha block:  derive the other six alphas.
        // Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
        alphas[2] = (6 * alphas[0] + 1 * alphas[1] + 3) / 7; // bit code 010
        alphas[3] = (5 * alphas[0] + 2 * alphas[1] + 3) / 7; // bit code 011
        alphas[4] = (4 * alphas[0] + 3 * alphas[1] + 3) / 7; // bit code 100
        alphas[5] = (3 * alphas[0] + 4 * alphas[1] + 3) / 7; // bit code 101
        alphas[6] = (2 * alphas[0] + 5 * alphas[1] + 3) / 7; // bit code 110
        alphas[7] = (1 * alphas[0] + 6 * alphas[1] + 3) / 7; // bit code 111
      }
      else 
      {
        // 6-alpha block.
        // Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
        alphas[2] = (4 * alphas[0] + 1 * alphas[1] + 2) / 5; // Bit code 010
        alphas[3] = (3 * alphas[0] + 2 * alphas[1] + 2) / 5; // Bit code 011
	alphas[4] = (2 * alphas[0] + 3 * alphas[1] + 2) / 5; // Bit code 100
	alphas[5] = (1 * alphas[0] + 4 * alphas[1] + 2) / 5; // Bit code 101
	alphas[6] = 0x00;
	// Bit code 110
	alphas[7] = 0xFF;
	// Bit code 111
      }

      // Upper and lower 4x2 pixel block are both decoded in the inner loop
      uint32 bits = csLittleEndian::Convert (*((uint32*)alphamask));
      uint32 bits2 = csLittleEndian::Convert (*((uint32*)&alphamask[3]));
      uint8* outAlpha2 = outAlpha + 8;
      for (uint j = 0; j < 2; j++) 
      {
        for (uint i = 0; i < 4; i++) 
        {
          *outAlpha++ = alphas[bits & 0x07];
          bits >>= 3;
          *outAlpha2++ = alphas[bits2 & 0x07];
          bits2 >>= 3;
        }
      }
    }

  #undef COLOR565_RED
  #undef COLOR565_GREEN
  #undef COLOR565_BLUE

  };

} // end of namespace dds
}
CS_PLUGIN_NAMESPACE_END(DDSImageIO)
