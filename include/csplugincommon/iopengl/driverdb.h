/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_IOPENGL_DRIVERDB_H__
#define __CS_CSPLUGINCOMMON_IOPENGL_DRIVERDB_H__

#include "iutil/cfgmgr.h"

/**\file
 * Interface to read custom GL driver databases
 */

struct iDocumentNode;

/**
 * Interface to read custom GL driver databases.
 * While in a perfect world the rendering would behave, succeed and fail on
 * every OpenGL implementation the same way, the actual reality requires
 * working around quirks and problems in OpenGL drivers and versions thereof.
 * CS provides the so-called "driver database" (<tt>/config/glshader.xml</tt>)
 * which contains configuration settings to work around known issues for 
 * certain drivers. It contains settings that are suitable an out-of-the-box
 * CrystalSpace; however, if an application employs advanced renderer features
 * (such as complex custom shaders), it may be desireable to provide 
 * driver-dependent tweaks for this custom content as well. For this purpose
 * this interface is provided.
 * \remarks
 * If you wanted to load CS' own driver database with this interface, the
 * following code fragment will give you a rough idea on how to achieve that:
 * \code
 * csRef<iGLDriverDatabase> driverDB = 
 *  scfQueryInterface<iGLDriverDatabase> (graphics2D);
 * if (driverDB.IsValid())
 * {
 *   csRef<iFile> dbFile = VFS->Open ("/config/gldrivers.xml", VFS_FILE_READ);
 *   csRef<iDocument> dbDocument = documentSystem->CreateDocument();
 *   dbDocument->Parse();
 *   csRef<iDocumentNode> dbRoot = doc->GetRoot()->GetNode ("gldriverdb");
 *   driverDB->ReadDatabase (dbRoot);
 * }
 * \endcode
 */
struct iGLDriverDatabase : public virtual iBase
{
  SCF_INTERFACE(iGLDriverDatabase, 0, 0, 1);
  
  /**
   * Read a custom driver database.
   * \param dbRoot Document node containing the <tt>\<configs\></tt> and 
   *   <tt>\<rules\></tt> nodes required by the driver DB as children.
   * \param configPriority Priority with which the individual tweaks are
   *   added to the configuration manager.
   * \param phase Simple rule filter; only rules whose "phase" attribute match
   *   this string are considered. 0 and an empty string are equivalent.
   */
  void ReadDatabase (iDocumentNode* dbRoot, 
    int configPriority = iConfigManager::ConfigPriorityPlugin + 20,
    const char* phase = 0);
};

#endif // __CS_CSPLUGINCOMMON_IOPENGL_DRIVERDB_H__
