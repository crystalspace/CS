#ifndef __BESYSG2D_H__
#define __BESYSG2D_H__

#include "cscom/com.h"
#include "cs2d/be/isysg2d.h"

// {13ECE929-6927-11d2-9C9A-90EB4EC10203}
//extern const GUID IID_IBeLibGraphicsInfo =
//{ 0x13ece929, 0x6927, 0x11d2, { 0x9c, 0x9a, 0x90, 0xeb, 0x4e, 0xc1, 0x2, 0x3 } };

/// csGraphics2DBe's implementation if IBeGraphicsInfo
class IXBeLibGraphicsInfo : public IBeLibGraphicsInfo
{
    DECLARE_IUNKNOWN()
    ///
    STDMETHOD(GetDisplay)( BView** dpy );
    STDMETHOD(GetWindow)( BWindow** cryst_window );
    STDMETHOD(DirectConnect)(direct_buffer_info *info);
};

#endif
