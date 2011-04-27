/*
  Copyright (C) 2007 by Marten Svanfeldt
                2004 by Anders Stenberg, Daniel Duhprey

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

#include "csgfx/imagemanipulate.h"
#include "csgfx/rgbpixel.h"
#include "csutil/csendian.h"
#include "csutil/objreg.h"
#include "imesh/terrain2.h"
#include "iutil/vfs.h"
#include "imap/loader.h"

#include "feederhelper.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

  template<typename Endianness>
  struct GetterFloat
  {
    static inline void Get (char*& buf, float&f)
    {
      f = csIEEEfloat::ToNative (Endianness::Convert (
	    csGetFromAddress::UInt32 (buf))); 
      buf += sizeof(uint32);
    }
    static inline size_t ItemSize()
    { return sizeof(uint32); }
  };

  struct GetterUint8
  {
    static inline void Get (char*& buf, float&f)
    {
      uint8 v = (uint8) (*buf);
      buf += sizeof (uint8);
      f = float(v) / 255.0f;
    }
    static inline size_t ItemSize()
    { return sizeof(uint8); }
  };

  template<typename Endianness>
  struct GetterUint16
  {
    static inline void Get (char*& buf, float&f)
    {
      uint16 v = Endianness::Convert (csGetFromAddress::UInt16 (buf));
      buf += sizeof (uint16);
      f = float(v) / 65535.0f;
    }
    static inline size_t ItemSize()
    { return sizeof(uint16); }
  };

  template<typename Endianness>
  struct GetterUint32
  {
    static inline void Get (char*& buf, float&f)
    {
      uint32 v = Endianness::Convert (csGetFromAddress::UInt32 (buf));
      buf += sizeof (uint32);
      f = float(v) / 4294967295.0f;
    }
    static inline size_t ItemSize()
    { return sizeof(uint32); }
  };

  template<typename Tgetter>
  class RawHeightmapReader
  {    
  public:
    bool ReadData (float* outputBuffer, size_t outputWidth, 
      size_t outputHeight, size_t outputPitch, float heightScale, float offset,
      char* inputBuffer)
    {
      for (size_t y = 0; y < outputHeight; ++y)
      {
        float* row = outputBuffer;
        for (size_t x = 0; x < outputWidth; ++x)
        {
          Tgetter::Get (inputBuffer, *row);
          *row = *row * heightScale + offset;
          row++;
        }
        outputBuffer += outputPitch;
      }
      
      return true;
    }
  };
  

  FeederHeightSourceType ParseFormatString (const csString& format)
  {
    if (format.IsEmpty ())
    {
      return HEIGHT_SOURCE_IMAGE;
    }

    static const char* formatStrings[] = 
    {
      "image", 
      "raw8",
      "raw16le",
      "raw16be",
      "raw32le",
      "raw32be",
      "rawfloatle",
      "rawfloatbe"
    };

    for (size_t i = 0; i < sizeof(formatStrings) / sizeof(char*); ++i)
    {
      if (format == formatStrings[i])
      {
        return (FeederHeightSourceType)i;
      }
    }

    return HEIGHT_SOURCE_IMAGE;
  }

  HeightFeederParser::HeightFeederParser (const csString& mapSource, 
    const csString& format, iLoader* imageLoader, iObjectRegistry* objReg)
    : sourceLocation (mapSource), sourceFormat (ParseFormatString (format)),
    imageLoader (imageLoader), objReg (objReg)
  {
    // Depending on format we might need some extra pointers, store those
    if (sourceFormat != HEIGHT_SOURCE_IMAGE)
    {
      // Need vfs, get it
      vfs = csQueryRegistry<iVFS> (objReg);
    }
  }

  bool HeightFeederParser::Load (float* outputBuffer, size_t outputWidth, 
    size_t outputHeight, size_t outputPitch, float heightScale, float offset)
  {
    if (sourceFormat == HEIGHT_SOURCE_IMAGE)
    {
      return LoadFromImage (outputBuffer, outputWidth, outputHeight,
        outputPitch, heightScale, offset);
    }
    
    // Handle loading from all other (raw) formats
    if (!vfs)
      return false;
    
    size_t valueSize = 0;
    switch (sourceFormat)
    {
    case HEIGHT_SOURCE_RAW8:
      valueSize = sizeof (uint8);
      break;
    case HEIGHT_SOURCE_RAW16LE:
    case HEIGHT_SOURCE_RAW16BE:
      valueSize = sizeof (uint16);
      break;
    case HEIGHT_SOURCE_RAW32LE:
    case HEIGHT_SOURCE_RAW32BE:
    case HEIGHT_SOURCE_RAWFLOATLE:
    case HEIGHT_SOURCE_RAWFLOATBE:
      valueSize = sizeof (uint32);
      break;
    case HEIGHT_SOURCE_IMAGE:
      // Can not happen.
      CS_ASSERT (false);
    }

    csRef<iDataBuffer> buf = vfs->ReadFile (sourceLocation.GetDataSafe (),
	false);
    if (!buf ||
	((outputHeight * outputWidth * valueSize) != buf->GetSize ()))
      return false;

    switch (sourceFormat)
    {
      case HEIGHT_SOURCE_RAW8:
        {
          RawHeightmapReader<GetterUint8> reader;
          return reader.ReadData (outputBuffer, outputWidth, outputHeight, 
            outputPitch, heightScale, offset, buf->GetData ());
        }
        break;
      case HEIGHT_SOURCE_RAW16LE:
        {
          RawHeightmapReader<GetterUint16<csLittleEndian> > reader;
          return reader.ReadData (outputBuffer, outputWidth, outputHeight, 
            outputPitch, heightScale, offset, buf->GetData ());
        }
        break;
      case HEIGHT_SOURCE_RAW16BE:
        {
          RawHeightmapReader<GetterUint16<csBigEndian> > reader;
          return reader.ReadData (outputBuffer, outputWidth, outputHeight, 
            outputPitch, heightScale, offset, buf->GetData ());
        }
        break;
      case HEIGHT_SOURCE_RAW32LE:
        {
          RawHeightmapReader<GetterUint32<csLittleEndian> > reader;
          return reader.ReadData (outputBuffer, outputWidth, outputHeight, 
            outputPitch, heightScale, offset, buf->GetData ());
        }
        break;
      case HEIGHT_SOURCE_RAW32BE:
        {
          RawHeightmapReader<GetterUint32<csBigEndian> > reader;
          return reader.ReadData (outputBuffer, outputWidth, outputHeight, 
            outputPitch, heightScale, offset, buf->GetData ());
        }
        break;
      case HEIGHT_SOURCE_RAWFLOATLE:
        {
          RawHeightmapReader<GetterFloat<csLittleEndian> > reader;
          return reader.ReadData (outputBuffer, outputWidth, outputHeight, 
            outputPitch, heightScale, offset, buf->GetData ());
        }
        break;
      case HEIGHT_SOURCE_RAWFLOATBE:
        {
          RawHeightmapReader<GetterFloat<csBigEndian> > reader;
          return reader.ReadData (outputBuffer, outputWidth, outputHeight, 
            outputPitch, heightScale, offset, buf->GetData ());
        }
        break;
      case HEIGHT_SOURCE_IMAGE:
	// @@@FIXME: Handle this case?
	break;
    }

    return false;
  }
  
  bool HeightFeederParser::LoadFromImage (float* outputBuffer,
      size_t outputWidth, size_t outputHeight, size_t outputPitch,
      float heightScale, float offset)
  {
    csRef<iImage> image = imageLoader->LoadImage (sourceLocation.GetDataSafe (),
      CS_IMGFMT_ANY);

    if (!image)
      return false;

    if ((size_t)image->GetWidth () != outputWidth || 
      (size_t)image->GetHeight () != outputHeight)
    {
      image = csImageManipulate::Rescale (image, (int)outputWidth,
	  (int)outputHeight);
    }

    if ((image->GetFormat () & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR)
    {
      const csRGBpixel *data = (csRGBpixel*)image->GetImageData ();

      for (size_t y = 0; y < outputHeight; ++y)
      {
        float* row = outputBuffer;

        for (size_t x = 0; x < outputWidth; ++x)
        {          
          const csRGBpixel& p = *data++;

          unsigned int h;
          h = p.red; h <<= 8;
          h |= p.green; h <<= 8;
          h |= p.blue;

          *row++ = (h / ((float) ((1<<24)-1))) * heightScale + offset;
        }

        outputBuffer += outputPitch;
      }
      return true;
    }
    else if ((image->GetFormat () & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
    {
      unsigned char *data = (unsigned char*)image->GetImageData ();
      const csRGBpixel *palette = image->GetPalette ();
      
      const float heightConstant = heightScale / 255.0f;

      for (size_t y = 0; y < outputHeight; ++y)
      {
        float* row = outputBuffer;

        for (size_t x = 0; x < outputWidth; ++x)
        {   
          const unsigned char p = *data++;
          const int h = palette[p].Intensity (); 

          *row++ = h * heightConstant + offset;
        }

        outputBuffer += outputPitch;
      }
      return true;
    }

    return false;
  }

  void SmoothHeightmap (float* heightBuffer, size_t width, size_t height, 
    size_t pitch)
  {
    // We need a temporary buffer for smoothing into
    float* tempBuffer = (float*)cs_malloc(width * height * sizeof(float));
    
    for (size_t passes = 0; passes < 2; ++passes)
    {
      // Smooth into tempBuffer

      // Copy first and last row
      for (size_t col = 0; col < width; ++col)
      {
        tempBuffer[0*width + col] = heightBuffer[0*pitch + col];
        tempBuffer[(height - 1)*width + col] = heightBuffer[(height - 1)*pitch + col];
      }

      // Handle interior
      for (size_t row = 1; row < height - 1; ++row)
      {
        // First column
        tempBuffer[row*width + 0] = heightBuffer[row*pitch + 0];

        // Rest
        for (size_t col = 1; col < width - 1; ++col)
        {
          float result = 0;
          
          result = 0.25f * heightBuffer[row*pitch + col - 1] +
                   0.50f * heightBuffer[row*pitch + col + 0] +
                   0.25f * heightBuffer[row*pitch + col + 1];

          tempBuffer[row*width + col] = result;
        }

        // Last column
        tempBuffer[row*width + width - 1] = heightBuffer[row*pitch + width - 1];
      }
      
      // Smooth back into the height buffer
      for (size_t row = 1; row < height - 1; ++row)
      {
        // Interior
        for (size_t col = 1; col < width - 1; ++col)
        {
          float result = 0;

          result = 0.25f * tempBuffer[(row - 1)*width + col] +
                   0.50f * tempBuffer[(row + 0)*width + col] +
                   0.25f * tempBuffer[(row + 1)*width + col];

          heightBuffer[row*pitch + col] = result;
        }      
      }
    }

    cs_free (tempBuffer);
  }

  NormalFeederParser::NormalFeederParser (const csString& mapSource, iLoader* imageLoader, iObjectRegistry* objReg)
    : sourceLocation (mapSource), imageLoader (imageLoader), objReg (objReg)
  {

  }

  bool NormalFeederParser::Load (csVector3* outputBuffer, size_t outputWidth, size_t outputHeight, size_t outputPitch)
  {
    csRef<iImage> image = imageLoader->LoadImage (sourceLocation.GetDataSafe (),
      CS_IMGFMT_ANY);

    if (!image)
      return false;

    if ((size_t)image->GetWidth () != outputWidth || 
      (size_t)image->GetHeight () != outputHeight)
    {
      image = csImageManipulate::Rescale (image, (int)outputWidth,
        (int)outputHeight);
    }

    if ((image->GetFormat () & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR)
    {
      const uint16 *data = (uint16*)image->GetImageData ();

      for (size_t y = 0; y < outputHeight; ++y)
      {
        csVector3* row = outputBuffer;

        for (size_t x = 0; x < outputWidth; ++x, ++row)
        {
          // Convert to 'single' precision float.
          row->x = csIEEEfloat::ToNative (csBigEndian::Convert(*data++));
          row->y = csIEEEfloat::ToNative (csBigEndian::Convert(*data++));

          // Calculate z.
          row->z = 1.0f - (row->x + row->y);
        }

        outputBuffer += outputPitch;
      }
      return true;
    }

    return false;
  }

}
CS_PLUGIN_NAMESPACE_END(Terrain2)
