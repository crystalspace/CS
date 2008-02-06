%module iengine
%import "bindings/core.i"
%import "bindings/ivideo.i" /* due to at least csZBufMode graph3d.h */
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/iengine.i"

