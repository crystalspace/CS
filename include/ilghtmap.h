
// ILightMap interface

#include "cscom/com.h"

#ifndef __ILIGHTMAP_H__
#define __ILIGHTMAP_H__

extern const IID IID_ILightMap;
struct HighColorCache_Data;

interface ILightMap : public IUnknown
{
  DECLARE_IUNKNOWN ();

  ///
  STDMETHOD (GetMap) (int nMap, unsigned char** ppResult);
  ///
  STDMETHOD (GetWidth) (int& nWidth);
  ///
  STDMETHOD (GetHeight) (int& nHeight);
  ///
  STDMETHOD (GetRealWidth) (int& nWidth);
  ///
  STDMETHOD (GetRealHeight) (int& nHeight);
  ///
  STDMETHOD (GetInVideoMemory) (bool& bVal);
  ///
  STDMETHOD (GetHighColorCache) (HighColorCache_Data** pVal);
  ///
  STDMETHOD (SetInVideoMemory) (bool bVal);
  ///
  STDMETHOD (SetHighColorCache) (HighColorCache_Data* pVal);
  ///
  STDMETHOD (GetMeanLighting) (int& r, int& g, int& b);
};

#endif
