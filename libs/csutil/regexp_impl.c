/* Just a wrapper to compile regexp.c when CS_HAS_REGEX isn't defined */

#ifndef CS_HAS_REGEX
#include "regex.c"
#endif
