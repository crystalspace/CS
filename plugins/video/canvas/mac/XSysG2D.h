
#include <QDOffscreen.h>
#include "cscom/com.h"
#include "ISysG2D.h"

#ifndef __XSYSG2D_H__
#define __XSYSG2D_H__

/// csGraphics2DMac's implementation of IMacGraphicsInfo
class IXMacGraphicsInfo : public IMacGraphicsInfo
{
    DECLARE_IUNKNOWN()
    ///
    STDMETHOD(Open)(char* szTitle);
    ///
    STDMETHOD(Close)();
    ///
    STDMETHOD(ActivateWindow)( WindowPtr theWindow, bool active );
    ///
    STDMETHOD(UpdateWindow)( WindowPtr theWindow, bool *updated );
    ///
    STDMETHOD(PointInWindow)( Point *thePoint, bool *inWindow );
    ///
    STDMETHOD(IsDrawSprocketsEnabled)( bool *isEnabled );
    ///
    STDMETHOD(SetColorPalette)( void );
};

#endif
