
#include "cscom/com.h"
#include "cs2d/openglwin/ISysG2D.h"

#ifndef __XSYSG2D_H__
#define __XSYSG2D_H__

/// csGraphics2DOpenGL's implementation if IOpenGLGraphicsInfo
class IXOpenGLGraphicsInfo : public IOpenGLGraphicsInfo
{
    DECLARE_IUNKNOWN()
    ///
    STDMETHOD(Open)(char* szTitle);
    ///
    STDMETHOD(Close)();
    ///
    STDMETHOD(SetColorPalette)(void);
};

#endif
