//=============================================================================
//
//	Copyright (C)1999-2003 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// OSXDynamicLibrary.cpp
//
//	Platform-independent C++ cover functions for platform-specific
//	functions for loading dynamic link libraries.  The actual library
//	loading functions have Objective-C bindings and are accessed via
//	plain-C stubs.
//
// *NOTE*
//	SCF blindly assumes that plugin unloading always succeeds, however with
//	Apple, unloading plugins is problematic and can only be done in	reverse
//	order.  Since SCF may ask to unload any random plugin, there is no
//	guarantee on Apple that the unload operation will succeed.
//	Consequently, we must manually maintain a list of loaded plugins.  If
//	SCF asks to unload a particular plugin but the plugin fails to unload,
//	then we remember that it is still loaded and simply return a reference
//	to it if SCF subsequently asks to re-load it.
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "cssys/csshlib.h"
#include "cssys/sysfunc.h"
#include "csutil/csvector.h"
#include "csutil/util.h"
#include "OSXLoadLibrary.h"
#include <sys/param.h>

#define CSPLUGIN_EXT ".csplugin"
#define CSBUNDLE_EXT ".csbundle"

class OSXPluginEntry
{
public:
  csLibraryHandle handle;
  char* path;
  OSXPluginEntry(csLibraryHandle h, char const* p) :
    handle(h), path(csStrNew(p)) {}
  ~OSXPluginEntry() { delete[] path; }
};

class OSXPluginArray : public csVector
{
public:
  virtual bool FreeItem(void* p) { delete (OSXPluginEntry*)p; return true; }
  virtual int Compare(void* p1, void* p2, int) const
  { return strcmp(((OSXPluginEntry*)p1)->path,((OSXPluginEntry*)p2)->path); }
  virtual int CompareKey(void* p, const void* k, int) const
  { return strcmp(((OSXPluginEntry*)p)->path, (char const*)k); }
};


//-----------------------------------------------------------------------------
// get_loaded_plugins
//-----------------------------------------------------------------------------
static OSXPluginArray& get_loaded_plugins()
{
  static OSXPluginArray list;
  return list;
}


//-----------------------------------------------------------------------------
// csLoadLibrary
// *1* The incoming path will probably have a .csplugin extension, but we need
//     to load the associated .csbundle.
// *2* Both successful and unsuccessful load requests are cached in the
//     loaded-plugins list so that we can quickly give the correct response if
//     we already know the answer from a previous attempt.
//-----------------------------------------------------------------------------
csLibraryHandle csLoadLibrary(char const* path)
{
  int const plugin_len = sizeof(CSPLUGIN_EXT) - 1;
  int const bundle_len = sizeof(CSBUNDLE_EXT) - 1;
  int const path_len = strlen(path);

  char* bin_name = new char[path_len + bundle_len + 1];
  strcpy(bin_name, path);
  
  if (path_len >= plugin_len && 					// *1*
    strcasecmp(bin_name + path_len - plugin_len, CSPLUGIN_EXT) == 0)
    strcpy(bin_name + path_len - plugin_len, CSBUNDLE_EXT);
  else if (path_len >= bundle_len && 
    strcasecmp(bin_name + path_len - bundle_len, CSBUNDLE_EXT) != 0)
    strcat(bin_name, CSBUNDLE_EXT);

  csLibraryHandle handle = 0;
  OSXPluginArray& loaded_plugins = get_loaded_plugins();
  int const n = loaded_plugins.FindSortedKey(bin_name);
  if (n >= 0)
    handle = ((OSXPluginEntry*)loaded_plugins[n])->handle;
  else
  {
    handle = (csLibraryHandle)OSXLoadLibrary(bin_name);
    loaded_plugins.InsertSorted(new OSXPluginEntry(handle, bin_name));	// *2*
  }

  delete[] bin_name;
  return handle;
}


//-----------------------------------------------------------------------------
// csGetLibrarySymbol
//-----------------------------------------------------------------------------
void* csGetLibrarySymbol(csLibraryHandle handle, char const* s)
{
  return OSXGetLibrarySymbol(handle, s);
}


//-----------------------------------------------------------------------------
// csUnloadLibrary
//	If SCF asks to unload a particular plugin and the unload operation
//	succeeds, then we remove the plugin's record from the loaded-plugin
//	list.  If it fails, then we leave the record alone; in which case if
//	SCF subsequently asks to re-load the plugin, we simply return a
//	reference to the already loaded plugin.
//-----------------------------------------------------------------------------
bool csUnloadLibrary(csLibraryHandle handle)
{
  if (OSXUnloadLibrary(handle))
  {
    OSXPluginArray& loaded_plugins = get_loaded_plugins();
    for (int i = loaded_plugins.Length() - 1; i >= 0; i--)
      if (((OSXPluginEntry const*)loaded_plugins[i])->handle == handle)
        { loaded_plugins.Delete(i); break; }
  }
  return true;
}


//-----------------------------------------------------------------------------
// csPrintLibraryError
//-----------------------------------------------------------------------------
void csPrintLibraryError(char const* name)
{
  fprintf(stderr, "ERROR: Failed to load plug-in module `%s'.\n", name);
  char const* s = OSXGetLibraryError();
  if (s != 0)
    fprintf(stderr, "Reason: %s\n", s);
}
