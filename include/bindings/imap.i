%module imap
%import "bindings/core.i"
%import "bindings/iengine.i"
%import "bindings/isndsys.i"
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/imap.i"

