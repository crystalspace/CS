#ifndef _SW_ISNDSYS_I_
#define _SW_ISNDSYS_I_

%module isndsys
%import "bindings/core.i"
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/isndsys.i"
#endif //_SW_ISNDSYS_I_

