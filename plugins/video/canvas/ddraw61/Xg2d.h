#include <ddraw.h>
#include "cscom/com.h"
#include "cs2d/ddraw61/IG2D.h"

#ifndef __XDD61G2D_H__
#define __XDD61G2D_H__

/// csGraphics2DDDraw6's implementation if IDDraw6GraphicsInfo
interface IXDDraw6GraphicsInfo : public IDDraw6GraphicsInfo
{
    DECLARE_IUNKNOWN()
    ///
    STDMETHOD(Open)(char* szTitle);
    ///
    STDMETHOD(Close)();
    ///
//    STDMETHOD(GetDirectDrawDriver)( LPDIRECTDRAW* lplpDirectDraw );
    STDMETHOD(GetDirectDrawDriver)( LPDIRECTDRAW4* lplpDirectDraw );
    ///
    STDMETHOD(GetDirectDrawPrimary)( LPDIRECTDRAWSURFACE4* lplpDirectDrawPrimary );
    ///
    STDMETHOD(GetDirectDrawBackBuffer)( LPDIRECTDRAWSURFACE4* lplpDirectDrawBackBuffer );
    ///
    STDMETHOD(SetColorPalette)(void);
    ///
    STDMETHOD(GetDirectDetection)(IDirectDetectionInternal** lplpDDetection );
};

#endif
