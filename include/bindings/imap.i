%module imap
%import "bindings/core.i"
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

#if defined(SWIGCSHARP) && !defined(SWIGIMPORTED)
%import "bindings/iengine.i"
#endif
%include "bindings/common/imap.i"

