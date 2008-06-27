%module ivaria
%import "bindings/core.i"
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

#if defined(SWIGCSHARP) && !defined(SWIGIMPORTED)
%import "bindings/iengine.i" /* due to at least iMaterialWrapper*/
#endif
%include "bindings/common/ivaria.i"

