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
#include "csobject/nameobj.h"
#include "csobject/dataobj.h"
#include "csobject/pobject.h"

CSOBJTYPE_IMPL(csNameObject,csObject);
CSOBJTYPE_IMPL(csDataObject,csObject);
CSOBJTYPE_IMPL(csPObject,csObject);

csNameObject::csNameObject (const char *n) : csObject ()
{
  if (n)
    strncpy (name, n, 30);
  else
    name [0] = 0;
  name [29] = 0;
}

const char* csNameObject::GetName (csObject& csobj)
{
  csObject *o = csobj.GetObj (csNameObject::Type ());
  if (o)
    return ((csNameObject*)o)->Name ();
  return NULL;
}

void csNameObject::AddName (csObject& csobj, const char* name)
{
  CHK (csNameObject* nameobj = new csNameObject (name));
  csobj.ObjAdd (nameobj);
}
