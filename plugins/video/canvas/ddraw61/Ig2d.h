#include <ddraw.h>
#include "cscom/com.h"
#include "cssys/win32/iddetect.h"

#ifndef __IDD6G2D_H__
#define __IDD6G2D_H__

extern const IID IID_IDDraw6GraphicsInfo;

/// IDDraw6GraphicsInfo interface -- for Win32-specific properties.
interface IDDraw6GraphicsInfo : public IUnknown
{
    ///
    STDMETHOD(Open)(char* szTitle) = 0;
    ///
    STDMETHOD(Close)() = 0;
    ///
//    STDMETHOD(GetDirectDrawDriver)( LPDIRECTDRAW* lplpDirectDraw ) = 0;
    STDMETHOD(GetDirectDrawDriver)( LPDIRECTDRAW4* lplpDirectDraw ) = 0;
    ///
    STDMETHOD(GetDirectDrawPrimary)( LPDIRECTDRAWSURFACE4* lplpDirectDrawPrimary ) = 0;
    ///
    STDMETHOD(GetDirectDrawBackBuffer)( LPDIRECTDRAWSURFACE4* lplpDirectDrawBackBuffer ) = 0;
    ///
    STDMETHOD(GetDirectDetection)(IDirectDetectionInternal** lplpDDetection ) = 0;
    ///
    STDMETHOD(SetColorPalette)(void) = 0;
};

#endif
