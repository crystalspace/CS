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

#include "glextmanager.h"
//---------------------------------------------------------------------------

csGLTexture::csGLTexture (csTextureHandle *Parent, iImage *Image)
  : csTexture (Parent)
{
  DG_TYPE (this, "csGLTexture");
  image_data = NULL;
  w = Image->GetWidth ();
  h = Image->GetHeight ();
  compute_masks ();
  size = 0;
  compressed = GL_FALSE;
}

csGLTexture::~csGLTexture ()
{
  delete [] image_data;
}

//---------------------------------------------------------------------------

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
	if (!(image->GetFormat () & CS_IMGFMT_ALPHA))
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

	int pixels = image->GetWidth () * image->GetHeight ();
	csRGBpixel *_src = (csRGBpixel *)image->GetImageData ();

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
	h = rows = image->GetHeight ();
	w = image->GetWidth();
	_src = (csRGBpixel *)image->GetImageData ();
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

bool csGLTextureHandle::transform (iImage *Image, csGLTexture *tex)
{
  uint8 *h;
  uint8 *&image_data = tex->get_image_data ();
  csRGBpixel *data = (csRGBpixel *)Image->GetImageData ();
  int n = Image->GetWidth ()*Image->GetHeight ();;
  int i=0;
  int nCompo;

  // First we determine the exact sourceformat if targetformat is given
  // without bit specifications.
  switch (csGLTextureManager::glformats[formatidx].sourceFormat)
  {
    case GL_ALPHA: i++;  // Fall thru
    case GL_BLUE: i++;  // Fall thru
    case GL_GREEN: i++;  // Fall thru
    case GL_RED:
      image_data = new uint8 [n];
      h = (uint8*)data;
      h += i;
      for (i=0; i<n; i++, h += 4)
        *image_data++ = *h;
      break;
    case GL_INTENSITY:
      image_data = new uint8 [n];
      for (i=0; i<n; i++, data++)
        *image_data++ = data->Intensity ();
      break;
    case GL_LUMINANCE:
      image_data = new uint8 [n];
      for (i=0; i<n; i++, data++)
        *image_data++ = data->Luminance ();
      break;
    case GL_LUMINANCE_ALPHA:
      image_data = new uint8 [n*2];
      for (i=0; i<n; i++, data++)
      {
        *image_data++ = data->Luminance ();
        *image_data++ = data->alpha;
      }
      break;
    default: // RGB/RGBA branch
      switch (sourceType)
      {
        case GL_UNSIGNED_BYTE:
	  nCompo = csGLTextureManager::glformats[formatidx].components;
	  h = image_data = new uint8 [n*nCompo];
	  for (i=0; i<n; i++, data++, h+=nCompo)
	    memcpy (h, data, nCompo);
	  break;
        case GL_UNSIGNED_BYTE_3_3_2:
	  h = image_data = new uint8 [n];
	  for (i=0; i<n; i++, data++)
	    *h++ = (data->red & 0xe0) | (data->green & 0xe0)>>5
	      | (data->blue >> 6);
	  break;
        case GL_UNSIGNED_SHORT_4_4_4_4:
	  {
	    image_data = new uint8 [n*2];
	    unsigned short *ush = (unsigned short *)image_data;
	    for (i=0; i<n; i++, data++)
	      *ush++ = ((unsigned short)(data->red & 0xf0))<<8
		| ((unsigned short)(data->green & 0xf0))<<4
		| (unsigned short)(data->blue & 0xf0)
		| (unsigned short)(data->alpha >> 4) ;
	  }
	  break;
        case GL_UNSIGNED_SHORT_5_5_5_1:
	  {
	    image_data = new uint8 [n*2];
	    unsigned short *ush = (unsigned short *)image_data;
	    for (i=0; i<n; i++, data++)
	    *ush++ = ((unsigned short)(data->red & 0xf8))<<8
		| ((unsigned short)(data->green & 0xf8))<<3
		| ((unsigned short)(data->blue & 0xf8))>>2
		| (unsigned short)(data->alpha >> 7) ;
	  }
	  break;
        case GL_UNSIGNED_SHORT_5_6_5:
	  {
	    image_data = new uint8 [n*2];
	    unsigned short *ush = (unsigned short *)image_data;
	    for (i=0; i<n; i++, data++)
	      *ush++ = ((unsigned short)(data->red & 0xf8))<<8
		| ((unsigned short)(data->green & 0xfc))<<3
		| (unsigned short)(data->blue >> 3);
	  }
	  break;
        default:
	  printf ("OpenGL Warning: no sourceType %x\n",
	  	(unsigned int)sourceType);
      }
  }

  if (csGLTextureManager::glformats[formatidx].compressedFormat != 0
    && tex->Compressable ())
  {
    GLuint t;
    glGenTextures (1, &t);
    glBindTexture (GL_TEXTURE_2D, t);
    glTexImage2D (GL_TEXTURE_2D, 0,
      csGLTextureManager::glformats[formatidx].compressedFormat,
      Image->GetWidth (), Image->GetHeight (), 0,
      csGLTextureManager::glformats[formatidx].sourceFormat,
      sourceType, image_data);
    glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB,
      &tex->compressed);

    /* if the compression has been successful */
    if (tex->compressed == GL_TRUE)
    {
      glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT,
        &tex->internalFormat);
      glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
        GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &tex->size);
      delete [] image_data;
      image_data = new uint8 [tex->size];
      R3D->ext.glGetCompressedTexImageARB (GL_TEXTURE_2D, 0,
        image_data);
    }
    else
    {
      tex->size = n * csGLTextureManager::glformats[formatidx].texelbytes;
    }

    glDeleteTextures (1, &t);
  }
  else
  {
    tex->size = n * csGLTextureManager::glformats[formatidx].texelbytes;
  }

  size += tex->size;
  return true;
}

csGLTextureHandle::csGLTextureHandle (
  iImage *image, int flags, GLenum sourceFormat, int bpp,
  csGLRender3D *R3D) : csTextureHandle (image, flags)
{
  (csGLTextureHandle::R3D = R3D)->IncRef ();
  (txtmgr = R3D->txtmgr)->IncRef ();
  has_alpha = false;
  this->sourceFormat = sourceFormat;
  this->bpp = bpp;
  size = 0;
  DG_TYPE (this, "csGLTextureHandle");
}

csGLTextureHandle::~csGLTextureHandle ()
{
  if (R3D->txtcache)
    R3D->txtcache->Uncache (this);
  txtmgr->UnregisterTexture (this);
  txtmgr->DecRef ();
  R3D->DecRef ();
  int i;
  for (i = vTex.Length ()-1; i >= 0; i--)
  {
    if (vTex [i])
    {
      DG_UNLINK (this, vTex[i]);
      delete vTex [i];
    }
  }
  vTex.DeleteAll ();
}

void csGLTextureHandle::Clear ()
{
}

csTexture *csGLTextureHandle::NewTexture (iImage *Image, bool ismipmap)
{
  ismipmap = false;
  return new csGLTexture (this, Image);
}

void csGLTextureHandle::ShowFormat ()
{
  printf ("TargetFormat: %s, size: %ld\n",
    csGLTextureManager::glformats[formatidx].name, size);
}

void csGLTextureHandle::InitTexture (csGLTextureManager *texman,
           csPixelFormat *pfmt)
{
  // Preserve original width/height so that in DrawPixmap subregions of
  // textures are calculated correctly. In other words, the app writer need
  // not know about opengl texture size adjustments. smgh
  if (!image) return;

  orig_width = image->GetWidth ();
  orig_height = image->GetHeight ();

  // If necessary rescale if bigger than maximum texture size
  if ((orig_width > texman->max_tex_size) ||
      (orig_height > texman->max_tex_size))
  {
    int nwidth = orig_width;
    int nheight = orig_height;
    if (orig_width > texman->max_tex_size) nwidth = texman->max_tex_size;
    if (orig_height > texman->max_tex_size) nheight = texman->max_tex_size;
    image->Rescale (nwidth, nheight);
  }

  // In opengl all textures, even non-mipmapped textures are required
  // to be powers of 2.
  AdjustSizePo2 ();

  // Determine the format and type of the source we gonna tranform the data to.
  FindFormatType ();
  CreateMipmaps ();
}

void csGLTextureHandle::CreateMipmaps ()
{
  if (!image) return;

  csRGBpixel *tc = transp ? &transp_color : (csRGBpixel *)NULL;

  //  printf ("delete old\n");
  // Delete existing mipmaps, if any
  int i;
  for (i = vTex.Length ()-1; i >= 0; i--)
  {
    if (vTex [i])
    {
      DG_UNLINK (this, vTex[i]);
      delete vTex [i];
    }
  }
  vTex.DeleteAll ();

  size = 0;
  //  printf ("push 0\n");
  csTexture* ntex = NewTexture (image, false);
  vTex.Push (ntex);
  DG_LINK (this, ntex);

  //  printf ("transform 0\n");
  transform (image, vTex[0]);

  // 2D textures uses just the top-level mipmap
  if ((flags & (CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS)) == CS_TEXTURE_3D)
  {
    // Create each new level by creating a level 2 mipmap from previous level
    // we do this down to 1x1 as opengl defines it

    iImage *prevImage = image;
    csRef<iImage> thisImage;

    int w = prevImage->GetWidth ();
    int h = prevImage->GetHeight ();
    int nTex = 0;

    ComputeMeanColor (vTex[nTex]->get_width (), vTex[nTex]->get_height (),
          (csRGBpixel *)prevImage->GetImageData ());

    prevImage->IncRef ();
    while (w != 1 || h != 1)
    {
      nTex++;
      //  printf ("make mipmap %d\n", nTex);
      thisImage = prevImage->MipMap (1, tc);
      if (txtmgr->sharpen_mipmaps)
      {
	thisImage = thisImage->Sharpen (tc, txtmgr->sharpen_mipmaps);
      }
      //  printf ("push %d\n", nTex);
      csTexture* ntex = NewTexture (thisImage, true);
      vTex.Push (ntex);
      DG_LINK (this, ntex);
      //  printf ("transform %d\n", nTex);
      transform (thisImage, vTex[nTex]);
      w = thisImage->GetWidth ();
      h = thisImage->GetHeight ();
      prevImage->DecRef ();
      thisImage->IncRef ();
      prevImage = thisImage;
    }

    prevImage->DecRef ();
  }
  else
    ComputeMeanColor (vTex[0]->get_width (), vTex[0]->get_height (),
      (csRGBpixel *)image->GetImageData ());
}

void csGLTextureHandle::ComputeMeanColor (int w, int h, csRGBpixel *src)
{
  int pixels = w * h;
  unsigned r = 0, g = 0, b = 0;
  int count = pixels;
  CS_ASSERT (pixels > 0);
  has_alpha = false;
  while (count--)
  {
    csRGBpixel &pix = *src++;
    r += pix.red;
    g += pix.green;
    b += pix.blue;
    if (pix.alpha < 255)
      has_alpha = true;
  }
  mean_color.red   = r / pixels;
  mean_color.green = g / pixels;
  mean_color.blue  = b / pixels;
}

bool csGLTextureHandle::GetMipMapDimensions (int mipmap, int &w, int &h)
{
  if (mipmap < vTex.Length ())
  {
    w = vTex[mipmap]->get_width ();
    h = vTex[mipmap]->get_height ();
    return true;
  }
  return false;
}

void csGLTextureHandle::Prepare ()
{
  InitTexture (txtmgr, &txtmgr->pfmt);
}

class csOFSCbOpenGL : public iOffscreenCanvasCallback
{
private:
  csGLTextureHandle* txt;

public:
  csOFSCbOpenGL (csGLTextureHandle* txt)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    csOFSCbOpenGL::txt = txt;
  }
  virtual ~csOFSCbOpenGL ()
  {
  }
  SCF_DECLARE_IBASE;
  virtual void FinishDraw (iGraphics2D*)
  {
    txt->UpdateTexture ();
  }
  virtual void SetRGB (iGraphics2D*, int, int, int, int)
  {
  }
};


SCF_IMPLEMENT_IBASE(csOFSCbOpenGL)
  SCF_IMPLEMENTS_INTERFACE(iOffscreenCanvasCallback)
SCF_IMPLEMENT_IBASE_END

iGraphics2D* csGLTextureHandle::GetCanvas ()
{
  if (!canvas)
  {
    csOFSCbOpenGL* ofscb = new csOFSCbOpenGL (this);
    csGLTexture *t = vTex[0];
    canvas = txtmgr->R3D->GetDriver2D ()->CreateOffscreenCanvas (
  	t->get_image_data (), t->get_width (), t->get_height (), 32,
	ofscb);
    ofscb->DecRef ();
  }
  return canvas;
}

void csGLTextureHandle::UpdateTexture ()
{
  if (R3D->txtcache)
    R3D->txtcache->Uncache (this);
}

//---------------------------------------------------------------------------

csGLTextureManager::csGLTextureManager (iObjectRegistry* object_reg,
  iGraphics2D* iG2D, iConfigFile *config, csGLRender3D *iR3D)
  : csTextureManager (object_reg, iG2D)
{
  R3D = iR3D;
  R3D->IncRef ();
  max_tex_size = R3D->GetMaxTextureSize ();
  read_config (config);
  Clear ();
}

csGLTextureManager::~csGLTextureManager ()
{
  csTextureManager::Clear ();
  R3D->DecRef ();
}

void csGLTextureManager::read_config (iConfigFile *config)
{
  sharpen_mipmaps = config->GetInt
        ("Video.OpenGL.SharpenMipmaps", 0);

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

void csGLTextureManager::SetPixelFormat (csPixelFormat &PixelFormat)
{
  pfmt = PixelFormat;
  max_tex_size = R3D->GetMaxTextureSize ();
  DetermineStorageSizes ();
}

void csGLTextureManager::PrepareTextures ()
{
  // Create mipmaps for all textures
  int i;
  for (i = 0; i < textures.Length (); i++)
    textures.Get (i)->Prepare ();
}

csPtr<iTextureHandle> csGLTextureManager::RegisterTexture (
	iImage* image, int flags)
{
  if (!image)
  {
    R3D->Report(CS_REPORTER_SEVERITY_BUG,
      "BAAAD!!! csGLTextureManager::RegisterTexture with NULL image!");

    csRef<iImage> im (csCreateXORPatternImage (32, 32, 5));
    image = im;
    im->IncRef ();	// Prevent smart pointer release.
  }

  csGLTextureHandle* txt = new csGLTextureHandle (
  	image, flags, GL_RGBA, pfmt.PixelBytes*8, R3D);
  textures.Push (txt);
  return csPtr<iTextureHandle> (txt);
}

void csGLTextureManager::UnregisterTexture (csGLTextureHandle *handle)
{
  int idx = textures.Find (handle);
  if (idx >= 0) textures.Delete (idx);
}

void csGLTextureManager::Clear ()
{
  for (int i=0; i < textures.Length (); i++)
    ((csGLTextureHandle *)textures.Get (i))->Clear ();

  csTextureManager::Clear ();
}

void csGLTextureManager::FreeImages ()
{
  int i;
  for (i = 0 ; i < textures.Length () ; i++)
  {
    textures.Get (i)->FreeImage ();
  }
}

