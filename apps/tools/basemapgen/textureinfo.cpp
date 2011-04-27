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

#include "crystalspace.h"
#include "textureinfo.h"

#include "basemapgen.h"

TextureInfo::TextureInfo (iDocumentNode* node, const char* file,
			  const char* texClass)
 : node (node), file (file), texClass (texClass) {}

iImage* TextureInfo::GetMip (uint mip)
{
  if (mips.GetSize() == 0)
  {
    mips.Push (basemapgen->LoadImage (file, CS_IMGFMT_TRUECOLOR));
  }
  
  iImage* img (mips[0]);
  if (mip == 0) return img;
  
  if ((mips.GetSize() > mip) && mips[mip])
    return mips[mip];
  
  /* Get mipmap for an image, using precomputed mipmaps as far as
     possible. */
  csRef<iImage> mipImg;
  uint hasMips = img->HasMipmaps();
  bool isPrecomputedMip = false;
  if (mip <= hasMips)
  {
    mipImg = img->GetMipmap (mip);
    isPrecomputedMip = true;
  }
  else
  {
    csRef<iImage> imgToMip;
    imgToMip = img->GetMipmap (hasMips);
    uint computeMip = mip;
    computeMip -= hasMips;
    if (mip >= 0)
      mipImg = csImageManipulate::Mipmap (imgToMip, computeMip);
    else
      mipImg = imgToMip;
  }
  
  // Apply sharpening
  const TextureClass& texClass (basemapgen->GetTextureClass (this->texClass));
  int mipSharpen = basemapgen->GetMipSharpen();
  if ((mipSharpen > 0) && texClass.GetSharpenMips ())
  {
    if (texClass.GetSharpenPrecomputed() || !isPrecomputedMip)
    {
      mipImg = csImageManipulate::Sharpen (mipImg, mipSharpen);
    }
  }
  
  mips.GetExtend (mip);
  mips.Put (mip, mipImg);
  return mipImg;
}
