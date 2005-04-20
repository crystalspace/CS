/*
    Copyright (C) 2004 by Jorrit Tyberghein

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
#include "litobjsel.h"
#include "ivaria/keyval.h"

bool litObjectSelectAnd::SelectObject (iObject* obj)
{
  size_t i;
  for (i = 0 ; i < a.Length () ; i++)
  {
    bool rc = a[i]->SelectObject (obj);
    if (!rc) return false;
  }
  return true;
}

bool litObjectSelectOr::SelectObject (iObject* obj)
{
  size_t i;
  for (i = 0 ; i < a.Length () ; i++)
  {
    bool rc = a[i]->SelectObject (obj);
    if (rc) return true;
  }
  return false;
}

bool litObjectSelectByKeyValue::SelectObject (iObject* obj)
{
  csRef<iObjectIterator> it = obj->GetIterator ();
  while (it->HasNext ())
  {
    csRef<iKeyValuePair> kp = SCF_QUERY_INTERFACE (it->Next (), iKeyValuePair);
    if (kp)
    {
      if (!strcmp (keyname, kp->GetKey ()))
      {
        const char* v = kp->GetValue (keyattrtype);
	if (v && !strcmp (keyattrvalue, v))
	  return true;
      }
    }
  }
  return false;
}

bool litObjectSelectByKeyValueRE::SelectObject (iObject* obj)
{
  csRef<iObjectIterator> it = obj->GetIterator ();
  while (it->HasNext ())
  {
    csRef<iKeyValuePair> kp = SCF_QUERY_INTERFACE (it->Next (), iKeyValuePair);
    if (kp)
    {
      if (!strcmp (keyname, kp->GetKey ()))
      {
        const char* v = kp->GetValue (keyattrtype);
	if (v && matcher.Match (v) == csrxNoError)
	  return true;
      }
    }
  }
  return false;
}

bool litObjectSelectByMWFlags::SelectObject (iObject* obj)
{
  csRef<iMeshWrapper> mesh = SCF_QUERY_INTERFACE (obj, iMeshWrapper);
  if (!mesh) return false;
  return (mesh->GetFlags ().Get () & mask) == value;
}

bool litObjectSelectByMOFlags::SelectObject (iObject* obj)
{
  csRef<iMeshWrapper> mesh = SCF_QUERY_INTERFACE (obj, iMeshWrapper);
  if (!mesh) return false;
  return (mesh->GetMeshObject ()->GetFlags ().Get () & mask) == value;
}

bool litObjectSelectByType::SelectObject (iObject* obj)
{
  csRef<iMeshWrapper> mesh = SCF_QUERY_INTERFACE (obj, iMeshWrapper);
  if (!mesh) return false;
  iMeshObjectFactory* factory = mesh->GetMeshObject ()->GetFactory ();
  if (!factory) return false;
  iMeshObjectType* otype = factory->GetMeshObjectType ();
  csRef<iFactory> ifact = SCF_QUERY_INTERFACE (otype, iFactory);
  if (!ifact) return false;
  return !strcmp (type, ifact->QueryClassID ()+strlen (
  	"crystalspace.mesh.object."));
}


