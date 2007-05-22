%module imap
%import "bindings/cspace.i"
%{
#include "crystalspace.h"
%}
LANG_FUNCTIONS

%include "imap/loader.h"
%include "imap/reader.h"
%include "imap/saver.h"

#undef INTERFACE_APPLY
#define INTERFACE_APPLY(x) INTERFACE_POST(x)
IMAP_APPLY_FOR_EACH_INTERFACE

