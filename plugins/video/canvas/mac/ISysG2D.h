#ifndef __ISYSG2D_H__
#define __ISYSG2D_H__

#include <QDOffscreen.h>
#include <Events.h>
#include "csutil/scf.h"

/// iMacGraphicsInfo interface -- for Mac-specific properties.
SCF_INTERFACE (iMacGraphicsInfo, 0, 0, 1) : public iBase
{
    ///
    STDMETHOD(Open)(char* szTitle) = 0;
    ///
    STDMETHOD(Close)() = 0;
    ///
    STDMETHOD(ActivateWindow)( WindowPtr theWindow, bool active ) = 0;
    ///
    STDMETHOD(UpdateWindow)( WindowPtr theWindow, bool *updated ) = 0;
    ///
    STDMETHOD(PointInWindow)( Point *thePoint, bool *inWindow ) = 0;
    ///
    STDMETHOD(DoesDriverNeedEvent)( bool *enabled ) = 0;
    ///
    STDMETHOD(SetColorPalette)( void ) = 0;
    ///
    STDMETHOD(WindowChanged)( void ) = 0;
    ///
    STDMETHOD(HandleEvent)( EventRecord *inEvent, bool *outEventWasProcessed ) = 0;
};

#endif
