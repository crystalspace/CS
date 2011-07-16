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

#ifndef __CS_CSUTIL_LOADABLE_MANAGER_H__
#define __CS_CSUTIL_LOADABLE_MANAGER_H__

#include "csutil/strhash.h"
#include "imap/loadable.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"

/**
 * Manages a mapping of tag names to iLoadable scf plugins.
 */
class csLoadableManager
{
public:
  csLoadableManager (iObjectRegistry* objectReg);

  /**
   * Registers loadables with the manager from a document node.
   */
  bool Register (iDocumentNode* node);

  /**
   * Registers a loadable with the manager from a plugin classID
   * and a tag name.
   */
  bool Register (const char* classID, const char* tag);

  /**
   * Registers a loadable with the manager from a plugin classID
   * and an array of tag names.
   */
  bool Register (const char* classID, const iStringArray* tags);

  /**
   * Finds the appropriate loadable for the specified tag.
   * Returns nullptr if none found.
   */
  iLoadable* GetLoadable (const char* tag);

protected:
  /**
   * Registers a loadable and tag pair with the manager.
   */
  void Register (iLoadable* loadable, const char* tag);

private:
  // Object register for error reporting.
  csRef<iObjectRegistry> objectReg;

  // Plugin manager for loadable lookups/loading.
  csRef<iPluginManager> pluginManager;

  // Provides string-id lookups.
  csStringHash stringHash;

  // Maps a stringID tag to a loadable plugin.
  csHash<csRef<iLoadable>, csStringID> loadables;
};

#endif // __CS_CSUTIL_LOADABLE_MANAGER_H__
