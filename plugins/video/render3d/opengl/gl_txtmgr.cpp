
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

#include <stdarg.h>
#include <math.h>
#include "cssysdef.h"
#include "gl_txtmgr.h"
#include "gl_txtcache.h"
#include "csutil/scanstr.h"
#include "csutil/debug.h"
#include "iutil/cfgfile.h"
#include "igraphic/image.h"
#include "csgfx/memimage.h"
#include "ivaria/reporter.h"
#include "csgfx/xorpat.h"
#include "csutil/util.h"
#include "csgfx/csimgvec.h"

#include "glextmanager.h"

// csGLTexture stuff

csGLTexture::csGLTexture(csGLTextureHandle *p)
{
  w = h = d = size = 0;
  image_data = 0;
  Parent = p;
  image_data = NULL;
  size = 0;
  compressed = GL_FALSE;
}

csGLTexture::~csGLTexture()
{
  if(image_data)
    delete[] image_data;
}
/*
*
* New iTextureHandle Implementation
* done by Phil Aumayr (phil@rarebyte.com)
*
*/
SCF_IMPLEMENT_IBASE(csGLTextureHandle)
  SCF_IMPLEMENTS_INTERFACE(iTextureHandle)
SCF_IMPLEMENT_IBASE_END

csGLTextureHandle::csGLTextureHandle (iImage* image, int flags, int target, int bpp,
                                      GLenum sourceFormat, csGLRender3D *iR3D)
{
  this->sourceFormat = sourceFormat;
  this->R3D = iR3D;
  this->flags = flags;
  this->target = target;
  cachedata = 0;
  this->bpp = bpp;
  
  images = new csImageVector();
  (*images)[0] = image;
}

csGLTextureHandle::csGLTextureHandle (csRef<iImageVector> image, int flags, int target, int bpp,
    GLenum sourceFormat, csGLRender3D *iR3D)
{
  images = image;
  
  this->bpp = bpp;
  this->flags = flags;
  this->target = target;
  this->sourceFormat = sourceFormat;
  this->R3D = iR3D;
}

void csGLTextureManager::DetermineStorageSizes ()
{
  int i=-1;
  int d = pfmt.PixelBytes;
  while (glformats[++i].components)
  {
    if (glformats[i].texelbytes == 0)
    {
      //glformats[i].texelbytes = glformats[i].components * 8; // @@@ why *8?
      glformats[i].texelbytes = glformats[i].components;
      if (glformats[i].texelbytes > d) glformats[i].texelbytes = d;
    }
  }
}

void csGLTextureHandle::Clear()
{
}

void csGLTextureHandle::FreeImage ()
{
  images = NULL;
}

int csGLTextureHandle::GetFlags ()
{
  return flags;
}

void csGLTextureHandle::SetKeyColor (bool Enable)
{
  transp_color.alpha = (uint8) Enable;
  texupdate_needed = true;
}

void csGLTextureHandle::SetKeyColor (uint8 red, uint8 green, uint8 blue)
{
  transp_color.red = red;
  transp_color.green = green;
  transp_color.blue = blue;
  transp_color.alpha = 1;
  texupdate_needed = true;
}

bool csGLTextureHandle::GetKeyColor ()
{
  return(transp_color.alpha == 1);
}

bool csGLTextureHandle::FindFormatType ()
{
  int i;

  for (i=0; csGLTextureManager::glformats[i].sourceFormat != sourceFormat
   && csGLTextureManager::glformats[i].components; i++);

  if (csGLTextureManager::glformats[i].sourceFormat != sourceFormat)
  {
    printf ("unknown source format \n");
    return false;
  }

  formatidx = i;

  // do we force it to some specific targetFormat ?
  if (csGLTextureManager::glformats[i].forcedFormat != 0)
  {
    targetFormat = csGLTextureManager::glformats[i].forcedFormat;
    for (i=0; csGLTextureManager::glformats[i].targetFormat != targetFormat
     && csGLTextureManager::glformats[i].components; i++);

    if (csGLTextureManager::glformats[i].targetFormat != targetFormat)
      formatidx = i;
  }

  sourceType = GL_UNSIGNED_BYTE;
  targetFormat = csGLTextureManager::glformats[formatidx].targetFormat;

  if (csGLTextureManager::glformats[formatidx].sourceFormat == GL_RGB
      || csGLTextureManager::glformats[formatidx].sourceFormat == GL_RGBA)
  {
    static GLenum formats [13][4] = {
      {GL_RGBA,     GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_BYTE         },
      {GL_RGBA8,    GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_BYTE         },
      {GL_RGB5_A1,  GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_BYTE         },
      {GL_RGB10_A2, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_BYTE         },
      {GL_RGBA16,   GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_BYTE         },
      {GL_RGB,      GL_UNSIGNED_BYTE_3_3_2,    GL_UNSIGNED_SHORT_5_6_5,   GL_UNSIGNED_BYTE         },
      {GL_RGB8,     GL_UNSIGNED_BYTE_3_3_2,    GL_UNSIGNED_SHORT_5_6_5,   GL_UNSIGNED_BYTE         },
      {GL_RGB10,    GL_UNSIGNED_BYTE_3_3_2,    GL_UNSIGNED_SHORT_5_6_5,   GL_UNSIGNED_BYTE         },
      {GL_RGB16,    GL_UNSIGNED_BYTE_3_3_2,    GL_UNSIGNED_SHORT_5_6_5,   GL_UNSIGNED_BYTE         },
      {GL_RGB4,     GL_UNSIGNED_BYTE_3_3_2,    GL_UNSIGNED_SHORT_5_6_5,   GL_UNSIGNED_SHORT_5_6_5  },
      {GL_RGB5,     GL_UNSIGNED_BYTE_3_3_2,    GL_UNSIGNED_SHORT_5_6_5,   GL_UNSIGNED_SHORT_5_6_5  },
      {GL_R3_G3_B2, GL_UNSIGNED_BYTE_3_3_2,    GL_UNSIGNED_BYTE_3_3_2,    GL_UNSIGNED_BYTE_3_3_2   },
      {GL_RGBA2,    GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4}
    };

    if (sourceFormat == GL_RGBA)
    {
      if (!has_alpha)
      {
	if (!((*images)[0]->GetFormat () & CS_IMGFMT_ALPHA))
	{
	  sourceFormat = GL_RGB;
	  // Again determine the formatidx and possible change it if we
	  // have a forced targetformat
	  for (i=0; csGLTextureManager::glformats[i].sourceFormat
	     != sourceFormat; i++);
	  formatidx = i;
	  if (csGLTextureManager::glformats[i].forcedFormat != 0)
	  {
	    targetFormat = csGLTextureManager::glformats[i].forcedFormat;
	    for (i=0; csGLTextureManager::glformats[i].targetFormat
	      != targetFormat; i++);
	    if (csGLTextureManager::glformats[i].targetFormat
	        != targetFormat)
	      formatidx = i;
	  }
	  targetFormat = csGLTextureManager::glformats[formatidx]
		.targetFormat;
	}
	else
	{
	  // With a histogram we now could determine how many alpha values
	  // we have here to possibly select a better targetFormat but due
	  // to laziness we leave it as is.
	}
      }
      else
      {
	targetFormat = (bpp == 8 ? GL_RGBA : bpp < 32 ? GL_RGB5_A1 : GL_RGBA8);
	for (i=0; csGLTextureManager::glformats[i].targetFormat
	  != targetFormat; i++);
	formatidx = i;

	int pixels = (*images)[0]->GetWidth () * (*images)[0]->GetHeight ();
	csRGBpixel *_src = (csRGBpixel *)(*images)[0]->GetImageData ();

	while (pixels--)
	{
	  // By default, every csRGBpixel initializes its alpha component to
	  // 255. Thus, we should just drop to zero alpha for transparent
	  // pixels, if any.
	  if (transp_color.eq (*_src)) _src->alpha = 0;
	  _src++;
	}

	// Now we draw borders inside all keycolored areas.
	// This removes the halos of keycolor when using bilinear filtering
	int h, rows, w, cols;
	h = rows = (*images)[0]->GetHeight ();
	w = (*images)[0]->GetWidth();
	_src = (csRGBpixel *)(*images)[0]->GetImageData ();
	while (rows--)
	{
	  cols = w;
	  while (cols--)
	  {
	    if (!_src[(rows*w)+cols].alpha)
	    {
	      int n=0, r=0, g=0, b=0, xl, xr, yt, yb;
	      if (!cols)
	      {
		xl = w-1;
		xr = 1;
	      }
	      else if (cols==w-1)
	      {
		xl = cols-1;
		xr = 0;
	      }
	      else
	      {
		xl = cols-1;
		xr = cols+1;
	      }

	      if (!rows)
	      {
		yt = h-1;
		yb = 1;
	      }
	      else if (rows==h-1)
	      {
		yt = rows-1;
		yb = 0;
	      }
	      else
	      {
		yt = rows-1;
		yb = rows+1;
	      }

#define CHECK_PIXEL(d) { \
  if (_src[(d)].alpha) \
  { \
    n++; \
    r+=_src[(d)].red; \
    g+=_src[(d)].green; \
    b+=_src[(d)].blue; \
        } \
}
	      CHECK_PIXEL((yt*w)+xl);
	      CHECK_PIXEL((yt*w)+cols);
	      CHECK_PIXEL((yt*w)+xr);
	      CHECK_PIXEL((rows*w)+xl);
	      CHECK_PIXEL((rows*w)+xr);
	      CHECK_PIXEL((yb*w)+xl);
	      CHECK_PIXEL((yb*w)+cols);
	      CHECK_PIXEL((yb*w)+xr);
#undef CHECK_PIXEL
	      if (n)
	      {
		_src[(rows*w)+cols].red = r / n;
		_src[(rows*w)+cols].green = g / n;
		_src[(rows*w)+cols].blue = b / n;
	      }
	    }
	  }
	}
      }
    }

    int d;
    for (i=0; i < 12; i++)
    {
      if (targetFormat == formats[i][0]) break;
    }
    d = (bpp == 32 ? 24 : bpp) >> 3;
    sourceType = formats[i][d];
  }

  return true;
}

void csGLTextureHandle::GetKeyColor (uint8 &red, uint8 &green, uint8 &blue)
{
  red = transp_color.red;
  green = transp_color.green;
  blue = transp_color.blue;
}

bool csGLTextureHandle::GetMipMapDimensions (int mipmap, int &mw, int &mh)
{
  if(cachedata)
  {
    csGLTexture *real_tex = (csGLTexture*) cachedata;

    // real_tex size has to be multiple of 2
    mw = real_tex->get_width() >> mipmap;
    mh = real_tex->get_height() >> mipmap;
  }
  else
  if(images.IsValid() && (*images)[0].IsValid())
  {
    // TODO: implement to get size from image array
  }
  else
    return false;

  return true; 
}

void csGLTextureHandle::GetOriginalDimensions (int& mw, int& mh)
{
  if(images.IsValid() && (*images)[0].IsValid())
  {
    mw = (*images)[0]->GetWidth();
    mh = (*images)[0]->GetHeight();
  }
}

bool csGLTextureHandle::GetMipMapDimensions (int mipmap, int &mw, int &mh, int &md)
{
  if(cachedata)
  {
    csGLTexture *real_tex = (csGLTexture*) cachedata;

    // real_tex size has to be multiple of 2
    mw = real_tex->get_width() >> mipmap;
    mh = real_tex->get_height() >> mipmap;
    md = real_tex->get_depth() >> mipmap;
  }
  else
  if(images.IsValid() && (*images)[0].IsValid())
  {
    // TODO: implement to get size from image array
  }
  else
    return false;

  return true; 
}

void csGLTextureHandle::GetOriginalDimensions (int& mw, int& mh, int &md)
{
  if(images.IsValid() && (*images)[0].IsValid())
  {
    mw = (*images)[0]->GetWidth();
    mh = (*images)[0]->GetHeight();
    md = images->Length();
  }
}

void csGLTextureHandle::SetTextureTarget(int target)
{
  this->target = target;
}

void csGLTextureHandle::GetMeanColor (uint8 &red, uint8 &green, uint8 &blue)
{
}

void *csGLTextureHandle::GetCacheData ()
{
  return cachedata;
}

void csGLTextureHandle::SetCacheData (void *d)
{
  cachedata = d;
}

void *csGLTextureHandle::GetPrivateObject ()
{
  return (csGLTextureHandle *)this;
}

bool csGLTextureHandle::GetAlphaMap ()
{
  return has_alpha;
}

void csGLTextureHandle::Prepare ()
{
  // In opengl all textures, even non-mipmapped textures are required
  // to be powers of 2.
  AdjustSizePo2 ();

  // Determine the format and type of the source we gonna tranform the data to.
  FindFormatType ();
  CreateMipMaps ();

}

void csGLTextureHandle::AdjustSizePo2 ()
{
  int i;
  for(i = 0; i < images->Length(); i++)
  {
    int newwidth  = (*images)[i]->GetWidth();
    int newheight = (*images)[i]->GetHeight();

    if (!csIsPowerOf2(newwidth))
      newwidth = csFindNearestPowerOf2 ((*images)[i]->GetWidth ()) / 2;

    if (!csIsPowerOf2 (newheight))
      newheight = csFindNearestPowerOf2 ((*images)[i]->GetHeight ()) / 2;

    if (newwidth != (*images)[i]->GetWidth () || newheight != (*images)[i]->GetHeight ())
      (*images)[i]->Rescale (newwidth, newheight);
  }
}

void csGLTextureHandle::CreateMipMaps()
{

  // Delete existing mipmaps, if any
  int i;
  for (i = vTex.Length ()-1; i >= 0; i--)
  {
    if (vTex [i])
    {
      delete vTex [i];
    }
  }
  vTex.DeleteAll ();
  
  // This function assumes that alle images in images have the same dimension / bpp
  // Calculate how many mip map levels we will need
  int mipmaplevelcount = 0;

  int lwidth, lheight, ldepth;
  GetOriginalDimensions(lwidth, lheight, ldepth);
  int ltemp = MAX(lwidth, MAX(lheight, ldepth));
  while(ltemp)
  {
    ltemp >>= 1;
    mipmaplevelcount++;
  };
  // lmipmaplevels now contains the number of mipmaplevels we need

  // For every Mipmaplevel we have to create a csGLTexture
  // and store it in vTex
  int lcurmip; 

  for (i=0; csGLTextureManager::glformats[i].sourceFormat != sourceFormat
   && csGLTextureManager::glformats[i].components; i++);

  for(lcurmip = 0; lcurmip < mipmaplevelcount; lcurmip++)
  {
    csGLTexture *newtex = new csGLTexture(this);

    newtex->w = (lwidth > 1) ? lwidth >> lcurmip : 1;
    newtex->h = (lheight > 1) ? lheight >> lcurmip : 1;
    newtex->d = (ldepth > 1) ? ldepth >> lcurmip : 1;

    newtex->size = lwidth * lheight * ldepth * csGLTextureManager::glformats[i].components;
    newtex->image_data = new uint8[newtex->size];
    newtex->internalFormat = csGLTextureManager::glformats[i].targetFormat;


    int lcurdepth;
    for(lcurdepth = 0; lcurdepth < ldepth; lcurdepth++)
    {
      csRef<iImage> tmpimg = (*images)[lcurdepth]->MipMap(lcurmip, 0);
      int curdepthsize = tmpimg->GetWidth() * tmpimg->GetHeight() * csGLTextureManager::glformats[i].components;

      memcpy(newtex->image_data + curdepthsize*lcurdepth, 
        tmpimg->GetImageData(), 
        curdepthsize);

//      tmpimg->DecRef();
    }
    vTex.Push (newtex);
  }
}

iGraphics2D* csGLTextureHandle::GetCanvas ()
{
  return R3D->GetDriver2D();
}

/*
*
*New iMaterialHandle Implementation
*done by Phil Aumayr (phil@rarebyte.com)
*
*/

SCF_IMPLEMENT_IBASE (csGLMaterialHandle)
  SCF_IMPLEMENTS_INTERFACE (iMaterialHandle)
SCF_IMPLEMENT_IBASE_END

csGLMaterialHandle::csGLMaterialHandle (iMaterial* m, csGLTextureManager *parent)
{
  SCF_CONSTRUCT_IBASE (NULL);

  num_texture_layers = 0;
  material = m;
  if (material != 0)
  {
    texture = material->GetTexture ();
    material->GetReflection (diffuse, ambient, reflection);
    material->GetFlatColor (flat_color);
    num_texture_layers = material->GetTextureLayerCount ();
    if (num_texture_layers > CS_MATERIAL_MAX_TEXTURE_LAYERS) num_texture_layers = CS_MATERIAL_MAX_TEXTURE_LAYERS;
    int i;
    for (i = 0 ; i < num_texture_layers ; i++)
    {
      texture_layers[i] = *(material->GetTextureLayer (i));
      texture_layer_translate[i] =
	texture_layers[i].uscale != 1 ||
	texture_layers[i].vscale != 1 ||
	texture_layers[i].ushift != 0 ||
	texture_layers[i].vshift != 0;
    }
  }
  texman = parent;
}

csGLMaterialHandle::csGLMaterialHandle (iTextureHandle* t, csGLTextureManager *parent)
{
  SCF_CONSTRUCT_IBASE (NULL);
  num_texture_layers = 0;
  diffuse = 0.7; ambient = 0; reflection = 0;
  texture = t;
  texman = parent;
}

csGLMaterialHandle::~csGLMaterialHandle ()
{
  FreeMaterial ();
  texman->UnregisterMaterial (this);
}

void csGLMaterialHandle::FreeMaterial ()
{
  material = 0;
}

void csGLMaterialHandle::Prepare ()
{
  if (material)
  {
    if (texture != material->GetTexture())
    {
      texture = material->GetTexture ();
    }
    material->GetReflection (diffuse, ambient, reflection);
    material->GetFlatColor (flat_color);
  }
}

/*
*
* New iTextureManager Implementation
* done by Phil Aumayr (phil@rarebyte.com)
*
*/


// make sure the lenient versions are listed ahead of specific ones
CS_GL_FORMAT_TABLE (csGLTextureManager::glformats)
  CS_GL_FORMAT (GL_RGBA, GL_RGBA, 4, 0)
  CS_GL_FORMAT (GL_RGBA8,GL_RGBA,  4, 4)
  CS_GL_FORMAT (GL_RGB5_A1, GL_RGBA, 4, 2)
  CS_GL_FORMAT (GL_RGB10_A2, GL_RGBA, 4, 4)
  CS_GL_FORMAT (GL_RGBA16,GL_RGBA, 4, 8)
  CS_GL_FORMAT (GL_RGB, GL_RGB, 3, 0)
  CS_GL_FORMAT (GL_RGB8, GL_RGB, 3, 3)
  CS_GL_FORMAT (GL_RGB10, GL_RGB, 3, 4)
  CS_GL_FORMAT (GL_RGB16, GL_RGB, 3, 6)
  CS_GL_FORMAT (GL_RGB4, GL_RGB, 3, 2)
  CS_GL_FORMAT (GL_RGB5, GL_RGB, 3, 2)
  CS_GL_FORMAT (GL_R3_G3_B2, GL_RGB, 3, 1)
  CS_GL_FORMAT (GL_RGBA2, GL_RGBA, 4, 1)
  CS_GL_FORMAT (GL_ALPHA, GL_ALPHA, 1, 1)
  CS_GL_FORMAT (GL_ALPHA4, GL_ALPHA, 1, 1)
  CS_GL_FORMAT (GL_ALPHA8, GL_ALPHA, 1, 1)
  CS_GL_FORMAT (GL_ALPHA12, GL_ALPHA, 1, 2)
  CS_GL_FORMAT (GL_ALPHA16, GL_ALPHA, 1, 2)
  CS_GL_FORMAT (GL_BLUE, GL_BLUE, 1, 1)
  CS_GL_FORMAT (GL_GREEN, GL_GREEN, 1, 1)
  CS_GL_FORMAT (GL_RED, GL_RED, 1, 1)
  CS_GL_FORMAT (GL_INTENSITY, GL_INTENSITY, 1, 1)
  CS_GL_FORMAT (GL_INTENSITY4, GL_INTENSITY, 1, 1)
  CS_GL_FORMAT (GL_INTENSITY8, GL_INTENSITY, 1, 1)
  CS_GL_FORMAT (GL_INTENSITY12, GL_INTENSITY, 1, 2)
  CS_GL_FORMAT (GL_INTENSITY16, GL_INTENSITY, 1, 2)
  CS_GL_FORMAT (GL_LUMINANCE, GL_LUMINANCE, 1, 1)
  CS_GL_FORMAT (GL_LUMINANCE4, GL_LUMINANCE, 1, 1)
  CS_GL_FORMAT (GL_LUMINANCE8, GL_LUMINANCE, 1, 1)
  CS_GL_FORMAT (GL_LUMINANCE12, GL_LUMINANCE, 1, 2)
  CS_GL_FORMAT (GL_LUMINANCE16, GL_LUMINANCE, 1, 2)
  CS_GL_FORMAT (GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, 2, 2)
  CS_GL_FORMAT (GL_LUMINANCE4_ALPHA4, GL_LUMINANCE_ALPHA, 2, 1)
  CS_GL_FORMAT (GL_LUMINANCE6_ALPHA2, GL_LUMINANCE_ALPHA, 2, 1)
  CS_GL_FORMAT (GL_LUMINANCE8_ALPHA8, GL_LUMINANCE_ALPHA, 2, 2)
  CS_GL_FORMAT (GL_LUMINANCE12_ALPHA4, GL_LUMINANCE_ALPHA, 2, 2)
  CS_GL_FORMAT (GL_LUMINANCE12_ALPHA12, GL_LUMINANCE_ALPHA, 2, 3)
  CS_GL_FORMAT (GL_LUMINANCE16_ALPHA16, GL_LUMINANCE_ALPHA, 2, 4)
CS_GL_FORMAT_TABLE_END


SCF_IMPLEMENT_IBASE(csGLTextureManager)
  SCF_IMPLEMENTS_INTERFACE(iTextureManager)
SCF_IMPLEMENT_IBASE_END

csGLTextureManager::csGLTextureManager (iObjectRegistry* object_reg,
        iGraphics2D* iG2D, iConfigFile *config,
        csGLRender3D *iR3D) : textures (16, 16), materials (16, 16)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csGLTextureManager::object_reg = object_reg;
  verbose = false;

  pfmt = *iG2D->GetPixelFormat ();

  R3D = iR3D;
  max_tex_size = R3D->GetMaxTextureSize ();
  read_config (config);
  Clear ();
}

void csGLTextureManager::read_config (iConfigFile *config)
{
  csRef<iConfigIterator> it (config->Enumerate ("Video.OpenGL.TargetFormat"));
  while (it->Next ())
    AlterTargetFormat (it->GetKey (true)+1, it->GetStr ());
}

void csGLTextureManager::AlterTargetFormat (const char *oldTarget, const char *newTarget)
{
    // first find the old target
  int theOld=0;
  while (glformats[theOld].name && strcmp (glformats[theOld].name, oldTarget))
    theOld++;

  if (glformats[theOld].name)
  {
    if (!strcmp (newTarget, "compressed") && R3D->ext.CS_GL_ARB_texture_compression)
    {
      GLint compressedFormat;
      // is the format compressable at all ?
      switch (glformats[theOld].sourceFormat)
      {
      case GL_RGB:
	compressedFormat = GL_COMPRESSED_RGB_ARB;
	break;
      case GL_RGBA:
	compressedFormat = GL_COMPRESSED_RGBA_ARB;
	break;
      case GL_ALPHA:
	compressedFormat = GL_COMPRESSED_ALPHA_ARB;
	break;
      case GL_LUMINANCE:
	compressedFormat = GL_COMPRESSED_LUMINANCE_ARB;
	break;
      case GL_LUMINANCE_ALPHA:
	compressedFormat = GL_COMPRESSED_LUMINANCE_ALPHA_ARB;
	break;
      case GL_INTENSITY:
	compressedFormat = GL_COMPRESSED_INTENSITY_ARB;
	break;
      default:
	printf ("%s is not compressable !\n", oldTarget);
	return;
      }
      glformats[theOld].compressedFormat = compressedFormat;
    }
    else
    {
      // does the new format exist ?
      int theNew=0;
      while (glformats[theNew].name
      		&& strcmp (glformats[theNew].name, newTarget))
	theNew++;

      if (glformats[theNew].name)
	if (glformats[theNew].sourceFormat == glformats[theOld].sourceFormat)
	  glformats[theOld].forcedFormat = glformats[theNew].targetFormat;
	else
	{
	  printf ("You can only force a new targetFormat if both, old and new targetFormat,"
	    " have the same sourceFormat\n");
	}
    }
  }
}
void csGLTextureManager::Clear()
{
  for (int i=0; i < textures.Length (); i++)
    ((csGLTextureHandle *)textures.Get (i))->Clear ();
}

void csGLTextureManager::UnregisterMaterial (csGLMaterialHandle* handle)
{
  int idx = materials.Find (handle);
  if (idx >= 0) materials.Delete (idx);
}

csPtr<iTextureHandle> csGLTextureManager::RegisterTexture (iImage *image, int flags)
{
  if (!image)
  {
    R3D->Report(CS_REPORTER_SEVERITY_BUG,
      "BAAAAAAAD!!! csGLTextureManager::RegisterTexture with NULL image!");
    return NULL;
  }

  csGLTextureHandle *txt = new csGLTextureHandle (image, flags, iTextureHandle::CS_TEX_IMG_2D,pfmt.PixelBytes*8, GL_RGBA, R3D);
  textures.Push(txt);
  return csPtr<iTextureHandle> (txt);
}

csPtr<iTextureHandle> csGLTextureManager::RegisterTexture (iImageVector *image, int flags, int target)
{
  if (!image)
  {
    R3D->Report(CS_REPORTER_SEVERITY_BUG,
      "BAAAAAAAD!!! csGLTextureManager::RegisterTexture with NULL image array!");
    return NULL;
  }

  csGLTextureHandle *txt = new csGLTextureHandle (image, flags, target,pfmt.PixelBytes*8, GL_RGBA, R3D);
  textures.Push(txt);
  return csPtr<iTextureHandle> (txt);
}

void csGLTextureManager::PrepareTextures ()
{
  // Create mipmaps for all textures
  int i;
  for (i = 0; i < textures.Length (); i++)
    textures.Get (i)->Prepare ();
}

void csGLTextureManager::FreeImages ()
{
  int i;
  for (i = 0 ; i < textures.Length () ; i++)
    textures.Get (i)->FreeImage ();
}

csPtr<iMaterialHandle> csGLTextureManager::RegisterMaterial (iMaterial* material)
{
  if (!material) return 0;
  csGLMaterialHandle *mat = new csGLMaterialHandle (material, this);
  materials.Push (mat);
  return csPtr<iMaterialHandle> (mat);
}

csPtr<iMaterialHandle> csGLTextureManager::RegisterMaterial (
      iTextureHandle* txthandle)
{
  if (!txthandle) return 0;
  csGLMaterialHandle *mat = new csGLMaterialHandle (txthandle, this);
  materials.Push (mat);
  return csPtr<iMaterialHandle> (mat);
}

void csGLTextureManager::PrepareMaterials ()
{
  int i;
  for (i = 0; i < materials.Length (); i++)
    materials.Get (i)->Prepare ();
}

void csGLTextureManager::FreeMaterials ()
{
  int i;
  for (i = 0; i < materials.Length (); i++)
    materials.Get (i)->FreeMaterial ();
}

void csGLTextureManager::SetVerbose (bool vb)
{
  verbose = vb;
}

int csGLTextureManager::GetTextureFormat ()
{
  return CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA;
}

void csGLTextureManager::SetPixelFormat (csPixelFormat &PixelFormat)
{
  pfmt = PixelFormat;
  max_tex_size = R3D->GetMaxTextureSize ();
  DetermineStorageSizes ();
}
