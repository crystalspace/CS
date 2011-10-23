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

#include "igraphic/imageio.h"
#include "iutil/vfs.h"

#include "gl_render3d.h"
#include "gl_txtmgr.h"
#include "gl_txtmgr_imagetex.h"

using namespace CS::Threading;

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

CS_LEAKGUARD_IMPLEMENT(csGLTextureManager);

static const csGLTextureClassSettings defaultSettings = 
  {GL_RGB, GL_RGBA, false, false, true, true, false};

csGLTextureManager::csGLTextureManager (iObjectRegistry* object_reg,
        iGraphics2D* iG2D, iConfigFile *config,
        csGLGraphics3D *iG3D) : 
  scfImplementationType (this), textures (16), compactTextures (false)
{
  csGLTextureManager::object_reg = object_reg;

  texturesLock.Initialize();
  
  G3D = iG3D;
  max_tex_size = G3D->GetMaxTextureSize ();

  G3D->ext->InitGL_ARB_texture_compression ();
  if (G3D->ext->CS_GL_ARB_texture_compression)
    G3D->ext->InitGL_EXT_texture_compression_s3tc (); 
  G3D->ext->InitGL_ARB_texture_float();
  /*if (G3D->ext->CS_GL_ARB_texture_float)
    G3D->ext->InitGL_ARB_half_float_pixel();*/

  G3D->ext->InitGL_ARB_pixel_buffer_object();
  
  G3D->ext->InitGL_SGIS_texture_lod();
  G3D->ext->InitGL_ARB_shadow ();

#define CS_GL_TEXTURE_FORMAT(fmt)					    \
  textureFormats.Put (#fmt, TextureFormat (fmt, true));		
#define CS_GL_TEXTURE_FORMAT_EXT(fmt, Ext)				    \
  textureFormats.Put (#fmt, TextureFormat (fmt, G3D->ext->CS_##Ext));		

  CS_GL_TEXTURE_FORMAT(GL_RGB);
  CS_GL_TEXTURE_FORMAT(GL_R3_G3_B2);
  CS_GL_TEXTURE_FORMAT(GL_RGB4);
  CS_GL_TEXTURE_FORMAT(GL_RGB5);
  CS_GL_TEXTURE_FORMAT(GL_RGB8);
  CS_GL_TEXTURE_FORMAT(GL_RGB10);
  CS_GL_TEXTURE_FORMAT(GL_RGB12);
  CS_GL_TEXTURE_FORMAT(GL_RGB16);
  CS_GL_TEXTURE_FORMAT(GL_RGBA);
  CS_GL_TEXTURE_FORMAT(GL_RGBA2);
  CS_GL_TEXTURE_FORMAT(GL_RGBA4);
  CS_GL_TEXTURE_FORMAT(GL_RGB5_A1);
  CS_GL_TEXTURE_FORMAT(GL_RGBA8);
  CS_GL_TEXTURE_FORMAT(GL_RGB10_A2);
  CS_GL_TEXTURE_FORMAT(GL_RGBA12);
  CS_GL_TEXTURE_FORMAT(GL_RGBA16);
  CS_GL_TEXTURE_FORMAT_EXT(GL_COMPRESSED_RGB_ARB, 
    GL_ARB_texture_compression);
  CS_GL_TEXTURE_FORMAT_EXT(GL_COMPRESSED_RGBA_ARB, 
    GL_ARB_texture_compression);
  CS_GL_TEXTURE_FORMAT_EXT(GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 
    GL_EXT_texture_compression_s3tc);
  CS_GL_TEXTURE_FORMAT_EXT(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 
    GL_EXT_texture_compression_s3tc);
  CS_GL_TEXTURE_FORMAT_EXT(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 
    GL_EXT_texture_compression_s3tc);
  CS_GL_TEXTURE_FORMAT_EXT(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 
    GL_EXT_texture_compression_s3tc);
#undef CS_GL_TEXTURE_FORMAT_EXT
#undef CS_GL_TEXTURE_FORMAT

  textureClasses.Put (textureClassIDs.Request ("default"), defaultSettings);

  read_config (config);
  InitFormats ();
}

csGLTextureManager::~csGLTextureManager()
{
  Clear ();
}
  
void csGLTextureManager::NextFrame (uint frameNum)
{
  pboCache.AdvanceTime (frameNum);
}
  
csRef<PBOWrapper> csGLTextureManager::GetPBOWrapper (size_t pboSize)
{
  csRef<PBOWrapper>* wrapper = pboCache.Query (pboSize, true);
  if (wrapper != 0) return *wrapper;
  csRef<PBOWrapper> newWrapper;
  newWrapper.AttachNew (new PBOWrapper (pboSize));
  pboCache.AddActive (newWrapper);
  return newWrapper;
}

void csGLTextureManager::read_config (iConfigFile *config)
{
  sharpen_mipmaps = config->GetInt
    ("Video.OpenGL.SharpenMipmaps", 0);
  texture_downsample = config->GetInt
    ("Video.OpenGL.TextureDownsample", 0);
  texture_filter_anisotropy = csMax(1.0f, config->GetFloat
    ("Video.OpenGL.TextureFilterAnisotropy", 1.0));
  tweaks.disableRECTTextureCompression = config->GetBool
    ("Video.OpenGL.DisableRECTTextureCompression", false);
  tweaks.enableNonPowerOfTwo2DTextures = config->GetBool
    ("Video.OpenGL.EnableNonPowerOfTwo2DTextures", false);
  tweaks.disableGenerateMipmap = config->GetBool
    ("Video.OpenGL.DisableGenerateMipmap", false);
  tweaks.generateMipMapsExcessOne = config->GetBool
    ("Video.OpenGL.GenerateOneExcessMipMap", false);
  
  const char* filterModeStr = config->GetStr (
    "Video.OpenGL.TextureFilter", "trilinear");
  rstate_bilinearmap = 2;
  if (strcmp (filterModeStr, "none") == 0)
    rstate_bilinearmap = 0;
  else if (strcmp (filterModeStr, "nearest") == 0)
    rstate_bilinearmap = 0;
  else if (strcmp (filterModeStr, "bilinear") == 0)
    rstate_bilinearmap = 1;
  else if (strcmp (filterModeStr, "trilinear") == 0)
    rstate_bilinearmap = 2;
  else
  {
    G3D->Report (CS_REPORTER_SEVERITY_WARNING, 
      "Invalid texture filter mode %s.", CS::Quote::Single (filterModeStr));
  }

  ReadTextureClasses (config);
}

const csGLTextureClassSettings* 
csGLTextureManager::GetTextureClassSettings (csStringID texclass)
{
  const csGLTextureClassSettings* ret = 
    textureClasses.GetElementPointer (texclass);
  return ret ? ret : &defaultSettings;
}

GLenum csGLTextureManager::ParseTextureFormat (const char* formatName, 
					       GLenum defaultFormat)
{
  csString extractedFormat;

  while ((formatName != 0) && (*formatName != 0))
  {
    const char* comma = strchr (formatName, ',');
    if (comma != 0)
    {
      extractedFormat.Replace (formatName, comma - formatName);
      formatName = comma + 1;
    }
    else
    {
      extractedFormat.Replace (formatName);
      formatName = 0;
    }
    const TextureFormat* textureFmt = textureFormats.GetElementPointer (
      extractedFormat.GetData());
    if (textureFmt == 0)
    {
      G3D->Report (CS_REPORTER_SEVERITY_ERROR,
	"Unknown texture format name %s", CS::Quote::Single (extractedFormat.GetData()));
    }
    else
    {
      if (textureFmt->supported)
	return textureFmt->format;
      else
      {
	// @@@ Report if verbose?
      }
    }
  }

  return defaultFormat;
}

void csGLTextureManager::ReadTextureClasses (iConfigFile* config)
{
  csString extractedClass;
  csRef<iConfigIterator> it = config->Enumerate (
    "Video.OpenGL.TextureClass.");
  while (it->HasNext())
  {
    it->Next();
    const char* keyName = it->GetKey (true);
    const char* dot = strchr (keyName, '.');
    if (dot != 0)
    {
      extractedClass.Replace (keyName, dot - keyName);

      csStringID classID = textureClassIDs.Request (extractedClass);
      csGLTextureClassSettings* settings = textureClasses.GetElementPointer (
	classID);
      if (settings == 0)
      {
	textureClasses.Put (classID, defaultSettings);
	settings = textureClasses.GetElementPointer (classID);
      }

      const char* optionName = dot + 1;
      if (strcasecmp (optionName, "FormatRGB") == 0)
      {
	settings->formatRGB = ParseTextureFormat (it->GetStr(),
	  GL_RGB);
      } 
      else if (strcasecmp (optionName, "FormatRGBA") == 0)
      {
	settings->formatRGBA = ParseTextureFormat (it->GetStr(),
	  GL_RGBA);
      } 
      else if (strcasecmp (optionName, "SharpenPrecomputedMipmaps") == 0)
      {
	settings->sharpenPrecomputedMipmaps = it->GetBool ();
      } 
      else if (strcasecmp (optionName, "ForceDecompress") == 0)
      {
	settings->forceDecompress = it->GetBool ();
      } 
      else if (strcasecmp (optionName, "AllowDownsample") == 0)
      {
	settings->allowDownsample = it->GetBool ();
      } 
      else if (strcasecmp (optionName, "AllowMipSharpen") == 0)
      {
	settings->allowMipSharpen = it->GetBool ();
      } 
      else if (strcasecmp (optionName, "RenormalizeGeneratedMips") == 0)
      {
	settings->renormalizeGeneratedMips = it->GetBool ();
      } 
      else
      {
	G3D->Report (CS_REPORTER_SEVERITY_ERROR,
	  "Unknown texture class option %s for %s", 
	  CS::Quote::Single (optionName),
	  CS::Quote::Single (extractedClass.GetData()));
      }
    }
    else
    {
      // @@@ Report?
    }
  }
}

void csGLTextureManager::Clear()
{
  MutexScopedLock lock(texturesLock);

  size_t i;
  for (i=0; i < textures.GetSize (); i++)
  {
    csGLBasicTextureHandle* tex = textures[i];
    if (tex != 0) tex->Clear ();
  }
  textures.DeleteAll ();
}

void csGLTextureManager::UnsetTexture (GLenum target, GLuint texture)
{
  csGLStateCache* statecache = csGLGraphics3D::statecache;
  if (!statecache) return;

  if (csGLGraphics3D::ext->CS_GL_ARB_multitexture)
  {
    int oldIU = -1;
    for (int u = 0; u < statecache->GetNumImageUnits(); u++)
    {
      if (statecache->GetTexture (target, u) == texture)
      {
	if (oldIU == -1)
          oldIU = statecache->GetCurrentImageUnit ();
        statecache->SetCurrentImageUnit (u);
	statecache->SetTexture (target, 0);
      }
    }
    if (oldIU != -1)
    {
      statecache->SetCurrentImageUnit (oldIU);
      statecache->ActivateImageUnit ();
    }
  }
  else
  {
    if (statecache->GetTexture (target) == texture)
      statecache->SetTexture (target, 0);
  }
}

bool csGLTextureManager::ImageTypeSupported (csImageType imagetype,
                                             iString* fail_reason)
{
  if ((imagetype == csimgCube)
      && !G3D->ext->CS_GL_ARB_texture_cube_map)
  {
    if (fail_reason) fail_reason->Replace (
      "Cubemap textures are not supported!");
    return false;
  }
  if ((imagetype == csimg3D)
      && !G3D->ext->CS_GL_EXT_texture3D)
  {
    if (fail_reason) fail_reason->Replace (
      "3D textures are not supported!");
    return false;
  }
  return true;
}

csPtr<iTextureHandle> csGLTextureManager::RegisterTexture (iImage *image,
	int flags, iString* fail_reason)
{
  if (!image)
  {
    if (fail_reason) fail_reason->Replace (
      "No image given to RegisterTexture!");
    return 0;
  }

  if (!ImageTypeSupported (image->GetImageType(), fail_reason))
    return 0;

  csGLTextureHandle *txt = new csGLTextureHandle (image, flags, G3D);

  MutexScopedLock lock(texturesLock);
  CompactTextures ();
  textures.Push(txt);
  return csPtr<iTextureHandle> (txt);
}

csPtr<iTextureHandle> csGLTextureManager::CreateTexture (int w, int h, int d,
      csImageType imagetype, const char* format, int flags,
      iString* fail_reason)
{
  CS::StructuredTextureFormat texFormat (
    CS::TextureFormatStrings::ConvertStructured (format));
  if (!texFormat.IsValid()) 
  {
    if (fail_reason) fail_reason->Replace ("invalid texture format");
    return 0;
  }

  /* @@@ TODO: Does it make sense to "optimize" formats?
         For example, NV is fastest when GL_BGR(A) is used. When a color format
         is specified, an "argb" format (CS notation for GL_BGR(A)) could be
         substituted instead. Or does the driver handle that automatically?
   */
   
  if (!ImageTypeSupported (imagetype, fail_reason))
    return 0;

  csGLBasicTextureHandle *txt = new csGLBasicTextureHandle (
      w, h, d, imagetype, flags, G3D);

  bool doClear = (flags & CS_TEXTURE_CREATE_CLEAR) != 0;
  if (!txt->SynthesizeUploadData (texFormat, fail_reason,
      doClear && !G3D->ext->CS_GL_EXT_framebuffer_object))
  {
    delete txt;
    return 0;
  }
  
  /*
    At least on an NV GeForce 8800M, driver versions up to & including
    190.18, Linux 64 bit, float textures have to be cleared before being
    used as a render target, otherwise they will exhibit artifacts after
    being rendered to. 
   */
  doClear |= (G3D->ext->CS_GL_EXT_framebuffer_object)
    && (texFormat.GetFormat() == CS::StructuredTextureFormat::Float);
  if (doClear && G3D->ext->CS_GL_EXT_framebuffer_object)
  {
    GLuint framebuffer;
    G3D->ext->glGenFramebuffersEXT (1, &framebuffer);
    G3D->ext->glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, framebuffer);
    
    const GLenum texTarget = txt->GetGLTextureTarget();
    const GLuint texHandle = txt->GetHandle();
    switch (texTarget)
    {
	case GL_TEXTURE_1D:
	  G3D->ext->glFramebufferTexture1DEXT (GL_FRAMEBUFFER_EXT,
	    GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_1D, texHandle, 0);
	  if (G3D->ext->glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT)
	      == GL_FRAMEBUFFER_COMPLETE_EXT)
	    glClear (GL_COLOR_BUFFER_BIT);
	  break;
	case GL_TEXTURE_2D:
	case GL_TEXTURE_RECTANGLE_ARB:
	  G3D->ext->glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, 
	    GL_COLOR_ATTACHMENT0_EXT, texTarget, texHandle, 0);
	  if (G3D->ext->glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT)
	      == GL_FRAMEBUFFER_COMPLETE_EXT)
	    glClear (GL_COLOR_BUFFER_BIT);
	  break;
	case GL_TEXTURE_CUBE_MAP:
	  {
	    for (int i = 0; i < 6; i++)
	    {
	      G3D->ext->glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, 
		GL_COLOR_ATTACHMENT0_EXT, 
		GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + i, 
		texHandle, 0);
	      if (G3D->ext->glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT)
		  == GL_FRAMEBUFFER_COMPLETE_EXT)
		glClear (GL_COLOR_BUFFER_BIT);
	    }
	  }
	  break;
	case GL_TEXTURE_3D:
	  {
	    for (int i = 0; i < 6; i++)
	    {
	      G3D->ext->glFramebufferTexture3DEXT (GL_FRAMEBUFFER_EXT, 
		GL_COLOR_ATTACHMENT0_EXT, texTarget, texHandle, 0, d);
	      if (G3D->ext->glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT)
		  == GL_FRAMEBUFFER_COMPLETE_EXT)
		glClear (GL_COLOR_BUFFER_BIT);
	    }
	  }
	  break;
    }
    
    G3D->ext->glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
    G3D->ext->glDeleteFramebuffersEXT (1, &framebuffer);
  }

  MutexScopedLock lock(texturesLock);
  CompactTextures ();
  textures.Push(txt);
  return csPtr<iTextureHandle> (txt);
}

void csGLTextureManager::CompactTextures ()
{
  if (!compactTextures) return;

  size_t i = 0;
  while (i < textures.GetSize ())
  {
    if (textures[i] == 0)
      textures.DeleteIndexFast (i);
    else
      i++;
  }
  
  compactTextures = false;
}

int csGLTextureManager::GetTextureFormat ()
{
  return CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA;
}

void csGLTextureManager::GetMaxTextureSize (int& w, int& h, int& aspect)
{
  w = max_tex_size;
  h = max_tex_size;
  aspect = max_tex_size;
}

void csGLTextureManager::DumpTextures (iVFS* VFS, iImageIO* iio, 
				       const char* dir)
{
  csString outfn;

  MutexScopedLock lock(texturesLock);

  for (size_t i = 0; i < textures.GetSize (); i++)
  {
    if (!textures[i]) continue;
    
    csRef<iImage> img = textures[i]->GetTextureFromGL ();
    if (img)
    {
      csRef<iDataBuffer> buf = iio->Save (img, "image/png");
      if (!buf)
      {
	G3D->Report (CS_REPORTER_SEVERITY_WARNING,
		     "Could not save texture.");
      }
      else
      {
	outfn.Format ("%s%zu.png", dir, i);
	if (!VFS->WriteFile (outfn, (char*)buf->GetInt8 (), buf->GetSize ()))
	{
	  G3D->Report (CS_REPORTER_SEVERITY_WARNING,
		       "Could not write to %s.", outfn.GetData ());
	}
	else
	{
	  G3D->Report (CS_REPORTER_SEVERITY_NOTIFY,
		       "Dumped texture to %s", outfn.GetData ());
	}
      }
    }
  }
}

}
CS_PLUGIN_NAMESPACE_END(gl3d)
