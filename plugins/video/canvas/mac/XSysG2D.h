
#include <QDOffscreen.h>
#include "csutil/scf.h"
#include "ISysG2D.h"

#ifndef __XSYSG2D_H__
#define __XSYSG2D_H__

/// csGraphics2DMac's implementation of iMacGraphicsInfo
class IXMacGraphicsInfo : public iMacGraphicsInfo
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
    STDMETHOD(DoesDriverNeedEvent)( bool *isEnabled );
    ///
    STDMETHOD(SetColorPalette)( void );
    ///
    STDMETHOD(WindowChanged)( void );
    ///
    STDMETHOD(HandleEvent)( EventRecord *inEvent, bool *outEventWasProcessed );
};

#endif
