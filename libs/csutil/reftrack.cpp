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
#include "csutil/sysfunc.h"

#include "reftrack.h"

const size_t RefInfoAllocChunk = 2*1024*1024;

csRefTracker::csRefTracker () : scfImplementationType (this), 
  riAlloc (RefInfoAllocChunk / sizeof (RefInfo))
{
}

csRefTracker::~csRefTracker ()
{
}

csRefTracker::RefInfo& csRefTracker::GetObjRefInfo (void* obj)
{
  while (true)
  {
    void* newAlias = aliases.Get (obj, nullptr);
    if (!newAlias) break;
    obj = newAlias;
  }
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
  CS::Threading::RecursiveMutexScopedLock lock (mutex);

  RefInfo& refInfo = GetObjRefInfo (object);
  RefAction& action = refInfo.actions.GetExtend (refInfo.actions.GetSize ());
  action.type = Increased;
  action.refCount = refCount;
  action.stack = csCallStackHelper::CreateCallStack (1, true);
  action.tag = 0;
  refInfo.refCount = refCount + 1;
}

void csRefTracker::TrackDecRef (void* object, int refCount)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);

  RefInfo& refInfo = GetObjRefInfo (object);
  RefAction& action = refInfo.actions.GetExtend (refInfo.actions.GetSize ());
  action.type = Decreased;
  action.refCount = refCount;
  action.stack = csCallStackHelper::CreateCallStack (1, true);
  action.tag = 0;
  refInfo.refCount = refCount - 1;
}

void csRefTracker::TrackConstruction (void* object)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);

  /*
    Move the already tracked object to the "old data".
    The new one might just coincidentally be alloced at the same spot.
   */
  RefInfo* oldRef = trackedRefs.Get (object, 0);
  if (oldRef != 0)
  {
    if (!oldRef->CorrectlyDestructed())
    {
      /* Although it's kind of odd - an object at the same memory location,
         but it was not correctly destroyed? Anyway, keep around
         for reporting later ... */
      oldRef->actions.ShrinkBestFit();
      OldRefInfo oldInfo = {object, oldRef};
      oldData.Push (oldInfo);
    }
    else
      riAlloc.Free (oldRef);
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
  CS::Threading::RecursiveMutexScopedLock lock (mutex);

  RefInfo& refInfo = GetObjRefInfo (object);
  RefAction& action = refInfo.actions.GetExtend (refInfo.actions.GetSize ());
  action.type = Destructed;
  action.refCount = refCount;
  action.stack = csCallStackHelper::CreateCallStack (1, true);
  action.tag = 0;
  refInfo.refCount = refCount;
  refInfo.flags |= RefInfo::flagDestructed;
  refInfo.actions.ShrinkBestFit();
}

void csRefTracker::MatchIncRef (void* object, int refCount, void* tag)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);

  RefInfo& refInfo = GetObjRefInfo (object);
  bool foundAction = false;
  size_t i = refInfo.actions.GetSize ();
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
    RefAction& action = refInfo.actions.GetExtend (refInfo.actions.GetSize ());
    action.type = Increased;
    action.refCount = refCount;
    action.stack = csCallStackHelper::CreateCallStack (1, true);
    action.tag = tag;
    refInfo.refCount = refCount + 1;
  }
}

void csRefTracker::MatchDecRef (void* object, int refCount, void* tag)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);

  RefInfo& refInfo = GetObjRefInfo (object);
  bool foundAction = false;
  size_t i = refInfo.actions.GetSize ();
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
    RefAction& action = refInfo.actions.GetExtend (refInfo.actions.GetSize ());
    action.type = Decreased;
    action.refCount = refCount;
    action.stack = csCallStackHelper::CreateCallStack (1, true);
    action.tag = tag;
    refInfo.refCount = refCount - 1;
  }
}

void csRefTracker::AddAlias (void* obj, void* mapTo)
{
  if (obj == mapTo) return;

  CS::Threading::RecursiveMutexScopedLock lock (mutex);

  aliases.PutUnique (obj, mapTo);
}

void csRefTracker::RemoveAlias (void* obj, void* mapTo)
{
  if (obj == mapTo) return;

  CS::Threading::RecursiveMutexScopedLock lock (mutex);

  aliases.Delete (obj, mapTo);
}

void csRefTracker::SetDescription (void* obj, const char* description)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);

  RefInfo& refInfo = GetObjRefInfo (obj);
  refInfo.descr = description;
}

void csRefTracker::SetDescriptionWeak (void* obj, const char* description)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);

  RefInfo& refInfo = GetObjRefInfo (obj);
  if (refInfo.descr == 0)
    refInfo.descr = description;
}

void csRefTracker::ReportOnObj (void* obj, RefInfo* info)
{
  bool okay = info->CorrectlyDestructed();
  if (!okay)
  {
    csPrintf ("LEAK: object %p (%s), refcount %d, %s\n",
      obj,
      info->descr ? info->descr : "<unknown>",
      info->refCount,
      (info->flags & RefInfo::flagDestructed) ? "destructed" : "not destructed");
    for (size_t i = 0; i < info->actions.GetSize (); i++)
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
  CS::Threading::RecursiveMutexScopedLock lock (mutex);

  while (oldData.GetSize () > 0)
  {
    const OldRefInfo& oldInfo = oldData[oldData.GetSize ()-1];

    ReportOnObj (oldInfo.obj, oldInfo.ri);
    riAlloc.Free (oldInfo.ri);
    oldData.Truncate (oldData.GetSize ()-1);
  }

  csHash<RefInfo*, void*>::GlobalIterator it (
    trackedRefs.GetIterator ());

  while (it.HasNext ())
  {
    void* obj;
    RefInfo* info = it.Next (obj);

    ReportOnObj (obj, info);
    riAlloc.Free (info);
  }
  trackedRefs.DeleteAll();
}
