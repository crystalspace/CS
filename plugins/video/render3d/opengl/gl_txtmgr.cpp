
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
#include "csutil/array.h"

#include "video/canvas/openglcommon/glextmanager.h"

// csGLTexture stuff

csGLTexture::csGLTexture(csGLTextureHandle *p, iImage *Image)
{
  d = 1;
  w = Image->GetWidth ();
  h = Image->GetHeight ();
  image_data = 0;
  Parent = p;
  image_data = 0;
  size = 0;
  compressed = GL_FALSE;
}

csGLTexture::~csGLTexture()
{
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
  SCF_CONSTRUCT_IBASE(0);
  this->target = target;
  (R3D = iR3D)->IncRef();
  (txtmgr = R3D->txtmgr)->IncRef();
  has_alpha = false;
  this->sourceFormat = sourceFormat;
  this->bpp = bpp;
  size = 0;
  was_render_target = false;

  images = new csImageVector();
  image->IncRef();
  images->AddImage(image,0);

  this->flags = flags;
  transp = false;
  transp_color.red = transp_color.green = transp_color.blue = 0;

  mean_color.red = mean_color.green = mean_color.blue = 0;
  if (image->HasKeycolor ())
  {
    int r,g,b;
    image->GetKeycolor (r,g,b);
    SetKeyColor (r, g, b);
  }
  cachedata = 0;
}

csGLTextureHandle::csGLTextureHandle (csRef<iImageVector> image, int flags, int target, int bpp,
    GLenum sourceFormat, csGLRender3D *iR3D)
{
  SCF_CONSTRUCT_IBASE(0);
  this->target = target;
  (R3D = iR3D)->IncRef();
  (txtmgr = R3D->txtmgr)->IncRef();
  has_alpha = false;
  this->sourceFormat = sourceFormat;
  this->bpp = bpp;
  size = 0;
  was_render_target = false;

  images = image;
  int i=0;
  for (i=0; i < images->Length(); i++)
    (*images)[i]->IncRef();

  this->flags = flags;
  transp = false;
  transp_color.red = transp_color.green = transp_color.blue = 0;

  if ((*images)[0]->HasKeycolor ())
  {
    int r,g,b;
    (*images)[0]->GetKeycolor (r,g,b);
    SetKeyColor (r, g, b);
  }
  cachedata = 0;
}

csGLTextureHandle::~csGLTextureHandle()
{
  for(int i=0; i<vTex.Length(); i++)
    delete vTex[i];
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
  images = 0;
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
      if (!transp)
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
	targetFormat = (bpp == 8 ? GL_RGBA : (bpp < 24 ? GL_RGB5_A1 : GL_RGBA8));
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
    //csGLTexture *real_tex = (csGLTexture*) cachedata;

    mw = orig_width >> mipmap;
    mh = orig_height >> mipmap;
    // real_tex size has to be multiple of 2
    //mw = real_tex->get_width() >> mipmap;
    //mh = real_tex->get_height() >> mipmap;
  }
  else
  if(images.IsValid() && (*images)[0].IsValid())
  {
    // Dunno if this is right, but it seems to work.
    mw = vTex[0]->get_width () >> mipmap;
    mh = vTex[0]->get_height () >> mipmap;
  }
  else
    return false;

  return true; 
}

void csGLTextureHandle::GetOriginalDimensions (int& mw, int& mh)
{
  if(images.IsValid() && (*images)[0].IsValid())
  {
    mw = (*images)[0]->GetWidth() << txtmgr->texture_downsample;
    mh = (*images)[0]->GetHeight() << txtmgr->texture_downsample;
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
  red = green = blue = 0;
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
    orig_width  = (*images)[i]->GetWidth();
    orig_height = (*images)[i]->GetHeight();

    int newwidth  = orig_width;
    int newheight = orig_height;

    if (!csIsPowerOf2(newwidth))
      newwidth = csFindNearestPowerOf2 ((*images)[i]->GetWidth ()) / 2;

    if (!csIsPowerOf2 (newheight))
      newheight = csFindNearestPowerOf2 ((*images)[i]->GetHeight ()) / 2;

    // downsample textures, if requested, but not 2D textures
    if (!(flags & (CS_TEXTURE_2D)))
    {
      newwidth >>= txtmgr->texture_downsample;
      newheight >>= txtmgr->texture_downsample;
    }

    // If necessary rescale if bigger than maximum texture size
    if (newwidth > txtmgr->max_tex_size) newwidth = txtmgr->max_tex_size;
    if (newheight > txtmgr->max_tex_size) newheight = txtmgr->max_tex_size;

    if (newwidth != (*images)[i]->GetWidth () || newheight != (*images)[i]->GetHeight ())
    {
      (*images)[i]->Rescale (newwidth, newheight);
    }
  }
}

csGLTexture* csGLTextureHandle::NewTexture (iImage *Image, bool ismipmap)
{
  ismipmap = false;
  return new csGLTexture (this, Image);
}

void csGLTextureHandle::CreateMipMaps()
{
  //int thissize;
  csRGBpixel *tc = transp ? &transp_color : (csRGBpixel *)0;

  //  printf ("delete old\n");
  // Delete existing mipmaps, if any
  int i;
  for(i=0; i<vTex.Length(); i++)
    delete vTex[i];
  vTex.DeleteAll ();

  size = 0;
  //  printf ("push 0\n");
  csGLTexture* ntex = NewTexture ((*images)[0], false);
  ntex->d = images->Length();
  ntex->components = csGLTextureManager::glformats[formatidx].components;
  vTex.Push (ntex);
  DG_LINK (this, ntex);

  //  printf ("transform 0\n");
  transform (images, vTex[0]);
  // Create each new level by creating a level 2 mipmap from previous level
  // we do this down to 1x1 as opengl defines it

  iImageVector* prevImages = images;
  csRef<iImageVector> thisImages =
      csPtr<iImageVector> (new csImageVector()); 
  csArray<int> nMipmaps;

  int w = (*prevImages)[0]->GetWidth ();
  int h = (*prevImages)[0]->GetHeight ();
  int nTex = 0;

  for (i=0; i < prevImages->Length(); i++)
  {
      nMipmaps.Push ((*prevImages)[i]->HasMipmaps());
  }
  
  while (w != 1 || h != 1)
  {
    nTex++;
    for (i=0; i<prevImages->Length();i++)
    {
      csRef<iImage> cimg;
      if (nMipmaps[i] != 0)
      {
	cimg = (*images)[i]->MipMap (nTex, tc);
	nMipmaps[i]--;
      }
      else
      {
        cimg = (*prevImages)[i]->MipMap (1, tc);
      }
      if (txtmgr->sharpen_mipmaps)
      {
	cimg = cimg->Sharpen (tc, txtmgr->sharpen_mipmaps);
      }
      (*thisImages)[i] = cimg;
    }

    csGLTexture* ntex = NewTexture ((*thisImages)[0], true);
    ntex->d = thisImages->Length();
    vTex.Push (ntex);
    DG_LINK (this, ntex);

    transform (thisImages, ntex);
    w = (*thisImages)[0]->GetWidth ();
    h = (*thisImages)[0]->GetHeight ();
    for (i=0; i < thisImages->Length(); i++)
    {
      (*prevImages)[i] = (*thisImages)[i];
    }
  }
}


bool csGLTextureHandle::transform (iImageVector *ImageVector, csGLTexture *tex)
{
  iImage* Image = (*ImageVector)[0];
  uint8 *h;
  uint8 *&image_data = tex->get_image_data ();
  //csRGBpixel *data = (csRGBpixel *)Image->GetImageData ();
  csRGBpixel *data = 0;
  int n = Image->GetWidth ()*Image->GetHeight ();
  int d = ImageVector->Length();
  int i=0, j=0;
  int nCompo;

  // First we determine the exact sourceformat if targetformat is given
  // without bit specifications.
  switch (csGLTextureManager::glformats[formatidx].sourceFormat)
  {
    case GL_ALPHA: i++;  // Fall thru
    case GL_BLUE: i++;  // Fall thru
    case GL_GREEN: i++;  // Fall thru
    case GL_RED:
      image_data = new uint8 [n * d];
      for (j=0; j < d; j++)
      {
        data = (csRGBpixel *)(*ImageVector)[j]->GetImageData();
        h = (uint8*)data;
        h += i;
        for (i=0; i<n; i++, h += 4)
          *image_data++ = *h;
      }
      break;
    case GL_INTENSITY:
      image_data = new uint8 [n*d];
      for (j=0; j < d; j++)
      {
        data = (csRGBpixel *)(*ImageVector)[j]->GetImageData();
        for (i=0; i<n; i++, data++)
          *image_data++ = data->Intensity ();
      }
      break;
    case GL_LUMINANCE:
      image_data = new uint8 [n*d];
      for (j=0; j < d; j++)
      {
        data = (csRGBpixel *)(*ImageVector)[j]->GetImageData();
        for (i=0; i<n; i++, data++)
          *image_data++ = data->Luminance ();
      }
      break;
    case GL_LUMINANCE_ALPHA:
      image_data = new uint8 [n*2*d];
      for (j=0; j < d; j++)
      {
        data = (csRGBpixel *)(*ImageVector)[j]->GetImageData();
        for (i=0; i<n; i++, data++)
        {
          *image_data++ = data->Luminance ();
          *image_data++ = data->alpha;
        }
      }
      break;
    default: // RGB/RGBA branch
      switch (sourceType)
      {
        case GL_UNSIGNED_BYTE:
	  nCompo = csGLTextureManager::glformats[formatidx].components;
	  h = image_data = new uint8 [n*nCompo*d];
          for (j=0; j<d; j++)
          {
            data = (csRGBpixel *)(*ImageVector)[j]->GetImageData();
	    for (i=0; i<n; i++, data++, h+=nCompo)
	      memcpy (h, data, nCompo);
          }
	  break;
        case GL_UNSIGNED_BYTE_3_3_2:
	  h = image_data = new uint8 [n*d];
          for (j=0; j < d; j++)
          {
            data = (csRGBpixel *)(*ImageVector)[j]->GetImageData();
	    for (i=0; i<n; i++, data++)
	      *h++ = (data->red & 0xe0) | (data->green & 0xe0)>>5
               | (data->blue >> 6);
          }
	  break;
        case GL_UNSIGNED_SHORT_4_4_4_4:
	  {
	    image_data = new uint8 [n*2*d];
	    unsigned short *ush = (unsigned short *)image_data;
            for (j=0; j < d; j++)
            {
              data = (csRGBpixel *)(*ImageVector)[j]->GetImageData();
	      for (i=0; i<n; i++, data++)
	        *ush++ = ((unsigned short)(data->red & 0xf0))<<8
                | ((unsigned short)(data->green & 0xf0))<<4
		| (unsigned short)(data->blue & 0xf0)
		| (unsigned short)(data->alpha >> 4) ;
            }
	  }
	  break;
        case GL_UNSIGNED_SHORT_5_5_5_1:
	  {
	    image_data = new uint8 [n*2*d];
	    unsigned short *ush = (unsigned short *)image_data;
            for (j=0; j < d; j++)
            {
              data = (csRGBpixel *)(*ImageVector)[j]->GetImageData();
	      for (i=0; i<n; i++, data++)
	      *ush++ = ((unsigned short)(data->red & 0xf8))<<8
	    	  | ((unsigned short)(data->green & 0xf8))<<3
		  | ((unsigned short)(data->blue & 0xf8))>>2
		  | (unsigned short)(data->alpha >> 7) ;
            }
	  }
	  break;
        case GL_UNSIGNED_SHORT_5_6_5:
	  {
	    image_data = new uint8 [n*2];
	    unsigned short *ush = (unsigned short *)image_data;
            for (j=0; j < d; j++)
            {
              data = (csRGBpixel *)(*ImageVector)[j]->GetImageData();
	      for (i=0; i<n; i++, data++)
	        *ush++ = ((unsigned short)(data->red & 0xf8))<<8
		  | ((unsigned short)(data->green & 0xfc))<<3
		  | (unsigned short)(data->blue >> 3);
            }
	  }
	  break;
        default:
	  printf ("OpenGL Warning: no sourceType %x\n",
	  	(unsigned int)sourceType);
      }
  }

  tex->size = n * d * csGLTextureManager::glformats[formatidx].components;
  
  size += tex->size;
  return true;
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
  SCF_CONSTRUCT_IBASE (0);

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
  SCF_CONSTRUCT_IBASE (0);
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
  SCF_CONSTRUCT_IBASE (0);
  csGLTextureManager::object_reg = object_reg;
  verbose = false;

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

  pfmt = *iG2D->GetPixelFormat ();

  R3D = iR3D;
  max_tex_size = R3D->GetMaxTextureSize ();
  read_config (config);
  Clear ();

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
}

void csGLTextureManager::read_config (iConfigFile *config)
{
  sharpen_mipmaps = config->GetInt
    ("Video.OpenGL.SharpenMipmaps", 0);
  texture_downsample = config->GetInt
    ("Video.OpenGL.TextureDownsample", 0);
  texture_filter_anisotropy = config->GetFloat
    ("Video.OpenGL.TextureFilterAnisotropy", 1.0);	
  texture_bits = config->GetInt
    ("Video.OpenGL.TextureBits", 0);
  if (!texture_bits) texture_bits = pfmt.PixelBytes*8;

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
    if (!strcmp (newTarget, "compressed") && R3D->ext->CS_GL_ARB_texture_compression)
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
      case GL_RGB5_A1:
	if (R3D->ext->CS_GL_EXT_texture_compression_s3tc)
	{
	  compressedFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	}
	else
	{
	  compressedFormat = GL_COMPRESSED_RGBA_ARB;
	}
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
      "BAAAAAAAD!!! csGLTextureManager::RegisterTexture with 0 image!");
    return 0;
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
      "BAAAAAAAD!!! csGLTextureManager::RegisterTexture with 0 image array!");
    return 0;
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

  // @@@ Dunno if this should be _here_ really.
  csRef<iShaderManager> shadman = 
    CS_QUERY_REGISTRY (object_reg, iShaderManager);
  shadman->AddChild (material);
  
  return csPtr<iMaterialHandle> (mat);
}

csPtr<iMaterialHandle> csGLTextureManager::RegisterMaterial (
      iTextureHandle* txthandle)
{
  if (!txthandle) return 0;
  csGLMaterialHandle *mat = new csGLMaterialHandle (txthandle, this);
  materials.Push (mat);

  // @@@ Dunno if this should be _here_ really.
  csRef<iShaderManager> shadman = 
    SCF_QUERY_INTERFACE (object_reg, iShaderManager);
  shadman->AddChild (mat->GetMaterial ());

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

csPtr<iSuperLightmap> csGLTextureManager::CreateSuperLightmap(int, int)
{
  return 0;
}

void csGLTextureManager::GetMaxTextureSize (int& w, int& h, int& aspect)
{
  w = max_tex_size;
  h = max_tex_size;
  aspect = max_tex_size;
}

