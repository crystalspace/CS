%module core
%include "bindings/common/core.i"
%import "bindings/csgeom.i" /* due to at least csVector3 required at iPath */
%import "bindings/csgfx.i" /* due to at lease csRGBpixel requierd at iImage */
