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
#include "csutil/pooledscfclass.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "csutil/scopedpointer.h"
#include "csutil/weakref.h"

#include "igraphic/image.h"
#include "iutil/databuff.h"
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
   * Whether the format is compressed. Accompanying "source format"
   * information is only valid for uncompressed textures.
   */
  bool isCompressed;

  /// Init with default values
  TextureStorageFormat () : targetFormat (0), isCompressed (false) {}
  /// Init with given format
  TextureStorageFormat (GLenum targetFormat, bool compressed) :
    targetFormat (targetFormat), isCompressed (compressed) {}
};

struct TextureSourceFormat
{
  /// Format of the source data
  GLenum format;
  /// Type of the source data
  GLenum type;

  /// Init with default values
  TextureSourceFormat () : format (0), type (0) {}
  /// Init for uncompressed texture
  TextureSourceFormat (GLenum format, GLenum type) :
    format (format), type (type) {}
    
  bool operator== (const TextureSourceFormat& other) const
  {
    return (format == other.format) && (type == other.type);
  }
};

struct csGLUploadData
{
  const void* image_data;
  int w, h, d;
  csRef<iBase> dataRef;
  TextureStorageFormat storageFormat;
  TextureSourceFormat sourceFormat;
  size_t compressedSize;
  int mip;
  int imageNum;

  csGLUploadData() : image_data(0) {}
};

struct csGLTextureClassSettings;

class TextureReadbackSimple :
  public scfImplementationPooled<scfImplementation1<TextureReadbackSimple,
                                                    iDataBuffer> >
{
  typedef scfImplementationPooled<scfImplementation1<TextureReadbackSimple,
                                                    iDataBuffer> > SuperClass;

  void* data;
  size_t size;
public:
  TextureReadbackSimple (void* data, size_t size) : SuperClass (this),
    data (data), size (size) {}
  ~TextureReadbackSimple()
  {
    cs_free (data);
  }
  
  char* GetData () const { return (char*)data; }
  int8* GetInt8 () { return (int8*)data; }
  size_t GetSize () const { return size; }
  uint8* GetUint8 () { return (uint8*)data; }
  char* operator * () const { return (char*)data; }
};

// For GetTextureTarget ()
#include "csutil/deprecated_warn_off.h"

class csGLBasicTextureHandle :
  public ThreadedCallable<csGLBasicTextureHandle>,
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
    flagExcessMaxMip = 1 << 27,

    // Flags below are used by csGLTextureHandle
    /// Does it have a keycolor?
    flagTransp = 1 << 26,
    /// Is the color valid?
    flagTranspSet = 1 << 25,
    
    /// Special flag to mark this texture is used in an FBO
    flagInFBO = 1 << 24,
    
    /// Texture has been uploaded
    flagUploaded = 1 << 23,

    flagLast,
    /// Mask to get only the "public" flags
    flagsPublicMask = flagLast - 2
  };

  csAlphaMode::AlphaType alphaType;
  bool IsTexupdateNeeded() const
  {
    return texFlags.Check ((uint32)flagTexupdateNeeded);
  }
  void SetTexupdateNeeded (bool b)
  {
    texFlags.SetBool ((uint32)flagTexupdateNeeded, b);
  }
  bool IsPrepared() const { return texFlags.Check (flagPrepared); }
  void SetPrepared (bool b) { texFlags.SetBool (flagPrepared, b); }
  bool IsUploaded() const { return texFlags.Check (flagUploaded); }
  void SetUploaded (bool b) { texFlags.SetBool (flagUploaded, b); }
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

  struct BlitBuffer
  {
    int x;
    int y;
    int width;
    int height;
    iTextureHandle::TextureBlitDataFormat format;
  };
  csHash<BlitBuffer, csPtrKey<uint8> > blitBuffers;
  uint8* QueryBlitBufferGeneric (int x, int y, int width, int height,
    size_t& pitch, TextureBlitDataFormat format, uint bufFlags);
  void ApplyBlitBufferGeneric (uint8* buf);

  int pboMapped;
  void* pboMapPtr;
  uint8* QueryBlitBufferPBO (int x, int y, int width, int height,
    size_t& pitch, TextureBlitDataFormat format, uint bufFlags);
  void ApplyBlitBufferPBO (uint8* buf);
protected:
  GLuint Handle;
  GLuint pbo;
  /// Upload the texture to GL.
  void Load ();
  // Needs to be done in the main thread.
  THREADED_CALLABLE_DECL(csGLBasicTextureHandle, Unload, csThreadReturn, HIGH, true, false);
  
  CS::Graphics::TextureComparisonMode texCompare;
  
  // Readback data
  TextureSourceFormat lastReadbackFormat;
  csRef<iDataBuffer> lastReadback;
  TextureSourceFormat desiredReadbackFormat;
  int desiredReadbackBPP;
  
  void SetDesiredReadbackFormat (const CS::StructuredTextureFormat&);
  template<typename Action>
  csPtr<iDataBuffer> ReadbackPerform (size_t readbackSize, Action& readbackAction);
public:
  /// The dimensions which were requested upon texture creation
  int orig_width, orig_height, orig_d;
  /// The dimensions actually used
  int actual_width, actual_height, actual_d;
  CS::Utility::ScopedPointer<csArray<csGLUploadData> > uploadData;
  csWeakRef<csGLGraphics3D> G3D;
  TextureType texType;

  iObjectRegistry* GetObjectRegistry() const;

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
  
  bool IsInFBO() const { return texFlags.Check (flagInFBO); }
  void SetInFBO (bool b) { texFlags.SetBool (flagInFBO, b); }
  
  bool IsSeamlessCubemapDisabled () const
  { return texFlags.Check (CS_TEXTURE_CUBEMAP_DISABLE_SEAMLESS); }
  
  void ReadbackFramebuffer ();
  
  /// Create a texture with given dimensions
  csGLBasicTextureHandle (int width, int height, int depth,
    csImageType imagetype, int flags, csGLGraphics3D *iG3D);
  /// Create from existing handle
  csGLBasicTextureHandle (csGLGraphics3D *iG3D, TextureType texType, GLuint Handle);

  virtual ~csGLBasicTextureHandle ();

  /**
   * Synthesize empty upload data structures for textures of the format
   * \a format. */
  bool SynthesizeUploadData (const CS::StructuredTextureFormat& format,
    iString* fail_reason, bool zeroTexture);

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
  void RegenerateMipmaps();

  virtual csAlphaMode::AlphaType GetAlphaType () const
  { return alphaType; }
  virtual void SetAlphaType (csAlphaMode::AlphaType alphaType)
  { this->alphaType = alphaType; }

  virtual void Precache ();
  virtual bool IsPrecached () { return Handle != 0; }

  virtual void SetTextureClass (const char* className);
  virtual const char* GetTextureClass ();

  virtual const char* GetImageName () const { return 0; }

  void UpdateTexture ();

  GLuint GetHandle ();
  void ChangeTextureCompareMode (
    const CS::Graphics::TextureComparisonMode& mode);

  /**
   * Return the texture target for this texture (e.g. GL_TEXTURE_2D)
   * \remark Returns GL_TEXTURE_CUBE_MAP for cubemaps.
   */
  GLenum GetGLTextureTarget() const;

  virtual TextureType GetTextureType () const
  {
    return texType;
  }

  /**
   * Ensure the texture's internal format is an uncompressed one.
   * Blitting to a compressed texture or using one as a render target may
   * incur a hefty performance penalty (due the performed compression of the
   * changed texels), so for texture intended for such use it's a good idea
   * to make sure the internal format is not compressed.
   * \param keepPixels Whether to keep the existing pixel data should be
   *   preserved.
   * \param newTexFormat New texture format the texture should have.
   * \remarks The texture handle must be bound properly before this method
   *   is called.
   */
  void EnsureUncompressed (bool keepPixels,
    TextureBlitDataFormat newTexFormat = (TextureBlitDataFormat)~0);

  uint8* QueryBlitBuffer (int x, int y, int width, int height,
    size_t& pitch, TextureBlitDataFormat format, uint bufFlags);
  void ApplyBlitBuffer (uint8* buf);
  BlitBufferNature GetBufferNature (uint8* buf);

  void SetMipmapLimits (int maxMip, int minMip = 0);
  void GetMipmapLimits (int& maxMip, int& minMip);
  
  csPtr<iDataBuffer> Readback (GLenum textarget,
    const CS::StructuredTextureFormat&, int);
  csPtr<iDataBuffer> Readback (const CS::StructuredTextureFormat&, int);
  
  /// Download texture from GL as an image
  csPtr<iImage> GetTextureFromGL ();
};

#include "csutil/deprecated_warn_on.h"

}
CS_PLUGIN_NAMESPACE_END(gl3d)

#endif // __CS_GL_TXTMGR_BASICTEX_H__

