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

#include "cssysdef.h"
#undef CS_REF_TRACKER
#include "csutil/callstack.h"
#include "csutil/scf.h"
#include "csutil/scopedmutexlock.h"
#include "csutil/sysfunc.h"
#include "reftrack.h"



csRefTracker::csRefTracker () : scfImplementationType (this), riAlloc(1000)
{
  (mutex = csMutex::Create ())->IncRef();
}

csRefTracker::~csRefTracker ()
{
  csMutex* tehMutex = mutex;
  mutex = 0; 
    // Set mutex to 0 as mutex DecRef() will cause RefTracker call
  tehMutex->DecRef();
}

csRefTracker::RefInfo& csRefTracker::GetObjRefInfo (void* obj)
{
  obj = aliases.Get (obj, obj);
  RefInfo* info = trackedRefs.Get (obj, 0);
  if (info == 0)
  {
    info = riAlloc.Alloc();
    trackedRefs.Put (obj, info);
  }
  return *info;
}

void csRefTracker::TrackIncRef (void* object, int refCount)
{
  if (!mutex) return;
  csScopedMutexLock lock (mutex);

  RefInfo& refInfo = GetObjRefInfo (object);
  RefAction& action = refInfo.actions.GetExtend (refInfo.actions.Length ());
  action.type = Increased;
  action.refCount = refCount;
  action.stack = csCallStackHelper::CreateCallStack (1);
  action.tag = 0;
  refInfo.refCount = refCount + 1;
}

void csRefTracker::TrackDecRef (void* object, int refCount)
{
  if (!mutex) return;
  csScopedMutexLock lock (mutex);

  RefInfo& refInfo = GetObjRefInfo (object);
  RefAction& action = refInfo.actions.GetExtend (refInfo.actions.Length ());
  action.type = Decreased;
  action.refCount = refCount;
  action.stack = csCallStackHelper::CreateCallStack (1);
  action.tag = 0;
  refInfo.refCount = refCount - 1;
}

void csRefTracker::TrackConstruction (void* object)
{
  if (!mutex) return;
  csScopedMutexLock lock (mutex);

  /*
    Move the already tracked object to the "old data".
    The new one might just coincidentally be alloced at the same spot.
   */
  RefInfo* oldRef = trackedRefs.Get (object, 0);
  if (oldRef != 0)
  {
    OldRefInfo oldInfo = {object, oldRef};
    oldData.Push (oldInfo);
    trackedRefs.DeleteAll (object);
  }
  /*
    @@@ It may happen that this pointer was aliased to some other location,
    but the alias hasn't been removed.
   */
  aliases.DeleteAll (object);
  TrackIncRef (object, 0);
}

void csRefTracker::TrackDestruction (void* object, int refCount)
{
  if (!mutex) return;
  csScopedMutexLock lock (mutex);

  RefInfo& refInfo = GetObjRefInfo (object);
  RefAction& action = refInfo.actions.GetExtend (refInfo.actions.Length ());
  action.type = Destructed;
  action.refCount = refCount;
  action.stack = csCallStackHelper::CreateCallStack (1);
  action.tag = 0;
  refInfo.destructed = true;
}

void csRefTracker::MatchIncRef (void* object, int refCount, void* tag)
{
  if (!mutex) return;
  csScopedMutexLock lock (mutex);

  RefInfo& refInfo = GetObjRefInfo (object);
  bool foundAction = false;
  size_t i = refInfo.actions.Length ();
  while (i > 0)
  {
    i--;
    if (refInfo.actions[i].refCount == refCount)
    {
      if (refInfo.actions[i].tag == 0)
      {
	refInfo.actions[i].tag = tag;
	foundAction = true;
      }
      break;
    }
  }
  if (!foundAction)
  {
    RefAction& action = refInfo.actions.GetExtend (refInfo.actions.Length ());
    action.type = Increased;
    action.refCount = refCount;
    action.stack = csCallStackHelper::CreateCallStack (1);
    action.tag = tag;
    refInfo.refCount = refCount + 1;
  }
}

void csRefTracker::MatchDecRef (void* object, int refCount, void* tag)
{
  if (!mutex) return;
  csScopedMutexLock lock (mutex);

  RefInfo& refInfo = GetObjRefInfo (object);
  bool foundAction = false;
  size_t i = refInfo.actions.Length ();
  while (i > 0)
  {
    i--;
    if (refInfo.actions[i].refCount == refCount)
    {
      if (refInfo.actions[i].tag == 0)
      {
	refInfo.actions[i].tag = tag;
	foundAction = true;
      }
      break;
    }
  }
  if (!foundAction)
  {
    RefAction& action = refInfo.actions.GetExtend (refInfo.actions.Length ());
    action.type = Decreased;
    action.refCount = refCount;
    action.stack = csCallStackHelper::CreateCallStack (1);
    action.tag = tag;
    refInfo.refCount = refCount - 1;
  }
  if (refCount == 0)
  {
    /*
      Ditch tracked object as a new one might just 
      coincidentally be alloced at the same spot.
    */
    trackedRefs.DeleteAll (object);
    RefInfo* ref = trackedRefs.Get (object, 0);
    riAlloc.Free (ref);
  }
}

void csRefTracker::AddAlias (void* obj, void* mapTo)
{
  if (!mutex) return;
  csScopedMutexLock lock (mutex);

  if (obj == mapTo) return;
  aliases.PutUnique (obj, mapTo);
}

void csRefTracker::RemoveAlias (void* obj, void* mapTo)
{
  if (!mutex) return;
  csScopedMutexLock lock (mutex);

  if (obj == mapTo) return;
  aliases.Delete (obj, mapTo);
}

void csRefTracker::SetDescription (void* obj, const char* description)
{
  if (!mutex) return;
  csScopedMutexLock lock (mutex);

  RefInfo& refInfo = GetObjRefInfo (obj);
  if (refInfo.descr == 0) refInfo.descr = description;
}

void csRefTracker::ReportOnObj (void* obj, RefInfo* info)
{
  bool okay = (info->refCount == 0) ||
    (info->destructed && (info->refCount <= 1));
  if (!okay)
  {
    csPrintf ("object %p (%s), refcount %d, %s\n",
      obj,
      info->descr ? info->descr : "<unknown>",
      info->refCount,
      info->destructed ? "destructed" : "not destructed");
    for (size_t i = 0; i < info->actions.Length(); i++)
    {
      csPrintf ("%s by %p from %d\n",
	(info->actions[i].type == Increased) ? "Increase" : "Decrease",
	info->actions[i].tag, 
	info->actions[i].refCount);
      if (info->actions[i].stack != 0)
	info->actions[i].stack->Print ();
    }
    csPrintf ("\n");
  }
}

void csRefTracker::Report ()
{
  if (!mutex) return;
  csScopedMutexLock lock (mutex);

  for (size_t i = 0; i < oldData.Length(); i++)
  {
    const OldRefInfo& oldInfo = oldData[i];

    ReportOnObj (oldInfo.obj, oldInfo.ri);
  }

  csHash<RefInfo*, void*>::GlobalIterator it (
    trackedRefs.GetIterator ());

  while (it.HasNext ())
  {
    void* obj;
    RefInfo* info = it.Next (obj);

    ReportOnObj (obj, info);
  }
}
