/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Copyright (C) 1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __IUTIL_PLUGIN_H__
#define __IUTIL_PLUGIN_H__

#include "csutil/scf.h"

struct iComponent;

/**
 * Query a pointer to some plugin through the Plugin Manager interface.
 * `Object' is a object that implements iPluginManager interface.
 * `Interface' is a interface name (iGraphics2D, iVFS and so on).
 */
#define CS_QUERY_PLUGIN(Object,Interface)				\
  (Interface *)((Object)->QueryPlugin (#Interface, VERSION_##Interface))

/**
 * Find a plugin by his class ID. First the plugin
 * with requested class identifier is found,
 * and after this it is queried for the respective interface; if it does
 * not implement the requested interface, NULL is returned.
 */
#define CS_QUERY_PLUGIN_CLASS(Object,ClassID,Interface)			\
  (Interface *)((Object)->QueryPlugin					\
  (ClassID, #Interface, VERSION_##Interface))

/**
 * Tell plugin manager driver to load a plugin.
 * `Object' is a object that implements iPluginManager interface.
 * `ClassID' is the class ID (`crystalspace.graphics3d.software').
 * `Interface' is a interface name (iGraphics2D, iVFS and so on).
 */
#define CS_LOAD_PLUGIN(Object,ClassID,Interface)			\
  (Interface *)((Object)->LoadPlugin					\
  (ClassID, #Interface, VERSION_##Interface))

/**
 * Same as CS_LOAD_PLUGIN but don't bother asking for a interface.
 * This is useful for unconditionally loading plugins.
 */
#define CS_LOAD_PLUGIN_ALWAYS(Object,ClassID)			\
  ((Object)->LoadPlugin (ClassID, NULL, 0))

SCF_VERSION (iPluginManager, 0, 0, 2);

/**
 * This is the plugin manager.
 */
struct iPluginManager : public iBase
{
  /// Load a plugin and initialize it.
  virtual iBase *LoadPlugin (const char *iClassID,
    const char *iInterface = NULL, int iVersion = 0) = 0;
  /// Get first of the loaded plugins that supports given interface ID.
  virtual iBase *QueryPlugin (const char *iInterface, int iVersion) = 0;
  /// Find a plugin given his class ID.
  virtual iBase *QueryPlugin (const char* iClassID,
  	const char *iInterface, int iVersion) = 0;
  /// Remove a plugin from system driver's plugin list.
  virtual bool UnloadPlugin (iComponent *iObject) = 0;
  /// Register a object that implements the iComponent interface as a plugin.
  virtual bool RegisterPlugin (const char *iClassID, iComponent *iObject) = 0;
  /// Get the number of loaded plugins in the plugin manager.
  virtual int GetPluginCount () = 0;
  /// Get the specified plugin from the plugin manager.
  virtual iBase* GetPlugin (int idx) = 0;
};

#endif // __IUTIL_PLUGIN_H__
