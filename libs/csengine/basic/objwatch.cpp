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
#include "qint.h"
#include "csengine/objwatch.h"
#include "iengine/light.h"
#include "iengine/movable.h"

//---------------------------------------------------------------------------

class csMovableListener : public iMovableListener
{
public:
  csObjectWatcher* watcher;

  csMovableListener (csObjectWatcher* watcher)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    csMovableListener::watcher = watcher;
  }
  virtual ~csMovableListener () { }

  SCF_DECLARE_IBASE;

  virtual void MovableChanged (iMovable* movable)
  {
    watcher->ReportOperation (CS_WATCH_MOVABLE_CHANGED, movable, NULL);
  }

  virtual void MovableDestroyed (iMovable* movable)
  {
    watcher->ReportOperation (CS_WATCH_MOVABLE_DESTROY, movable, NULL);
    watcher->RemoveMovable (movable);
  }
};

SCF_IMPLEMENT_IBASE(csMovableListener)
  SCF_IMPLEMENTS_INTERFACE(iMovableListener)
SCF_IMPLEMENT_IBASE_END

class csLightCallback : public iLightCallback
{
public:
  csObjectWatcher* watcher;

  csLightCallback (csObjectWatcher* watcher)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    csLightCallback::watcher = watcher;
  }
  virtual ~csLightCallback () { }

  SCF_DECLARE_IBASE;

  virtual void OnColorChange (iLight* light, const csColor& newcolor)
  {
    watcher->ReportOperation (CS_WATCH_LIGHT_COLOR, NULL, light);
  }

  virtual void OnPositionChange (iLight* light, const csVector3& newpos)
  {
    watcher->ReportOperation (CS_WATCH_LIGHT_MOVE, NULL, light);
  }

  virtual void OnSectorChange (iLight* light, iSector* newsector)
  {
    watcher->ReportOperation (CS_WATCH_LIGHT_SECTOR, NULL, light);
  }

  virtual void OnRadiusChange (iLight* light, float newradius)
  {
    watcher->ReportOperation (CS_WATCH_LIGHT_RADIUS, NULL, light);
  }

  virtual void OnDestroy (iLight* light)
  {
    watcher->ReportOperation (CS_WATCH_LIGHT_DESTROY, NULL, light);
    watcher->RemoveLight (light);
  }
};

SCF_IMPLEMENT_IBASE(csLightCallback)
  SCF_IMPLEMENTS_INTERFACE(iLightCallback)
SCF_IMPLEMENT_IBASE_END

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csObjectWatcher)
  SCF_IMPLEMENTS_INTERFACE(iObjectWatcher)
SCF_IMPLEMENT_IBASE_END

csObjectWatcher::csObjectWatcher ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  updatenr = 0;
  last_op = CS_WATCH_NONE;
  last_light = NULL;
  last_movable = NULL;
  light_callback = new csLightCallback (this);
  movable_listener = new csMovableListener (this);
}

csObjectWatcher::~csObjectWatcher ()
{
  Reset ();
  light_callback->DecRef ();
  movable_listener->DecRef ();
}

void csObjectWatcher::Reset ()
{
  int i;
  for (i = 0 ; i < movables.Length () ; i++)
  {
    movables[i]->RemoveListener (movable_listener);
  }
  for (i = 0 ; i < lights.Length () ; i++)
  {
    lights[i]->RemoveLightCallback (light_callback);
  }

  movables.SetLength (0);
  lights.SetLength (0);
}

void csObjectWatcher::ReportOperation (int op, iMovable* movable, iLight* light)
{
  last_op = op;
  last_movable = movable;
  last_light = light;
  updatenr++;
  int i = listeners.Length ()-1;
  while (i >= 0)
  {
    iObjectWatcherListener* l = listeners[i];
    l->ObjectChanged (op, movable, light);
    i--;
  }
}

void csObjectWatcher::WatchLight (iLight* light)
{
  light->SetLightCallback (light_callback);
  lights.Push (light);
}

void csObjectWatcher::RemoveLight (iLight* light)
{
  int i;
  for (i = 0 ; i < lights.Length () ; i++)
  {
    if (lights[i] == light)
    {
      light->RemoveLightCallback (light_callback);
      lights.Delete (i);
      break;
    }
  }
}

iLight* csObjectWatcher::GetLight (int idx)
{
  CS_ASSERT (idx >= 0 && idx < lights.Length ());
  return lights[idx];
}

void csObjectWatcher::WatchMovable (iMovable* movable)
{
  movable->AddListener (movable_listener);
  movables.Push (movable);
}

void csObjectWatcher::RemoveMovable (iMovable* movable)
{
  int i;
  for (i = 0 ; i < movables.Length () ; i++)
  {
    if (movables[i] == movable)
    {
      movable->RemoveListener (movable_listener);
      movables.Delete (i);
      break;
    }
  }
}

iMovable* csObjectWatcher::GetMovable (int idx)
{
  CS_ASSERT (idx >= 0 && idx < movables.Length ());
  return movables[idx];
}

