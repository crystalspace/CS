/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include "cssys/csshlib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "csutil/csstring.h"

#ifdef CS_DEBUG
#  define DLOPEN_MODE   RTLD_NOW// | RTLD_GLOBAL	// handy for debugging
#else
#  define DLOPEN_MODE   RTLD_LAZY// | RTLD_GLOBAL
#endif

csLibraryHandle csFindLoadLibrary (const char *iName) {
   printf("csFindLoadLibrary not on PS2\n");
   return NULL;
}

csLibraryHandle csLoadLibrary (const char* iName) {
   printf("csLoadLibrary not on PS2\n");
   return NULL;
}

void csPrintLibraryError (const char *iModule) {
   printf("csPrintLibraryError not on PS2\n");
}

void *csGetLibrarySymbol (csLibraryHandle Handle, const char *iName) {
   printf("csGetLibrarySymbol not on PS2\n");
   return NULL;
}

bool csUnloadLibrary (csLibraryHandle Handle) {
   printf("csUnloadLibrary not on PS2\n");
   return 0;
}
