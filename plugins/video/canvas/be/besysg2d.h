#ifndef __BESYSG2D_H__
#define __BESYSG2D_H__

#include "cscom/com.h"
#include "cs2d/be/isysg2d.h"

/// FIXME: I think this is unused and can be dropped (ES)
/// csGraphics2DBe's implementation if IBeGraphicsInfo
class IXBeLibGraphicsInfo : public IBeLibGraphicsInfo
{
    DECLARE_IUNKNOWN()
    STDMETHOD(GetDisplay)( BView** dpy );
    STDMETHOD(GetWindow)( BWindow** cryst_window );
    STDMETHOD(DirectConnect)(direct_buffer_info *info);
};

#endif
