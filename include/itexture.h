#include "cscom/com.h"
#include "types.h"

#ifndef ITEXTURE_H
#define ITEXTURE_H

extern const IID IID_ITextureHandle;

/** 
 * A texture handle as returned by ITextureManager.
 */
interface ITextureHandle : public IUnknown
{
  /// Set the transparent color.
  STDMETHOD (SetTransparent) (int red, int green, int blue) PURE;

  /// Get the transparent index (false if no transparency, true if transparency).
  STDMETHOD (GetTransparent) (bool& retval) PURE;

  /**
   * Get the dimensions for a given mipmap level (0 to 3).
   * This function is only valid if the texture has been registered
   * for 3D usage.
   */
  STDMETHOD (GetMipMapDimensions) (int mm, int& w, int& h) PURE;

  /**
   * Get the dimensions for the 2D texture.
   * This function is only valid if the texture has been registered
   * for 2D usage.
   */
  STDMETHOD (GetBitmapDimensions) (int& bw, int& bh) PURE;

  /**
   * Get the bitmap data for the 2D texture.
   * This function is only valid if the texture has been registered
   * for 2D usage.
   */
  STDMETHOD (GetBitmapData) (void** bmdata) PURE;

  /// Get the mean color as an index.
  STDMETHOD (GetMeanColor) (int& retval) PURE;

  /// Get the mean color.
  STDMETHOD (GetMeanColor) (float& r, float& g, float& b) PURE;

  /// Returns the number of colors in this texture.
  STDMETHOD (GetNumberOfColors) (int& retval) PURE;
};

#endif //ITEXTURE_H
