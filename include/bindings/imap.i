#ifndef _SW_IMAP_I_
#define _SW_IMAP_I_

%module imap
%import "bindings/core.i"
%import "bindings/iengine.i"
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/imap.i"
#endif //_SW_IMAP_I_
