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
#include "cssysdef.h"
#include "csqint.h"
#include "csutil/scf_implementation.h"
#include "iengine/light.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "plugins/engine/3d/objwatch.h"

//---------------------------------------------------------------------------

class csMovableListener : public scfImplementation1<csMovableListener,
                                                    iMovableListener>
{
public:
  csObjectWatcher* watcher;

  csMovableListener (csObjectWatcher* watcher)
    : scfImplementationType (this), watcher (watcher)
  {
  }
  virtual ~csMovableListener ()
  {
  }

  virtual void MovableChanged (iMovable* movable)
  {
    watcher->ReportOperation (CS_WATCH_MOVABLE_CHANGED, movable);
  }

  virtual void MovableDestroyed (iMovable* movable)
  {
    watcher->ReportOperation (CS_WATCH_MOVABLE_DESTROY, movable);
    watcher->RemoveMovable (movable);
  }
};


class csLightCallback : public scfImplementation1<csLightCallback,
                                                  iLightCallback>
{
public:
  csObjectWatcher* watcher;

  csLightCallback (csObjectWatcher* watcher)
    : scfImplementationType (this), watcher (watcher)
  {
  }
  virtual ~csLightCallback ()
  {
  }

  virtual void OnColorChange (iLight* light, const csColor& newcolor)
  {
    watcher->ReportOperation (CS_WATCH_LIGHT_COLOR, light);
  }

  virtual void OnPositionChange (iLight* light, const csVector3& newpos)
  {
    watcher->ReportOperation (CS_WATCH_LIGHT_MOVE, light);
  }

  virtual void OnSectorChange (iLight* light, iSector* newsector)
  {
    watcher->ReportOperation (CS_WATCH_LIGHT_SECTOR, light);
  }

  virtual void OnRadiusChange (iLight* light, float newradius)
  {
    watcher->ReportOperation (CS_WATCH_LIGHT_RADIUS, light);
  }

  virtual void OnDestroy (iLight* light)
  {
    watcher->ReportOperation (CS_WATCH_LIGHT_DESTROY, light);
    watcher->RemoveLight (light);
  }

  virtual void OnAttenuationChange (iLight* light, int newatt)
  {
    watcher->ReportOperation (CS_WATCH_LIGHT_ATTENUATION, light);
  }
};

class csSectorListener : public scfImplementation1<csSectorListener,
                                                  iSectorMeshCallback>
{
public:
  csObjectWatcher* watcher;

  csSectorListener (csObjectWatcher* watcher)
    : scfImplementationType (this), watcher (watcher)
  {
  }
  virtual ~csSectorListener ()
  {
  }

  virtual void NewMesh (iSector* sector, iMeshWrapper* mesh)
  {
    watcher->ReportOperation (CS_WATCH_SECTOR_NEWMESH, sector, mesh);
  }

  virtual void RemoveMesh (iSector* sector, iMeshWrapper* mesh)
  {
    watcher->ReportOperation (CS_WATCH_SECTOR_REMOVEMESH, sector, mesh);
  }
};


//---------------------------------------------------------------------------


csObjectWatcher::csObjectWatcher ()
  : scfImplementationType (this), updatenr (0), last_op (CS_WATCH_NONE), 
  last_light (0), last_movable (0), last_sector (0), last_mesh (0)
{
  light_callback = new csLightCallback (this);
  movable_listener = new csMovableListener (this);
  sector_listener = new csSectorListener (this);
}

csObjectWatcher::~csObjectWatcher ()
{
  Reset ();
  light_callback->DecRef ();
  movable_listener->DecRef ();
  sector_listener->DecRef ();
}

void csObjectWatcher::Reset ()
{
  size_t i;
  for (i = 0 ; i < movables.Length () ; i++)
  {
    movables[i]->RemoveListener (movable_listener);
  }
  for (i = 0 ; i < lights.Length () ; i++)
  {
    lights[i]->RemoveLightCallback (light_callback);
  }
  for (i = 0 ; i < sectors.Length () ; i++)
  {
    if (sectors[i])
      sectors[i]->RemoveSectorMeshCallback (sector_listener);
  }

  movables.DeleteAll ();
  lights.DeleteAll ();
  sectors.DeleteAll ();
}

void csObjectWatcher::ReportOperation (int op, iMovable* movable)
{
  last_op = op;
  last_movable = movable;
  last_light = 0;
  last_sector = 0;
  last_mesh = 0;
  updatenr++;
  size_t i = listeners.Length ();
  while (i-- > 0)
  {
    iObjectWatcherListener* l = listeners[i];
    l->ObjectChanged (op, movable);
  }
}

void csObjectWatcher::ReportOperation (int op, iLight* light)
{
  last_op = op;
  last_movable = 0;
  last_light = light;
  last_sector = 0;
  last_mesh = 0;
  updatenr++;
  size_t i = listeners.Length ();
  while (i-- > 0)
  {
    iObjectWatcherListener* l = listeners[i];
    l->ObjectChanged (op, light);
  }
}

void csObjectWatcher::ReportOperation (int op, iSector* sector, iMeshWrapper* mesh)
{
  last_op = op;
  last_movable = 0;
  last_light = 0;
  last_sector = sector;
  last_mesh = mesh;
  updatenr++;
  size_t i = listeners.Length ();
  while (i-- > 0)
  {
    iObjectWatcherListener* l = listeners[i];
    l->ObjectChanged (op, sector, mesh);
  }
}

void csObjectWatcher::WatchLight (iLight* light)
{
  light->SetLightCallback (light_callback);
  lights.Push (light);
}

void csObjectWatcher::RemoveLight (iLight* light)
{
  size_t i;
  for (i = 0 ; i < lights.Length () ; i++)
  {
    if (lights[i] == light)
    {
      light->RemoveLightCallback (light_callback);
      lights.DeleteIndex (i);
      break;
    }
  }
}

iLight* csObjectWatcher::GetLight (int idx)
{
  CS_ASSERT (idx >= 0 && (size_t)idx < lights.Length ());
  return lights[idx];
}

void csObjectWatcher::WatchMovable (iMovable* movable)
{
  movable->AddListener (movable_listener);
  movables.Push (movable);
}

void csObjectWatcher::RemoveMovable (iMovable* movable)
{
  size_t i;
  for (i = 0 ; i < movables.Length () ; i++)
  {
    if (movables[i] == movable)
    {
      movable->RemoveListener (movable_listener);
      movables.DeleteIndex (i);
      break;
    }
  }
}

iMovable* csObjectWatcher::GetMovable (int idx)
{
  CS_ASSERT (idx >= 0 && (size_t)idx < movables.Length ());
  return movables[idx];
}

void csObjectWatcher::WatchSector (iSector* sector)
{
  sector->AddSectorMeshCallback (sector_listener);
  sectors.Push (sector);
}

void csObjectWatcher::RemoveSector (iSector* sector)
{
  size_t i = sectors.Length ();
  while (i > 0)
  {
    i--;
    if (sectors[i] == sector)
    {
      sector->RemoveSectorMeshCallback (sector_listener);
      sectors.DeleteIndex (i);
    }
    else if (sectors[i] == 0)
    {
      sectors.DeleteIndex (i);
    }
  }
}

iSector* csObjectWatcher::GetSector (int idx)
{
  CS_ASSERT (idx >= 0 && (size_t)idx < sectors.Length ());
  return sectors[idx];
}

