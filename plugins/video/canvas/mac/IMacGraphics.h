#ifndef __IMACGRAPHICS_H__
#define __IMACGRAPHICS_H__

#include <Events.h>
#include "csutil/scf.h"

SCF_VERSION (iMacGraphics, 0, 0, 1);

/// iMacGraphicsInfo interface -- for Mac-specific properties.
struct iMacGraphics : public iBase
{
    virtual void ActivateWindow( WindowPtr theWindow, bool active ) = 0;
    ///
    virtual bool UpdateWindow( WindowPtr theWindow ) = 0;
    ///
    virtual bool PointInWindow( Point *thePoint ) = 0;
    ///
    virtual bool DoesDriverNeedEvent( void ) = 0;
    ///
    virtual void SetColorPalette( void ) = 0;
    ///
    virtual void WindowChanged( void ) = 0;
    ///
    virtual bool HandleEvent( EventRecord *inEvent ) = 0;
    ///
    virtual void PauseDisplayContext( void ) = 0;
    ///
    virtual void ActivateDisplayContext( void ) = 0;
};

#endif /* __IMACGRAPHICS_H__ */
