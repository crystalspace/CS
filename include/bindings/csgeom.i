%module csgeom
%import "bindings/core.i"
%{
#include "csgeom.h"
#include "csutil.h"
#include "cstool/initapp.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/csgeom.i"
