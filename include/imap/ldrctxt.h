/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __IMAP_LDRCTXT_H__
#define __IMAP_LDRCTXT_H__

#include "csutil/scf.h"

struct iMaterialWrapper;
struct iMeshFactoryWrapper;
struct iMeshWrapper;
struct iSector;
class csParser;

SCF_VERSION (iLoaderContext, 0, 0, 2);

/**
 * This interface gives the context for the loader.
 * It basically gives loading plugins a way to find materials,
 * meshes, and sectors. In all these cases region-qualified
 * names can be used (i.e. 'regionname/objectname') or normal
 * object names.
 * <p>
 * WARNING! When a context is created it should not be modified
 * afterwards. Some loader plugins will keep a reference to the
 * context when they support delayed loading. In that case they
 * still need the original contents in the context. So a loader
 * that creates a context should create a new one every time.
 */
struct iLoaderContext : public iBase
{
  /// Find a sector.
  virtual iSector* FindSector (const char* name) = 0;
  /// Find a material.
  virtual iMaterialWrapper* FindMaterial (const char* name) = 0;
  /// Find a mesh factory.
  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name) = 0;
  /// Find a mesh object.
  virtual iMeshWrapper* FindMeshObject (const char* name) = 0;
  /// Returns the parser used.
  virtual csParser* GetParser () = 0;
};

#endif

