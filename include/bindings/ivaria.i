#ifndef _IVARIA_I_
#define _IVARIA_I_

%module ivaria
%import "bindings/core.i"
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/ivaria.i"
#endif //_IVARIA_I_

