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

#include "cssysdef.h"
#include "csutil/loadablemanager.h"
#include "iutil/stringarray.h"

csLoadableManager::csLoadableManager (iObjectRegistry* objectReg)
: objectReg (objectReg)
{
  pluginManager = csQueryRegistry<iPluginManager> (objectReg);
}

bool csLoadableManager::Register (iDocumentNode* node)
{
  /**
   * <loadables>
   *   <loadable classID="crystalspace.blah.foo.bar">
   *     <tag>foo</tag>
   *     <tag>bar</tag>
   *   </loadable>
   * </loadables>
   */

  // Sanity check.
  if (strcmp (node->GetValue (), "loadables") != 0)
  {
    csReport (objectReg, CS_REPORTER_SEVERITY_ERROR, "csLoadableManager",
      "Invalid document node passed to 'Register' - missing 'loadables' root node.");
    return false;
  }

  csRef<iDocumentNodeIterator> loadableNodes = node->GetNodes ("loadable");
  while (loadableNodes->HasNext ())
  {
    csRef<iDocumentNode> loadableNode = loadableNodes->Next ();

    // Get the class ID of the loadable.
    const char* classID = loadableNode->GetAttributeValue ("classID");
    if (!classID)
    {
      csReport (objectReg, CS_REPORTER_SEVERITY_ERROR, "csLoadableManager",
        "Missing class ID in loadable!");
      return false;
    }

    // Load this plugin if not already loaded.
    csRef<iLoadable> loadable = csLoadPluginCheck<iLoadable> (pluginManager, classID);
    if (!loadable.IsValid ())
      return false;

    // Register each loadable-tag pair.
    csRef<iDocumentNodeIterator> tagNodes = loadableNode->GetNodes ("tag");
    while (tagNodes->HasNext ())
    {
      csRef<iDocumentNode> tagNode = tagNodes->Next ();

      Register (loadable, tagNode->GetContentsValue ());
    }
  }

  return true;
}

bool csLoadableManager::Register (const char* classID, const char* tag)
{
  // Load this plugin if not already loaded.
  csRef<iLoadable> loadable = csLoadPluginCheck<iLoadable> (pluginManager, classID);
  if (!loadable.IsValid ())
    return false;

  // Register the loadable-tag pair.
  Register (loadable, tag);
}

bool csLoadableManager::Register (const char* classID, const iStringArray* tags)
{
  // Load this plugin if not already loaded.
  csRef<iLoadable> loadable = csLoadPluginCheck<iLoadable> (pluginManager, classID);
  if (!loadable.IsValid ())
    return false;

  // Register each loadable-tag pair.
  for (size_t i = 0; i < tags->GetSize (); ++i)
  {
    Register (loadable, tags->Get (i));
  }
}

iLoadable* csLoadableManager::GetLoadable (const char* tag)
{
  csStringID id = stringHash.Request (tag);
  csRef<iLoadable> loadable = loadables.Get (id, csRef<iLoadable> ());
  return loadable;
}

void csLoadableManager::Register (iLoadable* loadable, const char* tag)
{
  // Get the string ID for this tag.
  csStringID id = stringHash.Request (tag);

  // Add to the hash.
  loadables.Put (id, loadable);
}
