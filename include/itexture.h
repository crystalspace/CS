#include "cscom/com.h"
#include "types.h"
#include "csgfxldr/csimage.h"	//@@@BAD

#ifndef ITEXTURE_H
#define ITEXTURE_H

extern const IID IID_ITextureHandle;

/** 
 * A texture handle as returned by ITextureManager.
 */
interface ITextureHandle : public IUnknown
{
  DECLARE_IUNKNOWN()

  /// Set the transparent color.
  STDMETHOD (SetTransparent) (int red, int green, int blue);

  /// Get the transparent index (-1 if no transparency, 0 if transparency).
  STDMETHOD (GetTransparent) (int& retval);

  /**
   * Get the dimensions for a given mipmap level (0 to 3).
   * This function is only valid if the texture has been registered
   * for 3D usage.
   */
  STDMETHOD (GetMipMapDimensions) (int mm, int& w, int& h);

  /**
   * Get the dimensions for the 2D texture.
   * This function is only valid if the texture has been registered
   * for 2D usage.
   */
  STDMETHOD (GetBitmapDimensions) (int& bw, int& bh);

  /**
   * Get the bitmap data for the 2D texture.
   * This function is only valid if the texture has been registered
   * for 2D usage.
   */
  STDMETHOD (GetBitmapData) (void** bmdata);

  /// Get the mean color as an index.
  STDMETHOD (GetMeanColor) (int& retval);

  /// Get the mean color.
  STDMETHOD (GetMeanColor) (float& r, float& g, float& b);

  /// Returns the number of colors in this texture.
  STDMETHOD (GetNumberOfColors) (int& retval);
};

#endif //ITEXTURE_H
