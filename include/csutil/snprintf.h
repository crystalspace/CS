#ifndef _PORTABLE_SNPRINTF_H_
#define _PORTABLE_SNPRINTF_H_

#include <stdarg.h>

#define PORTABLE_SNPRINTF_VERSION_MAJOR 2
#define PORTABLE_SNPRINTF_VERSION_MINOR 2

//  This file was modified slighty by Christopher Nelson to change the name of
// these functions for Crystal Space.  Questions? mailto:paradox@bbhc.org

extern "C" int cs_snprintf(char *, size_t, const char *, /*args*/ ...);
extern "C" int cs_vsnprintf(char *, size_t, const char *, va_list);
extern "C" int cs_asprintf  (char **ptr, const char *fmt, /*args*/ ...);
//extern "C" int cs_vasprintf (char **ptr, const char *fmt, va_list ap);
extern "C" int cs_asnprintf (char **ptr, size_t str_m, const char *fmt, /*args*/ ...);
//extern "C" int cs_vasnprintf(char **ptr, size_t str_m, const char *fmt, va_list ap);

#endif
