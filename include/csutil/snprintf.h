#ifndef _CRYSTAL_SNPRINTF_H_
#define _CRYSTAL_SNPRINTF_H_

extern "C" {

extern int cs_snprintf(char *, size_t, const char *, /*args*/ ...);
extern int cs_vsnprintf(char *, size_t, const char *, va_list);

}

#endif
