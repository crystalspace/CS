#ifndef _SW_CSGEOM_I_
#define _SW_CSGEOM_I_
%module csgeom
%import "bindings/core.i"
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/csgeom.i"
#endif //_SW_CSGEOM_I_

