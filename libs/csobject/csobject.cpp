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
#include "csobject/csobject.h"
#include "csobject/objiter.h"
#include "csobject/dataobj.h"
#include "csobject/pobject.h"

#include <stdlib.h>
#include <string.h>

#define CONTAINER_LIMIT_DELTA	16

class csObjContainer
{
public:
  // Amount of child objects
  unsigned short count, limit;
  // Some dumb compilers (i.e. Watcom) do not like [0] arrays
  csObject *obj [1];
  // Enlarge the array of children
  static void SetLimit (csObjContainer *&iContainer, int iLimit);
};

class csObjectIterator : public iObjectIterator
{
public:
  DECLARE_IBASE;
  csObject *Object;
  int Position;

  csObjectIterator (csObject *obj) : Object (obj)
  {
    CONSTRUCT_IBASE (NULL);
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

    while (1) {
      Position++;
      if (Position == Object->children->count)
      {
        Position = -1;
        return false;
      }
      return true;
    }
  }
  virtual void Reset()
  {
    if (Object->children == NULL)
      Position = -1;
    else
      Position = 0;
  }
  virtual iObject *GetObject () const
  {
    return Object->children->obj[Position];
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

IMPLEMENT_IBASE (csObjectIterator)
  IMPLEMENTS_INTERFACE (iObjectIterator)
IMPLEMENT_IBASE_END

void csObjContainer::SetLimit (csObjContainer *&iContainer, int iLimit)
{
  if (iLimit == 0)
  {
    free (iContainer);
    iContainer = NULL;
    return;
  }
  else if (iContainer == NULL)
  {
    iContainer = (csObjContainer *)malloc (
      sizeof (csObjContainer) + sizeof (csObject *) * (iLimit - 1));
    iContainer->count = 0;
  }
  else
    iContainer = (csObjContainer *)realloc (iContainer,
      sizeof (csObjContainer) + sizeof (csObject *) * (iLimit - 1));
  iContainer->limit = iLimit;
}

const csIdType csObject::Type ("csObject");

const csIdType& csObject::GetType () const
{
  return Type;
}

IMPLEMENT_IBASE (csObject)
  IMPLEMENTS_INTERFACE (iObject)
IMPLEMENT_IBASE_END

csObject::csObject () : csBase (), iObject (), children (NULL), Name (NULL)
{
  CONSTRUCT_IBASE (NULL);
  static CS_ID id = 0;
  csid = id++;
}

csObject::csObject (csObject& iObj) :
  csBase (), iObject (iObj), csid (iObj.csid), children (NULL), Name (NULL)
{
  CONSTRUCT_IBASE (NULL);
  if (iObj.children)
  {
    int size =
      sizeof(csObjContainer) + sizeof(csObject*) * (iObj.children->limit - 1);
    if (size > 0)
    {
      children = (csObjContainer *)malloc (size);
      memcpy (children, iObj.children, size);
    }
  }
}

csObject::~csObject ()
{
  int idx = 0;
  while (children && children->count > idx)
  {
    csObject* child = children->obj[idx];
    // If the parent of the child is equal to this object
    // then we know that deleting the child will do the needed
    // cleanup of the 'children' array. Otherwise we have to do
    // this ourselves.
    if (child->GetObjectParent () == this)
      delete child;
    else
    {
      delete child;
      idx++;	// Skip this index.
    }
  }
  // If there is still a 'children' array (possible if some
  // of the children didn't have this object as a parent) then
  // we delete this array here.
  if (children) free (children);

  delete [] Name;
}

csObjectNoDel::~csObjectNoDel ()
{
  if (children)
  {
    int i;
    // First we unlink all children from this parent. But
    // only if the parent is equal to this.
    for (i = 0 ; i < children->count ; i++)
    {
      csObject* child = children->obj[i];
      if (child->GetObjectParent () == this)
        child->SetObjectParent (NULL);
    }
    free (children);
    children = NULL;
  }
}

csObject *csObject::GetChild (const csIdType& iType, bool derived) const
{
  if (!children)
    return NULL;
  int i;
  if (derived)
  {
    for (i = 0; i < children->count; i++)
      if (children->obj [i]->GetType () >= iType)
        return children->obj [i];
  }
  else
  {
    for (i = 0; i < children->count; i++)
      if (&children->obj [i]->GetType () == &iType)
        return children->obj [i];
  }
  return NULL;
}

void csObject::ObjAdd (csObject *obj)
{
  if (!obj)
    return;

  if (!children)
    csObjContainer::SetLimit (children, CONTAINER_LIMIT_DELTA);
  else if (children->count >= children->limit)
    csObjContainer::SetLimit(children, children->limit + CONTAINER_LIMIT_DELTA);

  children->obj [children->count++] = obj;
  obj->SetObjectParent (this);
}

void csObject::ObjAdd (iObject *obj)
{
  if (!obj) return;
  // @@@ WARNING! We assume here that casting iObject to csObject works.
  ObjAdd ((csObject*)obj);
}

void csObject::ObjRelease (csObject *obj)
{ 
  if (!children || !obj)
    return;
  for (int i = 0; i < children->count; i++)
    if (children->obj [i] == obj)
    {
      obj->SetObjectParent (NULL);
      memmove (&children->obj [i], &children->obj [i + 1],
        (children->limit - (i + 1)) * sizeof (csObject *));
      if (--children->count <= children->limit - CONTAINER_LIMIT_DELTA)
        csObjContainer::SetLimit (children,
	  children->limit - CONTAINER_LIMIT_DELTA);
      break;
    }
}

void csObject::ObjRelease (iObject *obj)
{ 
  if (!children || !obj) return;
  // @@@ WARNING! We assume here that casting iObject to csObject works.
  ObjRelease ((csObject*)obj);
}

void csObject::ObjRemove (csObject *obj)
{ 
  ObjRelease (obj);
  delete obj; 
}

void csObject::ObjRemove (iObject *obj)
{ 
  ObjRelease (obj);
  obj->DecRef ();
}

void csObject::ObjReleaseAll ()
{
  while (children->count > 0)
    ObjRelease (children->obj[0]);
}

void csObject::ObjRemoveAll ()
{
  while (children->count > 0)
    ObjRemove (children->obj[0]);
}

void* csObject::GetChild (int InterfaceID, int Version,
	const char *Name, bool fn) const
{
  if (!children)
    return NULL;

  if (fn)
  {
    iObject *obj = GetChild(Name);
    return obj ? obj->QueryInterface (InterfaceID, Version) : NULL;
  }

  for (int i = 0; i < children->count; i++)
  {
    if (Name && strcmp(children->obj[i]->GetName (), Name))
      continue;

    void *obj = children->obj[i]->QueryInterface (InterfaceID, Version);
    if (obj) return obj;
  }

  return NULL;
}

iObject* csObject::GetChild (const char *Name) const
{
  if (!children || !Name)
    return NULL;
  
  for (int i = 0; i < children->count; i++)
  {
    if (!strcmp (children->obj[i]->GetName (), Name))
      return children->obj[i];
  }
  return NULL;
}

iObjectIterator *csObject::GetIterator ()
{
  return new csObjectIterator (this);
}

//----------------------------------------------------- Object iterator -----//

csObjIterator::csObjIterator (const csIdType &iType, const csObject &iObj,
	bool derived)
{
  csObjIterator::derived = derived;
  Reset (iType, iObj);
}

void csObjIterator::Reset (const csIdType &iType, const csObject &iObj)
{
  Type = &iType;
  Container = iObj.children;
  Index = -1;
  Next ();
}

csObject* csObjIterator::GetObj () const
{
  return Container ? Container->obj [Index] : NULL;
}

void csObjIterator::Next ()
{
  if (Container)
  {
    for (;;)
    {
      Index++;
      if (Index >= Container->count)
      {
        Container = NULL;
        break;
      }
      if (derived)
      {
        if (Container->obj [Index]->GetType () >= *Type)
	  break;
      }
      else
      {
        if (&Container->obj [Index]->GetType () == Type)
	  break;
      }
    }
  }
}

bool csObjIterator::FindName(const char* name)
{
  while (!IsFinished ())
  {
    if (strcmp (GetObj ()->GetName (), name) == 0)
      return true;
    Next ();
  }
  return false;
}

//------------------- miscelaneous simple classes derived from csObject -----//

IMPLEMENT_CSOBJTYPE (csDataObject, csObject);

IMPLEMENT_IBASE_EXT (csDataObject)
  IMPLEMENTS_EMBEDDED_INTERFACE (iDataObject)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csDataObject::DataObject)
  IMPLEMENTS_INTERFACE (iDataObject)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_CSOBJTYPE (csPObject, csObject);

csPObject::~csPObject ()
{
  if (parent)
  {
    parent->ObjRelease (this);
  }
}

