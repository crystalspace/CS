/*
    Copyright (C) 2001,2006 by Jorrit Tyberghein
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
 * Plugin manager interface
 */
 
/**
 * \addtogroup scf
 * @{ */


#include "csutil/scf.h"
#include "csutil/stringquote.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/threadmanager.h"
#include "ivaria/reporter.h"

/**
 * An iterator to iterate over all plugins in the plugin manager.
 */
struct iPluginIterator : public virtual iBase
{
  SCF_INTERFACE(iPluginIterator, 3,0,0);
  /// Are there more elements?
  virtual bool HasNext () = 0;
  /// Get next element.
  virtual iComponent* Next () = 0;
};

/**
 * This is the plugin manager.
 * The plugin manager is guaranteed thread-safe.
 *
 * Main creators of instances implementing this interface:
 * - csInitializer::CreateEnvironment()
 * - csInitializer::CreatePluginManager()
 * 
 * Main ways to get pointers to this interface:
 * - csQueryRegistry()
 */
struct iPluginManager : public virtual iBase
{
  SCF_INTERFACE(iPluginManager, 4, 0, 1);
  
  /**
   * LoadPluginInstance flags.
   * \da LoadPluginInstance
   */
  enum
  {
    /// Initialize plugin
    lpiInitialize = 1,
    /// Report loading errors
    lpiReportErrors = 2,
    /// Load dependent plugins
    lpiLoadDependencies = 4,
    /// Return an existing instance of the plugin if it exists, else create a new one.
    lpiReturnLoadedInstance = 8
  };
  
  /**
   * Load a plugin (optionally loading dependencies) and (optionally)
   * initialize it.
   * If the #lpiInitialize flag is given then the plugin will be initialized 
   * (that is, the iComponent's Initialize() method is called) and QueryOptions()
   * will be called. <b>This is risky.</b> In a multi-threaded situation two
   * threads may request the same plugin (almost) simultaneously; if one thread
   * does not request initialization, but another thread does (and assumes to
   * get an initialized instance), the other thread may in fact receive an
   * uninitialized instance (and likely break). Use this flag with caution!
   * If the #lpiReportErrors flag is given then loading failures are 
   * reported using the reporter.
   * If the #lpiLoadDependencies flag is given dependent plugins (as specified
   * in the plugin's metadata) will be loaded as well. Note that dependencies
   * are *always* loaded.
   * \param classID Class ID of the plugin to load.
   * \param loadFlags Load options.
   */
  virtual csPtr<iComponent> LoadPluginInstance (const char *classID,
                                                uint loadFlags) = 0;
  // Deprecated in 1.9
  CS_DEPRECATED_METHOD_MSG("Use LoadPluginInstance()")
  inline iBase* LoadPlugin (const char *classID, bool init = true, bool report = true)
  {
    uint flags = 0;
    if (init) flags |= lpiInitialize;
    if (report) flags |= lpiReportErrors;
    csRef<iComponent> comp (LoadPluginInstance (classID, flags));
    if (comp) comp->IncRef();
    return (iBase*)comp;
  }

  /**
   * Get one of the loaded plugins that supports given interface ID.
   * \warning Usage of this function is usually not safe since multiple
   * plugins can implement the same interface and there is no way to know
   * which one is the correct one.  This method will return a random plugin
   * providing the given interface.
   * It is usually better to use the object registry to obtain components
   * with a certain interface.
   */
  virtual csPtr<iComponent> QueryPluginInstance (const char *iInterface,
                                                 int iVersion) = 0;
  // Deprecated in 1.9
  CS_DEPRECATED_METHOD_MSG("Use QueryPluginInstance()")
  inline iBase* QueryPlugin (const char *iInterface, int iVersion)
  {
    csRef<iComponent> comp (QueryPluginInstance (iInterface, iVersion));
    if (comp) comp->IncRef();
    return (iBase*)comp;
  }
  
  //@{
  /**
   * Find a plugin given its class ID.
   * \warning It is valid to load multiple plugin instances for the same
   * plugin class IDs. If this is the case, querying for an instance of a
   * plugin class ID will return a random loaded plugin instance.
   */
  virtual csPtr<iComponent> QueryPluginInstance (const char* classID) = 0;
  virtual csPtr<iComponent> QueryPluginInstance (const char* classID,
  	const char *iInterface, int iVersion) = 0;
  //@}
  // Deprecated in 1.9
  CS_DEPRECATED_METHOD_MSG("Use QueryPluginInstance()")
  inline iBase* QueryPlugin (const char* classID,
  	const char *iInterface, int iVersion)
  {
    csRef<iComponent> comp (QueryPluginInstance (classID, iInterface, iVersion));
    if (comp) comp->IncRef();
    return (iBase*)comp;
  }
  
  /// Remove a plugin from the plugin manager's plugin list.
  virtual bool UnloadPluginInstance (iComponent *obj) = 0;
  // Deprecated in 1.9
  CS_DEPRECATED_METHOD_MSG("Use UnloadPluginInstance()")
  inline bool UnloadPlugin (iComponent *obj)
  { return UnloadPluginInstance (obj); }
  
  /// Register a object that implements the iComponent interface as a plugin.
  virtual bool RegisterPluginInstance (const char *classID, iComponent *obj) = 0;
  // Deprecated in 1.9
  CS_DEPRECATED_METHOD_MSG("Use RegisterPluginInstance()")
  inline bool RegisterPlugin (const char *classID, iComponent *obj)
  { return RegisterPluginInstance (classID, obj); }

  /**
   * Get an iterator to iterate over all loaded plugins in the plugin manager.
   * This iterator will contain a copy of the plugins so it will not lock
   * the plugin manager while looping over the plugins.
   */
  virtual csPtr<iPluginIterator> GetPluginInstances () = 0;
  // Deprecated in 1.9
  CS_DEPRECATED_METHOD_MSG("Use GetPluginInstances()")
  inline csPtr<iPluginIterator> GetPlugins ()
  { return GetPluginInstances (); }
  /// Unload all plugins from this plugin manager.
  virtual void Clear () = 0;

  /**
   * Query all options supported by given plugin instance and place into
   * OptionList.
   * Normally this is done automatically by LoadPlugin() if 'init' is true.
   * If 'init' is not true then you can call this function AFTER calling
   * object->Initialize().
   */
  virtual void QueryOptions (iComponent* object) = 0;
  
  /**
   * Set the class ID mapped to an object registry tag.
   * Tag mappings are used in dependency resolution while plugin loading.
   */
  virtual bool SetTagClassIDMapping (const char* tag, const char* classID) = 0;
  /// Remove a class ID mapping for a tag
  virtual bool UnsetTagClassIDMapping (const char* tag) = 0;
  /// Get the class ID mapped to a tag
  virtual const char* GetTagClassIDMapping (const char* tag) = 0;
  /**
   * Get all tags mapped to that class ID.
   * \a classID can be wildcard (ending in '.') which means all class IDs
   * starting with \a classID are considered,
   */
  virtual csPtr<iStringArray> GetClassIDTags (const char* classID) = 0;
  /**
   * Load a plugin for a tag.
   * Canonically equivalent to LoadTagPluginInstance (
   * GetTagClassIDMapping (\a tag), \a loadFlags).
   */
  virtual csPtr<iComponent> LoadTagPluginInstance (const char* tag,
    uint loadFlags) = 0;
  /**
   * Query a plugin for a tag.
   * Canonically equivalent to QueryPluginInstance (GetTagClassIDMapping (\a tag)).
   */
  virtual csPtr<iComponent> QueryTagPluginInstance (const char* tag) = 0;
  
};


/**
 * Find a plugin by its class ID. First the plugin with requested class
 * identifier is found, and after this it is queried for the respective
 * interface; if it does not implement the requested interface, 0 is returned.
 * \a Interface: Desired interface type (ex. iGraphics2D, iVFS, etc.).
 * \param mgr An object that implements iPluginManager.
 * \param ClassID The SCF class name (ex. crystalspace.graphics3d.software).
 */
template<class Interface>
inline csPtr<Interface> csQueryPluginClass (iPluginManager *mgr,
                                            const char* ClassID)
{
  csRef<iComponent> base = mgr->QueryPluginInstance (ClassID,
    scfInterfaceTraits<Interface>::GetName(),
    scfInterfaceTraits<Interface>::GetVersion());
  return scfQueryInterfaceSafe<Interface> (base);
}

/**
 * Compatbility macro.
 * \sa csQueryPluginClass<Interface> (Object, ClassID);
 */
#define CS_QUERY_PLUGIN_CLASS(Object,ClassID,Interface)			\
  csQueryPluginClass<Interface> (Object, ClassID)

/**
 * Tell plugin manager to load a plugin (and its dependencies).
 * \a Interface: Desired interface type (ex. iGraphics2D, iVFS, etc.).
 * \param mgr An object that implements iPluginManager.
 * \param ClassID The SCF class name (ex. crystalspace.graphics3d.software).
 */
template<class Interface>
inline csPtr<Interface> csLoadPlugin (iPluginManager *mgr,
                                      const char* ClassID,
                                      bool report = true,
                                      bool returnLoadedInstance = false)
{
  csRef<iComponent> base;
  uint flags = iPluginManager::lpiInitialize | iPluginManager::lpiLoadDependencies;
  if (report) flags |= iPluginManager::lpiReportErrors;
  if (returnLoadedInstance) flags |= iPluginManager::lpiReturnLoadedInstance;
  base = mgr->LoadPluginInstance (ClassID, flags);
  return scfQueryInterfaceSafe<Interface> (base);
}

/**
 * Tell plugin manager to load a plugin (and its dependencies).
 * \a Interface: Desired interface type (ex. iGraphics2D, iVFS, etc.).
 * \param object_reg object registry
 * \param ClassID The SCF class name (ex. crystalspace.graphics3d.software).
 */
template<class Interface>
inline csPtr<Interface> csLoadPlugin (iObjectRegistry* object_reg,
                                      const char* ClassID,
                                      bool report = true)
{
  csRef<iPluginManager> mgr = csQueryRegistry<iPluginManager> (object_reg);
  if (!mgr) return 0;
  return csLoadPlugin<Interface> (mgr, ClassID, report);
}

/**
 * Tell plugin manager to load a plugin but first check if the plugin
 * is not already loaded.
 * \a Interface: Desired interface type (ex. iGraphics2D, iVFS, etc.).
 * \param mgr An object that implements iPluginManager.
 * \param ClassID The SCF class name (ex. crystalspace.graphics3d.software).
 */
template<class Interface>
inline csPtr<Interface> csLoadPluginCheck (iPluginManager *mgr,
                                           const char* ClassID,
                                           bool report = true)
{
  csRef<Interface> i = csQueryPluginClass<Interface> (mgr, ClassID);
  if (i) return (csPtr<Interface>) i;
  i = csLoadPlugin<Interface> (mgr, ClassID, report, true);
  if (!i) return 0;
  return (csPtr<Interface>) i;
}

/**
 * Tell plugin manager to load a plugin but first check if the plugin
 * is not already loaded.
 * \a Interface: Desired interface type (ex. iGraphics2D, iVFS, etc.).
 * \param object_reg object registry
 * \param ClassID The SCF class name (ex. crystalspace.graphics3d.software).
 * \param report if true then we will report an error in case of error.
 */
template<class Interface>
inline csPtr<Interface> csLoadPluginCheck (iObjectRegistry* object_reg,
                                      const char* ClassID, bool report = true)
{
  csRef<iPluginManager> mgr = csQueryRegistry<iPluginManager> (object_reg);
  if (!mgr)
  {
    if (report)
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      	"crystalspace.plugin.load", "Couldn't find plugin manager!");
    return 0;
  }
  csRef<Interface> i = csLoadPluginCheck<Interface> (mgr, ClassID, report);
  if (!i)
  {
    if (report)
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      	"crystalspace.plugin.load", "Couldn't load plugin with class %s!",
		CS::Quote::Single (ClassID));
    return 0;
  }
  return (csPtr<Interface>) i;
}

/**
 * Same as csLoadPlugin() but does not bother asking for a interface.
 * This is useful for unconditionally loading plugins.
 */
inline csPtr<iComponent> csLoadPluginAlways (iPluginManager *mgr,
                                             const char* ClassID,
				             bool report = true)
{
  uint flags =
    iPluginManager::lpiInitialize | iPluginManager::lpiLoadDependencies;
  if (report) flags |= iPluginManager::lpiReportErrors;
  return mgr->LoadPluginInstance (ClassID, flags);
}

/**
 * Use this macro to query the object registry, loading a plugin if needed.  If
 * an object with a given interface exists in an object registry, get that
 * object from the registry. If the registry query fails, try to load a plugin
 * and get the interface from there. If that succeeds, the interface is added
 * to the registry for future use and given a tag the same name as the
 * requested interface. Example use:
 *
 * \code
 * csRef<iDynamics> dynamic_system = csQueryRegistryOrLoad<iDynamics> (
 	object_reg, "crystalspace.dynamics.ode");
 * \endcode
 *
 * \param Reg The object registry (of type iObjectRegistry).
 * \param classID The requested SCF class name (ex. "crystalspace.dynamice.ode")
 * \param report if true then we will report an error in case of error.
 * \todo This probably ought to be made more thread-safe by locking the object
 *   registry if possible.
 * \return a reference to the requested interface if successful. Otherwise
 *   this function returns 0.
 */
template<class Interface>
inline csPtr<Interface> csQueryRegistryOrLoad (iObjectRegistry *Reg,
	const char* classID, bool report = true)
{
  csRef<Interface> i = csQueryRegistry<Interface> (Reg);
  if (i) return (csPtr<Interface>)i;
  csRef<iPluginManager> plugmgr = csQueryRegistry<iPluginManager> (Reg);
  if (!plugmgr)
  {
    if (report)
      csReport (Reg, CS_REPORTER_SEVERITY_ERROR,
      	"crystalspace.plugin.query", "Plugin manager missing!");
    return 0;
  }
  i = csLoadPlugin<Interface> (plugmgr, classID, report);
  if (!i)
  {
    if (report)
      csReport (Reg, CS_REPORTER_SEVERITY_ERROR,
      	"crystalspace.plugin.query",
	"Couldn't load plugin with class %s!", CS::Quote::Single (classID));
    return 0;
  }
  if (!Reg->Register (i, scfInterfaceTraits<Interface>::GetName ()))
  {
    if (report)
      csReport (Reg, CS_REPORTER_SEVERITY_ERROR,
      	"crystalspace.plugin.query",
	"Couldn't register plugin with class %s!", CS::Quote::Single (classID));
    return 0;
  }
  return (csPtr<Interface>)i;
}

/** @} */

#endif // __CS_IUTIL_PLUGIN_H__
