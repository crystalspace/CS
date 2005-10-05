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
#include "csgfx/inv_cmap.h"
#include "csgfx/packrgb.h"
#include "csgfx/quantize.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "igraphic/image.h"
#include "ivaria/reporter.h"
#include "csqint.h"
#include "csgfx/imagemanipulate.h"
#include "csgfx/memimage.h"
#include "csgfx/xorpat.h"
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
  size_t pixNum = w * h;
  bitmap = new uint32[pixNum];
#ifdef CS_LITTLE_ENDIAN
  if (csPackRGBA::IsRGBpixelSane())
  {
    memcpy (bitmap, Image->GetImageData(), pixNum * sizeof (uint32));
  }
  else
#endif
  {
    csRGBpixel* p = (csRGBpixel*)Image->GetImageData();
    uint32* dst = bitmap;
    while (pixNum-- > 0)
    {
      *dst = p->red
	  | (p->green <<  8)
	  | (p->blue  << 16)
	  | (p->alpha << 24);
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

  if (image.IsValid() && image->HasKeyColor ())
  {
    int r,g,b;
    image->GetKeyColor (r,g,b);
    SetKeyColor (r, g, b);
  }
}

csSoftwareTextureHandle::~csSoftwareTextureHandle ()
{
  if (texman) texman->UnregisterTexture (this);
}

csSoftwareTexture* csSoftwareTextureHandle::NewTexture (iImage *newImage,
	bool ismipmap)
{
  csRef<iImage> Image;
  if (ismipmap && texman->sharpen_mipmaps)
  { 
    csRGBpixel *tc = transp ? &transp_color : (csRGBpixel *)0;
    Image = csImageManipulate::Sharpen (newImage, texman->sharpen_mipmaps, tc);
  }
  else
    Image = newImage;

  return new csSoftwareTexture (this, Image);
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

  // @@@ Jorrit: removed the following IncRef() because I can really
  // see no reason for it and it seems to be causing memory leaks.
#if 0
  // Increment reference counter on image since NewTexture() expects
  // a image with an already incremented reference counter
  image->IncRef ();
#endif
  tex [0] = NewTexture (image, false);

  // 2D textures uses just the top-level mipmap
  if ((flags & (CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS)) == CS_TEXTURE_3D)
  {
    // Create each new level by creating a level 2 mipmap from previous level
    csRef<iImage> i1 = csImageManipulate::Mipmap (image, 1, tc);
    csRef<iImage> i2 = csImageManipulate::Mipmap (i1, 1, tc);
    csRef<iImage> i3 = csImageManipulate::Mipmap (i2, 1, tc);

    tex [1] = NewTexture (i1, true);
    tex [2] = NewTexture (i2, true);
    tex [3] = NewTexture (i3, true);
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

SCF_IMPLEMENT_IBASE_INCREF(csSoftRendererLightmap)					
SCF_IMPLEMENT_IBASE_GETREFCOUNT(csSoftRendererLightmap)				
SCF_IMPLEMENT_IBASE_REFOWNER(csSoftRendererLightmap)				
SCF_IMPLEMENT_IBASE_REMOVE_REF_OWNERS(csSoftRendererLightmap)
SCF_IMPLEMENT_IBASE_QUERY(csSoftRendererLightmap)
SCF_IMPLEMENTS_INTERFACE(iRendererLightmap)
SCF_IMPLEMENT_IBASE_END

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

csSoftRendererLightmap::csSoftRendererLightmap ()
{
  SCF_CONSTRUCT_IBASE (0);

  dirty = false;
  data = 0;

  lightCellSize = 0;
  memset (cacheData, 0, sizeof (cacheData));
}

csSoftRendererLightmap::~csSoftRendererLightmap ()
{
  delete[] data;
  SCF_DESTRUCT_IBASE();
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

SCF_IMPLEMENT_IBASE(csSoftSuperLightmap)
SCF_IMPLEMENTS_INTERFACE(iSuperLightmap)
SCF_IMPLEMENT_IBASE_END

csSoftSuperLightmap::csSoftSuperLightmap (csSoftwareTextureManager* texman, 
					  int width, int height) : RLMs(32)
{
  SCF_CONSTRUCT_IBASE (0);
  w = width;
  h = height;
  tex.AttachNew (new csSoftwareTextureHandle (texman, 0, 0));
}

csSoftSuperLightmap::~csSoftSuperLightmap ()
{
  SCF_DESTRUCT_IBASE();
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

static uint8 *GenLightmapTable (int bits)
{
  uint8 *table = new uint8 [64 * 256];
  uint8 *dst = table;
  uint8 maxv = (1 << bits) - 1;
  int rshf = (13 - bits);
  int i, j;
  for (i = 0; i < 64; i++)
  {
    *dst++ = 0;
    for (j = 1; j < 256; j++)
    {
      int x = (i * j) >> rshf;
      *dst++ = (x > maxv) ? maxv : (x?x:1) ;
    }
  }
  return table;
}

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

  // Create multiplication tables
  lightmap_tables [0] = GenLightmapTable (pfmt.RedBits);

  if (pfmt.GreenBits == pfmt.RedBits)
    lightmap_tables [1] = lightmap_tables [0];
  else
    lightmap_tables [1] = GenLightmapTable (pfmt.GreenBits);

  if (pfmt.BlueBits == pfmt.RedBits)
    lightmap_tables [2] = lightmap_tables [0];
  else if (pfmt.BlueBits == pfmt.GreenBits)
    lightmap_tables [2] = lightmap_tables [1];
  else
    lightmap_tables [2] = GenLightmapTable (pfmt.BlueBits);
}

void csSoftwareTextureManager::read_config (iConfigFile *config)
{
  csTextureManager::read_config (config);
  dither_textures = config->GetBool
        ("Video.Software.TextureManager.DitherTextures", true);
  sharpen_mipmaps = config->GetInt
        ("Video.Software.TextureManager.SharpenMipmaps", 0);
}

csSoftwareTextureManager::~csSoftwareTextureManager ()
{
  delete [] lightmap_tables [0];
  if (lightmap_tables [1] != lightmap_tables [0])
    delete [] lightmap_tables [1];
  if (lightmap_tables [2] != lightmap_tables [1]
   && lightmap_tables [2] != lightmap_tables [0])
    delete [] lightmap_tables [2];
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
  int slmWidth, int slmHeight, int lm_x1, int lm_y1, int lm_x2, int lm_y2,
  float& lm_u1, float& lm_v1, float &lm_u2, float& lm_v2)
{
  lm_u1 = lm_x1;
  lm_v1 = lm_y1;
  /*lm_u2 = lm_x2 + 1;
  lm_v2 = lm_y2 + 1;
  lm_u1 = lm_x1;
  lm_v1 =*/ lm_u2 = lm_v2 = 0.0f;
}

} // namespace cspluginSoft3d
