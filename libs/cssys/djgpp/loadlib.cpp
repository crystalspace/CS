/*
    Dynamic library support for Crystal Space 3D library/DJGPP
    Written by Andrew Zabolotny <bit@eltech.ru>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef CS_STATIC_LINKED

#include "cssysdef.h"
#include "cssys/csshlib.h"

#include <dlfcn.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/nearptr.h>
#include <string.h>

/* dummy declation for external symbols, rather than using
   #include just to get (any) symbol definition... */
extern "C" void __builtin_delete ();
extern "C" void __builtin_vec_delete ();
extern "C" void __builtin_new ();
extern "C" void __builtin_vec_new ();
extern "C" void __pure_virtual ();
extern "C" void __dj_ctype_flags ();
extern "C" void _dos_getdrive ();
extern "C" void _dos_setdrive ();
extern "C" void getcwd ();
extern "C" void setjmp ();
extern "C" void longjmp ();

/*
 * These symbols will be exported to loaded DXEs.
 * Unfortunately, DJGPP does not support weak symbols
 * (weak symbols in COFF will work only with binutils 2.9+)
 * thus the following symbols will be in any your program that links
 * against this module. On the other hand, making these symbols
 * weak can result in symbol being NULL (and DXE not being loaded
 * because you won't be able to resolve missing symbols). Thus the
 * way it currently works is a bit costly (max ~20K of unused code)
 * but is guaranteed always to work.
 *
 * To build a list of unresolved symbols in a set of modules execute
 * the following command:
 *
 * dxe2gen --show-unres *.dxe | sort | uniq >file
 */
DXE_EXPORT_TABLE (syms)
  DXE_EXPORT (__builtin_delete)
  DXE_EXPORT (__builtin_new)
  DXE_EXPORT (__builtin_vec_delete)
  DXE_EXPORT (__builtin_vec_new)
  DXE_EXPORT (__dj_ctype_flags)
  DXE_EXPORT (__dj_stderr)
  DXE_EXPORT (__djgpp_selector_limit)
  DXE_EXPORT (__dpmi_allocate_ldt_descriptors)
  DXE_EXPORT (__dpmi_get_segment_base_address)
  DXE_EXPORT (__dpmi_get_segment_limit)
  DXE_EXPORT (__dpmi_int)
  DXE_EXPORT (__dpmi_physical_address_mapping)
  DXE_EXPORT (__dpmi_set_segment_base_address)
  DXE_EXPORT (__dpmi_set_segment_limit)
  DXE_EXPORT (__pure_virtual)
  DXE_EXPORT (_dos_getdrive)
  DXE_EXPORT (_dos_setdrive)
  DXE_EXPORT (_go32_info_block)
  DXE_EXPORT (abort)
  DXE_EXPORT (abs)
  DXE_EXPORT (calloc)
  DXE_EXPORT (ceil)
  DXE_EXPORT (cos)
  DXE_EXPORT (dosmemget)
  DXE_EXPORT (dosmemput)
  DXE_EXPORT (errno)
  DXE_EXPORT (exit)
  DXE_EXPORT (exp)
  DXE_EXPORT (fclose)
  DXE_EXPORT (fdopen)
  DXE_EXPORT (ferror)
  DXE_EXPORT (fflush)
  DXE_EXPORT (floor)
  DXE_EXPORT (fopen)
  DXE_EXPORT (fprintf)
  DXE_EXPORT (fputc)
  DXE_EXPORT (fread)
  DXE_EXPORT (free)
  DXE_EXPORT (fseek)
  DXE_EXPORT (ftell)
  DXE_EXPORT (fwrite)
  DXE_EXPORT (getcwd)
  DXE_EXPORT (getenv)
  DXE_EXPORT (gmtime)
  DXE_EXPORT (longjmp)
  DXE_EXPORT (malloc)
  DXE_EXPORT (memcpy)
  DXE_EXPORT (memmove)
  DXE_EXPORT (memset)
  DXE_EXPORT (memcmp)
  DXE_EXPORT (pow)
  DXE_EXPORT (printf)
  DXE_EXPORT (qsort)
  DXE_EXPORT (rand)
  DXE_EXPORT (realloc)
  DXE_EXPORT (rewind)
  DXE_EXPORT (setjmp)
  DXE_EXPORT (sin)
  DXE_EXPORT (sprintf)
  DXE_EXPORT (sqrt)
  DXE_EXPORT (sscanf)
  DXE_EXPORT (strcasecmp)
  DXE_EXPORT (strcat)
  DXE_EXPORT (strchr)
  DXE_EXPORT (strcmp)
  DXE_EXPORT (strcpy)
  DXE_EXPORT (strdup)
  DXE_EXPORT (strlen)
  DXE_EXPORT (strncasecmp)
  DXE_EXPORT (strncpy)
  DXE_EXPORT (strspn)
  DXE_EXPORT (strstr)
  DXE_EXPORT (vsprintf)
DXE_EXPORT_END

static const char *module;
static bool hint = true;

static void *_dlerrh (const char *symbol)
{
  printf ("dynamic loader: undefined symbol `%s' in module `%s'\n", symbol, module);
  if (hint)
  {
    hint = false;
    printf ("(hint: add the symbol to the symbol table in libs/cssys/djgpp/loadlib.cpp)\n");
  }
  return NULL;
}

static struct __dlinit
{
  __dlinit ()
  {
    dlregsym (syms);
    dlsetres (_dlerrh);
  }
} __dummy_dlinit;

csLibraryHandle csFindLoadLibrary (const char *iName)
{
  return csFindLoadLibrary (NULL, iName, ".dxe");
}

csLibraryHandle csLoadLibrary (const char* iName)
{
  return dlopen (module = iName, 0);
}

void csPrintLibraryError (const char *iModule)
{
  fprintf (stderr, "DLERROR (%s): %s\n", iModule, dlerror ());
}

void *csGetLibrarySymbol (csLibraryHandle Handle, const char *iName)
{
  return dlsym (Handle, iName);
}

bool csUnloadLibrary (csLibraryHandle Handle)
{
  return (dlclose (Handle) == 0);
}

#endif // CS_STATIC_LINKED
