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

#include "sysdef.h"
#include "csobject/fakertti.h"

csIdStr csIdType::default_idstr = NULL;

const csIdType& NULLCLASS::Type()
{ static csIdType id = csIdType();  return id; }

csIdType csIdFunc::Allocate(csIdStr s, const csIdType& id)
{
  csIdType newid;

  CHK(newid.entries = new csIdStr[id.length+2]);
  if (newid.entries)
  {
    newid.length = id.length+1;
    for (int i=0;  i< newid.length;  i++) newid.entries[i] = id.entries[i];
    newid.entries[newid.length] = s; 
    newid.base = &id;
  }  
  else newid.entries = &csIdType::default_idstr;
  return newid; 
}

csIdType csIdFunc::GetCommon(const csIdType& t1, const csIdType& t2)
{
  if (t2 >= t1) return t1;
  csIdType retval = t2;
  while ( !(t1 >= retval) && retval.GetBase() )
    retval = *(retval.GetBase());
  return retval;
}
