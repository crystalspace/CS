// IImageFile interface

#ifndef IIMAGE_H
#define IIMAGE_H

#include "cscom/com.h"
#include "csgfxldr/rgbpixel.h"	//@@@BAD?

struct Filter3x3;
struct Filter5x5;

extern const IID IID_IImageFile;

interface IImageFile : public IUnknown
{
  DECLARE_IUNKNOWN ();

  ///
  STDMETHOD (GetImageData) (RGBPixel** ppResult);
  ///
  STDMETHOD (GetWidth) (int& nWidth);
  ///
  STDMETHOD (GetHeight) (int& nHeight);
  ///
  STDMETHOD (GetSize) (int& nSize);
  ///
  STDMETHOD (MipMap) (int steps, Filter3x3* filt1, Filter5x5* filt2, IImageFile** nimage);
  ///
  STDMETHOD (MipMap) (int steps, IImageFile** nimage);
  ///
  STDMETHOD (Blend) (Filter3x3* filter, IImageFile** nimage);
};

#endif // IIMAGE_H
