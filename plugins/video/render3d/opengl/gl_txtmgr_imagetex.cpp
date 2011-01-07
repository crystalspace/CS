/*
    Copyright (C) 1998-2004 by Jorrit Tyberghein
	      (C) 2003 by Philip Aumayr
	      (C) 2004-2007 by Frank Richter

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

#include "csgfx/bakekeycolor.h"
#include "csgfx/imagecubemapmaker.h"
#include "csgfx/imagemanipulate.h"
#include "csgfx/imagememory.h"
#include "csgfx/packrgb.h"

#include "gl_render3d.h"
#include "gl_txtmgr.h"
#include "gl_txtmgr_imagetex.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

csGLTextureHandle::csGLTextureHandle (iImage* image, int flags, 
				      csGLGraphics3D *iG3D) : 
  csGLBasicTextureHandle (image->GetWidth(), image->GetHeight(),
    image->GetDepth(), image->GetImageType (), flags, iG3D),
  origName(0), transp_color (0, 0, 0)
{
//printf ("image='%s' format='%08x' rawformat='%s' type=%d\n",
    //image->GetName (), image->GetFormat (), image->GetRawFormat (),
    //image->GetImageType ()); fflush (stdout);
  this->image = image;
  if (image->GetFormat () & CS_IMGFMT_ALPHA)
    alphaType = csAlphaMode::alphaSmooth;
  else if (image->HasKeyColor ())
    alphaType = csAlphaMode::alphaBinary;
  else
    alphaType = csAlphaMode::alphaNone;

  if (image->HasKeyColor())
    SetTransp (true);
}

csGLTextureHandle::~csGLTextureHandle()
{
  cs_free (origName);
}


void csGLTextureHandle::FreeImage ()
{
  if (image.IsValid()) 
  {
    origName = CS::StrDup (image->GetName());
    if (IsTransp() && !IsTranspSet())
    {
      int r,g,b;
      image->GetKeyColor (r,g,b);
      SetKeyColor (r, g, b);
    }						 
  }
  image = 0;
}

void csGLTextureHandle::GetOriginalDimensions (int& mw, int& mh)
{
  mw = orig_width;
  mh = orig_height;
}

void csGLTextureHandle::GetOriginalDimensions (int& mw, int& mh, int &md)
{
  mw = orig_width;
  mh = orig_height;
  md = orig_d;
}

const char* csGLTextureHandle::GetImageName () const
{
  if (image.IsValid()) 
    return image->GetName();
  else
    return origName;
}

void csGLTextureHandle::SetKeyColor (bool Enable)
{
  SetTransp (Enable);
  SetTexupdateNeeded (true);
  if (Enable && alphaType == csAlphaMode::alphaNone)
    alphaType = csAlphaMode::alphaBinary;
  else if (!Enable && alphaType == csAlphaMode::alphaBinary)
    alphaType = csAlphaMode::alphaNone;
}

void csGLTextureHandle::SetKeyColor (uint8 red, uint8 green, uint8 blue)
{
  transp_color.red = red;
  transp_color.green = green;
  transp_color.blue = blue;
  if (alphaType == csAlphaMode::alphaNone)
    alphaType = csAlphaMode::alphaBinary;
  texFlags.Set (flagTransp | flagTranspSet | flagTexupdateNeeded);
}

bool csGLTextureHandle::GetKeyColor () const
{
  return IsTransp();
}

//#define MIPMAP_DEBUG

void csGLTextureHandle::CreateMipMaps()
{
  csRGBpixel *tc = IsTransp() ? &transp_color : (csRGBpixel *)0;

  const csGLTextureClassSettings* textureSettings = 
    txtmgr->GetTextureClassSettings (textureClass);
  /* Determine internal format of the texture. You can't mix glTexImage and 
   * glCompressedTexImage for different mip levels unless the internal format
   * is exactly the same. The target formats of the lower mip levels are later
   * checked against the target format of the first mip.
   */
  bool compressedTarget;
  GLenum targetFormat; 
  if ((texType == iTextureHandle::texTypeRect)
    && (txtmgr->tweaks.disableRECTTextureCompression))
    /* @@@ Hack: Some ATI drivers can't grok generic compressed formats for 
     * RECT textures, so force an uncompressed format in this case. */
    targetFormat = (alphaType != csAlphaMode::alphaNone) ? 
      GL_RGBA : GL_RGB;
  else
    targetFormat = (alphaType != csAlphaMode::alphaNone) ? 
      textureSettings->formatRGBA : textureSettings->formatRGB;
  targetFormat = DetermineTargetFormat (targetFormat, 
    !textureSettings->forceDecompress, image->GetRawFormat(), 
    compressedTarget);

  // Determine if and how many mipmaps we skip.
  const bool doReduce = !texFlags.Check (CS_TEXTURE_2D | CS_TEXTURE_NOMIPMAPS)
    && textureSettings->allowDownsample;
  int mipskip = doReduce ? txtmgr->texture_downsample : 0;
  while (((actual_width >> mipskip) > txtmgr->max_tex_size)
      || ((actual_height >> mipskip) > txtmgr->max_tex_size)
      || ((actual_d >> mipskip) > txtmgr->max_tex_size))
    mipskip++;

  // Delete existing mipmaps, if any
  FreshUploadData ();

  size_t i;
  size_t subImageCount = image->HasSubImages() + 1;
#ifdef MIPMAP_DEBUG
  for (i=0; i < subImageCount; i++)
  {
    csDebugImageWriter::DebugImageWrite (image->GetSubImage (i),
      "/tmp/mipdebug/%p_%zu_0.png", this, i);
  }
#endif
  if (texFlags.Check (CS_TEXTURE_NOMIPMAPS))
  {
    for (i=0; i < subImageCount; i++)
    {
      MakeUploadData (!textureSettings->forceDecompress, targetFormat, 
	image->GetSubImage ((uint)i), 0, (int)i);
    }
  }
  else
  {
    for (i=0; i < subImageCount; i++)
    {
      // Create each new level by creating a level 2 mipmap from previous level
      // we do this down to 1x1 as opengl defines it
      int w, h, d;
      int nTex = 0;
      int nMip = 0;
      csRef<iImage> thisImage = image->GetSubImage ((uint)i); 
      int nMipmaps = thisImage->HasMipmaps();

      do
      {
	w = thisImage->GetWidth ();
	h = thisImage->GetHeight ();
        d = thisImage->GetDepth ();

	if ((mipskip == 0) || ((w == 1) && (h == 1) && (d == 1)))
	  MakeUploadData (!textureSettings->forceDecompress, targetFormat, 
	    thisImage, nTex++, (int)i);

	if ((w == 1) && (h == 1) && (d == 1)) break;

	nMip++;
	csRef<iImage> origMip, cimg;
	bool precompMip = false;
	if (nMipmaps != 0)
	{
	  cimg = image->GetSubImage ((uint)i)->GetMipmap (nMip);
	  nMipmaps--;
	  precompMip = true;
	}
	else
	{
	  cimg = csImageManipulate::Mipmap (thisImage, 1, tc);
	}
	origMip = cimg;
	if (mipskip == 0) // don't postprocess when doing skip...
	{
	  if (txtmgr->sharpen_mipmaps 
	    && textureSettings->allowMipSharpen
	    && (cimg->GetDepth() == 1) // @@@ sharpen not "depth-safe"
	    && (!precompMip || textureSettings->sharpenPrecomputedMipmaps))
	  {
	    cimg = csImageManipulate::Sharpen (cimg, txtmgr->sharpen_mipmaps, 
	      tc);
	  }
	  if (!precompMip && textureSettings->renormalizeGeneratedMips)
	  {
	    cimg = csImageManipulate::RenormalizeNormals (cimg);
	  }
	}
  #ifdef MIPMAP_DEBUG
	csDebugImageWriter::DebugImageWrite (cimg,
	  "/tmp/mipdebug/%p_%zu_%d.png", this, i, nMip);
  #endif
	thisImage = origMip;
	if (mipskip != 0) mipskip--;
      }
      while (true);
    }
  }
}

void csGLTextureHandle::PrepareInt ()
{
  //@@@ Images may be lost if preparing twice. Some better way of solving it?
  if (!image.IsValid()) return;
  if (IsPrepared ()) return;
  SetPrepared (true);

  if (IsTransp() && !IsTranspSet())
  {
    int r,g,b;
    image->GetKeyColor (r,g,b);
    SetKeyColor (r, g, b);
  }						 

  csAlphaMode::AlphaType newAlphaType = csAlphaMode::alphaNone;

  // Do any resizing, if needed
  if (image->GetImageType() == csimgCube)
  {
    // Handle cube map faces
    csRef<csImageCubeMapMaker> newCube;
    int faceCount = MIN (image->HasSubImages() + 1, 6);
    for (int i = 0; i < faceCount; i++)
    {
      int newFaceW, newFaceH, newFaceD;
      csRef<iImage> imgFace = image->GetSubImage (i);
      ComputeNewPo2ImageSize (texFlags.Get(), 
	imgFace->GetWidth(), imgFace->GetHeight(), 1,
	newFaceW, newFaceH, newFaceD, txtmgr->max_tex_size);
      if (newFaceW != newFaceH) newFaceH = newFaceW;
      csRef<iImage> newFace = PrepareIntImage (newFaceW, newFaceH,
	imgFace->GetDepth (), imgFace, newAlphaType);
      if (newFace.IsValid())
      {
	// Create a new cube if we needed to resize one face.
	if (!newCube.IsValid()) 
	{
	  newCube.AttachNew (new csImageCubeMapMaker ());
	  newCube->SetName (image->GetName());
	}
	newCube->SetSubImage (i, newFace);
      }
    }
    if (faceCount < 6) // Ensure at least the 6 faces.
    {
      newCube.AttachNew (new csImageCubeMapMaker ());
      newCube->SetName (image->GetName());
    }
    if (newCube.IsValid())
    {
      for (int i = 0; i < faceCount; i++)
      {
	if (!newCube->SubImageSet (i))
	  newCube->SetSubImage (i, image->GetSubImage (i));
      }
      image = newCube;
    }
  }
  else
  {
    csRef<iImage> newImage = PrepareIntImage (actual_width, actual_height,
	actual_d, image, newAlphaType);
    if (newImage.IsValid()) image = newImage;
  }
  if (newAlphaType > alphaType) alphaType = newAlphaType;

  CreateMipMaps ();
  FreeImage ();
}

csRef<iImage> csGLTextureHandle::PrepareIntImage (
    int actual_width, int actual_height, int actual_depth, iImage* srcimage,
    csAlphaMode::AlphaType newAlphaType)
{
  csRef<iImage> newImage;
  if (actual_width != srcimage->GetWidth () || actual_height != srcimage->GetHeight () 
      || actual_depth != srcimage->GetDepth ())
  {
    newImage = csImageManipulate::Rescale (srcimage, actual_width, 
	actual_height, actual_depth);
  }
  if (IsTransp())
  {
    if (!newImage.IsValid()) 
      newImage.AttachNew (new csImageMemory (srcimage));
    // Set the alpha of keycolored images to 0.
    PrepareKeycolor (newImage, transp_color, newAlphaType);
  }
#if 0
  // Avoid accessing the image data until really needed
  else
    /* Check all alpha values for the actual alpha type.  */
    CheckAlpha  (image->GetWidth(), image->GetHeight(), 
	(csRGBpixel*)image->GetImageData (), 0, newAlphaType);
#endif
  if (newImage.IsValid()) return newImage;
  return 0;
}

bool csGLTextureHandle::MakeUploadData (bool allowCompressed, 
                                        GLenum targetFormat, 
                                        iImage* Image, int mipNum, 
                                        int imageNum)
{
  csGLUploadData& uploadData = this->uploadData->GetExtend (
    this->uploadData->GetSize());
  const char* rawFormat = Image->GetRawFormat();
  if (rawFormat)
  {
    csRef<iDataBuffer> imageRaw = Image->GetRawData();

    if (imageRaw.IsValid())
    {
      uploadData.dataRef = imageRaw;

      CS::StructuredTextureFormat texFormat (
        CS::TextureFormatStrings::ConvertStructured (rawFormat));
      TextureStorageFormat glFormat;
      TextureSourceFormat srcFormat;
      if (txtmgr->DetermineGLFormat (texFormat, glFormat, srcFormat)
        && !(glFormat.isCompressed && !allowCompressed))
      {
        uploadData.storageFormat = glFormat;
        uploadData.sourceFormat = srcFormat;
        uploadData.image_data = imageRaw->GetUint8();
        if (glFormat.isCompressed)
	  uploadData.compressedSize = imageRaw->GetSize();
	  
	if (desiredReadbackFormat.format == 0)
	 SetDesiredReadbackFormat (texFormat);
      }
    }
  }

  if (!uploadData.image_data)
  {
    if (csPackRGBA::IsRGBpixelSane())
    {
      uploadData.image_data = (uint8*)Image->GetImageData ();
      uploadData.dataRef = Image;
    }
    else
    {
      const size_t numPix = 
	Image->GetWidth() * Image->GetHeight() * Image->GetDepth();
      csRef<iDataBuffer> newDataBuf;
      newDataBuf.AttachNew (new CS::DataBuffer<> (
        numPix * sizeof (csRGBpixel)));
      csPackRGBA::PackRGBpixelToRGBA (newDataBuf->GetUint8(),
	(csRGBpixel*)Image->GetImageData(), numPix);
      uploadData.image_data = newDataBuf->GetUint8();
      uploadData.dataRef = newDataBuf;
    }
    //uploadData->size = n * 4;
    uploadData.sourceFormat.format = GL_RGBA;
    uploadData.sourceFormat.type = GL_UNSIGNED_BYTE;
    
    if (desiredReadbackFormat.format == 0)
      SetDesiredReadbackFormat (CS::TextureFormatStrings::ConvertStructured ("abgr8"));
  }
  uploadData.storageFormat.targetFormat = targetFormat;
  uploadData.w = Image->GetWidth();
  uploadData.h = Image->GetHeight();
  uploadData.d = Image->GetDepth();
  uploadData.mip = mipNum;
  uploadData.imageNum = imageNum;

  //size += uploadData->size * d;
  return true;
}

void csGLTextureHandle::CheckAlpha (int w, int h, int d, csRGBpixel *src, 
				    const csRGBpixel* transp_color, 
				    csAlphaMode::AlphaType& alphaType)
{
  int count = w * h * d;
  CS_ASSERT (count > 0);
  while (count--)
  {
    const csRGBpixel &pix = *src++;
    if ((!transp_color || !transp_color->eq (pix)) && pix.alpha)
    {
      if ((pix.alpha < 255) && (alphaType != csAlphaMode::alphaSmooth))
	alphaType = csAlphaMode::alphaSmooth;
    }
    else
    {
      if (alphaType == csAlphaMode::alphaNone)
	alphaType = transp_color ? csAlphaMode::alphaBinary : 
          csAlphaMode::alphaSmooth;
    }
  }
}


void csGLTextureHandle::PrepareKeycolor (csRef<iImage>& image,
					 const csRGBpixel& transp_color,
					 csAlphaMode::AlphaType& alphaType)
{
  int w = image->GetWidth();
  int h = image->GetHeight ();
  int d = image->GetDepth ();
  csRGBpixel *_src = (csRGBpixel *)image->GetImageData ();
  CheckAlpha (w, h, d, _src, &transp_color, alphaType);
  if (alphaType == csAlphaMode::alphaNone) return; // Nothing to fix up
  image = csBakeKeyColor::Image (image, transp_color);
}

void csGLTextureHandle::GetKeyColor (uint8 &red, uint8 &green, uint8 &blue) const
{
  if (image.IsValid() && image->HasKeyColor() && !IsTranspSet ())
  {
    int r,g,b;
    image->GetKeyColor (r,g,b);
    red = r; green = g; blue = b;
  }
  else
  {
    red = transp_color.red;
    green = transp_color.green;
    blue = transp_color.blue;
  }
}

}
CS_PLUGIN_NAMESPACE_END(gl3d)
