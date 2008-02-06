#ifndef _IVIDEO_I_
#define _IVIDEO_I_

%module ivideo
%import "bindings/core.i"
%import "bindings/csgfx.i"
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/ivideo.i"
#endif //_IVIDEO_I_
