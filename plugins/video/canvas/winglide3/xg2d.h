
#include "cscom/com.h"
#include "cs2d/winglide3/IG2D.h"

#ifndef __XG3XG2D_H__
#define __XG3XG2D_H__

/// csGraphics3DGlide3x's implementation if IGlide3xGraphicsInfo
class IXGlide3xGraphicsInfo : public IGlide3xGraphicsInfo
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
};

#endif
