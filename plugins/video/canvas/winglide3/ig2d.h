
#include "cscom/com.h"

#ifndef __IG3XG2D_H__
#define __IG3XG2D_H__

extern const IID IID_IGlide3xGraphicsInfo;

/// IGlide3xGraphicsInfo interface -- for Glide-specific properties.
interface IGlide3xGraphicsInfo : public IUnknown
{
  ///
  STDMETHOD(Open)(char* szTitle) = 0;
  ///
  STDMETHOD(Close)() = 0;
#if defined(OS_WIN32)
  ///
  STDMETHOD(GethWnd)(HWND * hWnd) = 0;
#endif
};

#endif
