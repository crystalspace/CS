%module imap

%import "bindings/allinterfaces.i"

%include "bindings/basepre.i"

#undef APPLY_FOR_ALL_INTERFACES
#define APPLY_FOR_ALL_INTERFACES IMAP_APPLY_FOR_EACH_INTERFACE

%include "imap/loader.h"
%include "imap/reader.h"
%include "imap/saver.h"

%include "bindings/basepost.i"

