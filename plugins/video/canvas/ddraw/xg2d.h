#include <ddraw.h>
#include "cscom/com.h"
#include "cs2d/ddraw/IG2D.h"

#ifndef __XDD3G2D_H__
#define __XDD3G2D_H__

/// csGraphics2DDDraw3's implementation if IDDraw3GraphicsInfo
class IXDDraw3GraphicsInfo : public IDDraw3GraphicsInfo
{
    DECLARE_IUNKNOWN()
    ///
    STDMETHOD(Open)(char* szTitle);
    ///
    STDMETHOD(Close)();
    ///
    STDMETHOD(GetDirectDrawDriver)( LPDIRECTDRAW* lplpDirectDraw );
    ///
    STDMETHOD(GetDirectDrawPrimary)( LPDIRECTDRAWSURFACE* lplpDirectDrawPrimary );
    ///
    STDMETHOD(GetDirectDrawBackBuffer)( LPDIRECTDRAWSURFACE* lplpDirectDrawBackBuffer );
    ///
    STDMETHOD(SetColorPalette)(void);
    ///
    STDMETHOD(GetDirectDetection)(IDirectDetectionInternal** lplpDDetection );
};

#endif
