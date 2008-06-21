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

#if defined(SWIGCSHARP) && !defined(SWIGIMPORTED)
%import "bindings/iengine.i" /* due to at least iMaterialWrapper*/
#endif
%include "bindings/common/imesh.i"
