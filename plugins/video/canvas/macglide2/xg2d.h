
#include "cscom/com.h"
#include "cs2d/macglide2/ig2d.h"

#ifndef __XG2XG2D_H__
#define __XG2XG2D_H__

/// csGraphics2DGlide's implementation of IGlide2xGraphicsInfo
class IXGlide2xGraphicsInfo : public IGlide2xGraphicsInfo
{
  DECLARE_IUNKNOWN()
  ///
  STDMETHOD(Open)(char* szTitle);
  ///
  STDMETHOD(Close)();
#if defined(OS_WIN32)
  ///
  STDMETHOD(GethWnd)(HWND * hWnd);
#endif
#if defined (OS_LINUX)
  STDMETHOD(GetDisplay)( Display** dpy );
#endif
};

#endif
