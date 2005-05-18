/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __CS_CFGACC_H__
#define __CS_CFGACC_H__

/**\file
 * Convenience class that can be used to deal with the sytem config manager
 */

#include "csextern.h"
#include "array.h"
#include "iutil/cfgmgr.h"

struct iConfigFile;
struct iObjectRegistry;

/**
 * This is a simple convenience class that can be used to deal with the
 * sytem config manager. It adds config files to the manager on demand and
 * automatically removes them when destroyed. In addition, you can use
 * the '->' operator to access the system configuration. Note that you must
 * use the same iObjectRegistry for all config files you add.
 */
class CS_CRYSTALSPACE_EXPORT csConfigAccess
{
public:
  /// constructor
  csConfigAccess();
  /// constructor (provides access to config manager, but doesn't add file)
  csConfigAccess(iObjectRegistry *object_reg);
  /// constructor (also adds a config file)
  csConfigAccess(iObjectRegistry *object_reg, const char *Filename,
    bool vfs = true, int Priority = iConfigManager::ConfigPriorityPlugin);
  /// constructor (also adds a config object)
  csConfigAccess(iObjectRegistry *object_reg, iConfigFile* file,
    int Priority = iConfigManager::ConfigPriorityPlugin);
  /// destructor
  ~csConfigAccess();

  /// add a config file
  void AddConfig(iObjectRegistry *object_reg, const char *Filename,
    bool vfs = true, int Priority = iConfigManager::ConfigPriorityPlugin);
  /// add a config object
  void AddConfig(iObjectRegistry *object_reg, iConfigFile* file,
    int Priority = iConfigManager::ConfigPriorityPlugin);
  /// access to system configuration
  iConfigFile *operator->();
  /// access to system configuration
  operator iConfigFile* ();

private:
  iObjectRegistry *object_reg;
  csArray<iConfigFile*> ConfigFiles;
};

#endif // __CS_CFGACC_H__
