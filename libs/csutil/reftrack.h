/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#ifndef __CS_UTIL_REFTRACK_H__
#define __CS_UTIL_REFTRACK_H__

#include "csextern.h"
#include "iutil/reftrack.h"
#include "csutil/array.h"
#include "csutil/blockallocator.h"
#include "csutil/callstack.h"
#include "csutil/hash.h"
#include "csutil/thread.h"

class CS_CRYSTALSPACE_EXPORT csRefTracker : public iRefTracker
{
  enum RefActionType
  {
    Increased, Decreased, Destructed
  };
  struct CS_CRYSTALSPACE_EXPORT RefAction
  {
    RefActionType type;
    int refCount;
    void* tag;
    csCallStack* stack;

    RefAction () 
    { 
      stack = 0; 
    }
    ~RefAction ()
    {
      if (stack) stack->Free();
    }
  };
  struct CS_CRYSTALSPACE_EXPORT RefInfo
  {
    csArray<RefAction> actions;
    int refCount;
    bool destructed;
    const char* descr;

    RefInfo() : destructed (false), descr(0) { }
  };
  csBlockAllocator<RefInfo> riAlloc;
  csHash<void*, void*> aliases;
  csHash<RefInfo*, void*> trackedRefs;
  struct CS_CRYSTALSPACE_EXPORT OldRefInfo
  {
    void* obj;
    RefInfo* ri;
  };
  csArray<OldRefInfo> oldData;
  //csRef<csMutex> mutex;
  csMutex* mutex;
  
  RefInfo& GetObjRefInfo (void* obj);

  void ReportOnObj (void* obj, RefInfo* info);
public:
  SCF_DECLARE_IBASE;

  csRefTracker ();
  virtual ~csRefTracker ();

  virtual void TrackIncRef (void* object, int refCount);
  virtual void TrackDecRef (void* object, int refCount);
  virtual void TrackConstruction (void* object);
  virtual void TrackDestruction (void* object, int refCount);
  
  virtual void MatchIncRef (void* object, int refCount, void* tag);
  virtual void MatchDecRef (void* object, int refCount, void* tag);

  virtual void AddAlias (void* obj, void* mapTo);
  virtual void RemoveAlias (void* obj, void* mapTo);

  virtual void SetDescription (void* obj, const char* description);

  void Report ();
};

#endif // __CS_UTIL_REFTRACK_H__
