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

#include "cssysdef.h"
#include "cssys/csshlib.h"
#ifdef __CYGWIN__
#include <sys/cygwin.h>
#endif
#include "csutil/csstrvec.h"
#include "csutil/util.h"
#include <windows.h>

static csStrVector ErrorMessages;

csLibraryHandle csFindLoadLibrary (const char *iName)
{
  ErrorMessages.DeleteAll();
  return csFindLoadLibrary (NULL, iName, ".dll");
}

csLibraryHandle csLoadLibrary (const char* iName)
{
  csLibraryHandle handle;
  DWORD errorCode;
#ifndef __CYGWIN__
  handle = LoadLibrary (iName);
  errorCode = GetLastError();
#else
  // Cygwin wants to have Win32 paths not Unix paths.
  char *tmp=new char[1024];
  if (cygwin_conv_to_win32_path (iName,tmp))
  {
    ErrorMessages.Push(
	csStrNew("LoadLibrary() Cygwin/Win32 path conversion failed."));
    delete[] tmp;
    return 0;
  }
  handle = LoadLibrary (tmp);

 // A load attempt might fail if the DLL depends implicitly upon some other
 // DLLs which reside in the Cygwin /bin directory.  To deal with this case, we
 // add the Cygwin /bin directory to the PATH environment variable and retry.
 if (handle == NULL)
 {
   char *OLD_PATH = new char[4096];
   char *DLLDIR = new char[1024];
   GetEnvironmentVariable("PATH", OLD_PATH, 4096);
   if (cygwin_conv_to_win32_path ("/bin/",DLLDIR))
   {
     ErrorMessages.Push(csStrNew(
       "LoadLibrary() '/bin/' Cygwin/Win32 path conversion failed."));
     delete[] DLLDIR;
     delete[] OLD_PATH;
     delete[] tmp;
     return 0;
   }
   SetEnvironmentVariable("PATH", DLLDIR);
   handle = LoadLibrary (tmp);
   errorCode = GetLastError();
   SetEnvironmentVariable("PATH", OLD_PATH);
   delete[] DLLDIR;
   delete[] OLD_PATH;
 }

  delete[] tmp;
#endif

  if (handle == NULL)
  {
    char *buf;
    FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &buf, 0, NULL);
    char *str = new char[strlen(buf) + strlen(iName) + 50];
    sprintf (str, "LoadLibrary('%s') error %d: %s",
	iName, (int)errorCode, buf);
    ErrorMessages.Push (str);
    LocalFree (buf);
    return NULL;
  }

  typedef const char* (*pfnGetPluginCompiler)();
  pfnGetPluginCompiler get_plugin_compiler = 
    (pfnGetPluginCompiler) GetProcAddress ((HMODULE)handle, "plugin_compiler");
  if (!get_plugin_compiler)
  {
    const char *noPluginCompiler =
      "%s: DLL does not export \"plugin_compiler\".\n";
    char *msg = new char[strlen(noPluginCompiler) + strlen(iName)];
    sprintf (msg, noPluginCompiler, iName);
    ErrorMessages.Push (msg);
    FreeLibrary ((HMODULE)handle);
    return NULL;
  }
  const char* plugin_compiler = get_plugin_compiler();
  if (strcmp(plugin_compiler, CS_COMPILER_NAME))
  {
    const char *compilerMismatch =
      "%s: plugin compiler mismatches app compiler: %s != "
      CS_COMPILER_NAME "\n";
    char *msg = new char[strlen(compilerMismatch) + strlen(iName) + 
      strlen(plugin_compiler)];
    sprintf (msg, compilerMismatch, iName, plugin_compiler);
    ErrorMessages.Push (msg);
    FreeLibrary ((HMODULE)handle);
    return NULL;
  }

  return handle;
}

void* csGetLibrarySymbol(csLibraryHandle Handle, const char* Name)
{
  void *ptr = (void*)GetProcAddress ((HMODULE)Handle, Name);
  if (!ptr)
  {
    char *Name2;
    Name2 = new char [strlen (Name) + 2];
    strcpy (Name2, "_");
    strcat (Name2, Name);
    ptr = (void*)GetProcAddress ((HMODULE)Handle, Name2);
    delete [] Name2;
  }
  return ptr;
}

bool csUnloadLibrary (csLibraryHandle Handle)
{
#if defined(CS_EXTENSIVE_MEMDEBUG) && defined(COMP_VC)
  // Why not? - Because the source file information
  // for would leaked objects get lost
  return true;
#else
  return FreeLibrary ((HMODULE)Handle)!=0;
#endif
}

void csPrintLibraryError (const char *iModule)
{
  char *str;
  while((str = (char*)ErrorMessages.Pop()) != NULL)
  {
    fprintf (stderr, "  %s", str);
    delete[] str;
  }
}
