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

struct iObjectRegistry;

/**
 * This utility class helps to load plugins based on request,
 * config file, and commandline.
 */
class csPluginLoader
{
  friend class csPluginList;

private:
  // The object registry.
  iObjectRegistry* object_reg;

public:
  /// Initialize.
  csPluginLoader (iObjectRegistry* object_reg);
  /// Deinitialize.
  virtual ~csPluginLoader ();

  /// A shortcut for requesting to load a plugin (before LoadPlugins()).
  void RequestPlugin (const char *iPluginName);

  /// Load the plugins.
  bool LoadPlugins ();
};

#endif // __CS_PLUGLDR_H__

