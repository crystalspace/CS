/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
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

#include "cssysdef.h"
#include "csutil/csobject.h"
#include "csutil/dataobj.h"
#include "csutil/typedvec.h"

#include <stdlib.h>
#include <string.h>

CS_DECLARE_TYPED_IBASE_VECTOR (csObjectContainer, iObject);

/*** Object Iterators ***/

class csObjectIterator : public iObjectIterator
{
public:
  SCF_DECLARE_IBASE;
  csObject *Object;
  int Position;

  csObjectIterator (csObject *obj) : Object (obj)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    Object->IncRef ();
    Reset ();
  }
  virtual ~csObjectIterator ()
  {
    Object->DecRef ();
  }
  virtual bool Next()
  {
    if (Position < 0)
      return false;

    Position++;
    if (Position == Object->Children->Length ())
    {
      Position = -1;
      return false;
    }
    return true;
  }
  virtual void Reset()
  {
    if (Object->Children == NULL || Object->Children->Length () < 1)
      Position = -1;
    else
      Position = 0;
  }
  virtual iObject *GetObject () const
  {
    return Object->Children->Get (Position);
  }
  virtual iObject* GetParentObj() const
  {
    return Object;
  }
  virtual bool IsFinished () const
  {
    return (Position < 0);
  }
  virtual bool FindName (const char* name)
  {
    while (!IsFinished ())
    {
      if (strcmp (GetObject ()->GetName (), name) == 0)
        return true;
      Next ();
    }
    return false;
  }
};

SCF_IMPLEMENT_IBASE (csObjectIterator)
  SCF_IMPLEMENTS_INTERFACE (iObjectIterator)
SCF_IMPLEMENT_IBASE_END

/*** csObject itself ***/

SCF_IMPLEMENT_IBASE (csObject)
  SCF_IMPLEMENTS_INTERFACE (iObject)
SCF_IMPLEMENT_IBASE_END

csObject::csObject (iBase* pParent) : Children (NULL), Name (NULL)
{
  SCF_CONSTRUCT_IBASE (pParent);
  static CS_ID id = 0;
  csid = id++;
  ParentObject = NULL;
}

csObject::~csObject ()
{
  ObjRemoveAll ();

  if (Children) delete Children;
  delete [] Name;

  /*
   * @@@ This should not be required for two reasons:
   * 1. If the parent keeps a pointer to this object, then the pointer was
   *    IncRef'ed, so this object cannot be deleted. Removing the object from
   *    its parent from here is only needed if the object was illegally
   *    deleted, not DecRef'ed.
   * 2. Several objects could contain this object as a child. The 'parent'
   *    poitner is not a safe way to find out which object contains this
   *    object as a child.
   */
  if (ParentObject)
  {
    ParentObject->ObjReleaseOld (this);
  }
}

void csObject::SetName (const char *iName)
{
  delete [] Name;
  Name = csStrNew (iName);
}

const char *csObject::GetName () const
{
  return Name;
}

CS_ID csObject::GetID () const
{
  return csid;
}

iObject* csObject::GetObjectParent () const
{
  return ParentObject;
}

void csObject::SetObjectParent (iObject *obj)
{
  ParentObject = obj;
}

void csObject::ObjAdd (iObject *obj)
{
  if (!obj)
    return;

  if (!Children)
    Children = new csObjectContainer ();

  obj->SetObjectParent (this);
  Children->Push (obj);
}

void csObject::ObjRemove (iObject *obj)
{ 
  if (!Children || !obj)
    return;

  int n = Children->Find (obj);
  if (n>=0)
  {
    obj->SetObjectParent (NULL);
    Children->Delete (n);
  }
}

void csObject::ObjReleaseOld (iObject *obj)
{ 
  if (!Children || !obj)
    return;

  int n = Children->Find (obj);
  if (n>=0)
  {
    obj->SetObjectParent (NULL);
    obj->IncRef ();
    Children->Delete (n);
  }
}

void csObject::ObjRemoveAll ()
{
  if (!Children)
    return;

  for (int i=0; i < Children->Length (); i++)
  {
    iObject* child = Children->Get (i);
    child->SetObjectParent (NULL);
    Children->Delete (i);
  }
}

void csObject::ObjAddChildren (iObject *Parent)
{
  iObjectIterator *it = Parent->GetIterator ();
  while (!it->IsFinished ())
  {
    ObjAdd (it->GetObject ());
    it->Next ();
  }
  it->DecRef ();
}

void* csObject::GetChild (int InterfaceID, int Version,
	const char *Name, bool fn) const
{
  if (!Children)
    return NULL;

  if (fn)
  {
    iObject *obj = GetChild (Name);
    return obj ? obj->QueryInterface (InterfaceID, Version) : NULL;
  }

  for (int i = 0; i < Children->Length (); i++)
  {
    if (Name) {
      const char *OtherName = Children->Get (i)->GetName ();
      if (!OtherName) continue;
      if (strcmp(OtherName, Name)) continue;
    }

    void *obj = Children->Get (i)->QueryInterface (InterfaceID, Version);
    if (obj) return obj;
  }

  return NULL;
}

iObject* csObject::GetChild (const char *Name) const
{
  if (!Children || !Name)
    return NULL;
  
  int i;
  for (i = 0; i < Children->Length (); i++)
  {
    if (!strcmp (Children->Get (i)->GetName (), Name))
      return Children->Get (i);
  }
  return NULL;
}

iObjectIterator *csObject::GetIterator ()
{
  return new csObjectIterator (this);
}

//------------------- miscelaneous simple classes derived from csObject -----//

SCF_IMPLEMENT_IBASE_EXT (csDataObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iDataObject)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csDataObject::DataObject)
  SCF_IMPLEMENTS_INTERFACE (iDataObject)
SCF_IMPLEMENT_EMBEDDED_IBASE_END
