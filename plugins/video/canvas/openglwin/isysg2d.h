
#include "cscom/com.h"

#ifndef __ISYSG2D_H__
#define __ISYSG2D_H__

extern const IID IID_IOpenGLGraphicsInfo;

/// IOpenGLGraphicsInfo interface -- for Win32-specific properties.
interface IOpenGLGraphicsInfo : public IUnknown
{
    ///
    STDMETHOD(Open)(const char* szTitle) = 0;
    ///
    STDMETHOD(Close)() = 0;
    ///
    STDMETHOD(SetColorPalette)(void) = 0;
};

#endif
