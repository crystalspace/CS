%module csgfx
%import "bindings/core.i"
%import "bindings/iengine.i" /* due to at least iTextureWrapper */
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/csgfx.i"
%import "bindings/ivideo.i" /* due to at least iTextureHandle */
