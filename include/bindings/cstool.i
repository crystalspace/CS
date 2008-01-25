#ifndef _SW_CSTOOL_I_
#define _SW_CSTOOL_I_

%module cstool
%import "bindings/core.i"
%import "bindings/iengine.i"
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/cstool.i"
#endif //_SW_CSTOOL_I_

