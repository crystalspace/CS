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

#ifndef __ISYS_PLUGIN_H__
#define __ISYS_PLUGIN_H__

#include "csutil/scf.h"

struct iObjectRegistry;
struct iEvent;

SCF_VERSION (iPlugin, 0, 0, 2);

/**
 * This is the general plug-in interface for CS.
 * All plug-ins must implement this interface.
 * During Initialize() call plug-in should do all initialization stuff,
 * such as registering with the system driver and so on.
 */
struct iPlugin : public iBase
{
  /// Initialize the plugin, and return success status
  virtual bool Initialize (iObjectRegistry *object_reg) = 0;
  /**
   * This is plugin's event handler. Plugin should register first
   * with an event queue, using iEventQueue::RegisterListener() method,
   * before he'll receive any events. The handler should return true
   * if the event has been handled indeed (and thus to not pass it further).
   * The default implementation of HandleEvent does nothing.
   * NOTE: do NOT return true unless you really handled the event
   * and want the event to not be passed further for processing by
   * other plugins.
   */
  virtual bool HandleEvent (iEvent &/*Event*/)
  { return false; }
};

/*
 * Plugins have an additional characteristic called "functionality ID".
 * This identifier is used by other plugins/engine/system driver to
 * find some plugin that user assigned to perform some function.
 * For example, the "VideoDriver" functionality identifier is used to
 * separate the main 3D graphics driver from other possibly loaded
 * driver that also implements the iGraphics3D interface (it could be
 * the secondary video driver for example).
 *<p>
 * The functionality ID is given in the system config file as left
 * side of assignment in "Plugins" section. For example, in the
 * following line:
 * <pre>
 * [Plugins]
 * VideoDriver = crystal.graphics3d.software
 * </pre>
 * "VideoDriver" is functionality identifier and "crystal.graphics3d.software"
 * is SCF class ID. No two plugins can have same functionality ID. When you
 * load a plugin with System::RequestPlugin() you can define functionality ID
 * after a double colon:
 * <pre>
 * System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
 * </pre>
 * If you load a plugin via the ::LoadPlugin method you just specify the
 * functionality ID as one of arguments.
 */

/// VFS plugin functionality identifier
#define CS_FUNCID_VFS		"VFS"
/// Functionality ID for the video driver
#define CS_FUNCID_VIDEO		"VideoDriver"
/// Canvas plugin funcID (AKA 2D graphics driver)
#define CS_FUNCID_CANVAS	"VideoCanvas"
/// Sound renderer
#define CS_FUNCID_SOUND		"SoundRender"
/// Image loader
#define CS_FUNCID_IMGLOADER	"ImageLoader"
/// Sound loader
#define CS_FUNCID_SNDLOADER	"SoundLoader"
/// Image loader
#define CS_FUNCID_LVLLOADER	"LevelLoader"
/// Font server
#define CS_FUNCID_FONTSERVER	"FontServer"
/// Network driver
#define CS_FUNCID_NETDRV	"NetDriver"
/// Console
#define CS_FUNCID_CONSOLE	"Console.Output"
/// 3D engine
#define CS_FUNCID_ENGINE	"Engine"
/// Skeleton Animation
#define CS_FUNCID_MOTION	"MotionManager"
/// Reporting
#define CS_FUNCID_REPORTER	"Reporter"
/// Model Importer
#define CS_FUNCID_CONVERTER	"Converter"
/// Model Crossbuilder
#define CS_FUNCID_CROSSBUILDER	"CrossBuilder"

/**
 * Query a pointer to some plugin through the Plugin Manager interface.
 * `Object' is a object that implements iPluginManager interface.
 * `Interface' is a interface name (iGraphics2D, iVFS and so on).
 */
#define CS_QUERY_PLUGIN(Object,Interface)				\
  (Interface *)((Object)->QueryPlugin (#Interface, VERSION_##Interface))

/**
 * Find a plugin by his functionality ID. First the plugin with requested
 * functionality identifier is found, and after this it is queried for the
 * respective interface; if it does not implement the requested interface,
 * NULL is returned. NULL is also returned if the plugin with respective
 * functionality is simply not found. If you need the plugin with given
 * functionality identifier no matter which interface it implements, ask
 * for some basic interface, say iBase or iPlugin.
 */
#define CS_QUERY_PLUGIN_ID(Object,FuncID,Interface)			\
  (Interface*)((Object)->QueryPlugin (FuncID, #Interface, VERSION_##Interface))

/**
 * Find a plugin by his class ID and functionality ID. First the plugin
 * with requested class identifier and functionality identifier is found,
 * and after this it is queried for the respective interface; if it does
 * not implement the requested interface, NULL is returned. NULL is also
 * returned if the plugin with respective functionality is simply not
 * found. If you need the plugin with given functionality identifier no
 * matter which interface it implements, ask for some basic interface,
 * say iBase or iPlugin.
 */
#define CS_QUERY_PLUGIN_CLASS(Object,ClassID,FuncID,Interface)		\
  (Interface *)((Object)->QueryPlugin					\
  (ClassID, FuncID, #Interface, VERSION_##Interface))

/**
 * Tell plugin manager driver to load a plugin.
 * `Object' is a object that implements iPluginManager interface.
 * `ClassID' is the class ID (`crystalspace.graphics3d.software').
 * `Interface' is a interface name (iGraphics2D, iVFS and so on).
 */
#define CS_LOAD_PLUGIN(Object,ClassID,FuncID,Interface)			\
  (Interface *)((Object)->LoadPlugin					\
  (ClassID, FuncID, #Interface, VERSION_##Interface))

/**
 * Same as CS_LOAD_PLUGIN but don't bother asking for a interface.
 * This is useful for unconditionally loading plugins.
 */
#define _CS_LOAD_PLUGIN(Object,ClassID,FuncID)				\
  ((Object)->LoadPlugin (ClassID, FuncID, NULL, 0))

SCF_VERSION (iPluginManager, 0, 0, 1);

/**
 * This is the plugin manager.
 */
struct iPluginManager : public iBase
{
  /// Load a plugin and initialize it.
  virtual iBase *LoadPlugin (const char *iClassID, const char *iFuncID,
    const char *iInterface = NULL, int iVersion = 0) = 0;
  /// Get first of the loaded plugins that supports given interface ID.
  virtual iBase *QueryPlugin (const char *iInterface, int iVersion) = 0;
  /// Find a plugin given his functionality ID.
  virtual iBase *QueryPlugin (const char *iFuncID, const char *iInterface,
  	int iVersion) = 0;
  /// Find a plugin given his class ID and functionality ID.
  virtual iBase *QueryPlugin (const char* iClassID, const char *iFuncID,
  	const char *iInterface, int iVersion) = 0;
  /// Remove a plugin from system driver's plugin list.
  virtual bool UnloadPlugin (iPlugin *iObject) = 0;
  /// Register a object that implements the iPlugin interface as a plugin.
  virtual bool RegisterPlugin (const char *iClassID, const char *iFuncID,
    iPlugin *iObject) = 0;
  /// Get the number of loaded plugins in the plugin manager.
  virtual int GetPluginCount () = 0;
  /// Get the specified plugin from the plugin manager.
  virtual iBase* GetPlugin (int idx) = 0;
};

#endif // __ISYS_PLUGIN_H__
