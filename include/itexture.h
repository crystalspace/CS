#include "csutil/scf.h"
#include "types.h"

#ifndef ITEXTURE_H
#define ITEXTURE_H

SCF_VERSION (iTextureHandle, 2, 0, 0);

/** 
 * A texture handle as returned by iTextureManager.
 */
struct iTextureHandle : public iBase
{
  /// Enable transparent color
  virtual void SetTransparent (bool Enable) = 0;

  /// Set the transparent color.
  virtual void SetTransparent (UByte red, UByte green, UByte blue) = 0;

  /// Get the transparent status (false if no transparency, true if transparency).
  virtual bool GetTransparent () = 0;

  /// Get the transparent color
  virtual void GetTransparent (UByte &red, UByte &green, UByte &blue) = 0;

  /**
   * Get the dimensions for a given mipmap level (0 to 3).
   * If the texture was registered just for 2D usage, mipmap levels above
   * 0 will return false.
   */
  virtual bool GetMipMapDimensions (int mipmap, int &mw, int &mh) = 0;

  /// Get the mean color.
  virtual void GetMeanColor (UByte &red, UByte &green, UByte &blue) = 0;

  /// Get data associated internally with this texture by texture cache
  virtual void *GetCacheData () = 0;

  /// Set data associated internally with this texture by texture cache
  virtual void SetCacheData (void *d) = 0;

  /**
   * Query the private object associated with this handle.
   * For internal usage by the 3D driver.
   */
  virtual void *GetPrivateObject () = 0;
};

#endif //ITEXTURE_H
