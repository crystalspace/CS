/*
    Copyright (C) 1998-2004 by Jorrit Tyberghein
	      (C) 2003 by Philip Aumayr
	      (C) 2004 by Frank Richter

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

#ifndef __CS_GL_TXTMGR_BASICTEX_H__
#define __CS_GL_TXTMGR_BASICTEX_H__

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

#include "csutil/leakguard.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "csutil/weakref.h"

#include "igraphic/image.h"
#include "ivideo/texture.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

class csGLGraphics3D;
class csGLTextureManager;

struct TextureStorageFormat
{
  /// Format in which to store the texture ('internalformat' in GL spec)
  GLenum targetFormat;
  /**
   * Whether the format is compressed. The \c format and \c type members are
   * only valid for uncompressed textures.
   */
  bool isCompressed;
  /// Format of the source data
  GLenum format;
  /// Type of the source data
  GLenum type;

  /// Init with default values
  TextureStorageFormat () : targetFormat (0), isCompressed (false), 
    format (0), type (0) {}
  /// Init for compressed texture
  TextureStorageFormat (GLenum compressedFormat) : targetFormat (compressedFormat), 
    isCompressed (true), format (0), type (0) {}
  /// Init for uncompressed texture
  TextureStorageFormat (GLenum targetFormat, GLenum format, GLenum type) : 
    targetFormat (targetFormat), isCompressed (false), format (format), 
    type (type) {}
};

struct csGLUploadData
{
  const void* image_data;
  int w, h, d;
  csRef<iBase> dataRef;
  TextureStorageFormat sourceFormat;
  size_t compressedSize;
  int mip;
  int imageNum;

  csGLUploadData() : image_data(0) {}
};

struct csGLTextureClassSettings;

class csGLBasicTextureHandle :
  public scfImplementation1<csGLBasicTextureHandle, 
			    iTextureHandle>
{
protected:
  CS_LEAKGUARD_DECLARE(csGLBasicTextureHandle);

  /// texturemanager handle
  csRef<csGLTextureManager> txtmgr;

  csStringID textureClass;

  /// Texture flags (combined public and private)
  csFlags texFlags;
  /// Private texture flags
  enum
  {
    flagTexupdateNeeded = 1 << 31, 
    flagPrepared = 1 << 30, 
    flagForeignHandle = 1 << 29,
    flagWasRenderTarget = 1 << 28,
    flagNeedMips = 1 << 27,

    // Flags below are used by csGLTextureHandle
    /// Does it have a keycolor?
    flagTransp = 1 << 26,
    /// Is the color valid?
    flagTranspSet = 1 << 25,

    flagLast,
    /// Mask to get only the "public" flags
    flagsPublicMask = flagLast - 2
  };

  csAlphaMode::AlphaType alphaType;
  bool IsTexupdateNeeded() const
  {
    return texFlags.Check (flagTexupdateNeeded);
  }
  void SetTexupdateNeeded (bool b)
  {
    texFlags.SetBool (flagTexupdateNeeded, b);
  }
  bool IsPrepared() const { return texFlags.Check (flagPrepared); }
  void SetPrepared (bool b) { texFlags.SetBool (flagPrepared, b); }
  bool IsTransp() const { return texFlags.Check (flagTransp); }
  void SetTransp (bool b) { texFlags.SetBool (flagTransp, b); }
  bool IsForeignHandle() const { return texFlags.Check (flagForeignHandle); }
  void SetForeignHandle (bool b) { texFlags.SetBool (flagForeignHandle, b); }
  bool IsTranspSet() const { return texFlags.Check (flagTranspSet); }
  void SetTranspSet (bool b) { texFlags.SetBool (flagTranspSet, b); }

  GLenum DetermineTargetFormat (GLenum defFormat, bool allowCompress,
    const char* rawFormat, bool& compressedTarget);

  static void ComputeNewPo2ImageSize (int texFlags, 
    int orig_width, int orig_height, int orig_depth,
    int& newwidth, int& newheight, int& newdepth,
    int max_tex_size);

  /// Make sure uploadData is valid, nice, and clean.
  void FreshUploadData ();
private:
  void AdjustSizePo2 ();
    
protected:
  GLuint Handle;
  /// Upload the texture to GL.
  void Load ();
  void Unload ();
public:
  /// The dimensions which were requested upon texture creation
  int orig_width, orig_height, orig_d;
  /// The dimensions actually used
  int actual_width, actual_height, actual_d;
  csArray<csGLUploadData>* uploadData;
  csWeakRef<csGLGraphics3D> G3D;
  int target;
  /// Format used for last Blit() call
  TextureBlitDataFormat texFormat;
  bool IsWasRenderTarget() const
  {
    return texFlags.Check (flagWasRenderTarget);
  }
  void SetWasRenderTarget (bool b)
  {
    texFlags.SetBool (flagWasRenderTarget, b);
  }
  bool IsNeedMips() const { return texFlags.Check (flagNeedMips); }
  void SetNeedMips (bool b) { texFlags.SetBool (flagNeedMips, b); }

  /// Create a texture with given dimensions
  csGLBasicTextureHandle (int width, int height, int depth,
    csImageType imagetype, int flags, csGLGraphics3D *iG3D);
  /// Create from existing handle
  csGLBasicTextureHandle (csGLGraphics3D *iG3D, int target, GLuint Handle);

  virtual ~csGLBasicTextureHandle ();

  /**
   * Synthesize empty upload data structures for textures of the format 
   * \a format. */
  bool SynthesizeUploadData (const CS::StructuredTextureFormat& format,
    iString* fail_reason);

  void Clear();

  void Unprepare () { SetPrepared (false); }
  /// Merge this texture into current palette, compute mipmaps and so on.
  virtual void PrepareInt ();

  /// Retrieve the flags set for this texture
  virtual int GetFlags () const;

  /// Enable key color
  virtual void SetKeyColor (bool /*Enable*/) { }

  /// Set the key color.
  virtual void SetKeyColor (uint8 red, uint8 green, uint8 blue) { }

  /// Get the key color status (false if disabled, true if enabled).
  virtual bool GetKeyColor () const { return false; }
  /// Get the key color
  virtual void GetKeyColor (uint8 &red, uint8 &green, uint8 &blue) const
  { red = green = blue = 0; }

  /**
   * Get the dimensions for a given mipmap level (0 to 3).
   * If the texture was registered just for 2D usage, mipmap levels above
   * 0 will return false. Note that the result of this function will
   * be the size that the renderer uses for this texture. In most cases
   * this corresponds to the size that was used to create this texture
   * but some renderers have texture size limitations (like power of two)
   * and in that case the size returned here will be the corrected size.
   * You can get the original image size with GetOriginalDimensions().
   */
  virtual bool GetRendererDimensions (int &mw, int &mh);

  //enum { CS_TEX_IMG_1D = 0, CS_TEX_IMG_2D, CS_TEX_IMG_3D, CS_TEX_IMG_CUBEMAP };
  /**
   * Texture Depth Indices are used for Cubemap interface
   */
  //enum { CS_TEXTURE_CUBE_POS_X = 0, CS_TEXTURE_CUBE_NEG_X, 
  //       CS_TEXTURE_CUBE_POS_Y, CS_TEXTURE_CUBE_NEG_Y,
  //       CS_TEXTURE_CUBE_POS_Z, CS_TEXTURE_CUBE_NEG_Z };

  /**
   * Get the dimensions for a given mipmap level (0 to 3).
   * If the texture was registered just for 2D usage, mipmap levels above
   * 0 will return false. Note that the result of this function will
   * be the size that the renderer uses for this texture. In most cases
   * this corresponds to the size that was used to create this texture
   * but some renderers have texture size limitations (like power of two)
   * and in that case the size returned here will be the corrected size.
   * You can get the original image size with GetOriginalDimensions().
   */
  virtual bool GetRendererDimensions (int &mw, int &mh, int &md);

  virtual void GetOriginalDimensions (int& mw, int& mh)
  {
    GetRendererDimensions (mw, mh);
  }
  virtual void GetOriginalDimensions (int& mw, int& mh, int &md)
  {
    GetRendererDimensions (mw, mh, md);
  }

  virtual void Blit (int x, int y, int width, int height,
    unsigned char const* data, TextureBlitDataFormat format = RGBA8888);
  void SetupAutoMipping();

  /// Get the texture target
  virtual int GetTextureTarget () const { return target; }

  /**
   * Query the private object associated with this handle.
   * For internal usage by the 3D driver.
   */
  virtual void *GetPrivateObject ();

  /**
   * Query if the texture has an alpha channel.<p>
   * This depends both on whenever the original image had an alpha channel
   * and of the fact whenever the renderer supports alpha maps at all.
   */
  virtual bool GetAlphaMap ();

  virtual csAlphaMode::AlphaType GetAlphaType () const
  { return alphaType; }
  virtual void SetAlphaType (csAlphaMode::AlphaType alphaType)
  { this->alphaType = alphaType; }

  virtual void Precache ();

  virtual void SetTextureClass (const char* className);
  virtual const char* GetTextureClass ();

  virtual const char* GetImageName () const { return 0; }

  void UpdateTexture ();

  GLuint GetHandle ();

  /**
   * Return the texture target for this texture (e.g. GL_TEXTURE_2D)
   * \remark Returns GL_TEXTURE_CUBE_MAP for cubemaps.
   */
  GLenum GetGLTextureTarget() const;
};

}
CS_PLUGIN_NAMESPACE_END(gl3d)

#endif // __CS_GL_TXTMGR_BASICTEX_H__

