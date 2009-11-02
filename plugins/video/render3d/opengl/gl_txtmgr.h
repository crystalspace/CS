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

#ifndef __CS_GL_NEWTXTMGR_H__
#define __CS_GL_NEWTXTMGR_H__

#include "csgfx/textureformatstrings.h"
#include "csutil/genericresourcecache.h"
#include "csutil/threadmanager.h"
#include "csutil/weakrefarr.h"

#include "iutil/cfgfile.h"
#include "ivideo/txtmgr.h"

#include "gl_txtmgr_basictex.h"
#include "pbo.h"

struct iImageIO;

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

class csGLGraphics3D;
class csGLTextureHandle;
class csGLTextureManager;

struct csGLTextureClassSettings
{
  GLenum formatRGB;
  GLenum formatRGBA;
  bool sharpenPrecomputedMipmaps;
  bool forceDecompress;
  bool allowDownsample;
  bool allowMipSharpen;
  bool renormalizeGeneratedMips;
};

/*
*
* New Texture Manager... done by Phil Aumayr (phil@rarebyte.com)
*
*/
class csGLTextureManager : 
  public scfImplementation1<csGLTextureManager,
			    iTextureManager>
{
private:
  typedef csWeakRefArray<csGLBasicTextureHandle> csTexVector;
  /// List of textures.
  csTexVector textures;
  bool compactTextures;

  /// Lock on textures vector.
  CS::Threading::Mutex texturesLock;

  csStringSet textureClassIDs;
  csHash<csGLTextureClassSettings, csStringID> textureClasses;
  struct TextureFormat
  {
    GLenum format;
    bool supported;

    TextureFormat (GLenum fmt, bool supp) : format (fmt), supported (supp) {}
  };
  csHash<TextureFormat, csString> textureFormats;

  GLenum ParseTextureFormat (const char* formatName, GLenum defaultFormat);
  void ReadTextureClasses (iConfigFile* config);

  iObjectRegistry *object_reg;

  // DetermineGLFormat helpers
  csHash<TextureStorageFormat, csString> specialFormats;
  void InitFormats ();
  bool FormatSupported (GLenum srcFormat, GLenum srcType);

  void CompactTextures ();
  
  bool ImageTypeSupported (csImageType imagetype, iString* fail_reason);
public:
  /* Format tables - maps component sizes to GL sizes.
   * A lot of source formats have the same 'type' bit only differ in 'format'.
   * So the tables below store the 'type' information for given component sizes
   * and the 'format' is chosen based on the input component order.
   */
  struct FormatTemplate
  {
    /// Component sizes
    int size[4];
    /// Target format index
    int targetFmtIndex;
    /// Source type
    GLenum srcType;
  };
private:
  bool FindFormats (const CS::StructuredTextureFormat& format,
    const FormatTemplate* templates, GLenum const* targetTable,
    int compCount, GLenum targetFormat, GLenum sourceFormat, 
    TextureStorageFormat& glFormat, TextureSourceFormat& srcFormat);

  bool DetermineIntegerFormat (const CS::StructuredTextureFormat& format,
    TextureStorageFormat& glFormat, TextureSourceFormat& sourceFormat);
  bool DetermineFloatFormat (const CS::StructuredTextureFormat& format,
    TextureStorageFormat& glFormat, TextureSourceFormat& sourceFormat);
public:
  CS_LEAKGUARD_DECLARE (csGLTextureManager);

  csWeakRef<csGLGraphics3D> G3D;

  int max_tex_size;
  /// Sharpen mipmaps?
  int sharpen_mipmaps;
  /// downsample textures?
  int texture_downsample;
  /// texture filtering anisotropy
  float texture_filter_anisotropy;
  /// Whether bilinear filtering should be used (0 = no, 1 = yes, 2 = trilinear)
  int rstate_bilinearmap;
  
  struct
  {
    /**
    * Whether to prevent uploading of NPOTS textures to a generic compressed 
    * format (causes crashes on some drivers).
    */
    bool disableRECTTextureCompression;
    /**
    * Whether to enable uploading of NPOTS textures as 2D textures.
    * Some ATI hardware (Radeon 9500+) has a "hidden" feature where you can 
    * specify NPOTS textures as 2D textures. (Normally they would have to be 
    * POTS.) 
    */
    bool enableNonPowerOfTwo2DTextures;
  
    /// Some drivers seem to ignore glGenerateMipmap calls
    bool disableGenerateMipmap;
    
    /**
     * Workaround a bug in NV OpenGL (169.12): when using GENERATE_MIPMAPS
     * the max lod level isn't generated but is black.
     * Solution: set max LOD level to desired level plus one.
     */
    bool generateMipMapsExcessOne;
  } tweaks;
    
  TextureReadbackSimple::Pool simpleTextureReadbacks;
  TextureReadbackPBO::Pool pboTextureReadbacks;
  
  typedef CS::Utility::GenericResourceCache<csRef<PBOWrapper>,
    uint, CacheSorting::PBO, 
    CS::Utility::ResourceCache::ReuseIfOnlyOneRef> PBOCache;
  PBOCache pboCache;
  csRef<PBOWrapper> GetPBOWrapper (size_t pboSize);

  csGLTextureManager (iObjectRegistry* object_reg,
        iGraphics2D* iG2D, iConfigFile *config,
        csGLGraphics3D *G3D);

  virtual ~csGLTextureManager ();

  void NextFrame (uint frameNum);

  /// Read configuration values from config file.
  void read_config (iConfigFile *config);
  void Clear ();

  const csGLTextureClassSettings* GetTextureClassSettings (csStringID texclass);
  csStringID GetTextureClassID (const char* className)
  {
    return textureClassIDs.Request (className);
  }
  const char* GetTextureClassName (csStringID classID)
  {
    return textureClassIDs.Request (classID);
  }
  
  /**
   * Helper function to make sure a texture isn't selected on any TU.
   * Useful when deleting a texture.
   */
  static void UnsetTexture (GLenum target, GLuint texture);


  /**
   * Determine the GL texture format for a structured texture format.
   */
  bool DetermineGLFormat (const CS::StructuredTextureFormat& format,
    TextureStorageFormat& glFormat, TextureSourceFormat& sourceFormat);


  virtual csPtr<iTextureHandle> RegisterTexture (iImage *image, int flags,
      iString* fail_reason = 0);
  virtual csPtr<iTextureHandle> CreateTexture (int w, int h,
      csImageType imagetype, const char* format, int flags,
      iString* fail_reason = 0)
  {
    return csGLTextureManager::CreateTexture (w, h, 1, imagetype, format,
      flags, fail_reason);
  }
  virtual csPtr<iTextureHandle> CreateTexture (int w, int h, int d,
      csImageType imagetype, const char* format, int flags,
      iString* fail_reason = 0);
  void MarkTexturesDirty () { compactTextures = true; }

  /**
   * Query the basic format of textures that can be registered with this
   * texture manager. It is very likely that the texture manager will
   * reject the texture if it is in an improper format. The alpha channel
   * is optional; the texture can have it and can not have it. Only the
   * bits that fit the CS_IMGFMT_MASK mask matters.
   */
  virtual int GetTextureFormat ();

  virtual void GetMaxTextureSize (int& w, int& h, int& aspect);

  void DumpTextures (iVFS* VFS, iImageIO* iio, const char* dir);

  iObjectRegistry* GetObjectRegistry() const { return object_reg; }
};

}
CS_PLUGIN_NAMESPACE_END(gl3d)

#endif // __CS_GL_TXTMGR_H__

