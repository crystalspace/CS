/*
    Copyright (C) 1998 by Jorrit Tyberghein

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
#include "soft_txt.h"
#include "csgfx/bakekeycolor.h"
#include "csgfx/imagemanipulate.h"
#include "csgfx/memimage.h"
#include "csgfx/packrgb.h"
#include "csgfx/xorpat.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "igraphic/image.h"
#include "ivaria/reporter.h"
#include "csqint.h"
#include "soft_g3d.h"

CS_PLUGIN_NAMESPACE_BEGIN(Soft3D)
{

void csSoftwareTexture::compute_masks ()
{
  shf_w = csLog2 (w);
  and_w = (1 << shf_w) - 1;
  shf_h = csLog2 (h);
  and_h = (1 << shf_h) - 1;
}

void csSoftwareTexture::ImageToBitmap (iImage *Image)
{
  csRef<iImage> img;
  if (parent->GetKeyColor ())
    img = csBakeKeyColor::Image (Image, parent->transp_color);
  else
    img = Image;

  size_t pixNum = w * h;
  bitmap = new uint32[pixNum];
#ifdef CS_LITTLE_ENDIAN
  if (csPackRGBA::IsRGBpixelSane() 
    && !parent->texman->G3D->pixelBGR)
  {
    memcpy (bitmap, img->GetImageData(), pixNum * sizeof (uint32));
  }
  else
#endif
  if (parent->texman->G3D->pixelBGR)
  {
    csRGBpixel* p = (csRGBpixel*)img->GetImageData();
    uint32* dst = bitmap;
    while (pixNum-- > 0)
    {
      Pixel px (p->blue, p->green, p->red, p->alpha);
      *dst = px.ui32;
      dst++; p++;
    }
  }
  else
  {
    csRGBpixel* p = (csRGBpixel*)img->GetImageData();
    uint32* dst = bitmap;
    while (pixNum-- > 0)
    {
      Pixel px (p->red, p->green, p->blue, p->alpha);
      *dst = px.ui32;
      dst++; p++;
    }
  }
}

//----------------------------------------------- csSoftwareTextureHandle ---//

csSoftwareTextureHandle::csSoftwareTextureHandle (
	csSoftwareTextureManager *texman, iImage *Image, int flags)
	: csTextureHandle (texman, flags), image (Image)
{
  if (flags & CS_TEXTURE_3D)
  {
    int newwidth = 0, newheight = 0, newdepth = 0;
    AdjustSizePo2 (image->GetWidth(), image->GetHeight(), image->GetDepth(),
      newwidth, newheight, newdepth);
    if (newwidth != image->GetWidth () || newheight != image->GetHeight ())
      image = csImageManipulate::Rescale (image, newwidth, newheight, newdepth);
  }
  this->texman = texman;
  prepared = false;
  memset (tex, 0, sizeof (tex));

  if (image.IsValid() && image->GetFormat () & CS_IMGFMT_ALPHA)
    alphaType = csAlphaMode::alphaSmooth;
  else if (image.IsValid() && image->HasKeyColor ())
  {
    alphaType = csAlphaMode::alphaBinary;
    int r,g,b;
    image->GetKeyColor (r,g,b);
    SetKeyColor (r, g, b);
  }
  else
    alphaType = csAlphaMode::alphaNone;
}

csSoftwareTextureHandle::~csSoftwareTextureHandle ()
{
  if (texman) texman->UnregisterTexture (this);
}

void csSoftwareTextureHandle::CreateMipmaps ()
{
  if (!image) return;

  csRGBpixel *tc = transp ? &transp_color : (csRGBpixel *)0;

  // Delete existing mipmaps, if any
  int i;
  for (i = 0; i < 4; i++)
  {
    delete tex [i];
  }

  tex [0] = new csSoftwareTexture (this, image);

  // 2D textures uses just the top-level mipmap
  if ((flags & (CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS)) == CS_TEXTURE_3D)
  {
    csRef<iImage> thisImage = image;
    int nMipmaps = thisImage->HasMipmaps();
    
    for (int i = 1; i < 4; i++)
    {
      csRef<iImage> newImage;
      if (i <= nMipmaps)
	newImage = image->GetMipmap (i);
      if (!newImage.IsValid())
      {
	newImage = csImageManipulate::Mipmap (thisImage, 1, tc);
	if (texman->sharpen_mipmaps)
	  newImage = csImageManipulate::Sharpen (newImage, texman->sharpen_mipmaps, tc);
      }
      thisImage = newImage;

      if (texman->debugMipmaps)
      {
	newImage.AttachNew (new csImageMemory (newImage));
	size_t numPx = newImage->GetWidth() * newImage->GetHeight();
	csRGBpixel* p = (csRGBpixel*)newImage->GetImageData();
	while (numPx-- > 0)
	{
	  if (i >= 2) p->red = 0;
	  if ((i == 1) || (i == 3)) p->blue = 0;
	  if (i <= 2) p->green = 0;
	  p++;
	}
      }

      tex [i] = new csSoftwareTexture (this, newImage);
    }
  }
}

void csSoftwareTextureHandle::PrepareInt ()
{
  if (prepared) return;
  prepared = true;
  CreateMipmaps ();
  image = 0;
}

void csSoftwareTextureHandle::Blit (int x, int y, int width, int height,
				    unsigned char const* data, 
				    TextureBlitDataFormat format)
{
  csSoftwareTexture *tex0 = (csSoftwareTexture *)tex[0];
  uint32* bitmap = tex0->bitmap;
  uint8 *src = (uint8*)data;

  int tex_w, tex_h;
  tex_w = tex0->w;
  tex_h = tex0->h;

  int blit_w = x+width;
  int blit_h = y+height;

  if (blit_w > tex_w) blit_w = tex_w;
  if (blit_h > tex_h) blit_h = tex_h;

  if (x > tex_w) return;
  if (y > tex_h) return;

  int rx, ry;
  for (ry = y; ry < blit_h; ry++)
  {
    uint32* linestart = bitmap + (ry*tex_w + x);
#ifdef CS_LITTLE_ENDIAN
    if (format == RGBA8888)
    {
      memcpy (linestart, src, blit_w * sizeof (uint32));
      src += blit_w * sizeof (uint32);
    }
    else
#endif
    {
      for (rx = x; rx < blit_w; rx++)
      {
        
	uint8 r,g,b,a;
#ifndef CS_LITTLE_ENDIAN
	if (format == RGBA8888)
	{
	  r = *src++;
	  g = *src++;
	  b = *src++;
	  a = *src++;
	}
	else
#endif
	{
	  b = *src++;
	  g = *src++;
	  r = *src++;
	  a = *src++;
	}
	*linestart++ = r
		    | (g <<  8)
		    | (b << 16)
		    | (a << 24);
      }
    }
  }

  // We don't generate mipmaps or so...
  flags |= CS_TEXTURE_NOMIPMAPS;
}

bool csSoftwareTextureHandle::GetRendererDimensions (int &mw, int &mh)
{
  PrepareInt ();
  if (!tex[0]) return false;
  mw = tex[0]->w;
  mh = tex[0]->h;
  return true;
}

//----------------------------------------------- csSoftwareTextureManager ---//

csSoftwareTextureManager::csSoftwareTextureManager (
  iObjectRegistry *object_reg,
  csSoftwareGraphics3DCommon *iG3D, iConfigFile *config)
  : csTextureManager (object_reg, iG3D->GetDriver2D())
{
  read_config (config);
  G3D = iG3D;
}

void csSoftwareTextureManager::SetPixelFormat(csPixelFormat const& PixelFormat)
{
  pfmt = PixelFormat;
}

void csSoftwareTextureManager::read_config (iConfigFile *config)
{
  csTextureManager::read_config (config);
  sharpen_mipmaps = config->GetInt
        ("Video.Software.TextureManager.SharpenMipmaps", 0);
  debugMipmaps = config->GetBool
        ("Video.Software.TextureManager.DebugMipmaps", false);
}

csSoftwareTextureManager::~csSoftwareTextureManager ()
{
  Clear ();
}

void csSoftwareTextureManager::Clear ()
{
  csTextureManager::Clear();
}

int csSoftwareTextureManager::GetTextureFormat ()
{
  return CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA;
}

uint32 csSoftwareTextureManager::encode_rgb (int r, int g, int b)
{
  return
    ((r >> (8 - pfmt.RedBits))   << pfmt.RedShift) |
    ((g >> (8 - pfmt.GreenBits)) << pfmt.GreenShift) |
    ((b >> (8 - pfmt.BlueBits))  << pfmt.BlueShift);
}

csPtr<iTextureHandle> csSoftwareTextureManager::RegisterTexture (iImage* image,
  int flags)
{
  if (!image)
  {
    G3D->Report(CS_REPORTER_SEVERITY_BUG,
      "BAAAD!!! csSoftwareTextureManager::RegisterTexture with 0 image!");

    csRef<iImage> im (csCreateXORPatternImage(32, 32, 5));
    image = im;
    im->IncRef ();	// Avoid smart pointer cleanup. @@@ UGLY
  }

  csSoftwareTextureHandle *txt = new csSoftwareTextureHandle (
  	this, image, flags);
  textures.Push (txt);
  return csPtr<iTextureHandle> (txt);
}

void csSoftwareTextureManager::UnregisterTexture (
		csSoftwareTextureHandle* handle)
{
  size_t idx = textures.Find (handle);
  if (idx != csArrayItemNotFound) textures.DeleteIndexFast (idx);
}

csPtr<iSuperLightmap> csSoftwareTextureManager::CreateSuperLightmap (
  int /*width*/, int /*height*/)
{
  return 0;
}

void csSoftwareTextureManager::GetMaxTextureSize (int& w, int& h, int& aspect)
{
  w = h = 2048;
  aspect = 32768;
}

}
CS_PLUGIN_NAMESPACE_END(Soft3D)
