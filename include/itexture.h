#include "csutil/scf.h"
#include "types.h"

#ifndef ITEXTURE_H
#define ITEXTURE_H

struct csHighColorCacheData;

SCF_VERSION (iTextureHandle, 1, 0, 0);

/** 
 * A texture handle as returned by iTextureManager.
 */
struct iTextureHandle : public iBase
{
  /// Set the transparent color.
  virtual void SetTransparent (int red, int green, int blue) = 0;

  /// Get the transparent status (false if no transparency, true if transparency).
  virtual bool GetTransparent () = 0;

  /// Get the transparent color
  virtual void GetTransparent (int &red, int &green, int &blue) = 0;

  /**
   * Get the dimensions for a given mipmap level (0 to 3).
   * This function is only valid if the texture has been registered
   * for 3D usage.
   */
  virtual void GetMipMapDimensions (int mm, int& w, int& h) = 0;

  /**
   * Get the bitmap data for the given mipmap.
   * This function is not always available: it depends on implementation.
   */
  virtual void *GetMipMapData (int mm) = 0;

  /**
   * Get the dimensions for the 2D texture.
   * This function is only valid if the texture has been registered
   * for 2D usage.
   */
  virtual void GetBitmapDimensions (int& bw, int& bh) = 0;

  /**
   * Get the bitmap data for the 2D texture.
   * This function is only valid if the texture has been registered
   * for 2D usage.
   */
  virtual void *GetBitmapData () = 0;

  /// Get the mean color.
  virtual void GetMeanColor (float& r, float& g, float& b) = 0;

  /// Get the mean color index
  virtual int GetMeanColor () = 0;

  /// Used internally by the 2D driver
  virtual csHighColorCacheData *GetHighColorCacheData () = 0;

  ///
  virtual void SetHighColorCacheData (csHighColorCacheData *d) = 0;

  /// Query whenever the texture is in texture cache
  virtual bool IsCached () = 0;

  /// Set "in-cache" state
  virtual void SetInCache (bool InCache) = 0;

  /**
   * Query the private object (3D driver internal use) associated with
   * this handle. This is somewhat ugly, how do you think?
   */
  virtual void *GetPrivateObject () = 0;
};

#endif //ITEXTURE_H
