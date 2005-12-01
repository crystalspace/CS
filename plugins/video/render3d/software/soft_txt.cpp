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

namespace cspluginSoft3d
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

//----------------------------------------------------------------------------//

void csSoftRendererLightmap::DecRef ()
{
  if (scfRefCount == 1)							
  {									
    CS_ASSERT (slm != 0);
    slm->FreeRLM (this);
    return;								
  }									
  scfRefCount--;							
}

csSoftRendererLightmap::csSoftRendererLightmap () : 
  scfImplementationType (this)
{
  dirty = false;
  data = 0;

  lightCellSize = 0;
  memset (cacheData, 0, sizeof (cacheData));
}

csSoftRendererLightmap::~csSoftRendererLightmap ()
{
  delete[] data;
}

void csSoftRendererLightmap::SetSize (size_t lmPixels)
{
  delete[] data;
  lmSize = lmPixels;
  data = new csRGBpixel[lmSize];
  lmSize *= sizeof (csRGBpixel);
}

void csSoftRendererLightmap::GetSLMCoords (int& left, int& top, 
                                           int& width, int& height)
{
  left = rect.xmin; top  = rect.xmin;
  width = rect.Width (); height = rect.Height ();
}

void csSoftRendererLightmap::SetData (csRGBpixel* data)
{
  memcpy (csSoftRendererLightmap::data, data,
    lmSize);
  dirty = true;
}

void csSoftRendererLightmap::SetLightCellSize (int size)
{
  lightCellSize = size;
  lightCellShift = csLog2 (lightCellSize);
}

int csSoftRendererLightmap::GetLightCellSize ()
{
  return lightCellSize;
}

//---------------------------------------------------------------------------

csSoftSuperLightmap::csSoftSuperLightmap (csSoftwareTextureManager* texman, 
					  int width, int height) : 
  scfImplementationType (this), RLMs(32)
{
  w = width;
  h = height;
  tex.AttachNew (new csSoftwareTextureHandle (texman, 0, 0));
}

csSoftSuperLightmap::~csSoftSuperLightmap ()
{
}

void csSoftSuperLightmap::FreeRLM (csSoftRendererLightmap* rlm)
{
  // IncRef() ourselves manually.
  // Otherwise freeing the RLM could trigger our own destruction -
  // causing an assertion in block allocator (due to how BA frees items and
  // the safety assertions on BA destruction.)
  scfRefCount++;
  int id = idmap.GetKey (rlm, ~0);
  if (id != ~0)
  {
    idmap.Delete (id, rlm);
  }
  RLMs.Free (rlm);
  DecRef ();
}

csSoftRendererLightmap* csSoftSuperLightmap::GetRlmForID (int id)
{
  return idmap.Get (id, 0);
}

csPtr<iRendererLightmap> csSoftSuperLightmap::RegisterLightmap (int left, int top, 
                                                                int width, int height)
{
  csSoftRendererLightmap* rlm = RLMs.Alloc ();
  rlm->SetSize (width * height);
  rlm->slm = this;
  rlm->rect.Set (left, top, left + width, top + height);

  rlm->u1 = left;
  rlm->v1 = top;
  rlm->u2 = left + width;
  rlm->v2 = top  + height;

  const int id = ComputeRlmID (left, top);
  idmap.Put (id, rlm);

  return csPtr<iRendererLightmap> (rlm);
}

csPtr<iImage> csSoftSuperLightmap::Dump ()
{
  return 0;
}

iTextureHandle* csSoftSuperLightmap::GetTexture ()
{
  return tex;
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
  int width, int height)
{
  return csPtr<iSuperLightmap> (new csSoftSuperLightmap (this, width, height));
}

void csSoftwareTextureManager::GetMaxTextureSize (int& w, int& h, int& aspect)
{
  w = h = 2048;
  aspect = 32768;
}

void csSoftwareTextureManager::GetLightmapRendererCoords (
  int /*slmWidth*/, int /*slmHeight*/, int lm_x1, int lm_y1, int /*lm_x2*/,
  int /*lm_y2*/, float& lm_u1, float& lm_v1, float &lm_u2, float& lm_v2)
{
  lm_u1 = lm_x1;
  lm_v1 = lm_y1;
  /*lm_u2 = lm_x2 + 1;
  lm_v2 = lm_y2 + 1;
  lm_u1 = lm_x1;
  lm_v1 =*/ lm_u2 = lm_v2 = 0.0f;
}

} // namespace cspluginSoft3d
