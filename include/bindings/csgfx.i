%module csgfx
%import "bindings/core.i"
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/csgfx.i"
#if defined(SWIGCSHARP) && !defined(SWIGIMPORTED)
%import "bindings/iengine.i" /* due to at least iTextureWrapper */
%import "bindings/ivideo.i" /* due to at least iTextureHandle */
#endif
