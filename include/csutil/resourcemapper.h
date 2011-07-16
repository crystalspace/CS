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
      void Initialise (iObjectRegistry* objectReg_)
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
         * <resources>
         *   <resource classID="crystalspace.blah.foo.bar">
         *     <tag>foo</tag>
         *     <tag>bar</tag>
         *   </resource>
         * </resources>
         */

        // Sanity check.
        if (strcmp (node->GetValue (), "resources") != 0)
        {
          csReport (objectReg, CS_REPORTER_SEVERITY_ERROR, "CS::Resource::Mapper",
            "Invalid document node passed to 'Register' - missing 'resources' root node.");
          return false;
        }

        csRef<iDocumentNodeIterator> resourceNodes = node->GetNodes ("resource");
        while (resourceNodes->HasNext ())
        {
          csRef<iDocumentNode> resourceNode = resourceNodes->Next ();

          // Get the class ID of the loadable.
          const char* classID = resourceNode->GetAttributeValue ("classID");
          if (!classID)
          {
            csReport (objectReg, CS_REPORTER_SEVERITY_ERROR, "CS::Resource::Mapper",
              "Missing class ID in resource!");
            return false;
          }

          // Load this plugin if not already loaded.
          csRef<PluginType> plugin = csLoadPluginCheck<PluginType> (pluginManager, classID);
          if (!plugin.IsValid ())
            return false;

          // Register each loadable-tag pair.
          csRef<iDocumentNodeIterator> tagNodes = resourceNode->GetNodes ("tag");
          while (tagNodes->HasNext ())
          {
            csRef<iDocumentNode> tagNode = tagNodes->Next ();

            Register (plugin, tagNode->GetContentsValue ());
          }
        }

        return true;
      }

      /**
       * Registers a plugin with the mapper from a plugin classID
       * and name.
       */
      bool Register (const char* classID, const char* name)
      {
        // Load this plugin if not already loaded.
        csRef<PluginType> loadable = csLoadPluginCheck<PluginType> (pluginManager, classID);
        if (!loadable.IsValid ())
          return false;

        // Register the loadable-tag pair.
        Register (loadable, name);
      }

      /**
      * Registers a plugin with the mapper from a plugin classID
      * and an array of names.
      */
      bool Register (const char* classID, const iStringArray* names)
      {
        // Load this plugin if not already loaded.
        csRef<PluginType> loadable = csLoadPluginCheck<PluginType> (pluginManager, classID);
        if (!loadable.IsValid ())
          return false;

        // Register each loadable-tag pair.
        for (size_t i = 0; i < names->GetSize (); ++i)
        {
          Register (loadable, names->Get (i));
        }
      }

      /**
       * Finds the appropriate plugin for the specified name.
       * Returns nullptr if none found.
       */
      PluginType* GetLoadable (const char* name)
      {
        csStringID id = stringHash.Request (name);
        csRef<PluginType> plugin = plugins.Get (id, csRef<PluginType> ());
        return plugin;
      }

    protected:
      /**
       * Registers a name with a plugin.
       */
      void Register (PluginType* plugin, const char* name)
      {
        // Get the string ID for this tag.
        csStringID id = stringHash.Request (name);

        // Add to the hash.
        plugins.Put (id, plugin);
      }

    private:
      // Object register for error reporting.
      csRef<iObjectRegistry> objectReg;

      // Plugin manager for plugin lookups/loading.
      csRef<iPluginManager> pluginManager;

      // Provides string-id lookups.
      csStringHash stringHash;

      // Maps a stringID tag to a plugin.
      csHash<csRef<PluginType>, csStringID> plugins;
    };

    typedef Mapper<iResourceLoader> LoaderMapper;
    typedef Mapper<iResourceSaver> SaverMapper;
  }
}

#endif // __CS_CSUTIL_LOADABLE_MANAGER_H__
