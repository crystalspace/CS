#ifndef _SW_IMESH_I_
#define _SW_IMESH_I_

%module imesh
%import "bindings/core.i"
%import "bindings/ivaria.i"
%import "bindings/csgeom.i"
%import "bindings/ivideo.i"
%import "bindings/iengine.i"
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/imesh.i"
#endif //_SW_IMESH_I_
