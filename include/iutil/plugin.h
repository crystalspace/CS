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
#include "iutil/objreg.h"
#include "ivaria/reporter.h"


struct iComponent;

/**
 * An iterator to iterate over all plugins in the plugin manager.
 */
struct iPluginIterator : public virtual iBase
{
  SCF_INTERFACE(iPluginIterator, 2,0,0);
  /// Are there more elements?
  virtual bool HasNext () = 0;
  /// Get next element.
  virtual iBase* Next () = 0;
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
  SCF_INTERFACE(iPluginManager, 2,0,0);
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
 * \a Interface: Desired interface type (ex. iGraphics2D, iVFS, etc.).
 * \param mgr An object that implements iPluginManager.
 * \param ClassID The SCF class name (ex. crystalspace.graphics3d.software).
 */
template<class Interface>
inline csPtr<Interface> csQueryPluginClass (iPluginManager *mgr,
                                            const char* ClassID)
{
  iBase* base = mgr->QueryPlugin (ClassID,
    scfInterfaceTraits<Interface>::GetName(),
    scfInterfaceTraits<Interface>::GetVersion());

  if (base == 0) return csPtr<Interface> (0);

  Interface *x = (Interface*)base->QueryInterface (
    scfInterfaceTraits<Interface>::GetID (),
    scfInterfaceTraits<Interface>::GetVersion ());

  base->DecRef (); //release our base interface

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
 * \a Interface: Desired interface type (ex. iGraphics2D, iVFS, etc.).
 * \param mgr An object that implements iPluginManager.
 * \param ClassID The SCF class name (ex. crystalspace.graphics3d.software).
 */
template<class Interface>
inline csPtr<Interface> csLoadPlugin (iPluginManager *mgr,
                                      const char* ClassID)
{
  iBase* base = mgr->LoadPlugin (ClassID);

  if (base == 0) return csPtr<Interface> (0);

  Interface *x = (Interface*)base->QueryInterface (
    scfInterfaceTraits<Interface>::GetID (),
    scfInterfaceTraits<Interface>::GetVersion ());

  base->DecRef (); //release our base interface

  return csPtr<Interface> (x);
}

/**
 * Tell plugin manager to load a plugin.
 * \a Interface: Desired interface type (ex. iGraphics2D, iVFS, etc.).
 * \param object_reg object registry
 * \param ClassID The SCF class name (ex. crystalspace.graphics3d.software).
 */
template<class Interface>
inline csPtr<Interface> csLoadPlugin (iObjectRegistry* object_reg,
                                      const char* ClassID)
{
  csRef<iPluginManager> mgr = csQueryRegistry<iPluginManager> (object_reg);
  if (!mgr) return 0;
  return csLoadPlugin<Interface> (mgr, ClassID);
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
                                      const char* ClassID)
{
  csRef<Interface> i = csQueryPluginClass<Interface> (mgr, ClassID);
  if (i) return (csPtr<Interface>) i;
  i = csLoadPlugin<Interface> (mgr, ClassID);
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
  csRef<Interface> i = csLoadPluginCheck<Interface> (mgr, ClassID);
  if (!i)
  {
    if (report)
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      	"crystalspace.plugin.load", "Couldn't load plugin with class '%s'!",
		ClassID);
    return 0;
  }
  return (csPtr<Interface>) i;
}

/**
 * \deprecated Compatibility macro.
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
  iBase* base = mgr->LoadPlugin (ClassID);
  return csPtr<iBase> (base);
}

/**
 * \deprecated Compatibility macro.
 * \sa csLoadPluginAlways (mgr, ClassID);
 */
#define CS_LOAD_PLUGIN_ALWAYS(Object,ClassID)				\
  csLoadPluginAlways (Object, ClassID)

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
  i = csLoadPlugin<Interface> (plugmgr, classID);
  if (!i)
  {
    if (report)
      csReport (Reg, CS_REPORTER_SEVERITY_ERROR,
      	"crystalspace.plugin.query",
	"Couldn't load plugin with class '%s'!", classID);
    return 0;
  }
  if (!Reg->Register (i, scfInterfaceTraits<Interface>::GetName ()))
  {
    if (report)
      csReport (Reg, CS_REPORTER_SEVERITY_ERROR,
      	"crystalspace.plugin.query",
	"Couldn't register plugin with class '%s'!", classID);
    return 0;
  }
  return (csPtr<Interface>)i;
}

/** @} */

#endif // __CS_IUTIL_PLUGIN_H__
