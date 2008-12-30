/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Texture manager for software renderer

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

#ifndef __CS_NULL_TXT_H__
#define __CS_NULL_TXT_H__

#include "igraphic/image.h"
#include "iutil/databuff.h"
#include "iutil/string.h"
#include "ivideo/shader/shader.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "csgfx/rgbpixel.h"
#include "csutil/scf_implementation.h"
#include "csutil/weakrefarr.h"

class csTextureManagerNull;
struct iObjectRegistry;
struct iConfigFile;

// For GetTextureTarget ()
#include "csutil/deprecated_warn_off.h"

/**
 * csTextureHandleNull represents a texture and all its mipmapped
 * variants.
 */
class csTextureHandleNull : public scfImplementation1<csTextureHandleNull,
                                                      iTextureHandle>
{
protected:
  /// Texture usage flags: 2d/3d/etc
  int flags;

  /// Does color 0 mean "transparent" for this texture?
  bool transp;
  /// The transparent color
  csRGBpixel transp_color;

  csStringID texClass;

  csString imageName;

  /// Is already prepared.
  bool prepared;

  /// The texture manager
  csRef<csTextureManagerNull> texman;

  int w, h, d;
  int orig_w, orig_h, orig_d;
  
  void AdjustSizePo2 (int, int, int, int&, int&, int&);
  void CalculateNextBestPo2Size (int texFlags, const int orgDim, int& newDim);
public:
  /// Create the mipmapped texture object
  csTextureHandleNull (csTextureManagerNull *txtmgr, iImage *image, int flags);
  csTextureHandleNull (csTextureManagerNull *txtmgr, int w, int h, int d,
    int flags);
  /// Destroy the object and free all associated storage
  virtual ~csTextureHandleNull ();

  int GetFlags () const { return flags; }

  /// Enable transparent color
  virtual void SetKeyColor (bool Enable);

  /// Set the transparent color.
  virtual void SetKeyColor (uint8 red, uint8 green, uint8 blue);

  /**
   * Get the transparent status (false if no transparency, true if
   * transparency).
   */
  virtual bool GetKeyColor () const;

  /// Get the transparent color
  virtual void GetKeyColor (uint8 &r, uint8 &g, uint8 &b) const;

  csAlphaMode::AlphaType GetAlphaType () const { return csAlphaMode::alphaNone; }
  virtual void SetAlphaType (csAlphaMode::AlphaType alphaType) { }

  virtual void SetTextureClass (const char* className);
  virtual const char* GetTextureClass ();

  void Precache () { }
  bool IsPrecached () { return true; }
  bool GetRendererDimensions (int &mw, int &mh)
  { mw = w; mh = h; return true; }
  bool GetRendererDimensions (int &mw, int &mh, int &md)
  { mw = w; mh = h; md = d; return true; }
  void GetOriginalDimensions (int& mw, int& mh, int& md)
  { mw = orig_w; mh = orig_h; md = orig_d; }
  void GetOriginalDimensions (int& mw, int& mh)
  { mw = orig_w; mh = orig_h; }
  const char* GetImageName () const { return imageName; }
  virtual void Blit (int, int, int, int, unsigned char const*,
    TextureBlitDataFormat) {}
  virtual TextureType GetTextureType () const { return texType2D; }

  uint8* QueryBlitBuffer (int x, int y, int width, int height,
    size_t& pitch, TextureBlitDataFormat format, uint bufFlags)
  {
    pitch = width * 4;
    return (uint8*)cs_malloc (height * pitch);
  }
  void ApplyBlitBuffer (uint8* buf) { cs_free (buf); }
  BlitBufferNature GetBufferNature (uint8* buf) { return natureDirect; }
  
  void SetMipmapLimits (int maxMip, int minMip = 0) {}
  void GetMipmapLimits (int& maxMip, int& minMip)
  { maxMip = 1000; minMip = 0; }
  
  csPtr<iDataBuffer> Readback (const CS::StructuredTextureFormat&, int)
  { return 0; }
};

#include "csutil/deprecated_warn_on.h"

/**
 * Software version of the texture manager. This instance of the
 * texture manager is probably the most involved of all 3D rasterizer
 * specific texture manager implementations because it needs to do
 * a lot of work regarding palette management and the creation
 * of lots of lookup tables.
 */
class csTextureManagerNull :
  public scfImplementation1<csTextureManagerNull,
                            iTextureManager>
{
private:
  /// We need a pointer to the 2D driver
  iGraphics2D *G2D;

  typedef csWeakRefArray<csTextureHandleNull> csTexVector;

  /// Lock on textures vector.
  CS::Threading::Mutex texturesLock;

  /// List of textures.
  csTexVector textures;

  /// Clear (free) all textures
  void Clear ()
  {
    CS::Threading::MutexScopedLock lock(texturesLock);
    textures.DeleteAll ();
  }
public:
  CS::ShaderVarStringID nameDiffuseTexture;

  csStringSet texClassIDs;
  
  ///
  csTextureManagerNull (iObjectRegistry *object_reg,
  	iGraphics2D *iG2D, iConfigFile *config);
  ///
  virtual ~csTextureManagerNull ();

  int GetTextureFormat ()
  { return CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA; }

  virtual csPtr<iTextureHandle> RegisterTexture (iImage* image, int flags,
      iString* fail_reason = 0);
  virtual csPtr<iTextureHandle> CreateTexture (int w, int h,
      csImageType imagetype, const char* format, int flags,
      iString* fail_reason = 0)
  {
    return csTextureManagerNull::CreateTexture (w, h, 1, imagetype, format,
      flags, fail_reason);
  }
  virtual csPtr<iTextureHandle> CreateTexture (int w, int h, int d,
      csImageType imagetype, const char* format, int flags,
      iString* fail_reason = 0);
  virtual void UnregisterTexture (csTextureHandleNull* handle);

  virtual void GetMaxTextureSize (int& w, int& h, int& aspect);
};

#endif // __CS_NULL_TXT_H__
