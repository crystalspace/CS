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
#include "csobject/csobj.h"
#include "csobject/objtree.h"

CSOBJTYPE_IMPL(csObject,NULLCLASS);

csObject::csObject() : csBase(), objtree(NULL)
{
#ifdef __USE_CS_ID_CODE
  static CS_ID id = 0;
  csid_value = id++;
#endif
}

csObject::~csObject() 
{ if (objtree) CHKB(delete objtree); }

csObject::csObject(csObject& csobj) : csBase(), objtree(NULL)
#ifdef __USE_CS_ID_CODE
  , csid_value(csobj.csid_value)
#endif
{
  if (csobj.objtree) objtree = csobj.objtree->GetCopy(); 
}

csObject* csObject::GetObj(const csIdType& objtype)
{
  if (!objtree) return NULL;
  csObjIterator i(objtree->GetNode(objtype));  
  if (i.IsNull()) return NULL;
  return &(*i);
}

csObjIterator csObject::ObjGet(const csIdType& objtype) 
{ 
  if (objtree) return csObjIterator(objtree->GetNode(objtype));
  return csObjIterator(NULL);
}

void csObject::ObjAdd(csObject* obj)
{ 
  if (objtree) objtree->ObjAdd(obj);
  else CHKB(objtree = new csObjTree(obj)); 
  if (obj) obj->SetObjectParent(this);
}

void csObject::ObjRelease(csObject* obj)
{ 
  if (!objtree) return;
  objtree->ObjRelease(obj); 
  if (objtree->IsEmpty())
  { 
    CHK(delete objtree);
    objtree=NULL;
  }
}

void csObject::ObjRemove(csObject* obj)
{ 
  ObjRelease(obj);
  if (obj) CHKB(delete obj); 
}
