/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef __CS_PLUGLDR_H__
#define __CS_PLUGLDR_H__

/**\file
 * Plugin loader
 */

#include "csextern.h"
#include "csutil/csstring.h"
#include "csutil/parray.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/util.h"

struct iConfigFile;
struct iCommandLineParser;
struct iObjectRegistry;

/**
 * This utility class helps to load plugins based on request,
 * config file, and commandline.
 */
class CS_CRYSTALSPACE_EXPORT csPluginLoader
{
private:
  // The object registry.
  iObjectRegistry* object_reg;
  
  struct csPluginLoadRec
  {
    csString Tag;
    csString ClassID;
    csRef<iBase> plugin;
  
    csPluginLoadRec (const char* iTag, const char* iClassID)
    : Tag (iTag), ClassID (iClassID) {}
  };
  // Requested plugins.
  csPDelArray<csPluginLoadRec> requested_plugins;

public:
  /// Initialize.
  csPluginLoader (iObjectRegistry* object_reg);
  /// Deinitialize.
  virtual ~csPluginLoader ();

  /**
   * A shortcut for requesting to load a plugin (before LoadPlugins()).
   * If you want this class to register the plugin as a default for
   * some interface then you should use the interface name as the
   * tag name (e.g. 'iGraphics3D').
   */
  void RequestPlugin (const char* pluginName, const char* tagName);

  /**
   * Replace a tag that was requested earlier with the given plugin, or add
   * the plugin and tag.
   * \param pluginName Plugin ID of the plugin to load for the tag.
   * \param tagName Tag to change the plugin of.
   * \return \c true if a plugin was requested for \a tagName and replaced with
   *   \a pluginName; \c false if a new request was added.
   */
  bool ReplaceRequestedPlugin (const char* pluginName, const char* tagName);
  
  /**
   * Add plugins from configuration.
   * \a prefix is the prefix of the configuration keys to scan. The tag
   * name is the key name (minus the prefix), the plugin ID is the associated
   * value. Previously requested plugins with the same tag are replaced.
   */
  void AddConfigurationPlugins (iConfigFile* config, const char* prefix);
  
  /**
   * Add plugins from command line.
   * Handles the <tt>-video</tt>, <tt>-canvas</tt> and <tt>-plugin</tt> command
   * line parameters. Previously requested plugins with the tags for these
   * parameters are replaced.
   */
  void AddCommandLinePlugins (iCommandLineParser* commandLine);

  /// Load the plugins.
  bool LoadPlugins ();
};

#endif // __CS_PLUGLDR_H__

