// parser.h is only here for backward compatibility
#ifdef CS_COMPILER_GCC
  #warning imap/parser.h is deprecated; use imap/loader.h instead
#endif
#ifdef CS_COMPILER_MSVC
  #pragma message ("imap/parser.h is deprecated; use imap/loader.h instead")
#endif

#include "imap/loader.h"
