%module imesh
%import "bindings/core.i"
%import "bindings/ivaria.i"
%import "bindings/csgeom.i"
%import "bindings/ivideo.i"
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/imesh.i"
