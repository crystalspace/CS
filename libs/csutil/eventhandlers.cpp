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

#include "csutil/hash.h"
#include "csutil/eventhandlers.h"
#include "csutil/scf.h"

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

CS_SPECIALIZE_TEMPLATE
class csHashComputer<iEventHandler*> : public csHashComputerIntegral<void *> {};

csHandlerID csEventHandlerRegistry::GetGenericID (const csString &name)
{
  CS_ASSERT (name.FindFirst(':') == (size_t)-1);
  csHandlerID res;
  if (names.Contains(name)) 
  {
    res = names.Request(name);
  } 
  else 
  {
    res = names.Request(name);
    csString p;
    p = name + csString(":pre");
    handlerPres.PutUnique(res, names.Request((const char *)p));
    p = name + csString(":post");
    handlerPosts.PutUnique(res, names.Request((const char *)p));
  }
  return res;
}

csHandlerID csEventHandlerRegistry::GetGenericPreBoundID (csHandlerID id)
{
  if (IsInstance(id))
    return GetGenericPreBoundID(instantiation.Get(id, CS_HANDLER_INVALID));
  return handlerPres.Get(id, CS_HANDLER_INVALID);
}

csHandlerID csEventHandlerRegistry::GetGenericPostBoundID (csHandlerID id)
{
  if (IsInstance(id))
    return GetGenericPostBoundID(instantiation.Get(id, CS_HANDLER_INVALID));
  return handlerPosts.Get(id, CS_HANDLER_INVALID);
}

csHandlerID csEventHandlerRegistry::GetID (iEventHandler *handler)
{
  csHandlerID res = handlerToID.Get (handler, CS_HANDLER_INVALID);
  if (res != CS_HANDLER_INVALID)
    return res;

  csHandlerID generic = GetGenericID (handler->GenericName());

  csString iname;
  iname.Format ("%s:%" PRIu32, handler->GenericName(), instanceCounter++);
  res = names.Request (iname);

  instantiation.PutUnique (res, generic);

  handlerToID.PutUnique (handler, res);
  idToHandler.PutUnique (res, handler);

  return res;
}

void csEventHandlerRegistry::ReleaseID (csHandlerID id)
{
  CS_ASSERT (IsInstance (id));
  iEventHandler *h = idToHandler.Get (id, 0);
  CS_ASSERT (h);
  handlerToID.DeleteAll (h);
  idToHandler.DeleteAll (id);
  instantiation.DeleteAll (id);
}

void csEventHandlerRegistry::ReleaseID (iEventHandler *handler)
{
  CS_ASSERT(handlerToID.Get (handler, CS_HANDLER_INVALID) !=
  	CS_HANDLER_INVALID);
  csHandlerID id = handlerToID.Get (handler, CS_HANDLER_INVALID);
  handlerToID.DeleteAll (handler);
  idToHandler.DeleteAll (id);
  instantiation.DeleteAll (id);
}

const char * csEventHandlerRegistry::GetString (csHandlerID id)
{
  return names.Request(id);
}

iEventHandler *csEventHandlerRegistry::GetHandler (csHandlerID id)
{
  CS_ASSERT(IsInstance(id));
  return idToHandler.Get (id, 0);
}

bool const csEventHandlerRegistry::IsInstanceOf (csHandlerID instanceid,
	csHandlerID genericid)
{
  CS_ASSERT(IsInstance(instanceid));
  return (instantiation.Get (instanceid, CS_HANDLER_INVALID) == genericid);
}

bool const csEventHandlerRegistry::IsInstance (csHandlerID id)
{
  return (instantiation.Get (id, CS_HANDLER_INVALID) != CS_HANDLER_INVALID);
}

csHandlerID const csEventHandlerRegistry::GetGeneric (csHandlerID id)
{
  CS_ASSERT(IsInstance(id));
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
