// IAlphaMap interface

#ifndef IALPHAIMAGE_H
#define IALPHAIMAGE_H

#include "cscom/com.h"

extern const IID IID_IAlphaMapFile;

interface IAlphaMapFile : public IUnknown
{
  ///
  STDMETHOD (GetAlphaMapData) (UByte** ppResult) PURE;
  ///
  STDMETHOD (GetWidth) (int& nWidth) PURE;
  ///
  STDMETHOD (GetHeight) (int& nHeight) PURE;
  ///
  STDMETHOD (GetSize) (int& nSize) PURE;
};

#endif // IIMAGE_H
