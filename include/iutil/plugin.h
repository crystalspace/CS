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

#ifndef __CS_IUTIL_PLUGIN_H__
#define __CS_IUTIL_PLUGIN_H__

/**\file
 */
 
/**
 * \addtogroup scf
 * @{ */


#include "csutil/scf.h"

struct iComponent;

/**
 * Find a plugin by his class ID. First the plugin
 * with requested class identifier is found,
 * and after this it is queried for the respective interface; if it does
 * not implement the requested interface, 0 is returned.
 */
#define CS_QUERY_PLUGIN_CLASS(Object,ClassID,Interface)			\
  csPtr<Interface> ((Interface *)((Object)->QueryPlugin			\
  (ClassID, #Interface, scfInterface<Interface>::GetVersion())))

/**
 * Tell plugin manager driver to load a plugin.
 * `Object' is a object that implements iPluginManager interface.
 * `ClassID' is the class ID (`crystalspace.graphics3d.software').
 * `Interface' is a interface name (iGraphics2D, iVFS and so on).
 */
#define CS_LOAD_PLUGIN(Object,ClassID,Interface)			\
  csPtr<Interface> ((Interface *)((Object)->LoadPlugin			\
  (ClassID, #Interface, scfInterface<Interface>::GetVersion())))

SCF_VERSION (iPluginIterator, 0, 0, 1);

/**
 * An iterator to iterate over all plugins in the plugin manager.
 */
struct iPluginIterator : public iBase
{
  /// Are there more elements?
  virtual bool HasNext () = 0;
  /// Get next element.
  virtual iBase* Next () = 0;
};

/**
 * Same as CS_LOAD_PLUGIN but don't bother asking for a interface.
 * This is useful for unconditionally loading plugins.
 */
#define CS_LOAD_PLUGIN_ALWAYS(Object,ClassID)			\
  csPtr<iBase> ((Object)->LoadPlugin (ClassID, 0, 0))

SCF_VERSION (iPluginManager, 0, 2, 0);

/**
 * This is the plugin manager.
 * The plugin manager is guaranteed thread-safe.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>csInitializer::CreateEnvironment()
 *   <li>csInitializer::CreatePluginManager()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>CS_QUERY_REGISTRY()
 *   </ul>
 */
struct iPluginManager : public iBase
{
  /**
   * Load a plugin and (optionally) initialize it.
   * If 'init' is true then the plugin will be initialized and QueryOptions()
   * will be called.
   */
  virtual iBase *LoadPlugin (const char *classID,
    const char *iInterface = 0, int iVersion = 0, bool init = true) = 0;

  /**
   * Get first of the loaded plugins that supports given interface ID.
   * Warning! Usage of this function is usually not safe since multiple
   * plugins can implement the same interface and there is no way to know
   * which one is the correct one. It is better to use the object registry
   * to find about single loaded components.
   */
  virtual iBase *QueryPlugin (const char *iInterface, int iVersion) = 0;
  /// Find a plugin given his class ID.
  virtual iBase *QueryPlugin (const char* classID,
  	const char *iInterface, int iVersion) = 0;
  /// Remove a plugin from system driver's plugin list.
  virtual bool UnloadPlugin (iComponent *obj) = 0;
  /// Register a object that implements the iComponent interface as a plugin.
  virtual bool RegisterPlugin (const char *classID, iComponent *obj) = 0;

  /**
   * Get an iterator to iterate over all loaded plugins in the plugin manager.
   * This iterator will contain a copy of the plugins so it will not lock
   * the plugin manager while looping over the plugins.
   */
  virtual csPtr<iPluginIterator> GetPlugins () = 0;
  /// Unload all plugins from this plugin manager.
  virtual void Clear () = 0;

  /**
   * Query all options supported by given plugin and place into OptionList.
   * Normally this is done automatically by LoadPlugin() if 'init' is true.
   * If 'init' is not true then you can call this function AFTER calling
   * object->Initialize().
   */
  virtual void QueryOptions (iComponent* object) = 0;
};

/** @} */

#endif // __CS_IUTIL_PLUGIN_H__
