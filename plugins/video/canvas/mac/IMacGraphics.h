#ifndef __IMACGRAPHICS_H__
#define __IMACGRAPHICS_H__

#include <QDOffscreen.h>
#include <Events.h>
#include "csutil/scf.h"

SCF_VERSION (iMacGraphics, 0, 0, 1);

/// iMacGraphicsInfo interface -- for Mac-specific properties.
struct iMacGraphics : public iBase
{
    virtual void ActivateWindow( WindowPtr theWindow, bool active ) = 0;
    ///
    virtual void UpdateWindow( WindowPtr theWindow, bool *updated ) = 0;
    ///
    virtual void PointInWindow( Point *thePoint, bool *inWindow ) = 0;
    ///
    virtual void DoesDriverNeedEvent( bool *enabled ) = 0;
    ///
    virtual void SetColorPalette( void ) = 0;
    ///
    virtual void WindowChanged( void ) = 0;
    ///
    virtual void HandleEvent( EventRecord *inEvent, bool *outEventWasProcessed ) = 0;
};

#endif /* __IMACGRAPHICS_H__ */
