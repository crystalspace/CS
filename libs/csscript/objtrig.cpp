/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#include "sysdef.h"
#include "csscript/objtrig.h"
#include "csscript/csscript.h"

CSOBJTYPE_IMPL(csObjectTrigger,csPObject);

csObjectTrigger::csObjectTrigger () : csPObject(),activate_triggers() {}

void csObjectTrigger::NewActivateTrigger (csScript* script, csObject* ob)
{
  activate_triggers.add_trigger(script,ob);
}

void csObjectTrigger::DoActivateTriggers ()
{
  activate_triggers.perform(GetObjectParent());
}

void csObjectTrigger::DoActivateTriggers (csObject& obj)
{
  csObjectTrigger* o = GetTrigger(obj); 
  if (o) o->DoActivateTriggers();
}

csObjectTrigger* csObjectTrigger::GetTrigger(csObject& obj)
{
  return (csObjectTrigger*)obj.GetObj(csObjectTrigger::Type());
}
