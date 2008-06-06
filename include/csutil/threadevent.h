/*
  Copyright (C) 2008 by Michael Gist

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_CSUTIL_THREADEVENT_H__
#define __CS_CSUTIL_THREADEVENT_H__

#include "csutil/array.h"
#include "csutil/scf_implementation.h"
#include "iutil/job.h"

#define EXECUTE_METHOD(methodIndex, object, args) \
  csRef<ThreadEvent> te; \
  te.AttachNew(new ThreadEvent(object, methodIndex, args)); \
  ThreadManager::GetThreadManager()->QueueEvent(te); \

class ThreadedCallable
{
public:
  ThreadedCallable* GetThreadedCallable() { return this; }
  virtual void RunMethod(uint methodIndex, csArray<void*> args) = 0;
};

class ThreadEvent : public scfImplementation1<ThreadEvent, iJob>
{
public:
  ThreadEvent(ThreadedCallable* object, uint methodIndex, csArray<void*> args) :
      scfImplementationType (this), object(object), methodIndex(methodIndex), args(args)
  {
  }

  void Run()
  {
    object->RunMethod(methodIndex, args);
  }

private:
  ThreadedCallable* object;
  uint methodIndex;
  csArray<void*> args;
};

#endif // __CS_CSUTIL_THREADEVENT_H__
