/*
  Crystal Space Event Naming
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

#include "csutil/hash.h"
#include "csutil/eventnames.h"
#include "csutil/scf.h"

#include "ivideo/graph2d.h"

#ifdef ADB_DEBUG
#include <iostream>
#endif

csEventNameRegistry::csEventNameRegistry(iObjectRegistry *r) :
  scfImplementationType (this), object_reg (r), parentage (), names ()
{
}

csEventNameRegistry::~csEventNameRegistry()
{
}

csEventID csEventNameRegistry::GetID (const csString &name)
{
#ifdef ADB_DEBUG
  std::cerr << "csEventNameRegistry <" 
	    << std::hex << ((unsigned long) this) << std::dec
	    << "> GetID(" 
	    << name 
	    << ") = ";
#endif
  csEventID result;
  if (names.Contains(name))
  {
    result = names.Request(name);
  }
  else
  {
    result = names.Request(name);
    if (name.FindLast('.') != (size_t)-1)
    { 
      /* This is a sub-name, populate Parentage table while
	 ensuring (recursively) for parent registration */
      csString parent = name.Slice (0, name.FindLast('.'));
      parentage.PutUnique(result, GetID(parent));
    } 
    else if (strlen(name)>0) 
    {
      parentage.PutUnique(result, GetID(csString("")));
    }
    // else this is the root event, "", which has no parent.
  }
#ifdef ADB_DEBUG
  std::cerr << result << std::endl;
#endif
  return result;
}

const char * csEventNameRegistry::GetString (const csEventID id)
{
  CS_ASSERT(id != CS_EVENT_INVALID);
  CS_ASSERT(names.Request (id) != 0);
  return names.Request (id);
}

const char * csEventNameRegistry::GetString (iObjectRegistry *object_reg,
					     const csEventID id)
{
  csRef<iEventNameRegistry> nameRegistry = GetRegistry (object_reg);
  if (nameRegistry != 0)
    return nameRegistry->GetString (id);
  else
    return 0;
}

csEventID csEventNameRegistry::GetParentID (const csEventID id)
{
  CS_ASSERT(id != CS_EVENT_INVALID);
  return parentage.Get (id, CS_EVENT_INVALID);
}

bool csEventNameRegistry::IsImmediateChildOf(const csEventID child,
	const csEventID parent)
{
  CS_ASSERT(child != CS_EVENT_INVALID);
  CS_ASSERT(parent != CS_EVENT_INVALID);
  if (parentage.Get (child, CS_EVENT_INVALID) == parent)
    return true;
  else
    return false;
}

bool csEventNameRegistry::IsKindOf(const csEventID child,
	const csEventID parent)
{
  CS_ASSERT(child != CS_EVENT_INVALID);
  CS_ASSERT(parent != CS_EVENT_INVALID);

  csEventID asc = child;
  do  /* Linear in depth (number of dot-separated segments) of name. */
  {
    if (asc == parent)
      return true;
    asc = parentage.Get (asc, CS_EVENT_INVALID);
  } 
  while (asc!=CS_EVENT_INVALID);
  return false;
}

csRef<iEventNameRegistry> csEventNameRegistry::GetRegistry (
  iObjectRegistry *object_reg) 
{
  csRef<iEventNameRegistry> name_reg = 
    csQueryRegistry<iEventNameRegistry> (object_reg);
  if (name_reg == 0) 
  {
    name_reg.AttachNew (new csEventNameRegistry (object_reg));
    object_reg->Register (name_reg, "iEventNameRegistry");
  }
  CS_ASSERT (name_reg != 0);
  return name_reg;
}


/* 
   Tempted to move csevCanvasOp() to eventnames.h as an inline function?
   Doing so would require that header to include ivideo/graph2d.h, 
   which doesn't have much to do with event names.  
   Let's keep the inclusion graph as simple as possible, shall we?
*/
csEventID csevCanvasOp (csRef<iEventNameRegistry> &name_reg, 
			const iGraphics2D* g2d, const csString &y)
{
  csString name ("crystalspace.canvas.");
  name.Append (g2d->GetName());
  name.Append (".");
  name.Append (y);
  return name_reg->GetID (name);
}



/*
  These are only here until they go away, which will be soon.
*/


/**
 * Broadcasted before csevProcess on every frame.
 * This event will go away soon, since it was a kludge to
 * work around the lack of subscription priorities/scheduling.
 * Should be replaced with subscriptions to csevFrame with subscription 
 * ordering.
 */
CS_DEPRECATED_METHOD csEventID csevPreProcess(iObjectRegistry *reg) {
  return csEventNameRegistry::GetID((reg), "crystalspace.deprecated.preprocess");
}
CS_DEPRECATED_METHOD csEventID csevPreProcess(iEventNameRegistry *reg) {
  return csEventNameRegistry::GetID((reg), "crystalspace.deprecated.preprocess");
}

/**
 * Broadcasted every frame.
 * This event will go away soon, replaced by csevFrame.
 */
CS_DEPRECATED_METHOD csEventID csevProcess(iObjectRegistry *reg) {
  return csEventNameRegistry::GetID((reg), "crystalspace.deprecated.process");
}
CS_DEPRECATED_METHOD csEventID csevProcess(iEventNameRegistry *reg) {
  return csEventNameRegistry::GetID((reg), "crystalspace.deprecated.process");
}

/**
 * Broadcasted after csevProcess on every frame.
 * This event will go away soon, since it was a kludge to
 * work around the lack of subscription priorities/scheduling.
 * Should be replaced with subscriptions to csevFrame with subscription 
 * ordering.
 */
CS_DEPRECATED_METHOD csEventID csevPostProcess(iObjectRegistry *reg) {
  return csEventNameRegistry::GetID((reg), "crystalspace.deprecated.postprocess");
}
CS_DEPRECATED_METHOD csEventID csevPostProcess(iEventNameRegistry *reg) {
  return csEventNameRegistry::GetID((reg), "crystalspace.deprecated.postprocess");
}

/**
 * Broadcasted after csevPostProcess on every frame.
 * This event will go away soon, since it was a kludge to
 * work around the lack of subscription priorities/scheduling.
 * Should be replaced with subscriptions to csevFrame with subscription 
 * ordering.
 */
CS_DEPRECATED_METHOD csEventID csevFinalProcess(iObjectRegistry *reg) {
  return csEventNameRegistry::GetID((reg), "crystalspace.deprecated.finalprocess");
}
CS_DEPRECATED_METHOD csEventID csevFinalProcess(iEventNameRegistry *reg) {
  return csEventNameRegistry::GetID((reg), "crystalspace.deprecated.finalprocess");
}
