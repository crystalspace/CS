#ifndef __CS_SNPRINTF_H__
#define __CS_SNPRINTF_H__

#include <stdarg.h>
#include "csextern.h"

#if defined(__cplusplus)
//extern "C" {
#endif

extern CS_CSUTIL_EXPORT int cs_snprintf(char *, size_t, const char *, /*args*/ ...)
    CS_GNUC_PRINTF (3, 4);
extern CS_CSUTIL_EXPORT int cs_vsnprintf(char *, size_t, const char *, va_list)
    CS_GNUC_PRINTF (3, 0);

#if defined(__cplusplus)
//}
#endif

#endif // __CS_SNPRINTF_H__
