#ifndef __CS_SNPRINTF_H__
#define __CS_SNPRINTF_H__

#include <stdarg.h>
#include "csextern.h"

#if defined(__cplusplus)
//extern "C" {
#endif

/// Portable implementation of snprintf()
extern CS_CSUTIL_EXPORT int cs_snprintf (char* buf, size_t bufSize, 
					 const char* format, /*args*/ ...)
    CS_GNUC_PRINTF (3, 4);
/// Portable implementation of vsnprintf()
extern CS_CSUTIL_EXPORT int cs_vsnprintf (char *, size_t, const char *, va_list)
    CS_GNUC_PRINTF (3, 0);
/**
 * Portable implementation of asprintf().
 * \remark Like asprintf(), the string was allocated with malloc() and needs to
 *  be freed with free().
 */
extern CS_CSUTIL_EXPORT int cs_asprintf (char **, const char *, ...)
    CS_GNUC_PRINTF (2, 3);
/**
 * Portable implementation of vasprintf().
 * \copydoc asprintf
 */
extern CS_CSUTIL_EXPORT int cs_vasprintf (char **, const char *, va_list)
    CS_GNUC_PRINTF (2, 0);

#if defined(__cplusplus)
//}
#endif

#endif // __CS_SNPRINTF_H__
