%module imap
%import "bindings/core.i"
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
%import "bindings/iengine.i"
INLINE_FUNCTIONS
#endif

%include "bindings/common/imap.i"

