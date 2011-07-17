/*
    Copyright (C) 2011 by Mike Gist and Jelle Hellemans

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

#ifndef __CS_DAMN_RESOURCE_H__
#define __CS_DAMN_RESOURCE_H__

#include "csutil/scf_interface.h"
#include <csutil/scf_implementation.h>

//#include "csutil/threading/future.h"
#include "future.h"

struct iResource;
struct iResourceListener;
struct iMeshFactoryWrapper;

/**
 * The iLoading interface.
 */
struct iLoading : public virtual iBase
{
  SCF_INTERFACE (iLoading, 1,0,0);
  virtual const char* GetName() = 0;
  virtual bool Ready () const = 0;
  virtual csRef<iResource> Get() = 0;
  /**
   * The OnLoaded will always be trigger in the mainloop
   * so you don't have to worry about locking the engine 
   * or the resource itself.
   */
  virtual void AddListener(iResourceListener* listener) = 0;
  virtual void RemoveListener(iResourceListener* listener) = 0;

protected:
  friend void iResourceTrigger(iLoading*);
  virtual void TriggerCallback() = 0;
};

inline void iResourceTrigger(iLoading* b)
{
  b->TriggerCallback();
}

/**
 * The iResourceListener interface.
 */
struct iResourceListener : public virtual iBase
{
  SCF_INTERFACE (iResourceListener, 1,0,0);
  virtual void OnLoaded (iLoading* resource) = 0;
};


class Loading : public scfImplementation1<Loading, iLoading>
{
public:
  Loading() : scfImplementationType(this)
  {
  }
  
  Loading(const CS::Threading::Future<csRef<iResource> >& ref) : scfImplementationType(this)
  {
    p = ref;
  }
  
  csRef<iResource> operator->()
  {
      return p.Get();
  }
  
  virtual csRef<iResource> Get()
  {
      p.Wait();
      return p.Get();
  }
  
  virtual const char* GetName()
  {
    return 0;
  }
  
  virtual bool Ready() const 
  {
    return p.Ready();
  }
  
  
  virtual void AddListener(iResourceListener* listener)
  {
    if (Ready())
      listener->OnLoaded(this);
    else
      listeners.Push(listener);
  }
  
  virtual void RemoveListener(iResourceListener* listener) 
  {
    listeners.Delete(listener);
  }
  
private:
  CS::Threading::Future<csRef<iResource> > p;
  
  csRefArray<iResourceListener> listeners;
  virtual void TriggerCallback() 
  {
    for (size_t i = 0; i < listeners.GetSize(); i++)
      listeners.Get(i)->OnLoaded(this);
    listeners.DeleteAll();
  }
};

/**
 * The iResourceCache interface.
 */
struct iFormatAbstractor : public virtual iBase
{
  SCF_INTERFACE (iFormatAbstractor, 1,0,0);
  
  /**
   * Add a given resource to the cache.
   * @param abstraction The abstraction's name.
   * @param format The underlying format for the abstraction.
   */
  virtual void AddAbstraction (const char* abstraction, const char* format) = 0;

  /**
   * Add a given resource to the cache.
   * @param abstraction The abstraction of the format you want to query.
   */
  virtual const char* GetFormat (const char* abstraction) const = 0;
};

#endif // __CS_IMAP_RESOURCE_H__

