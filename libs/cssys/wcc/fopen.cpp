#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "sysdef.h"
#include "system/system.h"

FILE* csSystemDriver::fopen (char* filename, char* mode)
{
  char new_filename[256];
  char* s = filename, * d = new_filename;
  while (*s)
  {
    if (*s == '/') *d++ = '\\';
    else *d++ = *s;
    s++;
  }
  *d = 0;
  return ::fopen (new_filename, mode);
}

