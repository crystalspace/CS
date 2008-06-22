%module cstool
%import "bindings/core.i"
%import "bindings/iengine.i"
%import "bindings/ivaria.i"
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/cstool.i"
