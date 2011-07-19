/*
  Copyright (C) 2011 by Michael Gist

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_CSUTIL_RESOURCE_MANAGER_H__
#define __CS_CSUTIL_RESOURCE_MANAGER_H__

#include "csutil/strhash.h"
#include "imap/resource.h"
#include "csutil/resource.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/stringarray.h"

namespace CS
{
  namespace Resource
  {
    /**
     * Maps names to iResource(Loader/Saver) plugins.
     */
    template<class PluginType>
    class Mapper
    {
    public:
      void Initialize (iObjectRegistry* objectReg_)
      {
        objectReg = objectReg_;
        pluginManager = csQueryRegistry<iPluginManager> (objectReg);
      }

      /**
       * Registers resource load/savers with the mapper from a document node.
       */
      bool Register (iDocumentNode* node)
      {
        /**
         * <resourcehandlers>
         *   <resourcehandler classID="crystalspace.blah.foo.bar">
         *     <resourcetype>foo</resourcetype>
         *     <resourcetype>bar</resourcetype>
         *   </resourcehandler>
         * </resourcehandlers>
         */

        // Sanity check.
        if (strcmp (node->GetValue (), "resourcehandlers") != 0)
        {
          csReport (objectReg, CS_REPORTER_SEVERITY_ERROR, "CS::Resource::Mapper",
            "Invalid document node passed to 'Register' - missing 'resources' root node.");
          return false;
        }

        csRef<iDocumentNodeIterator> resourceHandlerNodes = node->GetNodes ("resourcehandler");
        while (resourceHandlerNodes->HasNext ())
        {
          csRef<iDocumentNode> resourceHandlerNode = resourceHandlerNodes->Next ();

          // Get the class ID of the loadable.
          const char* classID = resourceHandlerNode->GetAttributeValue ("classID");
          if (!classID)
          {
            csReport (objectReg, CS_REPORTER_SEVERITY_ERROR, "CS::Resource::Mapper",
              "Missing class ID in resource handler!");
            return false;
          }

          // Load this plugin if not already loaded.
          csRef<PluginType> plugin = csLoadPluginCheck<PluginType> (pluginManager, classID);
          if (!plugin.IsValid ())
            return false;

          // Register each plugin-TypeID pair.
          csRef<iDocumentNodeIterator> resourceNodes = resourceHandlerNode->GetNodes ("resourcetype");
          while (resourceNodes->HasNext ())
          {
            csRef<iDocumentNode> resourceNode = resourceNodes->Next ();
            Register (plugin, CS::Resource::GetTypeID (resourceNode->GetContentsValue ()));
          }
        }

        return true;
      }

      /**
       * Registers a plugin with the mapper from a plugin classID
       * and resource TypeID.
       */
      bool Register (const char* classID, CS::Resource::TypeID typeID)
      {
        // Load this plugin if not already loaded.
        csRef<PluginType> loadable = csLoadPluginCheck<PluginType> (pluginManager, classID);
        if (!loadable.IsValid ())
          return false;

        // Register the plugin-typeID pair.
        Register (loadable, typeID);
      }

      /**
       * Finds the appropriate plugin for the specified resource TypeID.
       * Returns nullptr if none found.
       */
      PluginType* GetLoadable (CS::Resource::TypeID typeID)
      {
        csRef<PluginType> plugin = plugins.Get (typeID, csRef<PluginType> ());
        return plugin;
      }

    protected:
      /**
       * Registers a resource TypeID with a plugin.
       */
      void Register (PluginType* plugin, CS::Resource::TypeID typeID)
      {
        // Add to the hash.
        plugins.Put (typeID, plugin);
      }

    private:
      // Object registry for error reporting.
      iObjectRegistry* objectReg;

      // Plugin manager for plugin lookups/loading.
      csRef<iPluginManager> pluginManager;

      // Maps a stringID tag to a plugin.
      csHash<csRef<PluginType>, CS::Resource::TypeID> plugins;
    };

    typedef Mapper<iResourceLoader> LoaderMapper;
    typedef Mapper<iResourceSaver> SaverMapper;
  }
}

#endif // __CS_CSUTIL_LOADABLE_MANAGER_H__
