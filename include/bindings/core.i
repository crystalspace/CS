%module core
%include "bindings/common/core.i"
#if defined(SWIGCSHARP) && !defined(SWIGIMPORTED)
%import "bindings/csgeom.i" /* due to at least csVector3 required at iPath */
%import "bindings/csgfx.i" /* due to at least csRGBpixel required at iImage */
#endif

