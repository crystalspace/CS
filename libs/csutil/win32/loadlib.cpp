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
#include "csutil/csshlib.h"
#include "csutil/sysfunc.h"
#include "csutil/syspath.h"
#include "csutil/csmmap.h"
#ifdef __CYGWIN__
#include <sys/cygwin.h>
#endif
#include "csutil/csstring.h"
#include "csutil/physfile.h"
#include "csutil/scfstr.h"
#include "csutil/scfstringarray.h"
#include "csutil/strhash.h"
#include "csutil/util.h"
#include "csutil/xmltiny.h"
#include "iutil/document.h"
#include <windows.h>

#include "csutil/win32/wintools.h"

static csStringArray ErrorMessages;

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
     ErrorMessages.Push(
       "LoadLibraryEx() '/bin/' Cygwin/Win32 path conversion failed.");
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
    char *buf = cswinGetErrorMessage (errorCode);
    csString s;
    s << "LoadLibraryEx(" << dllPath << ") error " << (int)errorCode << ": "
      << buf;
    ErrorMessages.Push (s);
    delete[] buf;
    return 0;
  }

  typedef const char* (*pfnGetPluginCompiler)();
  pfnGetPluginCompiler get_plugin_compiler = 
    (pfnGetPluginCompiler) csGetLibrarySymbol (handle, "plugin_compiler");
  if (!get_plugin_compiler)
  {
    csString s;
    s << dllPath << ": DLL does not export \"plugin_compiler\".";
    ErrorMessages.Push (s);
    FreeLibrary ((HMODULE)handle);
    return 0;
  }
  const char* plugin_compiler = get_plugin_compiler();
  if (strcmp(plugin_compiler, CS_COMPILER_NAME) != 0)
  {
    csString s;
    s << dllPath << ": plugin compiler does not match application compiler: "
      << plugin_compiler << " != " CS_COMPILER_NAME;
    ErrorMessages.Push (s);
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

#if (defined(CS_EXTENSIVE_MEMDEBUG) && defined(CS_COMPILER_MSVC)) || \
  defined(CS_MEMORY_TRACKER) || defined(CS_REF_TRACKER)
/*
  Lazily unload libs - Because if unloading happens immediately,
  the source file information for leaked objects would get lost.
  But unload them nevertheless so memory checker don't report leaked
  handles.
  Similar for reftracking - but here it's debug symbols.
 */
#define LAZY_UNLOAD

struct LazyUnloader
{
  csArray<HMODULE> modules;

  LazyUnloader() {}
  ~LazyUnloader()
  {
    for (size_t i = 0; i < modules.Length(); i++)
    {
      FreeLibrary (modules[i]);
    }
  }
};

static LazyUnloader lazyUnloader;

#endif

bool csUnloadLibrary (csLibraryHandle Handle)
{
#ifdef LAZY_UNLOAD
  lazyUnloader.modules.Push ((HMODULE)Handle);
  return true;
#else
  return FreeLibrary ((HMODULE)Handle) != 0;
#endif
}

void csPrintLibraryError (const char *iModule)
{
  char *str;
  while (ErrorMessages.Length () > 0)
  {
    str = (char*)ErrorMessages.Pop();
    if (str != 0) csPrintfErr ("  %s\n", str);
    delete[] str;
  }
}

static void AppendStrVecString (iStringArray*& strings, const char* str)
{
  if (!strings)
  {
    strings = new scfStringArray ();
  }
  strings->Push (str);
}

static void AppendWin32Error (const char* text,
			      DWORD errorCode,
			      iStringArray*& strings)
{
  char *buf = cswinGetErrorMessage (errorCode);

  csString errorMsg;
  errorMsg.Format ("%s: %s [%.8" PRIx32 "]", text, buf, 
    errorCode);
  AppendStrVecString (strings, errorMsg);
  delete[] buf;
}

#if defined(CS_EMBED_PLUGIN_META)

static csRef<iString> InternalGetPluginMetadata (const char* fullPath, 
						 csRef<iDocument>& metadata)
{
  csRef<iString> result;
  metadata = 0;

  HMODULE hLibrary =
    LoadLibraryEx (fullPath, 0, LOADLIBEX_FLAGS | LOAD_LIBRARY_AS_DATAFILE);
  if (hLibrary != 0)
  {
    HRSRC res = FindResource (hLibrary, MAKEINTRESOURCE (0x444d), RT_RCDATA);
    if (res != 0)
    {
      HGLOBAL resDataHandle = LoadResource (hLibrary, res);
      if (resDataHandle != 0)
      {
	LPVOID resData = LockResource (resDataHandle);
	if (resData != 0)
	{
	  csRef<iDocumentSystem> docsys =
	    csPtr<iDocumentSystem>(new csTinyDocumentSystem ());
	  csRef<iDocument> doc = docsys->CreateDocument();

	  char const* errmsg = doc->Parse ((char const*)resData, true);
	  if (errmsg == 0)	// Parse successful.
	    metadata = doc;
	  else			// Parse failed.
	  {
	    csString errstr;
	    errstr.Format ("Error parsing metadata from '%s': %s",
	      fullPath, errmsg);
	    result.AttachNew(new scfString (errstr));
	  }
	}
      }
    }
    FreeLibrary (hLibrary);
  }

  // We do not emit an error message for the case when the DLL contains no
  // metadata since this is not considered an error condition.  This case can
  // arise, for instance, if we are asked to extract metadata from a DLL which
  // is not a CS plugin (a valid situation if non-CS DLLs are found by the
  // plugin scanner in the same directories as CS plugins).

  return result;
}

#endif

csRef<iString> csGetPluginMetadata (const char* fullPath, 
				    csRef<iDocument>& metadata)
{
  // @@@ There's a small inefficiency here.  This function, when given a
  // filename of <blah>.dll or <blah>.csplugin, first checks <blah>.dll for
  // embedded metadata and then for the existence of a <blah>.csplugin file.
  // However, the csScanPluginDir() function already emits a <blah>.csplugin
  // file name when such a file was found, <blah>.dll otherwise.  This
  // information can probably be reused.

  CS_ALLOC_STACK_ARRAY (char, dllPath, strlen (fullPath) + 5);
  strcpy (dllPath, fullPath);
  char* dot = strrchr (dllPath, '.');

  bool isCsPlugin = (dot && (strcasecmp (dot, ".csplugin") == 0));
  if (!dot || isCsPlugin || strcasecmp (dot, ".dll") != 0)
  {
    if (isCsPlugin)
      strcpy (dot, ".dll");
    else
      strcat (dllPath, ".dll");
  }

  csRef<iString> result;
  metadata = 0;

#if defined(CS_EMBED_PLUGIN_META)
  result = InternalGetPluginMetadata (dllPath, metadata);
#endif

  // Check whether a .csplugin file exists as well
  // @@@ This makes the assumption that such might only exists if this function
  // was called with a <blah>.csplugin filename.
  if (isCsPlugin)
  {
    csPhysicalFile file (fullPath, "rb");

    if (file.GetStatus() == VFS_STATUS_OK)
    {
      if (metadata.IsValid())
      {
	csString errstr;
	errstr.Format ("'%s' contains embedded metadata, "
	  "but external '%s' exists as well. Ignoring the latter.",
	  dllPath, fullPath);
	result.AttachNew (new scfString (errstr));
      }
      else
      {
        csRef<iDocumentSystem> docsys =
	  csPtr<iDocumentSystem>(new csTinyDocumentSystem ());
	csRef<iDocument> doc = docsys->CreateDocument();

	char const* errmsg = doc->Parse (&file, true);
	if (errmsg == 0)	// Parse successful.
	  metadata = doc;
	else			// Parse failed.
	{
	  csString errstr;
	  errstr.Format ("Error parsing metadata from '%s': %s",
	    fullPath, errmsg);
	  result.AttachNew (new scfString (errstr));
	}
      }
    }
  }

  return result;
}

inline static void AddLower (csStringHash& hash, const char* str)
{
  static int id = 0;

  csString tmp (str);
  tmp.Downcase ();
  hash.Register (tmp, id++);
}

void InternalScanPluginDir (iStringArray*& messages,
			    const char* dir, 
			    csRef<iStringArray>& plugins,
			    bool recursive)
{
  csStringHash files;
  csStringHash dirs;

  csString filemask;

  // The directory sometimes has a trailing path separator attached.
  if (!strlen(dir) || dir[strlen(dir)-1] == CS_PATH_SEPARATOR)
      filemask << dir  << "*.*";
  else
      filemask << dir << CS_PATH_SEPARATOR << "*.*";

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

  // Now go over all the files.  This way files in a dir will have precedence
  // over files a subdir.
  {
    csStringHash::GlobalIterator fileIt(files.GetIterator());
    csString fullPath;

    csRef<iString> msg;

    while (fileIt.HasNext())
    {
      csStringID id = fileIt.Next();

      const char* fileName = files.Request (id);

      const char* ext = strrchr (fileName, '.');
      // ignore .csplugins, there are checked explicitly.
      if ((strcasecmp (ext, ".dll") == 0))
      {
	fullPath.Clear();
	// The directory sometimes has a trailing path separator attached.
	if (!strlen(dir) || dir[strlen(dir)-1] == CS_PATH_SEPARATOR)
	  fullPath << dir << fileName;
	else
	  fullPath << dir << CS_PATH_SEPARATOR << fileName;

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

          plugins->Push (cspluginPath);
	}
	else
	{
          plugins->Push (fullPath);
	}
      }
    }

    // release some memory.
    files.Clear();
  }
  {
    csStringHash::GlobalIterator dirIt(dirs.GetIterator());
    csString fullPath;

    while (dirIt.HasNext())
    {
      csStringID id = dirIt.Next();

      fullPath.Clear();
      fullPath << dir << CS_PATH_SEPARATOR << dirs.Request (id);

      iStringArray* subdirMessages = 0;
      InternalScanPluginDir (subdirMessages, fullPath, plugins,
	recursive);
      
      if (subdirMessages != 0)
      {
	for (size_t i = 0; i < subdirMessages->Length(); i++)
	{
	  AppendStrVecString (messages, subdirMessages->Get (i));
	}

	subdirMessages->DecRef();
      }
    }
  }
}

csRef<iStringArray> csScanPluginDir (const char* dir, 
				   csRef<iStringArray>& plugins,
				   bool recursive)
{
  iStringArray* messages = 0;

  if (!plugins)
    plugins.AttachNew (new scfStringArray ());

  InternalScanPluginDir (messages, dir, plugins,  
    recursive);
	 
  return csPtr<iStringArray> (messages);
}

csRef<iStringArray> csScanPluginDirs (csPluginPaths* dirs, 
				    csRef<iStringArray>& plugins)
{
  iStringArray* messages = 0;

  if (!plugins)
    plugins.AttachNew (new scfStringArray ());

  for (size_t i = 0; i < dirs->GetCount (); i++)
  {
    iStringArray* dirMessages = 0;
    InternalScanPluginDir (dirMessages, (*dirs)[i].path, 
      plugins, (*dirs)[i].scanRecursive);
    
    if (dirMessages != 0)
    {
      csString tmp;
      tmp.Format ("The following issue(s) came up while scanning '%s':",
	(*dirs)[i].path);

      AppendStrVecString (messages, tmp);

      for (size_t j = 0; j < dirMessages->Length(); j++)
      {
	tmp.Format (" %s", dirMessages->Get (j));
	AppendStrVecString (messages, tmp);
      }
      dirMessages->DecRef();
    }
  }
	 
  return csPtr<iStringArray> (messages);
}
