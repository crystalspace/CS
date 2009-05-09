/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 1998-2000 by Ivan Avramovic <ivan@avramovic.com>

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

#ifndef __CS_LDRPLUG_H__
#define __CS_LDRPLUG_H__

#include "csutil/array.h"
#include "csutil/csstring.h"
#include "csutil/ref.h"
#include "csutil/threading/rwmutex.h"
#include "imap/reader.h"
#include "iutil/document.h"

struct iBase;
struct iObjectRegistry;
struct iPluginManager;

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  struct csLoaderPluginRec
  {
    csString ShortName;
    csString ClassID;
    csRef<iBase> Component;
    csRef<iLoaderPlugin> Plugin;
    csRef<iBinaryLoaderPlugin> BinPlugin;
    csRef<iDocumentNode> defaults;

    csLoaderPluginRec (const char* shortName, const char *classID,
      iBase* component, iLoaderPlugin *plugin, iBinaryLoaderPlugin* binPlugin)
    {
      if (shortName) ShortName = shortName;
      ClassID = classID;
      Component = component;
      Plugin = plugin;
      BinPlugin = binPlugin;
    }

    void SetDefaults (iDocumentNode* defaults)
    {
      csLoaderPluginRec::defaults = defaults;
    }
  };

  class csLoadedPluginVector
  {
  public:
    iPluginManager* plugin_mgr;

    // constructor
    csLoadedPluginVector();
    // destructor
    ~csLoadedPluginVector ();
    /**
    * Find a plugin by its name or load it if it doesn't exist.
    * Supports both binary and normal plugins. Returns 'false' if the
    * plugin doesn't exist.
    */
    bool FindPlugin (const char* Name, iLoaderPlugin*& plug,
      iBinaryLoaderPlugin*& binplug, iDocumentNode*& defaults);
    /// Find a plugin's class ID by its name. Returns 0 if it is not found.
    const char* FindPluginClassID (const char* Name);
    // add a new plugin record
    void NewPlugin (const char* ShortName, iDocumentNode* child);
    /**
    * Delete all loaded plugins.
    */
    void DeleteAll ();

    void SetObjectRegistry (iObjectRegistry* object_reg)
    {
      csLoadedPluginVector::object_reg = object_reg;
    }

  private:
    /// Mutex to make the plugin vector thread-safe.
    CS::Threading::ReadWriteMutex mutex;

    iObjectRegistry* object_reg;

    csArray<csLoaderPluginRec*> vector;

    // Find a loader plugin record
    struct csLoaderPluginRec* FindPluginRec (const char* name);
    // Return the loader plugin from a record, possibly loading the plugin now
    bool GetPluginFromRec (csLoaderPluginRec*, iLoaderPlugin*& plug,
      iBinaryLoaderPlugin*& binplug);
  };
}
CS_PLUGIN_NAMESPACE_END(csparser)

#endif // __CS_LDRPLUG_H__
