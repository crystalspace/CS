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

#include "ivaria/reporter.h"

#include "csgfx/imagecubemapmaker.h"
#include "csgfx/imagememory.h"
#include "csutil/measuretime.h"

#include "gl_render3d.h"
#include "gl_txtmgr_basictex.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

CS_LEAKGUARD_IMPLEMENT(csGLBasicTextureHandle);

static void CalculateNextBestPo2Size (int texFlags, 
                                      const int orgDim, int& newDim)
{
  const int sizeFlags = CS_TEXTURE_SCALE_UP | CS_TEXTURE_SCALE_DOWN;
  
  newDim = csFindNearestPowerOf2 (orgDim);
  if (newDim != orgDim)
  {
    if ((texFlags & sizeFlags) == CS_TEXTURE_SCALE_UP)
      /* newDim is fine */;
    else if ((texFlags & sizeFlags) == CS_TEXTURE_SCALE_DOWN)
      newDim >>= 1;
    else
    {
      int dU = newDim - orgDim;
      int dD = orgDim - (newDim >> 1);
      if (dD < dU)
        newDim >>= 1;
    }
  }
}

csGLBasicTextureHandle::csGLBasicTextureHandle (int width,
                                                int height,
                                                int depth,
                                                csImageType imagetype, 
                                                int flags, 
                                                csGLGraphics3D *iG3D) : 
  scfImplementationType (this), txtmgr (iG3D->txtmgr), 
  textureClass (txtmgr->GetTextureClassID ("default")),
  alphaType (csAlphaMode::alphaNone), Handle (0), pbo (0),
  orig_width (width), orig_height (height), orig_d (depth),
  G3D (iG3D), texFormat((TextureBlitDataFormat)-1)
{
  switch (imagetype)
  {
    case csimgCube:
      texType = texTypeCube;
      break;
    case csimg3D:
      texType = texType3D;
      break;
    default:
      texType = texType2D;
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
      || txtmgr->tweaks.enableNonPowerOfTwo2DTextures
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
    else if (!txtmgr->tweaks.enableNonPowerOfTwo2DTextures
      && !G3D->ext->CS_GL_ARB_texture_non_power_of_two)
      /* Note that 'enableNonPowerOfTwo2DTextures' is the flag for ATI's
       * support of non-POT _2D_ textures; that is, the textures, being
       * NPOTS, need to go to the 2D target, not RECT. 
       * Same when ARB_tnpot is available. */
      texType = texTypeRect;
  }
  texFlags.Set (flagsPublicMask, flags);

  // In opengl all textures, even non-mipmapped textures are required
  // to be powers of 2.
  AdjustSizePo2 ();
}

csGLBasicTextureHandle::csGLBasicTextureHandle (
    csGLGraphics3D *iG3D, TextureType texType, GLuint Handle) : 
  scfImplementationType (this),
  txtmgr (iG3D->txtmgr), 
  textureClass (txtmgr->GetTextureClassID ("default")),
  alphaType (csAlphaMode::alphaNone),
  Handle (Handle),
  pbo (0),
  orig_width (0),
  orig_height (0),
  orig_d (0),
  G3D (iG3D),
  texType (texType),
  texFormat ((TextureBlitDataFormat)-1)
{
  SetForeignHandle (true);
}

csGLBasicTextureHandle::~csGLBasicTextureHandle()
{
  Clear ();
  txtmgr->MarkTexturesDirty ();
  if (pbo != 0) txtmgr->G3D->ext->glDeleteBuffersARB (1, &pbo);
  if (IsInFBO() && (G3D->GetR2TBackend() != 0))
  {
    G3D->GetR2TBackend()->CleanupFBOs();
  }
}

iObjectRegistry* csGLBasicTextureHandle::GetObjectRegistry() const 
{ 
  return txtmgr->GetObjectRegistry();
}

bool csGLBasicTextureHandle::SynthesizeUploadData (
  const CS::StructuredTextureFormat& format,
  iString* fail_reason, bool zeroTexture)
{
  TextureStorageFormat glFormat;
  TextureSourceFormat srcFormat;
  if (!txtmgr->DetermineGLFormat (format, glFormat, srcFormat)) 
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
  
  desiredReadbackFormat = srcFormat;
  desiredReadbackBPP = 0;
  for (int i = 0; i < format.GetComponentCount(); i++)
    desiredReadbackBPP += format.GetComponentSize (i);
  desiredReadbackBPP = ((desiredReadbackBPP+7)/8);

  const void* zeroData = 0;
  csRef<iBase> zeroRef;
  if (zeroTexture)
  {
    int texelBits = 0;
    for (int i = 0; i < format.GetComponentCount(); i++)
      texelBits += format.GetComponentSize (i);
    size_t zeroSize = actual_width * actual_height * actual_d
      * ((texelBits+7)/8);
    CS::DataBuffer<>* zeroBuf = new CS::DataBuffer<> (zeroSize);
    memset (zeroBuf->GetData(), 0, zeroSize);
    zeroData = zeroBuf->GetData();
    zeroRef.AttachNew (zeroBuf);
  }

  const int imgNum = (texType == texTypeCube) ? 6 : 1;

  FreshUploadData ();
  if (texFlags.Check (CS_TEXTURE_NOMIPMAPS))
  {
    for (int i = 0; i < imgNum; i++)
    {
      csGLUploadData upload;
      upload.image_data = zeroData;
      upload.dataRef = zeroRef;
      upload.w = actual_width;
      upload.h = actual_height;
      upload.d = actual_d;
      upload.storageFormat = glFormat;
      upload.sourceFormat = srcFormat;
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
	upload.image_data = zeroData;
	upload.dataRef = zeroRef;
        upload.w = w;
        upload.h = h;
        upload.d = d;
        upload.storageFormat = glFormat;
        upload.sourceFormat = srcFormat;
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
  uploadData.Reset();
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
  CalculateNextBestPo2Size (texFlags, orig_width, newwidth);
  CalculateNextBestPo2Size (texFlags, orig_height, newheight);
  CalculateNextBestPo2Size (texFlags, orig_depth, newdepth);

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
  if (uploadData.IsValid())
    uploadData->DeleteAll();
  else
    uploadData.Reset (new csArray<csGLUploadData>);
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
    bool isWholeImage = (x == 0) && (y == 0) && (width == actual_width)
      && (height == actual_height);

    if (!IsWasRenderTarget())
    {
      SetWasRenderTarget (true);
      SetupAutoMipping();
    }

    // Pull texture data and set as RGBA/BGRA again, to prevent compression 
    // (slooow) on subsequent glTexSubImage() calls.
    if (!isWholeImage)
    {
      EnsureUncompressed (true, format);
    }
    else
    {
      texFormat = format;

      glTexImage2D (textarget, 0, GL_RGBA8, actual_width, 
	actual_height, 0, textureFormat, GL_UNSIGNED_BYTE, data);
      return;
    }
  }
  // Do the copy.
  glTexSubImage2D (textarget, 0, x, y, 
      width, height,
      textureFormat, GL_UNSIGNED_BYTE, data);
  RegenerateMipmaps();
}

void csGLBasicTextureHandle::SetupAutoMipping()
{
  // Set up mipmap generation
  if ((!(texFlags.Get() & CS_TEXTURE_NOMIPMAPS))
    && (!G3D->ext->CS_GL_EXT_framebuffer_object 
      || txtmgr->tweaks.disableGenerateMipmap))
  {
    GLenum textarget = GetGLTextureTarget();
    if (G3D->ext->CS_GL_SGIS_generate_mipmap)
    {
      glTexParameteri (textarget, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
      if (G3D->ext->CS_GL_SGIS_texture_lod
          && txtmgr->tweaks.generateMipMapsExcessOne)
      {
        texFlags.SetBool (flagExcessMaxMip, true);
	GLint maxLevel;
	glGetTexParameteriv (textarget, GL_TEXTURE_MAX_LEVEL_SGIS, &maxLevel);
	glTexParameteri (textarget, GL_TEXTURE_MAX_LEVEL_SGIS, maxLevel+1);
      }
    }
    else
      glTexParameteri (textarget, GL_TEXTURE_MIN_FILTER,
	txtmgr->rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);
  }
}

void csGLBasicTextureHandle::RegenerateMipmaps()
{
  if ((!(texFlags.Get() & CS_TEXTURE_NOMIPMAPS))
    && G3D->ext->CS_GL_EXT_framebuffer_object
    && !txtmgr->tweaks.disableGenerateMipmap)
  {
    G3D->ActivateTexture (this);
    G3D->ext->glGenerateMipmapEXT (GetGLTextureTarget());
  }
}

void csGLBasicTextureHandle::Load ()
{
  if (IsUploaded() || IsForeignHandle()) return;
  
  static const GLint textureMinFilters[3] = {GL_NEAREST_MIPMAP_NEAREST, 
    GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR};
  static const GLint textureMagFilters[3] = {GL_NEAREST, GL_LINEAR, 
    GL_LINEAR};

  GLRENDER3D_CHECKED_COMMAND(G3D, glGenTextures (1, &Handle));
  
  /* @@@ FIXME: That seems to happen with PS occasionally.
     Reason unknown. */
  CS_ASSERT(uploadData);
  if (!uploadData)
  {
    G3D->Report (CS_REPORTER_SEVERITY_WARNING, "WEIRD: no uploadData in %s!",
		 CS_FUNCTION_NAME);
    return;
  }

  const int texFilter = texFlags.Check (CS_TEXTURE_NOFILTER) ? 0 : 
    txtmgr->rstate_bilinearmap;
  const GLint magFilter = textureMagFilters[texFilter];
  const GLint minFilter = textureMinFilters[texFilter];
  const GLint wrapMode = 
    (texFlags.Check (CS_TEXTURE_CLAMP)) ? GL_CLAMP_TO_EDGE : GL_REPEAT;

  if (texType == texType1D)
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
  else if (texType == texType2D)
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
      if (uploadData.storageFormat.isCompressed)
      {
	G3D->ext->glCompressedTexImage2DARB (GL_TEXTURE_2D, uploadData.mip, 
	  uploadData.storageFormat.targetFormat, uploadData.w, uploadData.h, 
	  0, (GLsizei)uploadData.compressedSize, uploadData.image_data);
      }
      else
      {
	glTexImage2D (GL_TEXTURE_2D, uploadData.mip, 
	  uploadData.storageFormat.targetFormat, 
	  uploadData.w, uploadData.h, 0, uploadData.sourceFormat.format, 
	  uploadData.sourceFormat.type, uploadData.image_data);
      }
    }
  }
  else if (texType == texType3D)
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
      if (uploadData.storageFormat.isCompressed)
      {
	G3D->ext->glCompressedTexImage3DARB (GL_TEXTURE_3D, uploadData.mip, 
	  uploadData.storageFormat.targetFormat, uploadData.w, uploadData.h, 
	  uploadData.d, 0, (GLsizei)uploadData.compressedSize, 
	  uploadData.image_data);
      }
      else
      {
	G3D->ext->glTexImage3DEXT (GL_TEXTURE_3D, uploadData.mip, 
	  uploadData.storageFormat.targetFormat, uploadData.w, uploadData.h, 
          uploadData.d, 0, uploadData.sourceFormat.format, 
          uploadData.sourceFormat.type, uploadData.image_data);
      }
    }
  }
  else if (texType == texTypeCube)
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
    if (G3D->ext->CS_GL_AMD_seamless_cubemap_per_texture)
    {
      bool seamless = !texFlags.Check (CS_TEXTURE_CUBEMAP_DISABLE_SEAMLESS);
      glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_SEAMLESS,
		       seamless ? GL_TRUE : GL_FALSE);
    }

    size_t i;
    for (i = 0; i < uploadData->GetSize (); i++)
    {
      const csGLUploadData& uploadData = this->uploadData->Get (i);

      if (uploadData.storageFormat.isCompressed)
      {
	G3D->ext->glCompressedTexImage2DARB (
	  GL_TEXTURE_CUBE_MAP_POSITIVE_X + uploadData.imageNum, 
	  uploadData.mip, 
	  uploadData.storageFormat.targetFormat, uploadData.w, uploadData.h, 
	  0, (GLsizei)uploadData.compressedSize, uploadData.image_data);
      }
      else
      {
	glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X + uploadData.imageNum, 
	  uploadData.mip, uploadData.storageFormat.targetFormat, 
	  uploadData.w, uploadData.h,
	  0, uploadData.sourceFormat.format, uploadData.sourceFormat.type,
	  uploadData.image_data);
      }
    }
  }
  else if (texType == texTypeRect)
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
      if (uploadData.storageFormat.isCompressed)
      {
	G3D->ext->glCompressedTexImage2DARB (GL_TEXTURE_RECTANGLE_ARB, 
          uploadData.mip, uploadData.storageFormat.targetFormat, 
          uploadData.w, uploadData.h, 0, 
          (GLsizei)uploadData.compressedSize, uploadData.image_data);
      }
      else
      {
	glTexImage2D (GL_TEXTURE_RECTANGLE_ARB, uploadData.mip, 
	  uploadData.storageFormat.targetFormat, 
	  uploadData.w, uploadData.h, 0, uploadData.sourceFormat.format, 
	  uploadData.sourceFormat.type, uploadData.image_data);
      }
    }
  }

  uploadData.Reset();
  SetUploaded (true);
}

THREADED_CALLABLE_IMPL(csGLBasicTextureHandle, Unload)
{
  if (!IsUploaded() || IsForeignHandle()) return false;
  if (texType == texType1D)
    csGLTextureManager::UnsetTexture (GL_TEXTURE_1D, Handle);
  else if (texType == texType2D)
    csGLTextureManager::UnsetTexture (GL_TEXTURE_2D, Handle);
  else if (texType == texType3D)
    csGLTextureManager::UnsetTexture (GL_TEXTURE_3D, Handle);
  else if (texType == texTypeCube)
    csGLTextureManager::UnsetTexture (GL_TEXTURE_CUBE_MAP, Handle);
  else if (texType == texTypeRect)
    csGLTextureManager::UnsetTexture (GL_TEXTURE_RECTANGLE_ARB, Handle);
  glDeleteTextures (1, &Handle);
  Handle = 0;
  SetUploaded (false);
  return true;
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
  return Handle;
}
  
void csGLBasicTextureHandle::ChangeTextureCompareMode (
  const CS::Graphics::TextureComparisonMode& mode)
{
  if (!G3D->ext->CS_GL_ARB_shadow) return;

  GLenum textarget = GetGLTextureTarget();
  csGLGraphics3D::statecache->SetTexture (textarget, GetHandle ());
  
  if (mode.mode != texCompare.mode)
  {
    GLint compareMode = GL_NONE;
    switch (mode.mode)
    {
      case CS::Graphics::TextureComparisonMode::compareNone:
	//compareMode = GL_NONE;
	break;
      case CS::Graphics::TextureComparisonMode::compareR:
	compareMode = GL_COMPARE_R_TO_TEXTURE;
	break;
    }
    glTexParameteri (textarget, GL_TEXTURE_COMPARE_MODE, compareMode);
    texCompare.mode = mode.mode;
  }
  if (mode.mode
    && (mode.function != texCompare.function))
  {
    GLint compareFunc = GL_LEQUAL;
    switch (mode.function)
    {
      case CS::Graphics::TextureComparisonMode::funcLEqual:
	//compareFunc = GL_LEQUAL;
	break;
      case CS::Graphics::TextureComparisonMode::funcGEqual:
	compareFunc = GL_GEQUAL;
	break;
    }
    glTexParameteri (textarget, GL_TEXTURE_COMPARE_FUNC, compareFunc);
    texCompare.function = mode.function;
  }
}

GLenum csGLBasicTextureHandle::GetGLTextureTarget() const
{
  switch (texType)
  {
    case texType1D:
      return GL_TEXTURE_1D;
    case texType2D:
      return GL_TEXTURE_2D;
    case texType3D:
      return texType3D;
    case texTypeCube:
      return GL_TEXTURE_CUBE_MAP;
    case texTypeRect:
      return GL_TEXTURE_RECTANGLE_ARB;
    default:
      return 0;
  }
}

void csGLBasicTextureHandle::EnsureUncompressed (bool keepPixels,
  TextureBlitDataFormat newTexFormat)
{
  if (newTexFormat == (TextureBlitDataFormat)~0) newTexFormat = texFormat;

  GLenum target = GetGLTextureTarget();
  // @@@ FIXME: support more than 2D
  if (target != GL_TEXTURE_2D) return;

  bool doConvert = newTexFormat != texFormat;
  if (!doConvert && G3D->ext->CS_GL_ARB_texture_compression)
  {
    GLint isCompressed;
    glGetTexLevelParameteriv (target, 0, GL_TEXTURE_COMPRESSED_ARB, 
      &isCompressed);
    doConvert = isCompressed != 0;
  }
  if (!doConvert) return;

  GLuint textureFormat = (newTexFormat == RGBA8888) ? GL_RGBA : GL_BGRA;
  uint8* pixelData = 0;
  /* @@@ FIXME: This really should use the actual internal (base?) format and 
   * not just RGBA. */
  if (keepPixels || (newTexFormat != texFormat))
  {
    pixelData = (uint8*)cs_malloc (actual_width * actual_height * 4);
    glGetTexImage (target, 0, textureFormat, GL_UNSIGNED_BYTE, 
      pixelData);
  }

  glTexImage2D (target, 0, GL_RGBA8, actual_width, actual_height, 
    0, textureFormat, GL_UNSIGNED_BYTE, pixelData);

  if (pixelData != 0) cs_free (pixelData);

  texFormat = newTexFormat;
}

uint8* csGLBasicTextureHandle::QueryBlitBuffer (int x, int y, 
                                                int width, int height,
                                                size_t& pitch, 
                                                TextureBlitDataFormat format,
                                                uint bufFlags)
{ 
  if (txtmgr->G3D->ext->CS_GL_ARB_pixel_buffer_object)
  {
    return QueryBlitBufferPBO (x, y, width, height, pitch, format, bufFlags);
  }
  else
  {
    return QueryBlitBufferGeneric (x, y, width, height, pitch, format, bufFlags);
  }
}

void csGLBasicTextureHandle::ApplyBlitBuffer (uint8* buf)
{ 
  if (txtmgr->G3D->ext->CS_GL_ARB_pixel_buffer_object)
  {
    ApplyBlitBufferPBO (buf);
  }
  else
  {
    ApplyBlitBufferGeneric (buf);
  }
  RegenerateMipmaps();
}

iTextureHandle::BlitBufferNature csGLBasicTextureHandle::GetBufferNature (uint8* buf)
{
  if (txtmgr->G3D->ext->CS_GL_ARB_pixel_buffer_object)
    return natureDirect;
  else
    return natureIndirect;
}

uint8* csGLBasicTextureHandle::QueryBlitBufferGeneric (int x, int y, 
                                                       int width, int height,
                                                       size_t& pitch, 
                                                       TextureBlitDataFormat format,
                                                       uint bufFlags)
{ 
  GLenum textarget = GetGLTextureTarget();
  if ((textarget != GL_TEXTURE_2D) && (textarget != GL_TEXTURE_RECTANGLE_ARB))
    return 0;

  BlitBuffer blitBuf;
  blitBuf.x = x;
  blitBuf.y = y;
  blitBuf.width = width;
  blitBuf.height = height;
  blitBuf.format = format;
  /* @@@ FIXME: Improve - prolly makes sense to reuse allocated blocks of 
     memory. */
  uint8* p = (uint8*)cs_malloc (width * height * 4);
  blitBuffers.Put (p, blitBuf);
  pitch = width * 4;
  return p;
}

void csGLBasicTextureHandle::ApplyBlitBufferGeneric (uint8* buf)
{ 
  BlitBuffer* blitBuf = blitBuffers.GetElementPointer (buf);
  if (blitBuf != 0)
  {
    Blit (blitBuf->x, blitBuf->y, blitBuf->width, blitBuf->height, buf, 
      blitBuf->format);
    blitBuffers.DeleteAll (buf);
    cs_free (buf);
  }
}

uint8* csGLBasicTextureHandle::QueryBlitBufferPBO (int x, int y, 
                                                   int width, int height,
                                                   size_t& pitch, 
                                                   TextureBlitDataFormat format,
                                                   uint bufFlags)
{ 
  GLenum textarget = GetGLTextureTarget();
  if ((textarget != GL_TEXTURE_2D) && (textarget != GL_TEXTURE_RECTANGLE_ARB))
    return 0;

  bool isWholeImage = (x == 0) && (y == 0) && (width == actual_width)
    && (height == actual_height);

  texFormat = format;
  if (pbo == 0)
  {
    GLuint textureFormat = (texFormat == RGBA8888) ? GL_RGBA : GL_BGRA;
    csGLGraphics3D::statecache->SetBufferARB (GL_PIXEL_UNPACK_BUFFER_ARB, 0, true);
    Precache ();
    G3D->ActivateTexture (this);

    G3D->ext->glGenBuffersARB (1, &pbo);
    csGLGraphics3D::statecache->SetBufferARB (GL_PIXEL_UNPACK_BUFFER_ARB, pbo, true);
    G3D->ext->glBufferDataARB (GL_PIXEL_UNPACK_BUFFER_ARB, 
      actual_width * actual_height * 4, 0, GL_DYNAMIC_DRAW_ARB);
    pboMapped = 0;
    if ((bufFlags & iTextureHandle::blitbufRetainArea) || !isWholeImage)
    {
      csGLGraphics3D::statecache->SetBufferARB (GL_PIXEL_PACK_BUFFER_ARB, pbo, true);
      glGetTexImage (textarget, 0, textureFormat, GL_UNSIGNED_BYTE, 0);
      csGLGraphics3D::statecache->SetBufferARB (GL_PIXEL_PACK_BUFFER_ARB, 0, true);
    }
    glTexImage2D (textarget, 0, GL_RGBA8, actual_width, actual_height, 
      0, textureFormat, GL_UNSIGNED_BYTE, 0);
  }
  else
  {
    csGLGraphics3D::statecache->SetBufferARB (GL_PIXEL_UNPACK_BUFFER_ARB, pbo, true);
  }
  if (pboMapped == 0)
  {
    GLenum bufAccess = 
      (bufFlags & blitbufReadable) ? GL_READ_WRITE_ARB : GL_WRITE_ONLY_ARB;
    pboMapPtr = G3D->ext->glMapBufferARB (GL_PIXEL_UNPACK_BUFFER_ARB, 
      bufAccess);
  }
  pboMapped++;
  csGLGraphics3D::statecache->SetBufferARB (GL_PIXEL_UNPACK_BUFFER_ARB, 0, true);
  uint8* p = (uint8*)pboMapPtr;
  p += (y * actual_width + x) * 4;
  pitch = actual_width * 4;
  return p;
}

void csGLBasicTextureHandle::ApplyBlitBufferPBO (uint8* buf)
{ 
  pboMapped--;
  if (pboMapped == 0)
  {
    csGLGraphics3D::statecache->SetBufferARB (GL_PIXEL_UNPACK_BUFFER_ARB, pbo, true);
    G3D->ext->glUnmapBufferARB (GL_PIXEL_UNPACK_BUFFER_ARB);
    G3D->ActivateTexture (this);
    if (!IsWasRenderTarget())
    {
      SetWasRenderTarget (true);
      SetupAutoMipping();
    }
    GLuint textureFormat = (texFormat == RGBA8888) ? GL_RGBA : GL_BGRA;
    glTexSubImage2D (GetGLTextureTarget(), 0, 
      0, 0, actual_width, actual_height,
      textureFormat, GL_UNSIGNED_BYTE,
      0);
    csGLGraphics3D::statecache->SetBufferARB (GL_PIXEL_UNPACK_BUFFER_ARB, 0, true);
  }
}
  
void csGLBasicTextureHandle::SetMipmapLimits (int maxMip, int minMip)
{
  if (G3D->ext->CS_GL_SGIS_texture_lod)
  {
    GLenum textarget = GetGLTextureTarget();
    csGLGraphics3D::statecache->SetTexture (textarget, GetHandle ());
    if (texFlags.Check (flagExcessMaxMip))
      maxMip++;
    glTexParameteri (textarget, GL_TEXTURE_BASE_LEVEL_SGIS, minMip);
    glTexParameteri (textarget, GL_TEXTURE_MAX_LEVEL_SGIS, maxMip);
  }
}

void csGLBasicTextureHandle::GetMipmapLimits (int& maxMip, int& minMip)
{
  if (G3D->ext->CS_GL_SGIS_texture_lod)
  {
    GLenum textarget = GetGLTextureTarget();
    csGLGraphics3D::statecache->SetTexture (textarget, GetHandle ());
    GLint baseLevel, maxLevel;
    glGetTexParameteriv (textarget, GL_TEXTURE_BASE_LEVEL_SGIS, &baseLevel);
    glGetTexParameteriv (textarget, GL_TEXTURE_MAX_LEVEL_SGIS, &maxLevel);
    minMip = baseLevel;
    maxMip = maxLevel;
    if (texFlags.Check (flagExcessMaxMip))
      maxMip--;
  }
  else
  {
    maxMip = 1000;
    minMip = 0;
  }
}

void csGLBasicTextureHandle::SetDesiredReadbackFormat (const CS::StructuredTextureFormat& format)
{
  TextureStorageFormat glFormat;
  TextureSourceFormat sourceFormat;
  if (!txtmgr->DetermineGLFormat (format, glFormat, sourceFormat))
  {
    desiredReadbackFormat = TextureSourceFormat();
    desiredReadbackBPP = 0;
    return;
  }
    
  desiredReadbackFormat = sourceFormat;

  int s = 0;
  for (int i = 0; i < format.GetComponentCount(); i++)
    s += format.GetComponentSize (i);
  s = ((s+7)/8);
  desiredReadbackBPP = s;
}

template<typename Action>
csPtr<iDataBuffer> csGLBasicTextureHandle::ReadbackPerform (
  size_t readbackSize, Action& readbackAction)
{
  if (G3D->ext->CS_GL_ARB_pixel_buffer_object)
  {
    csRef<PBOWrapper> pbo = txtmgr->GetPBOWrapper (readbackSize);
    GLuint _pbo = pbo->GetPBO (GL_PIXEL_PACK_BUFFER_ARB);
    csGLGraphics3D::statecache->SetBufferARB (GL_PIXEL_PACK_BUFFER_ARB, _pbo, true);
    readbackAction (0);
    csGLGraphics3D::statecache->SetBufferARB (GL_PIXEL_PACK_BUFFER_ARB, 0, true);
    
    csRef<iDataBuffer> db;
#include "csutil/custom_new_disable.h"
    db.AttachNew (new (txtmgr->pboTextureReadbacks) TextureReadbackPBO (
      pbo, readbackSize));
#include "csutil/custom_new_enable.h"
    return csPtr<iDataBuffer> (db);
  }
  else
  {
    void* data = cs_malloc (readbackSize);
  
    readbackAction (data);
    
    csRef<iDataBuffer> db;
#include "csutil/custom_new_disable.h"
    db.AttachNew (new (txtmgr->simpleTextureReadbacks) TextureReadbackSimple (
      data, readbackSize));
#include "csutil/custom_new_enable.h"
    return csPtr<iDataBuffer> (db);
  }
}
  
#include "csutil/custom_new_disable.h"

struct ReadbackActionGetTexImage
{
  GLenum target;
  GLint level;
  GLenum format;
  GLenum type;

  ReadbackActionGetTexImage (GLenum target, GLint level, GLenum format, GLenum type)
   : target (target), level (level), format (format), type (type) {}

  void operator() (GLvoid *pixels) const
  {
    glGetTexImage (target, level, format, type, pixels);
  }
};

csPtr<iDataBuffer> csGLBasicTextureHandle::Readback (GLenum textarget,
    const CS::StructuredTextureFormat& format, int mip)
{
  if (format.GetFormat() == CS::StructuredTextureFormat::Special)
    return 0;
  
  if ((mip > 0) && (texFlags.Get() & CS_TEXTURE_NOMIPMAPS)) return 0;

  TextureStorageFormat glFormat;
  TextureSourceFormat sourceFormat;
  if (!txtmgr->DetermineGLFormat (format, glFormat, sourceFormat))
    return 0;
    
  if (lastReadbackFormat == sourceFormat)
  {
    return csPtr<iDataBuffer> (lastReadback);
  }
  SetDesiredReadbackFormat (format);
  
  int w = csMax (actual_width >> mip, 1);
  int h = csMax (actual_height >> mip, 1);
  int d = csMax (actual_d >> mip, 1);
  size_t byteSize = w * h * d * desiredReadbackBPP;
  
  ReadbackActionGetTexImage action (textarget, mip, sourceFormat.format, sourceFormat.type);
  return ReadbackPerform (byteSize, action);
}
  
struct ReadbackActionReadPixels
{
  GLint x;
  GLint y;
  GLsizei width;
  GLsizei height;
  GLenum format;
  GLenum type;

  ReadbackActionReadPixels (GLint x, GLint y, GLsizei width, GLsizei height,
    GLenum format, GLenum type)
   : x (x), y (y), width (width), height (height), format (format), type (type) {}

  void operator() (GLvoid *pixels) const
  {
    glReadPixels (x, y, width, height, format, type, pixels);
  }
};

void csGLBasicTextureHandle::ReadbackFramebuffer ()
{
  size_t byteSize = actual_width * actual_height * desiredReadbackBPP;
  
  ReadbackActionReadPixels action (0, 0, actual_width, actual_height,
    desiredReadbackFormat.format, desiredReadbackFormat.type);
  lastReadback = ReadbackPerform (byteSize, action);
  lastReadbackFormat = desiredReadbackFormat;
}

#include "csutil/custom_new_enable.h"

csPtr<iDataBuffer> csGLBasicTextureHandle::Readback (
  const CS::StructuredTextureFormat& format, int mip)
{
  GLenum textarget = GetGLTextureTarget();
  textarget = (textarget == GL_TEXTURE_CUBE_MAP) 
      ? GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB : textarget;
  csGLGraphics3D::statecache->SetTexture (textarget, GetHandle ());
  return Readback (textarget, format, mip);
}
  
class ReadbackImage :
  public scfImplementationExt0<ReadbackImage, csImageBase>
{
  int w, h, d;
  csRef<iDataBuffer> data;
public:
  ReadbackImage (int w, int h, int d, iDataBuffer* data)
   : scfImplementationType (this), w (w), h (h), d (d), data (data)
  {}
  
  const void* GetImageData() { return data->GetData(); }
  int GetWidth() const { return w; }
  int GetHeight() const { return h; }
  int GetDepth() const { return d; }
  csRef<iDataBuffer> GetRawData() const { return data; }
  /* @@@ Lies, damn lies: causes at least depth data to be reinterpreted as
     RGBA data. Useful for dumping, but not necessarily other scenarios */
  int GetFormat() const
  { return CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA; }
  const char* GetRawFormat() const { return "abgr8"; }
};

csPtr<iImage> csGLBasicTextureHandle::GetTextureFromGL ()
{
  // @@@ hmm... or just return an empty image?
  if (GetHandle () == (GLuint)~0) return 0;

  GLenum textarget = GetGLTextureTarget();
  GLenum usetarget = (textarget == GL_TEXTURE_CUBE_MAP) 
      ? GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB : textarget;
  csGLGraphics3D::statecache->SetTexture (textarget, GetHandle ());
  
  CS::StructuredTextureFormat texFormat;
  GLint depthSize;
  glGetTexLevelParameteriv (usetarget, 0, GL_TEXTURE_DEPTH_SIZE, &depthSize);
  if (depthSize > 0)
  {
    // Depth texture
    texFormat = CS::StructuredTextureFormat ('d', 32);
  }
  else
  {
    // Color texture
    texFormat = CS::StructuredTextureFormat ('a', 8, 'b', 8, 'g', 8, 'r', 8);
  }
  
  if (textarget == GL_TEXTURE_CUBE_MAP)
  {
    csRef<iImage> face[6];
    for (int f = 0; f < 6; f++)
    {
      csRef<iDataBuffer> texData (Readback (
        GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB+f, texFormat, 0));
      face[f].AttachNew (new ReadbackImage (actual_width, actual_height,
        actual_d, texData));
    }
    csImageCubeMapMaker* img = new csImageCubeMapMaker (
      face[0], face[1], face[2], face[3], face[4], face[5]);
    return csPtr<iImage> (img);
  }
  else
  {
    csRef<iDataBuffer> texData (Readback (usetarget, texFormat, 0));
    ReadbackImage* img = new ReadbackImage (actual_width, actual_height,
      actual_d, texData);
    return csPtr<iImage> (img);
  }
}

}
CS_PLUGIN_NAMESPACE_END(gl3d)
