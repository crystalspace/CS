// IAlphaMap interface

#ifndef IALPHAIMAGE_H
#define IALPHAIMAGE_H

#include "cscom/com.h"

extern const IID IID_IAlphaMapFile;

interface IAlphaMapFile : public IUnknown
{
  DECLARE_IUNKNOWN ();

  ///
  STDMETHOD (GetAlphaMapData) (UByte** ppResult);
  ///
  STDMETHOD (GetWidth) (int& nWidth);
  ///
  STDMETHOD (GetHeight) (int& nHeight);
  ///
  STDMETHOD (GetSize) (int& nSize);
};

#endif // IIMAGE_H
