/*
    The Crystal Space geometry loader interface
    Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_IMAP_LOADERTHREAD_H__
#define __CS_IMAP_LOADERTHREAD_H__

/**\file 
 * Geometry loader interface
 */
/**\addtogroup loadsave
 * @{ */
#include "csutil/scf.h"
#include "csutil/refcount.h"
#include "ivideo/txtmgr.h"
#include "igraphic/image.h"

struct iMeshObjectFactory;
struct iMeshObject;

/**
 * Sector specification used by the threaded loader.
 */
struct csSectorSpecification : public csRefCount
{
  char* sectname;
};

/**
 * Mesh factory specification used by the threaded loader.
 */
struct csMeshFactorySpecification : public csRefCount
{
  char* meshfactname;
  /// key name/values...
  /// Addon?
  csRef<iMeshObjectFactory> factory;
  csMatrix3 move_matrix;
  csVector3 move_vector;
};

/**
 * Mesh object specification used by the threaded loader.
 */
struct csMeshObjectSpecification : public csRefCount
{
  char* meshobjname;
  // key name/values...
  // Addon?
  // Imposter?
  char* priorityname;
  csRef<iMeshObject> object;
  csMatrix3 move_matrix;
  csVector3 move_vector;
  bool nolighting;
  bool noshadows;
  bool invisible;
  bool detail;
  bool camera;
  bool badoccluder;
  bool goodoccluder;
  bool closed;
  bool convex;
  csZBufMode zbuf_mode;
};

SCF_VERSION (iLoadingStatus, 0, 0, 1);

/**
 * This interface is used by LoadMapFileThreaded(). It contains the current
 * loading status. The main thread can query the current status and
 * act accordingly. All functions in this class are mutex protected.
 * <p>
 * This status object acts like a queue of objects. The main app can
 * query this queue and add objects from it. This has to be done in the
 * right order (i.e. sectors before objects and so on).
 */
struct iLoadingStatus : public iBase
{
  /**
   * Return true if loading is finished. In that case you can drop the
   * reference to this loading status object.
   */
  virtual bool HasLoadingFinished () = 0;

  /**
   * Query the number of sectors in the queue.
   */
  virtual int GetSectorCount () = 0;
  /**
   * Fetch the next sector and remove it from the queue.
   */
  virtual csPtr<csSectorSpecification> FetchSector () = 0;

  /**
   * Query the number of mesh factory in the queue.
   */
  virtual int GetMeshFactoryCount () = 0;
  /**
   * Fetch the next mesh factory and remove it from the queue.
   */
  virtual csPtr<csMeshFactorySpecification> FetchMeshFactory () = 0;

  /**
   * Query the number of mesh objects in the queue.
   */
  virtual int GetMeshObjectCount () = 0;
  /**
   * Fetch the next mesh object and remove it from the queue.
   */
  virtual csPtr<csMeshObjectSpecification> FetchMeshObject () = 0;
};

/** } */

#endif // __CS_IMAP_LOADERTHREAD_H__

