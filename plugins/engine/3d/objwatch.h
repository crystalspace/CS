/*
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

#ifndef __CS_OBJWATCH_H__
#define __CS_OBJWATCH_H__

#include "csutil/array.h"
#include "csutil/refarr.h"
#include "csutil/weakrefarr.h"
#include "csutil/scf_implementation.h"
#include "iengine/objwatch.h"

struct iLight;
struct iMovable;
class csLightCallback;
class csMovableListener;
class csSectorListener;

/**
 * This class implements iObjectWatcher and is capable of keeping
 * track of lights and movables.
 */
class csObjectWatcher : public scfImplementation1<csObjectWatcher,
                                                  iObjectWatcher>
{
private:
  // Lights we are watching.
  csArray<iLight*> lights;
  // Movables we are watching.
  csArray<iMovable*> movables;
  // Sectors we are watching.
  csWeakRefArray<iSector> sectors;
  // Our light listener.
  csLightCallback* light_callback;
  // Our movable listener.
  csMovableListener* movable_listener;
  // Our sector listener.
  csSectorListener* sector_listener;
  // Listeners that are listening to this object watcher.
  csRefArray<iObjectWatcherListener> listeners;

  // Update number.
  long updatenr;

  // Last operation.
  int last_op;
  // Last light (or 0 if last operation has nothing to do with lights).
  iLight* last_light;
  // Last movable (or 0 if last operation has nothing to do with movables).
  iMovable* last_movable;
  // Last sector (or 0 if last operation has nothing to do with sectors).
  iSector* last_sector;
  // Last mesh (or 0 if last operation has nothing to do with meshes).
  iMeshWrapper* last_mesh;

public:
  /**
   * Create a default movable.
   */
  csObjectWatcher ();

  /// Destructor.
  virtual ~csObjectWatcher ();

  void ReportOperation (int op, iMovable* movable);
  void ReportOperation (int op, iLight* light);
  void ReportOperation (int op, iSector* sector, iMeshWrapper* mesh);

  virtual void WatchLight (iLight* light);
  virtual void RemoveLight (iLight* light);
  virtual int GetWatchedLightCount () const { return (int)lights.Length (); }
  virtual iLight* GetLight (int idx);

  virtual void WatchMovable (iMovable* movable);
  virtual void RemoveMovable (iMovable* movable);
  virtual int GetWatchedMovableCount () const
  {
    return (int)movables.Length ();
  }
  virtual iMovable* GetMovable (int idx);

  virtual void WatchSector (iSector* sector);
  virtual void RemoveSector (iSector* sector);
  virtual int GetWatchedSectorCount () const { return (int)sectors.Length (); }
  virtual iSector* GetSector (int idx);

  virtual void Reset ();

  virtual uint32 GetWatchNumber () const { return updatenr; }
  virtual int GetLastOperation () const { return last_op; }
  virtual iLight* GetLastLight () const { return last_light; }
  virtual iMovable* GetLastMovable () const { return last_movable; }
  virtual iSector* GetLastSector () const { return last_sector; }
  virtual iMeshWrapper* GetLastMeshWrapper () const { return last_mesh; }

  virtual void AddListener (iObjectWatcherListener* cb)
  {
    listeners.Push (cb);
  }

  virtual void RemoveListener (iObjectWatcherListener* cb)
  {
    listeners.Delete (cb);
  }
};

#endif // __CS_OBJWATCH_H__

