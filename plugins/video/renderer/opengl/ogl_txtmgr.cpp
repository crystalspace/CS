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
#include "cssys/sysdriv.h"
#include "ogl_g3dcom.h"
#include "ogl_txtmgr.h"
#include "ogl_txtcache.h"
#include "ogl_dyntexback.h"
#include "ogl_dyntexsoft.h"
#include "csutil/scanstr.h"
#include "csutil/inifile.h"
#include "isystem.h"
#include "iimage.h"
#include "lightdef.h"

#define SysPrintf System->Printf

//---------------------------------------------------------------------------

csTextureOpenGL::csTextureOpenGL (csTextureMM *Parent, iImage *Image)
  : csTexture (Parent)
{
  image = Image;
  w = Image->GetWidth ();
  h = Image->GetHeight ();
  compute_masks ();
}

csTextureOpenGL::~csTextureOpenGL ()
{
  if (image) image->DecRef ();
}

//---------------------------------------------------------------------------

void csTextureOpenGLDynamic::CreateInterfaces (csTextureMMOpenGL *mm_tex, 
   csGLDynTexType type, csGraphics3DOGLCommon *parentG3D, csPixelFormat *pfmt)
{
  switch (type)
  {
    case BACK_BUFFER_TEXTURE:
    {
      csOpenGLDynamicBackBuffer *bbtexG3D = new csOpenGLDynamicBackBuffer(NULL);
      bbtexG3D->SetTarget (parentG3D, mm_tex);
      texG3D = (iGraphics3D*) bbtexG3D;
      break;
    }
    case SOFTWARE_TEXTURE:
    {
      csOpenGLDynamicSoftware *stexG3D = new csOpenGLDynamicSoftware (NULL);
      stexG3D->SetTarget (parentG3D, mm_tex);

      if (stexG3D->CreateOffScreenRenderer (NULL, w, h, pfmt, 
				       image->GetImageData (), NULL, 0))
	texG3D = (iGraphics3D*) stexG3D;
      break;
    }
    case AUXILIARY_BUFFER_TEXTURE:
    default:
    break;
  }
}

csTextureOpenGLDynamic::~csTextureOpenGLDynamic ()
{ 
  if (texG3D)
	texG3D->DecRef (); 
}

//---------------------------------------------------------------------------

csTextureMMOpenGL::csTextureMMOpenGL (iImage *image, int flags,
  csGraphics3DOGLCommon *iG3D) : csTextureMM (image, flags)
{
  G3D = iG3D;
}

csTextureMMOpenGL::~csTextureMMOpenGL ()
{
  if (G3D && G3D->texture_cache)
    G3D->texture_cache->Uncache (this);
}

csTexture *csTextureMMOpenGL::NewTexture (iImage *Image)
{
  if ((flags & CS_TEXTURE_DYNAMIC) == CS_TEXTURE_DYNAMIC)
    return new csTextureOpenGLDynamic (this, Image);
  else
    return new csTextureOpenGL (this, Image);
}

void csTextureMMOpenGL::InitTexture (int max_tex_size, csPixelFormat *pfmt,
				     csGLDynTexType dyn_tex_type)
{
  // Preserve original width/height so that in DrawPixmap subregions of
  // textures are calculated correctly. In other words, the app writer need
  // not know about opengl texture size adjustments. smgh
  if (!image) return;
  orig_width = image->GetWidth ();
  orig_height = image->GetHeight ();

  if ((orig_width > max_tex_size) ||
      (orig_height > max_tex_size))
  {
    int nwidth = orig_width;
    int nheight = orig_height; 
    if (orig_width > max_tex_size) nwidth = max_tex_size;
    if (orig_height > max_tex_size) nheight = max_tex_size;
    image->Rescale (nwidth, nheight);
  }

  // In opengl all textures, even non-mipmapped textures are required 
    // to be powers of 2.
  AdjustSizePo2 ();

  CreateMipmaps ();
  if ((GetFlags() & CS_TEXTURE_DYNAMIC) == CS_TEXTURE_DYNAMIC)
    CreateDynamicTexture (G3D, dyn_tex_type, pfmt);
}


void csTextureMMOpenGL::ComputeMeanColor ()
{
  // Compute the mean color from the smallest mipmap available
  csTextureOpenGL *tex = NULL;
  for (int i = 3; i >= 0; i--)
  {
    tex = (csTextureOpenGL *)get_texture (i);
    if (tex) break;
  }
  if (!tex) return;

  int pixels = tex->get_width () * tex->get_height ();
  RGBPixel *src = tex->get_image_data ();
  unsigned r = 0, g = 0, b = 0;
  int count = pixels;
  while (count--)
  {
    RGBPixel pix = *src++;
    r += pix.red;
    g += pix.green;
    b += pix.blue;
  }
  mean_color.red   = r / pixels;
  mean_color.green = g / pixels;
  mean_color.blue  = b / pixels;
}

iGraphics3D *csTextureMMOpenGL::GetDynamicTextureInterface ()
{
  if ((flags & CS_TEXTURE_DYNAMIC) != CS_TEXTURE_DYNAMIC)
    return NULL;

  return ((csTextureOpenGLDynamic*)tex[0])->texG3D;
}

void csTextureMMOpenGL::CreateDynamicTexture (csGraphics3DOGLCommon *parentG3D, 
   csGLDynTexType type, csPixelFormat *pfmt)
{
  ((csTextureOpenGLDynamic*)tex[0])->CreateInterfaces (this, type,
						       parentG3D, pfmt);
}

//---------------------------------------------------------------------------

csTextureManagerOpenGL::csTextureManagerOpenGL (iSystem* iSys,
  iGraphics2D* iG2D, csIniFile *config, csGraphics3DOGLCommon *iG3D)
  : csTextureManager (iSys, iG2D)
{
  G3D = iG3D;
  read_config (config);
  Clear ();
}

csTextureManagerOpenGL::~csTextureManagerOpenGL ()
{
  Clear ();
}

void csTextureManagerOpenGL::read_config (csIniFile *config)
{
  const char *dynamic_texture_type = 
    config->GetStr ("OpenGL", "DYNAMIC_TEXTURE");

  if (!strcmp (dynamic_texture_type, "software"))
    dyn_tex_type = SOFTWARE_TEXTURE;
  else if (!strcmp (dynamic_texture_type, "back_buffer"))
    dyn_tex_type = BACK_BUFFER_TEXTURE;
  else if (!strcmp (dynamic_texture_type, "auxiliary_buffer"))
    dyn_tex_type = AUXILIARY_BUFFER_TEXTURE;
  else // default
    dyn_tex_type = BACK_BUFFER_TEXTURE;
}

void csTextureManagerOpenGL::PrepareTextures ()
{
  // hack, pixel format is now not yet determined until G2D is opened
  pfmt = *G3D->GetDriver2D()->GetPixelFormat ();
  max_tex_size = G3D->max_texture_size;

  if (verbose) SysPrintf(MSG_INITIALIZATION, "Preparing textures...\n");
  if (verbose) SysPrintf(MSG_INITIALIZATION, 
			 "Maximum texture size: %dx%d\n", 
			 max_tex_size, max_tex_size);
  if (verbose) SysPrintf(MSG_INITIALIZATION, "  Creating texture mipmaps...\n");

  // Create mipmaps for all textures
  for (int i = 0; i < textures.Length (); i++)
    ((csTextureMMOpenGL *)textures.Get (i))->InitTexture
                                         (max_tex_size, &pfmt, dyn_tex_type);
}

iTextureHandle *csTextureManagerOpenGL::RegisterTexture (iImage* image,
  int flags)
{
  if (!image) return NULL;

  csTextureMMOpenGL* txt = new csTextureMMOpenGL (image, flags, G3D);
  textures.Push (txt);
  return txt;
}

void csTextureManagerOpenGL::PrepareTexture (iTextureHandle *handle)
{
  // hack, pixel format is now not yet determined until G2D is opened
  pfmt = *G3D->GetDriver2D()->GetPixelFormat ();
  max_tex_size = G3D->max_texture_size;

  if (!handle) return;

  ((csTextureMMOpenGL*)handle->GetPrivateObject ())->InitTexture
                                            (max_tex_size, &pfmt, dyn_tex_type);
}

void csTextureManagerOpenGL::UnregisterTexture (iTextureHandle* handle)
{
  csTextureMMOpenGL *tex_mm = (csTextureMMOpenGL *)handle->GetPrivateObject ();
  G3D->UncacheTexture (handle);
  int idx = textures.Find (tex_mm);
  if (idx >= 0) textures.Delete (idx);
}
