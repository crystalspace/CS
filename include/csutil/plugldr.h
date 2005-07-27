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

#include "csextern.h"
#include "csutil/parray.h"
#include "csutil/ref.h"
#include "csutil/scf.h"

struct iObjectRegistry;

/** \internal
 * Helper class for csPluginLoader.
 */
struct csPluginLoadRec
{
  char* Tag;
  char* ClassID;
  csRef<iBase> plugin;

  csPluginLoadRec (const char* iTag, const char* iClassID)
  { Tag = csStrNew (iTag); ClassID = csStrNew (iClassID); }
  ~csPluginLoadRec ()
  { delete [] ClassID; delete [] Tag; }
};

/** \internal
 * Helper class for csPluginLoader.
 */
class CS_CRYSTALSPACE_EXPORT csPluginList : public csPDelArray<csPluginLoadRec>
{
public:
  bool Sort (iObjectRegistry* object_reg);
private:
  bool RecurseSort (iObjectRegistry*, size_t row, size_t* order, size_t* loop,
    bool *matrix);
};

/**
 * This utility class helps to load plugins based on request,
 * config file, and commandline.
 */
class CS_CRYSTALSPACE_EXPORT csPluginLoader
{
  friend class csPluginList;

private:
  // The object registry.
  iObjectRegistry* object_reg;
  // Requested plugins.
  csPluginList requested_plugins;

public:
  /// Initialize.
  csPluginLoader (iObjectRegistry* object_reg);
  /// Deinitialize.
  virtual ~csPluginLoader ();

  /**
   * A shortcut for requesting to load a plugin (before LoadPlugins()).
   * If you want this class to register the plugin as a default for
   * some interface then you should use the interface name as the
   * tag name (i.e. 'iGraphics3D').
   * Note that plugins requested with some tag here get lowest precendence.
   * The commandline has highest priority followed by the config file.
   * If after this no plugin with the given tag exists then RequestPlugin()
   * will work.
   */
  void RequestPlugin (const char* pluginName, const char* tagName);

  /// Load the plugins.
  bool LoadPlugins ();
};

#endif // __CS_PLUGLDR_H__

