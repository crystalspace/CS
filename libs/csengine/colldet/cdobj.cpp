/*
    Copyright (C) 1998 by Jorrit Tyberghein
    csObject library (C) 1999 by Ivan Avramovic <ivan@avramovic.com>
  
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

#include <string.h>
#include "sysdef.h"
#include "csengine/cdobj.h"
#include "csengine/collider.h"

CSOBJTYPE_IMPL(csColliderPointerObject,csObject);

csColliderPointerObject::csColliderPointerObject(csCollider* pCollider, bool AutoDelete)
{
  m_pCollider  = pCollider;
  m_AutoDelete = AutoDelete;
}

csColliderPointerObject::~csColliderPointerObject()
{
  if (m_AutoDelete)
    delete m_pCollider;
}

csCollider* csColliderPointerObject::GetCollider(csObject& csobj)
{
  csObject *o = csobj.GetObj (csColliderPointerObject::Type ());
  if (o) return ((csColliderPointerObject*) o)->m_pCollider;
  return NULL;
}

void csColliderPointerObject::SetCollider(csObject& csobj, csCollider* pCollider, bool AutoDelete)
{
  CHK(csColliderPointerObject* collobj = new csColliderPointerObject (pCollider, AutoDelete));
  csobj.ObjAdd (collobj); 
}
