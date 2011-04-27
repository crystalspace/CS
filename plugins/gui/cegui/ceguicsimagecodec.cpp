/*
    Copyright (C) 2005 Dan Hardfeldt, Seth Yastrov and Jelle Hellemans

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

#include "crystalspace.h"
#include "ceguicsimagecodec.h"

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  //----------------------------------------------------------------------------//
  ImageCodec::ImageCodec(iObjectRegistry* obj_reg) :
      CEGUI::ImageCodec("CS::ImageCodec - Integrated ImageCodec using the CS engine."),
      obj_reg(obj_reg)
  {
  }

  //----------------------------------------------------------------------------//
  CEGUI::Texture* ImageCodec::load(const CEGUI::RawDataContainer& data, CEGUI::Texture* result)
  {
    csRef<iDataBuffer> buf;
    buf.AttachNew(new CS::DataBuffer<CS::Memory::AllocatorMalloc>((char*)data.getDataPtr(), data.getSize(), false));

    csRef<iLoader> loader = csQueryRegistry<iLoader> (obj_reg);
    csRef<iImage> image = loader->LoadImage(buf, CS_IMGFMT_ANY);

    // discover the pixel format and number of pixel components
    CEGUI::Texture::PixelFormat format;
    switch (image->GetFormat())
    {
    case CS_IMGFMT_INVALID:
      throw CEGUI::FileIOException("CS::ImageCodec::load: File data was of an unsupported format.");
      break;

    case CS_IMGFMT_TRUECOLOR:
      format = CEGUI::Texture::PF_RGB;
      break;

    case CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA:
      format = CEGUI::Texture::PF_RGBA;
      break;

    default:
      image = CS::ImageAutoConvert(image, CS_IMGFMT_TRUECOLOR);
      format = CEGUI::Texture::PF_RGB;
      break;
    }

    // load the resulting image into the texture
    result->loadFromMemory(image->GetImageData(),
                           CEGUI::Size(image->GetWidth(),
                                        image->GetHeight()),
                           format);

    return result;
  }

  //----------------------------------------------------------------------------//

} CS_PLUGIN_NAMESPACE_END(cegui)
