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
#include "csutil/csshlib.h"
#include "csutil/sysfunc.h"
#include "csutil/array.h"
#include "csutil/csstring.h"
#include "csutil/util.h"
#include "OSXLoadLibrary.h"
#include <string.h>
#include <sys/param.h>

#define CSPLUGIN_EXT ".csplugin"
#define CSBUNDLE_EXT ".csbundle"


//-----------------------------------------------------------------------------
// Loaded-plugin maintenance utilities.
//-----------------------------------------------------------------------------
class OSXPluginEntry
{
public:
  csLibraryHandle handle;
  csString path;
  OSXPluginEntry(csLibraryHandle h, csString p) : handle(h), path(p) {}
  OSXPluginEntry(OSXPluginEntry const& r) : handle(r.handle), path(r.path) {}
  ~OSXPluginEntry() {}
  OSXPluginEntry& operator=(OSXPluginEntry const& r)
  {
    if (&r != this)
    {
      handle = r.handle;
      path = r.path;
    }
    return *this;
  }
};

class OSXPluginArray : public csArray<OSXPluginEntry>
{
public:
  OSXPluginEntry const* find(csString path) const
  {
    for (int i = 0, n = Length(); i < n; i++)
      if (Get(i).path == path)
	return &Get(i);
    return 0;
  }
  void insert(csLibraryHandle handle, csString path)
  {
    OSXPluginEntry const e(handle, path);
    Push(e);
  }
  void remove(csLibraryHandle handle)
  {
    for (int i = Length() - 1; i >= 0; i--)
      if (Get(i).handle == handle)
        { DeleteIndex(i); break; }
  }
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
  OSXPluginEntry const* e = loaded_plugins.find(bin_name);
  if (e != 0)
    handle = e->handle;
  else
  {
    handle = (csLibraryHandle)OSXLoadLibrary(bin_name);
    loaded_plugins.insert(handle, bin_name);				// *2*
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
    get_loaded_plugins().remove(handle);
  return true;
}


//-----------------------------------------------------------------------------
// csPrintLibraryError
//-----------------------------------------------------------------------------
void csPrintLibraryError(char const* name)
{
  csFPrintf(stderr, "ERROR: Failed to load plug-in module `%s'.\n", name);
  char const* s = OSXGetLibraryError();
  if (s != 0)
    csFPrintf(stderr, "Reason: %s\n", s);
}
