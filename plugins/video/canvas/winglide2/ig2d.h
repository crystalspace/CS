
#include "cscom/com.h"

#ifndef __IG2XG2D_H__
#define __IG2XG2D_H__

extern const IID IID_IGlide2xGraphicsInfo;

#if defined(OS_LINUX)
  #define XK_MISCELLANY 1
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
#endif 

/// IGlide2xGraphicsInfo interface -- for Glide-specific properties.
interface IGlide2xGraphicsInfo : public IUnknown
{
  ///
  STDMETHOD(Open)(char* szTitle) = 0;
  ///
  STDMETHOD(Close)() = 0;
#if defined(OS_WIN32)
  ///
  STDMETHOD(GethWnd)(HWND * hWnd) = 0;
#endif
#if defined (OS_LINUX)
  STDMETHOD(GetDisplay)( Display** dpy ) = 0;
#endif    
};

#endif

