
#include <ddraw.h>
#include "cscom/com.h"
#include "cssys/win32/iddetect.h"

#ifndef __IDD3G2D_H__
#define __IDD3G2D_H__

extern const IID IID_IDDraw3GraphicsInfo;

/// IDDraw3GraphicsInfo interface -- for win32 DDraw-specific properties.
interface IDDraw3GraphicsInfo : public IUnknown
{
    ///
    STDMETHOD(Open)(const char* szTitle) = 0;
    ///
    STDMETHOD(Close)() = 0;
    ///
    STDMETHOD(GetDirectDrawDriver)( LPDIRECTDRAW* lplpDirectDraw ) = 0;
    ///
    STDMETHOD(GetDirectDrawPrimary)( LPDIRECTDRAWSURFACE* lplpDirectDrawPrimary ) = 0;
    ///
    STDMETHOD(GetDirectDrawBackBuffer)( LPDIRECTDRAWSURFACE* lplpDirectDrawBackBuffer ) = 0;
    ///
    STDMETHOD(GetDirectDetection)(IDirectDetectionInternal** lplpDDetection ) = 0;
    ///
    STDMETHOD(SetColorPalette)(void) = 0;
};

#endif
