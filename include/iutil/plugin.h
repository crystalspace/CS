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
 * Use this macro to query the object registry, loading a plugin if needed.  If
 * an object with a given interface exists in an object registry, get that
 * object from the registry. If the registry query fails, try to load a plugin
 * and get the interface from there. If that succeeds, the interface is added
 * to the registry for future use and given a tag the same name as the
 * requested interface. Example use:
 *
 * \code
 * csRef\<iDynamics\> dynamic_system;
 * CS_QUERY_REGISTRY_PLUGIN(dynamic_system, object_reg, 
 *   "crystalspace.dynamics.ode", iDynamics);
 * \endcode
 *
 * \param obj csRef to hold the discovered or created/loaded object.
 * \param object_reg The object registry (of type iObjectRegistry).
 * \param scf_id The requested SCF class name (ex. "crystalspace.dynamice.ode")
 * \param interface The interface to requested from class scf_id
 *   (ex. iDynamics). This argument is also stringified and used as the objects
 *   tag in the registry.
 * \todo This probably ought to be made more thread-safe by locking the object
 *   registry if possible.
 */
#define CS_QUERY_REGISTRY_PLUGIN(obj,object_reg,scf_id,interface) \
do { \
  obj = CS_QUERY_REGISTRY(object_reg, interface); \
  if (!obj.IsValid()) \
  { \
    csRef<iPluginManager> mgr = CS_QUERY_REGISTRY(object_reg,iPluginManager); \
    if (!mgr.IsValid()) \
    { \
      csReport(object_reg, CS_REPORTER_SEVERITY_ERROR, \
        "crystalspace.plugin.query", "Plugin manager missing from " \
        "object-registry when attempting to query/load class: %s", scf_id); \
    } \
    obj = CS_LOAD_PLUGIN(mgr, scf_id, interface); \
    if (!obj.IsValid()) \
    { \
      csReport(object_reg, CS_REPORTER_SEVERITY_WARNING, \
        "crystalspace.plugin.query", "Failed to load class \"%s\" with " \
        "interface \"" #interface "\"", scf_id); \
    } \
    if (!object_reg->Register(obj, #interface))	\
    { \
      csReport(object_reg, CS_REPORTER_SEVERITY_WARNING, \
	"crystalspace.plugin.query", "Failed to register class \"%s\" with " \
	"tag \"" #interface "\" in the object-registry.", scf_id); \
    } \
  } \
} while (0)

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
    bool init = true) = 0;

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


/**
 * Find a plugin by its class ID. First the plugin with requested class
 * identifier is found, and after this it is queried for the respective
 * interface; if it does not implement the requested interface, 0 is returned.
 * \param mgr An object that implements iPluginManager.
 * \param ClassID The SCF class name (ex. crystalspace.graphics3d.software).
 * \param Interface Desired interface type (ex. iGraphics2D, iVFS, etc.).
 */
template<class Interface>
inline csPtr<Interface> csQueryPluginClass (iPluginManager *mgr,
                                            const char* ClassID)
{
  iBase* base (mgr->QueryPlugin (ClassID,
    scfInterfaceTraits<Interface>::GetName(),
    scfInterfaceTraits<Interface>::GetVersion()));

  if (base == 0) return csPtr<Interface> (0);

  Interface *x = (Interface*)base->QueryInterface (
    scfInterfaceTraits<Interface>::GetID (),
    scfInterfaceTraits<Interface>::GetVersion ());

  if (x) base->DecRef (); //release our base interface

  return csPtr<Interface> (x);
}

/**
 * Compatbility macro.
 * \sa csQueryPluginClass<Interface> (Object, ClassID);
 */
#define CS_QUERY_PLUGIN_CLASS(Object,ClassID,Interface)			\
  csQueryPluginClass<Interface> (Object, ClassID)

/**
 * Tell plugin manager to load a plugin.
 * \param mgr An object that implements iPluginManager.
 * \param ClassID The SCF class name (ex. crystalspace.graphics3d.software).
 * \param Interface Desired interface type (ex. iGraphics2D, iVFS, etc.).
 */
template<class Interface>
inline csPtr<Interface> csLoadPlugin (iPluginManager *mgr,
                                      const char* ClassID)
{
  iBase* base (mgr->LoadPlugin (ClassID));

  if (base == 0) return csPtr<Interface> (0);

  Interface *x = (Interface*)base->QueryInterface (
    scfInterfaceTraits<Interface>::GetID (),
    scfInterfaceTraits<Interface>::GetVersion ());

  if (x) base->DecRef (); //release our base interface

  return csPtr<Interface> (x);
}

/**
 * Compatbility macro.
 * \sa csLoadPlugin<Interface> (mgr, ClassID);
 */
#define CS_LOAD_PLUGIN(Object,ClassID,Interface)			\
  csLoadPlugin<Interface> (Object, ClassID)

/**
 * Same as csLoadPlugin() but does not bother asking for a interface.
 * This is useful for unconditionally loading plugins.
 */
inline csPtr<iBase> csLoadPluginAlways (iPluginManager *mgr,
                                        const char* ClassID)
{
  iBase* base (mgr->LoadPlugin (ClassID));
  return csPtr<iBase> (base);
}

/**
 * Compatbility macro.
 * \sa csLoadPluginAlways (mgr, ClassID);
 */
#define CS_LOAD_PLUGIN_ALWAYS(Object,ClassID)				\
  csLoadPluginAlways (Object, ClassID)

/** @} */

#endif // __CS_IUTIL_PLUGIN_H__
