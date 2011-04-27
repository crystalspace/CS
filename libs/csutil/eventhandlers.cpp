/*
  Crystal Space Event Handler Naming
  Copyright (C) 2005 by Adam D. Bradley <artdodge@cs.bu.edu>

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

#include "iutil/objreg.h"
#include "iutil/eventh.h"

#include "csutil/scf.h"
#include "csutil/hash.h"
#include "csutil/eventhandlers.h"
#include "csutil/eventnames.h"

#include <string.h>

#ifdef ADB_DEBUG
#include <iostream>
#endif

csEventHandlerRegistry::csEventHandlerRegistry(iObjectRegistry *r) :
  scfImplementationType (this), object_reg (r),
  instanceCounter (1)
{
}

csEventHandlerRegistry::~csEventHandlerRegistry()
{
}

csHandlerID csEventHandlerRegistry::GetGenericID (const char *name)
{
  csString nameStr = name;
  CS_ASSERT (nameStr.FindFirst(':') == (size_t)-1);
  csHandlerID res;
  mutex.UpgradeLock();
  if (names.Contains(nameStr)) 
  {
    res = names.Request(nameStr);
  } 
  else 
  {
    res = names.Request(nameStr);
    csString p;
    p = nameStr + ":pre";
    mutex.UpgradeUnlockAndWriteLock();
    handlerPres.PutUnique(res, names.Request((const char *)p));
    p = nameStr + ":post";
    handlerPosts.PutUnique(res, names.Request((const char *)p));
    mutex.WriteUnlockAndUpgradeLock();
  }
  mutex.UpgradeUnlock();
  return res;
}

csHandlerID csEventHandlerRegistry::GetGenericPreBoundID (csHandlerID id)
{
  CS::Threading::ScopedReadLock lock(mutex);
  if (IsInstance(id))
    return GetGenericPreBoundID(instantiation.Get(id, CS_HANDLER_INVALID));
  return handlerPres.Get(id, CS_HANDLER_INVALID);
}

csHandlerID csEventHandlerRegistry::GetGenericPostBoundID (csHandlerID id)
{
  CS::Threading::ScopedReadLock lock(mutex);
  if (IsInstance(id))
    return GetGenericPostBoundID(instantiation.Get(id, CS_HANDLER_INVALID));
  return handlerPosts.Get(id, CS_HANDLER_INVALID);
}

csHandlerID csEventHandlerRegistry::RegisterID (iEventHandler *handler)
{
  mutex.UpgradeLock();
  csHandlerID res = handlerToID.Get (handler, CS_HANDLER_INVALID);
  if (res != CS_HANDLER_INVALID)
  {
    KnownEventHandler* knownHandler = idToHandler.GetElementPointer (res);
    mutex.UpgradeUnlockAndWriteLock();
    knownHandler->refcount++;
    mutex.WriteUnlock();
    return res;
  }

  mutex.UpgradeUnlock();
  csHandlerID generic = GetGenericID (handler->GenericName());
  mutex.WriteLock();

  csString iname;
  iname.Format ("%s:%" PRIu32, handler->GenericName(), instanceCounter++);
  res = names.Request (iname);

  instantiation.PutUnique (res, generic);

  handlerToID.PutUnique (handler, res);
  idToHandler.PutUnique (res, handler);

  mutex.WriteUnlock();
  return res;
}

csHandlerID csEventHandlerRegistry::GetID (iEventHandler *handler)
{
  CS::Threading::ScopedReadLock lock(mutex);
  csHandlerID res = handlerToID.Get (handler, CS_HANDLER_INVALID);
  return res;
}

csHandlerID csEventHandlerRegistry::GetID (const char *name)
{
  CS::Threading::ScopedReadLock lock(mutex);
  return names.Request (name);
}

void csEventHandlerRegistry::ReleaseID (csHandlerID id)
{

  CS_ASSERT (IsInstance (id));
  mutex.UpgradeLock();
  KnownEventHandler* knownHandler = idToHandler.GetElementPointer (id);
  CS_ASSERT (knownHandler);
  mutex.UpgradeUnlockAndWriteLock();
  if ((--knownHandler->refcount) == 0)
  {
    handlerToID.DeleteAll (
      static_cast<iEventHandler*> (knownHandler->handler));
    idToHandler.DeleteAll (id);
    instantiation.DeleteAll (id);
  }
  mutex.WriteUnlock();
}

void csEventHandlerRegistry::ReleaseID (iEventHandler *handler)
{
  mutex.ReadLock();
  csHandlerID id = handlerToID.Get (handler, CS_HANDLER_INVALID);
  mutex.ReadUnlock();
  CS_ASSERT(id != CS_HANDLER_INVALID);
  ReleaseID (id);
}

const char * csEventHandlerRegistry::GetString (csHandlerID id)
{
  CS::Threading::ScopedReadLock lock(mutex);
  return names.Request(id);
}

iEventHandler *csEventHandlerRegistry::GetHandler (csHandlerID id)
{
  CS_ASSERT(IsInstance(id));
  mutex.ReadLock();
  const KnownEventHandler* knownHandler = idToHandler.GetElementPointer (id);
  mutex.ReadUnlock();
  return knownHandler ? (iEventHandler*)(knownHandler->handler) : 
    (iEventHandler*)0;
}

bool csEventHandlerRegistry::IsInstanceOf (csHandlerID instanceid,
	csHandlerID genericid)
{
  CS_ASSERT(IsInstance(instanceid));
  CS::Threading::ScopedReadLock lock(mutex);
  return (instantiation.Get (instanceid, CS_HANDLER_INVALID) == genericid);
}

bool csEventHandlerRegistry::IsInstance (csHandlerID id)
{
  CS::Threading::ScopedReadLock lock(mutex);
  return (instantiation.Get (id, CS_HANDLER_INVALID) != CS_HANDLER_INVALID);
}

csHandlerID const csEventHandlerRegistry::GetGeneric (csHandlerID id)
{
  CS_ASSERT(IsInstance(id));
  CS::Threading::ScopedReadLock lock(mutex);
  return instantiation.Get (id, CS_HANDLER_INVALID);
}

csRef<iEventHandlerRegistry> csEventHandlerRegistry::GetRegistry (
	iObjectRegistry *object_reg)
{
  csRef<iEventHandlerRegistry> handler_reg = 
    csQueryRegistry<iEventHandlerRegistry> (object_reg);
  if (handler_reg == 0) 
  {
    handler_reg.AttachNew (new csEventHandlerRegistry (object_reg));
    object_reg->Register (handler_reg, "iEventHandlerRegistry");
  }
  CS_ASSERT (handler_reg != 0);
  return handler_reg;
}





const csHandlerID * 
FrameSignpost_Logic3D::GenericPrec(csRef<iEventHandlerRegistry> &,
				   csRef<iEventNameRegistry> &,
				   csEventID) const
{
  return 0;
}

const csHandlerID * 
FrameSignpost_Logic3D::GenericSucc(csRef<iEventHandlerRegistry> &r1,
	    csRef<iEventNameRegistry> &r2,
	    csEventID e) const 
{
  if (e == csevFrame(r2))
  {
    static csHandlerID constraint[2] =
    {
      FrameSignpost_3D2D::StaticID (r1), 
      CS_HANDLERLIST_END
    };
    return constraint;
  }
  return 0;
}



const csHandlerID * 
FrameSignpost_3D2D::GenericPrec(csRef<iEventHandlerRegistry> &,
				csRef<iEventNameRegistry> &,
				csEventID) const
{
  return 0;
}

const csHandlerID * 
FrameSignpost_3D2D::GenericSucc(csRef<iEventHandlerRegistry> &r1,
				csRef<iEventNameRegistry> &r2,
				csEventID e) const 
{
  if (e == csevFrame(r2))
  {
    static csHandlerID constraint[2] =
    {
      FrameSignpost_2DConsole::StaticID (r1),
      CS_HANDLERLIST_END
    };
    return constraint;
  }
  return 0;
}



const csHandlerID * 
FrameSignpost_2DConsole::GenericPrec(csRef<iEventHandlerRegistry> &,
				   csRef<iEventNameRegistry> &,
				   csEventID) const
{
  return 0;
}

const csHandlerID * 
FrameSignpost_2DConsole::GenericSucc(csRef<iEventHandlerRegistry> &r1,
				   csRef<iEventNameRegistry> &r2,
				   csEventID e) const 
{
  if (e == csevFrame(r2))
  {
    static csHandlerID constraint[2] =
    { 
      FrameSignpost_ConsoleDebug::StaticID (r1),
      CS_HANDLERLIST_END
    };
    return constraint;
  }
  return 0;
}



const csHandlerID * 
FrameSignpost_ConsoleDebug::GenericPrec(csRef<iEventHandlerRegistry> &,
					csRef<iEventNameRegistry> &,
					csEventID) const
{
  return 0;
}

const csHandlerID * 
FrameSignpost_ConsoleDebug::GenericSucc(csRef<iEventHandlerRegistry> &r1,
					csRef<iEventNameRegistry> &r2,
					csEventID e) const 
{
  if (e == csevFrame(r2))
  {
    static csHandlerID constraint[2] =
    { 
      FrameSignpost_ConsoleDebug::StaticID (r1), 
      CS_HANDLERLIST_END
    };
    return constraint;
  }
  return 0;
}



const csHandlerID * 
FrameSignpost_DebugFrame::GenericPrec(csRef<iEventHandlerRegistry> &,
				      csRef<iEventNameRegistry> &,
				      csEventID) const
{
  return 0;
}

const csHandlerID * 
FrameSignpost_DebugFrame::GenericSucc(csRef<iEventHandlerRegistry> &,
				      csRef<iEventNameRegistry> &,
				      csEventID) const 
{
  return 0;
}



