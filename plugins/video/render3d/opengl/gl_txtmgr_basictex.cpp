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

#include "csplugincommon/render3d/txtmgr.h"

#include "gl_render3d.h"
#include "gl_txtmgr_basictex.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

CS_LEAKGUARD_IMPLEMENT(csGLBasicTextureHandle);

csGLBasicTextureHandle::csGLBasicTextureHandle (int width,
                                                int height,
                                                int depth,
                                                csImageType imagetype, 
                                                int flags, 
                                                csGLGraphics3D *iG3D) : 
  scfImplementationType (this), txtmgr (iG3D->txtmgr), 
  textureClass (txtmgr->GetTextureClassID ("default")), Handle (0), 
  orig_width (width), orig_height (height), orig_d (depth),
  uploadData(0), G3D (iG3D), texFormat((TextureBlitDataFormat)-1)
{
  switch (imagetype)
  {
    case csimgCube:
      target = CS_TEX_IMG_CUBEMAP;
      break;
    case csimg3D:
      target = CS_TEX_IMG_3D;
      break;
    default:
      target = CS_TEX_IMG_2D;
      break;
  }
  const uint npotsNeededFlags = (CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP);
  if (flags & CS_TEXTURE_NPOTS)
  {
    // For NPOTS we need...
    bool npotsValid = (
      // The extension
      (G3D->ext->CS_GL_ARB_texture_rectangle
      || G3D->ext->CS_GL_EXT_texture_rectangle
      || G3D->ext->CS_GL_NV_texture_rectangle
      || txtmgr->enableNonPowerOfTwo2DTextures
      || G3D->ext->CS_GL_ARB_texture_non_power_of_two)
      // Certain additional texture flags, unless we have ARB_tnpot
      && (((flags & npotsNeededFlags) == npotsNeededFlags) 
        || G3D->ext->CS_GL_ARB_texture_non_power_of_two))
      // A 2D image, unless we have ARB_tnpot
      && ((imagetype == csimg2D)
        || G3D->ext->CS_GL_ARB_texture_non_power_of_two);
    if (!npotsValid)
    {
      flags &= ~CS_TEXTURE_NPOTS;
    }
    else if (!txtmgr->enableNonPowerOfTwo2DTextures
      && !G3D->ext->CS_GL_ARB_texture_non_power_of_two)
      /* Note that 'enableNonPowerOfTwo2DTextures' is the flag for ATI's
       * support of non-POT _2D_ textures; that is, the textures, being
       * NPOTS, need to go to the 2D target, not RECT. 
       * Same when ARB_tnpot is available. */
      target = CS_TEX_IMG_RECT;
  }
  texFlags.Set (flagsPublicMask, flags);

  // In opengl all textures, even non-mipmapped textures are required
  // to be powers of 2.
  AdjustSizePo2 ();
}

csGLBasicTextureHandle::csGLBasicTextureHandle (csGLGraphics3D *iG3D,
                                                int target, GLuint Handle) : 
  scfImplementationType (this), txtmgr (iG3D->txtmgr), 
  textureClass (txtmgr->GetTextureClassID ("default")), Handle (Handle), 
  orig_width (0), orig_height (0), orig_d (0),
  uploadData(0), G3D (iG3D), texFormat((TextureBlitDataFormat)-1), 
  target (target), alphaType (csAlphaMode::alphaNone)
{
  SetForeignHandle (true);
}

csGLBasicTextureHandle::~csGLBasicTextureHandle()
{
  Clear ();
  txtmgr->UnregisterTexture (this);
}

bool csGLBasicTextureHandle::SynthesizeUploadData (
  const CS::StructuredTextureFormat& format,
  iString* fail_reason)
{
  TextureStorageFormat glFormat;
  if (!txtmgr->DetermineGLFormat (format, glFormat)) 
  {
    if (fail_reason) fail_reason->Replace ("no GL support for texture format");
    return 0;
  }

  if (glFormat.isCompressed)
  {
    /* To support creation of compressed textures:
       - compute size of compressed data
       - create texture from dummy data

       But we don't support them yet.
     */
    if (fail_reason) fail_reason->Replace ("compressed formats not supported");
    return false;
  }

  const int imgNum = (target == GL_TEXTURE_CUBE_MAP) ? 6 : 1;

  FreshUploadData ();
  if (texFlags.Check (CS_TEXTURE_NOMIPMAPS))
  {
    for (int i = 0; i < imgNum; i++)
    {
      csGLUploadData upload;
      upload.image_data = 0;
      upload.w = actual_width;
      upload.h = actual_height;
      upload.d = actual_d;
      upload.sourceFormat = glFormat;
      upload.mip = 0;
      upload.imageNum = i;

      uploadData->Push (upload);
    }
  }
  else
  {
    for (int i = 0; i < imgNum; i++)
    {
      // Create each new level by creating a level 2 mipmap from previous level
      // we do this down to 1x1 as opengl defines it
      int w  = actual_width, h = actual_height, d = actual_d;
      int nMip = 0;

      do
      {
        csGLUploadData upload;
        upload.image_data = 0;
        upload.w = w;
        upload.h = h;
        upload.d = d;
        upload.sourceFormat = glFormat;
        upload.mip = nMip;
        upload.imageNum = i;

        uploadData->Push (upload);

	if ((w == 1) && (h == 1) && (d == 1)) break;

	nMip++;
        w = csMax (w >> 1, 1);
        h = csMax (h >> 1, 1);
        d = csMax (d >> 1, 1);
      }
      while (true);
    }
  }
  return true;
}

void csGLBasicTextureHandle::Clear()
{
  if (uploadData != 0)
  {
    delete uploadData;
    uploadData = 0;
  }
  Unload ();
}

int csGLBasicTextureHandle::GetFlags () const
{
  return texFlags.Get() & flagsPublicMask;
}

bool csGLBasicTextureHandle::GetRendererDimensions (int &mw, int &mh)
{
  mw = actual_width; mh = actual_height;
  return true;
}

// Check the two below for correctness
bool csGLBasicTextureHandle::GetRendererDimensions (int &mw, int &mh, int &md)
{
  mw = actual_width;
  mh = actual_height;
  md = actual_d;
  return true;
}

void *csGLBasicTextureHandle::GetPrivateObject ()
{
  return (csGLBasicTextureHandle *)this;
}

bool csGLBasicTextureHandle::GetAlphaMap () 
{
  return (alphaType != csAlphaMode::alphaNone);
}

void csGLBasicTextureHandle::PrepareInt ()
{
}

GLenum csGLBasicTextureHandle::DetermineTargetFormat (GLenum defFormat, 
						 bool allowCompress,
						 const char* rawFormat, 
						 bool& compressedFormat)
{
  GLenum targetFormat = defFormat;
  compressedFormat = false;

  if (rawFormat)
  {
    if (G3D->ext->CS_GL_EXT_texture_compression_s3tc 
      && allowCompress)
    {
      if (strcmp (rawFormat, "*dxt1") == 0)
      {
	targetFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	compressedFormat = true;
      }
      else if (strcmp (rawFormat, "*dxt1a") == 0)
      {
	targetFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	compressedFormat = true;
      }
      else if (strcmp (rawFormat, "*dxt3") == 0)
      {
	targetFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	compressedFormat = true;
      }
      else if (strcmp (rawFormat, "*dxt5") == 0)
      {
	targetFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	compressedFormat = true;
      }
    }
  }
  return targetFormat;
}

void csGLBasicTextureHandle::ComputeNewPo2ImageSize (int texFlags,
  int orig_width, int orig_height, int orig_depth,
  int& newwidth, int& newheight, int& newdepth,
  int max_tex_size)
{
  csTextureHandle::CalculateNextBestPo2Size (texFlags, orig_width, newwidth);
  csTextureHandle::CalculateNextBestPo2Size (texFlags, orig_height, newheight);
  csTextureHandle::CalculateNextBestPo2Size (texFlags, orig_depth, newdepth);

  // If necessary rescale if bigger than maximum texture size,
  // but only if a dimension has changed. For textures that are
  // already PO2, a lower mipmap will be selected in CreateMipMaps()
  if ((newwidth != orig_width) && (newwidth > max_tex_size)) 
    newwidth = max_tex_size;
  if ((newheight != orig_width) && (newheight > max_tex_size)) 
    newheight = max_tex_size;
  if ((newdepth != orig_depth) && (newdepth > max_tex_size)) 
    newdepth = max_tex_size;
}

void csGLBasicTextureHandle::FreshUploadData ()
{
  if (uploadData != 0)
    uploadData->DeleteAll();
  else
    uploadData = new csArray<csGLUploadData>;
}

void csGLBasicTextureHandle::AdjustSizePo2 ()
{
  if (texFlags.Check (CS_TEXTURE_NPOTS)) 
  {
    actual_width = MIN(orig_width, G3D->maxNpotsTexSize);
    actual_height = MIN(orig_height, G3D->maxNpotsTexSize);
    actual_d = MIN(orig_d, G3D->maxNpotsTexSize);
    return;
  }

  int newwidth, newheight, newd;

  ComputeNewPo2ImageSize (texFlags.Get(), orig_width, orig_height, orig_d, 
    newwidth, newheight, newd, txtmgr->max_tex_size);

  actual_width = newwidth;
  actual_height = newheight;
  actual_d = newd;
}

void csGLBasicTextureHandle::Blit (int x, int y, int width,
    int height, unsigned char const* data, TextureBlitDataFormat format)
{
  // @@@ Keycolor not yet supported here!
  
  GLenum textarget = GetGLTextureTarget();
  if ((textarget != GL_TEXTURE_2D) && (textarget != GL_TEXTURE_RECTANGLE_ARB))
    return;

  // Activate the texture.
  Precache ();
  G3D->ActivateTexture (this);
  GLuint textureFormat = (format == RGBA8888) ? GL_RGBA : GL_BGRA;
  // Make sure mipmapping is ok.
  if (!IsWasRenderTarget() || (texFormat != format))
  {
    texFormat = format;
    bool isWholeImage = (x == 0) && (y == 0) && (width == actual_width)
      && (height == actual_height);

    // Pull texture data and set as RGBA/BGRA again, to prevent compression 
    // (slooow) on subsequent glTexSubImage() calls.
    if (!isWholeImage)
    {
      uint8* pixels = new uint8[actual_width * actual_height * 4];
      glGetTexImage (textarget, 0, textureFormat, GL_UNSIGNED_BYTE, 
	pixels);

      if (!IsWasRenderTarget())
      {
	SetWasRenderTarget (true);
	SetupAutoMipping();
      }

      glTexImage2D (textarget, 0, GL_RGBA8, actual_width, 
	actual_height, 0, textureFormat, GL_UNSIGNED_BYTE, pixels);
      delete[] pixels;
    }
    else
    {
      if (!IsWasRenderTarget())
      {
	SetWasRenderTarget (true);
	SetupAutoMipping();
      }

      glTexImage2D (textarget, 0, GL_RGBA8, actual_width, 
	actual_height, 0, textureFormat, GL_UNSIGNED_BYTE, data);
      return;
    }
  }
  // Do the copy.
  glTexSubImage2D (textarget, 0, x, y, 
      width, height,
      textureFormat, GL_UNSIGNED_BYTE, data);
  //SetNeedMips (true);
}

void csGLBasicTextureHandle::SetupAutoMipping()
{
  // Set up mipmap generation
  if ((!(texFlags.Get() & CS_TEXTURE_NOMIPMAPS))
    /*&& (!G3D->ext->CS_GL_EXT_framebuffer_object)*/)
  {
    if (G3D->ext->CS_GL_SGIS_generate_mipmap)
      glTexParameteri (GetGLTextureTarget(), GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    else
      glTexParameteri  (GetGLTextureTarget(), GL_TEXTURE_MIN_FILTER,
	txtmgr->rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);
  }
}

void csGLBasicTextureHandle::Load ()
{
  if (Handle != 0) return;

  static const GLint textureMinFilters[3] = {GL_NEAREST_MIPMAP_NEAREST, 
    GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR};
  static const GLint textureMagFilters[3] = {GL_NEAREST, GL_LINEAR, 
    GL_LINEAR};

  glGenTextures (1, &Handle);

  const int texFilter = texFlags.Check (CS_TEXTURE_NOFILTER) ? 0 : 
    txtmgr->rstate_bilinearmap;
  const GLint magFilter = textureMagFilters[texFilter];
  const GLint minFilter = textureMinFilters[texFilter];
  const GLint wrapMode = 
    (texFlags.Check (CS_TEXTURE_CLAMP)) ? GL_CLAMP_TO_EDGE : GL_REPEAT;

  if (target == CS_TEX_IMG_1D)
  {
    G3D->statecache->SetTexture (GL_TEXTURE_1D, Handle);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, minFilter);

    if (G3D->ext->CS_GL_EXT_texture_filter_anisotropic)
    {
      glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
        txtmgr->texture_filter_anisotropy);
    }

    // @@@ Implement upload!
  }
  else if (target == CS_TEX_IMG_2D)
  {
    G3D->statecache->SetTexture (GL_TEXTURE_2D, Handle);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
      (texFlags.Check (CS_TEXTURE_NOMIPMAPS)) ? magFilter : minFilter);

    if (G3D->ext->CS_GL_EXT_texture_filter_anisotropic)
    {
      glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
        txtmgr->texture_filter_anisotropy);
    }

    size_t i;
    for (i = 0; i < uploadData->GetSize(); i++)
    {
      const csGLUploadData& uploadData = this->uploadData->Get (i);
      if (uploadData.sourceFormat.isCompressed)
      {
	G3D->ext->glCompressedTexImage2DARB (GL_TEXTURE_2D, uploadData.mip, 
	  uploadData.sourceFormat.targetFormat, uploadData.w, uploadData.h, 
	  0, (GLsizei)uploadData.compressedSize, uploadData.image_data);
      }
      else
      {
	glTexImage2D (GL_TEXTURE_2D, uploadData.mip, 
	  uploadData.sourceFormat.targetFormat, 
	  uploadData.w, uploadData.h, 0, uploadData.sourceFormat.format, 
	  uploadData.sourceFormat.type, uploadData.image_data);
      }
    }
  }
  else if (target == CS_TEX_IMG_3D)
  {
    G3D->statecache->Enable_GL_TEXTURE_3D ();
    G3D->statecache->SetTexture (GL_TEXTURE_3D, Handle);
    glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapMode);

    // @@@ Not sure if the following makes sense with 3D textures.
    glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER,
      (texFlags.Check (CS_TEXTURE_NOMIPMAPS)) ? magFilter : minFilter);

    if (G3D->ext->CS_GL_EXT_texture_filter_anisotropic)
    {
      glTexParameterf (GL_TEXTURE_3D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
        txtmgr->texture_filter_anisotropy);
    }

    size_t i;
    for (i = 0; i < uploadData->GetSize (); i++)
    {
      const csGLUploadData& uploadData = this->uploadData->Get (i);
      if (uploadData.sourceFormat.isCompressed)
      {
	G3D->ext->glCompressedTexImage3DARB (GL_TEXTURE_3D, uploadData.mip, 
	  uploadData.sourceFormat.targetFormat, uploadData.w, uploadData.h, 
	  uploadData.d, 0, (GLsizei)uploadData.compressedSize, 
	  uploadData.image_data);
      }
      else
      {
	G3D->ext->glTexImage3DEXT (GL_TEXTURE_3D, uploadData.mip, 
	  uploadData.sourceFormat.targetFormat, uploadData.w, uploadData.h, 
          uploadData.d, 0, uploadData.sourceFormat.format, 
          uploadData.sourceFormat.type, uploadData.image_data);
      }
    }
  }
  else if (target == CS_TEX_IMG_CUBEMAP)
  {
    G3D->statecache->SetTexture (GL_TEXTURE_CUBE_MAP, Handle);
    // @@@ Temporarily force clamp, although I don't know if REPEAT
    // makes sense with cubemaps.
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, 
      GL_CLAMP_TO_EDGE/*wrapMode*/);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, 
      GL_CLAMP_TO_EDGE/*wrapMode*/);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, 
      GL_CLAMP_TO_EDGE/*wrapMode*/);

    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilter);

    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
      (texFlags.Check (CS_TEXTURE_NOMIPMAPS)) ? magFilter : minFilter);

    if (G3D->ext->CS_GL_EXT_texture_filter_anisotropic)
    {
      glTexParameterf (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT,
        txtmgr->texture_filter_anisotropy);
    }

    size_t i;
    for (i = 0; i < uploadData->GetSize (); i++)
    {
      const csGLUploadData& uploadData = this->uploadData->Get (i);

      if (uploadData.sourceFormat.isCompressed)
      {
	G3D->ext->glCompressedTexImage2DARB (
	  GL_TEXTURE_CUBE_MAP_POSITIVE_X + uploadData.imageNum, 
	  uploadData.mip, 
	  uploadData.sourceFormat.targetFormat, uploadData.w, uploadData.h, 
	  0, (GLsizei)uploadData.compressedSize, uploadData.image_data);
      }
      else
      {
	glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X + uploadData.imageNum, 
	  uploadData.mip, uploadData.sourceFormat.targetFormat, 
	  uploadData.w, uploadData.h,
	  0, uploadData.sourceFormat.format, uploadData.sourceFormat.type,
	  uploadData.image_data);
      }
    }
  }
  else if (target == CS_TEX_IMG_RECT)
  {
    G3D->statecache->SetTexture (GL_TEXTURE_RECTANGLE_ARB, Handle);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, wrapMode);

    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,
      (texFlags.Check (CS_TEXTURE_NOMIPMAPS)) ? magFilter : minFilter);

    if (G3D->ext->CS_GL_EXT_texture_filter_anisotropic)
    {
      glTexParameterf (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAX_ANISOTROPY_EXT,
        txtmgr->texture_filter_anisotropy);
    }

    size_t i;
    for (i = 0; i < uploadData->GetSize (); i++)
    {
      const csGLUploadData& uploadData = this->uploadData->Get (i);
      if (uploadData.sourceFormat.isCompressed)
      {
	G3D->ext->glCompressedTexImage2DARB (GL_TEXTURE_RECTANGLE_ARB, 
          uploadData.mip, uploadData.sourceFormat.targetFormat, 
          uploadData.w, uploadData.h, 0, 
          (GLsizei)uploadData.compressedSize, uploadData.image_data);
      }
      else
      {
	glTexImage2D (GL_TEXTURE_RECTANGLE_ARB, uploadData.mip, 
	  uploadData.sourceFormat.targetFormat, 
	  uploadData.w, uploadData.h, 0, uploadData.sourceFormat.format, 
	  uploadData.sourceFormat.type, uploadData.image_data);
      }
    }
  }

  delete uploadData; uploadData = 0;
}

void csGLBasicTextureHandle::Unload ()
{
  if ((Handle == 0) || IsForeignHandle()) return;
  if (target == CS_TEX_IMG_1D)
    csGLTextureManager::UnsetTexture (GL_TEXTURE_1D, Handle);
  else if (target == CS_TEX_IMG_2D)
    csGLTextureManager::UnsetTexture (GL_TEXTURE_2D, Handle);
  else if (target == CS_TEX_IMG_3D)
    csGLTextureManager::UnsetTexture (GL_TEXTURE_3D, Handle);
  else if (target == CS_TEX_IMG_CUBEMAP)
    csGLTextureManager::UnsetTexture (GL_TEXTURE_CUBE_MAP, Handle);
  else if (target == CS_TEX_IMG_RECT)
    csGLTextureManager::UnsetTexture (GL_TEXTURE_RECTANGLE_ARB, Handle);
  glDeleteTextures (1, &Handle);
  Handle = 0;
}

void csGLBasicTextureHandle::Precache ()
{
  PrepareInt ();
  Load ();
}

void csGLBasicTextureHandle::SetTextureClass (const char* className)
{
  textureClass = txtmgr->GetTextureClassID (className ? className : "default");
}

const char* csGLBasicTextureHandle::GetTextureClass ()
{
  return txtmgr->GetTextureClassName (textureClass);
}

void csGLBasicTextureHandle::UpdateTexture ()
{
  Unload ();
}

GLuint csGLBasicTextureHandle::GetHandle ()
{
  Precache ();
  if ((!(texFlags.Get() & CS_TEXTURE_NOMIPMAPS))
    && (G3D->ext->CS_GL_EXT_framebuffer_object)
    && IsNeedMips())
  {
    G3D->statecache->SetTexture (GL_TEXTURE_2D, Handle);
    G3D->ext->glGenerateMipmapEXT (GL_TEXTURE_2D);
    SetNeedMips (false);
  }
  return Handle;
}

GLenum csGLBasicTextureHandle::GetGLTextureTarget() const
{
  switch (target)
  {
    case CS_TEX_IMG_1D:
      return GL_TEXTURE_1D;
    case CS_TEX_IMG_2D:
      return GL_TEXTURE_2D;
    case CS_TEX_IMG_3D:
      return GL_TEXTURE_3D;
    case CS_TEX_IMG_CUBEMAP:
      return GL_TEXTURE_CUBE_MAP;
    case CS_TEX_IMG_RECT:
      return GL_TEXTURE_RECTANGLE_ARB;
    default:
      return 0;
  }
}

}
CS_PLUGIN_NAMESPACE_END(gl3d)
