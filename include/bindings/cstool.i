%module cstool
%import "bindings/core.i"
%import "bindings/iengine.i"
%import "bindings/ivaria.i"
%{
#include "csgeom.h"
#include "csgfx.h"
#include "csutil.h"
#include "cstool.h"
#include "iengine.h"
#include "ivaria.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/cstool.i"
