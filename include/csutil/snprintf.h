#ifndef _CRYSTAL_SNPRINTF_H_
#define _CRYSTAL_SNPRINTF_H_

#include <stdarg.h>

#if defined(__cplusplus)
//extern "C" {
#endif

extern int cs_snprintf(char *, size_t, const char *, /*args*/ ...);
extern int cs_vsnprintf(char *, size_t, const char *, va_list);

#if defined(__cplusplus)
//}
#endif

#endif
