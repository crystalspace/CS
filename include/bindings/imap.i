
#ifndef SWIGIMPORTED
  %module imap
  %include "bindings/allinterfaces.i"
  #define APPLY_FOR_ALL_INTERFACES_PRE APPLY_FOR_ALL_INTERFACES
  #define APPLY_FOR_ALL_INTERFACES_POST IMAP_APPLY_FOR_EACH_INTERFACE

  %include "bindings/basepre.i"
#endif


%include "imap/loader.h"
%include "imap/reader.h"
%include "imap/saver.h"


#ifndef SWIGIMPORTED
  %include "bindings/basepost.i"
#endif

