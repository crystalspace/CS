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

#define CS_SYSDEF_PROVIDE_HARDWARE_MMIO
#include "cssysdef.h"
#include "cssys/csshlib.h"
#include "cssys/sysfunc.h"
#include "cssys/syspath.h"
#include "cssys/csmmap.h"
#ifdef __CYGWIN__
#include <sys/cygwin.h>
#endif
#include "csutil/csstring.h"
#include "csutil/csstrvec.h"
#include "csutil/physfile.h"
#include "csutil/scfstr.h"
#include "csutil/scfstrv.h"
#include "csutil/strhash.h"
#include "csutil/util.h"
#include "csutil/xmltiny.h"
#include "iutil/document.h"
#include <windows.h>

static csStrVector ErrorMessages;

#define LOADLIBEX_FLAGS		    LOAD_WITH_ALTERED_SEARCH_PATH

csLibraryHandle csLoadLibrary (const char* iName)
{
  csLibraryHandle handle;
  DWORD errorCode;

  CS_ALLOC_STACK_ARRAY (char, dllPath, strlen (iName) + 5);
  strcpy (dllPath, iName);
  char* dot = strrchr (dllPath, '.');
  if ((!dot) || (strcasecmp (dot, ".dll") != 0))
  {
    if (dot && (strcasecmp (dot, ".csplugin") == 0))
    {
      strcpy (dot, ".dll");
    }
    else
    {
      strcat (dllPath, ".dll");
    }
  }

  handle = LoadLibraryEx (dllPath, 0, LOADLIBEX_FLAGS);
  errorCode = GetLastError();

#ifdef __CYGWIN__
 // A load attempt might fail if the DLL depends implicitly upon some other
 // DLLs which reside in the Cygwin /bin directory.  To deal with this case, we
 // add the Cygwin /bin directory to the PATH environment variable and retry.
 if (handle == 0)
 {
   char *OLD_PATH = new char[4096];
   char *DLLDIR = new char[1024];
   GetEnvironmentVariable("PATH", OLD_PATH, 4096);
   if (cygwin_conv_to_win32_path ("/bin/",DLLDIR))
   {
     ErrorMessages.Push(csStrNew(
       "LoadLibraryEx() '/bin/' Cygwin/Win32 path conversion failed."));
     delete[] DLLDIR;
     delete[] OLD_PATH;
     return 0;
   }
   SetEnvironmentVariable("PATH", DLLDIR);
   handle = LoadLibraryEx (dllPath, 0, LOADLIBEX_FLAGS);
   errorCode = GetLastError();
   SetEnvironmentVariable("PATH", OLD_PATH);
   delete[] DLLDIR;
   delete[] OLD_PATH;
 }
#endif

  if (handle == 0)
  {
    char *buf;
    FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        0, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &buf, 0, 0);
    char *str = new char[strlen(buf) + strlen(iName) + 50];
    sprintf (str, "LoadLibraryEx('%s') error %d: %s",
	dllPath, (int)errorCode, buf);
    ErrorMessages.Push (str);
    LocalFree (buf);
    return 0;
  }

  typedef const char* (*pfnGetPluginCompiler)();
  pfnGetPluginCompiler get_plugin_compiler = 
    (pfnGetPluginCompiler) GetProcAddress ((HMODULE)handle, "plugin_compiler");
  if (!get_plugin_compiler)
  {
    const char *noPluginCompiler =
      "%s: DLL does not export \"plugin_compiler\".\n";
    char *msg = new char[strlen(noPluginCompiler) + strlen(iName)];
    sprintf (msg, noPluginCompiler, dllPath);
    ErrorMessages.Push (msg);
    FreeLibrary ((HMODULE)handle);
    return 0;
  }
  const char* plugin_compiler = get_plugin_compiler();
  if (strcmp(plugin_compiler, CS_COMPILER_NAME) != 0)
  {
    const char *compilerMismatch =
      "%s: plugin compiler mismatches app compiler: %s != "
      CS_COMPILER_NAME "\n";
    char *msg = new char[strlen(compilerMismatch) + strlen(iName) + 
      strlen(plugin_compiler)];
    sprintf (msg, compilerMismatch, dllPath, plugin_compiler);
    ErrorMessages.Push (msg);
    FreeLibrary ((HMODULE)handle);
    return 0;
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
  // for leaked objects would get lost
  return true;
#else
  return FreeLibrary ((HMODULE)Handle)!=0;
#endif
}

void csPrintLibraryError (const char *iModule)
{
  char *str;
  while((str = (char*)ErrorMessages.Pop()) != 0)
  {
    fprintf (stderr, "  %s", str);
    delete[] str;
  }
}

static void AppendStrVecString (iStrVector*& strings, const char* str)
{
  if (!strings)
  {
    strings = new scfStrVector ();
  }
  strings->Push (csStrNew (str));
}

static void AppendWin32Error (const char* text,
			      DWORD errorCode,
			      iStrVector*& strings)
{
  char *buf;

  FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | 
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      0, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) &buf, 0, 0);

  csString errorMsg;
  errorMsg.Format ("%s: %s [%.8x]", text, buf, 
    (unsigned)errorCode);
  AppendStrVecString (strings, errorMsg);
  LocalFree (buf);
}

static csRef<iString> InternalGetPluginMetadata (const char* fullPath, 
						 csRef<iDocument>& metadata,
						 bool& metadataValid)
{
  iString* result = 0;

  HMODULE hLibrary = LoadLibraryEx (fullPath, 0, 
    LOADLIBEX_FLAGS | LOAD_LIBRARY_AS_DATAFILE);
  if (hLibrary != 0)
  {
    HRSRC res = FindResource (hLibrary, MAKEINTRESOURCE (0x444d),
      RT_RCDATA);
    if (res != 0)
    {
      HGLOBAL resDataHandle = LoadResource (hLibrary, res);
      if (resDataHandle != 0)
      {
	LPVOID resData = LockResource (resDataHandle);
	if (resData != 0)
	{
	  if (!metadata)
	  {
	    /*
	      TinyXML documents hold references to the document system.
	      So we have to create a new csTinyDocumentSystem (). Using just
	      'csTinyDocumentSystem docsys' would result in a crash when the
	      documents try to DecRef() to already destructed document system.
	    */
	    csRef<iDocumentSystem> docsys = csPtr<iDocumentSystem>
	      (new csTinyDocumentSystem ());
	    metadata = docsys->CreateDocument();
	  }
	  char const* errmsg = metadata->Parse ((char*)resData);
	  if (errmsg != 0)
	  {
	    csString errstr;
	    errstr.Format ("Error parsing metadata from '%s': %s",
	      fullPath, errmsg);

	    result = new scfString (errstr);
	  }
	  else
	  {
	    metadataValid = true;
	  }
	}
      }
    }

    FreeLibrary (hLibrary);
  }
  /*
    No error messages are emitted - this simply may not be a plugin...
    And "errors" would be confusing in this case.
    We can't really determine whether a DLL is intended to be a CS plugin
    or not.
    */

  return csPtr<iString> (result);
}

csRef<iString> csGetPluginMetadata (const char* fullPath, 
				    csRef<iDocument>& metadata)
{
  /*
    @@@ There's a small inefficiency here.
    This function, when given a filename of <blah>.dll or 
    <blah>.csplugin, first checks <blah>.dll for embedded
    metadata and then for the existence of a <blah>.csplugin
    file. However, the csScanPluginDir() function already
    emits a <blah>.csplugin file name when such a file was
    found, <blah>.dll otherwise. This information can
    probably be reused.
   */

  CS_ALLOC_STACK_ARRAY (char, dllPath, strlen (fullPath) + 5);
  strcpy (dllPath, fullPath);
  char* dot = strrchr (dllPath, '.');

  bool isCsPlugin = (dot && (strcasecmp (dot, ".csplugin") == 0));

  if ((!dot) || isCsPlugin || (strcasecmp (dot, ".dll") != 0))
  {
    if (isCsPlugin)
    {
      strcpy (dot, ".dll");
    }
    else
    {
      strcat (dllPath, ".dll");
    }
  }

  bool metadataValid = false;
  csRef<iString> result = 
    InternalGetPluginMetadata (dllPath, metadata, metadataValid);

  /* Check whether a .csplugin file exists as well */

  /*
    @@@ This makes the assumption that such might only exists if
     this function was called with a <blah>.csplugin filename.
   */

  if (isCsPlugin)
  {
    csPhysicalFile file (fullPath, "rb");

    if (file.GetStatus() == VFS_STATUS_OK)
    {
      if (metadataValid)
      {
	csString errstr;
	errstr.Format ("'%s' contains embedded metadata, "
	  "but external '%s' exists as well. Ignoring the latter.",
	  dllPath, fullPath);

	result.AttachNew (new scfString (errstr));
      }
      else
      {
	if (metadata == 0)
	{
	  csRef<iDocumentSystem> docsys = csPtr<iDocumentSystem>
	    (new csTinyDocumentSystem ());
	  metadata = docsys->CreateDocument();
	}
	char const* errmsg = metadata->Parse (&file);

	if (errmsg != 0)
	{
	  csString errstr;
	  errstr.Format ("Error parsing metadata from '%s': %s",
	    fullPath, errmsg);

	  result.AttachNew (new scfString (errstr));
	}
	else
	{
	  metadataValid	= true;
	}
      }
    }
  }

  if (!metadataValid)
  {
    metadata = 0;
  }

  return (result);
}

extern char* csGetConfigPath ();

inline static void AddLower (csStringHash& hash, const char* str)
{
  static int id = 0;

  csString tmp (str);
  tmp.strlwr ();
  hash.Register (tmp, id++);
}

void InternalScanPluginDir (iStrVector*& messages,
			    const char* dir, 
			    csRef<iStrVector>& plugins,
			    bool recursive)
{
  csStringHash files;
  csStringHash dirs;

  csString filemask;

  // The directory sometimes has a trailing path separator attached.
  if (!strlen(dir) || dir[strlen(dir)-1]==PATH_SEPARATOR)
      filemask << dir  << "*.*";
  else
      filemask << dir << PATH_SEPARATOR << "*.*";

  WIN32_FIND_DATA findData;
  HANDLE hSearch = FindFirstFile (filemask, &findData);
  if (hSearch != INVALID_HANDLE_VALUE)
  {
    do
    {
      if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
	/*
	  instead of processing them immediately, first a list of
	  directories is filled.
	 */
	if (recursive && (strcmp (findData.cFileName, ".") != 0)
	  && (strcmp (findData.cFileName, "..") != 0))
	{
	  // Add file names in lower case, as Win32 doesn't care about FS case.
	  AddLower (dirs, findData.cFileName);
	}
      }
      else
      {
	/*
	  instead of processing them immediately, first a list of
	  plugin files is created.
	 */
	char* ext = strrchr (findData.cFileName, '.');
        if (ext && ((strcasecmp (ext, ".dll") == 0) || 
	  (strcasecmp (ext, ".csplugin") == 0)))
	{
	  AddLower (files, findData.cFileName);
	}
      }
    }
    while (FindNextFile (hSearch, &findData));
    FindClose (hSearch);
  }
  else
  {
    DWORD errorCode = GetLastError();

    /*
      The plugin paths list also contains non-existent entries -
      don't emit an error for those.
     */
    if (errorCode != ERROR_PATH_NOT_FOUND)
    {
      AppendWin32Error ("FindFirst() call failed",
	errorCode,
	messages);
    }
  }

  // now go over all the files. This way files in a dir will have precedence over
  // files a subdir.
  {
    csStringHashIterator fileIt (&files);
    csString fullPath;

    csRef<iString> msg;

    while (fileIt.HasNext())
    {
      csStringID id = fileIt.Next();

      const char* fileName = files.Request (id);

      char* ext = strrchr (fileName, '.');
      // ignore .csplugins, there are checked explicitly.
      if ((strcasecmp (ext, ".dll") == 0))
      {
	fullPath.Clear();
	// The directory sometimes has a trailing path separator attached.
	if (!strlen(dir) || dir[strlen(dir)-1]==PATH_SEPARATOR)
	  fullPath << dir << fileName;
	else
	  fullPath << dir << PATH_SEPARATOR << fileName;

	/*
	  Check whether the DLL has a companion .csplugin.
	 */
	char cspluginFile [MAX_PATH + 10];

	strcpy (cspluginFile, fileName);
	char* dot = strrchr (cspluginFile, '.');
	strcpy (dot, ".csplugin");

	csStringID cspID = files.Request (cspluginFile);
	if (cspID != csInvalidStringID)
	{
	  char cspluginPath [MAX_PATH + 10];

	  strcpy (cspluginPath, fullPath);
	  char* dot = strrchr (cspluginPath, '.');
	  strcpy (dot, ".csplugin");

          plugins->Push (csStrNew (cspluginPath));
	}
	else
	{
          plugins->Push (csStrNew (fullPath));
	}
      }
    }

    // release some memory.
    files.Clear();
  }
  {
    csStringHashIterator dirIt (&dirs);
    csString fullPath;

    while (dirIt.HasNext())
    {
      csStringID id = dirIt.Next();

      fullPath.Clear();
      fullPath << dir << PATH_SEPARATOR << dirs.Request (id);

      iStrVector* subdirMessages = 0;
      InternalScanPluginDir (subdirMessages, fullPath, plugins,
	recursive);
      
      if (subdirMessages != 0)
      {
	for (int i = 0; i < subdirMessages->Length(); i++)
	{
	  AppendStrVecString (messages, subdirMessages->Get (i));
	}

	subdirMessages->DecRef();
      }
    }
  }
}

csRef<iStrVector> csScanPluginDir (const char* dir, 
				   csRef<iStrVector>& plugins,
				   bool recursive)
{
  iStrVector* messages = 0;

  if (!plugins)
    plugins.AttachNew (new scfStrVector ());

  InternalScanPluginDir (messages, dir, plugins,  
    recursive);
	 
  return csPtr<iStrVector> (messages);
}

csRef<iStrVector> csScanPluginDirs (csPluginPaths* dirs, 
				    csRef<iStrVector>& plugins)
{
  iStrVector* messages = 0;

  if (!plugins)
    plugins.AttachNew (new scfStrVector ());

  for (int i = 0; i < dirs->GetCount (); i++)
  {
    iStrVector* dirMessages = 0;
    InternalScanPluginDir (dirMessages, (*dirs)[i].path, 
      plugins, (*dirs)[i].scanRecursive);
    
    if (dirMessages != 0)
    {
      csString tmp;
      tmp.Format ("The following issue(s) came up while scanning '%s':",
	(*dirs)[i].path);

      AppendStrVecString (messages, tmp);

      for (int j = 0; j < dirMessages->Length(); j++)
      {
	tmp.Format (" %s", dirMessages->Get (j));
	AppendStrVecString (messages, tmp);
      }
      dirMessages->DecRef();
    }
  }
	 
  return csPtr<iStrVector> (messages);
}

