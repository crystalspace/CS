%include "imap/loader.h"
%include "imap/reader.h"
%include "imap/saver.h"

/* POST */
#ifndef SWIGIMPORTED
#undef APPLY_FOR_ALL_INTERFACES_POST
#define APPLY_FOR_ALL_INTERFACES_POST IMAP_APPLY_FOR_EACH_INTERFACE
%include "bindings/common/basepost.i"
cs_lang_include(imappost.i)
#endif
