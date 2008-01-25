#ifndef _SW_CORE_I_
#define _SW_CORE_I_

%module core
%include "bindings/common/core.i"
#ifndef SWIGIMPORTED
%import "bindings/csgeom.i" /* due to at least csVector3 required at iPath */
%import "bindings/csgfx.i" /* due to at least csRGBpixel required at iImage */
#endif

#endif //_SW_CORE_I_

