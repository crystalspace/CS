/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "csutil/strhash.h"
#include "csutil/util.h"

csStringHash::csRegisteredString* csStringHash::csRegisteredString::Alloc (
  const char* str)
{
  const size_t strLen = strlen (str);
  char* newStrBuf = new char[sizeof (IDtype) + strLen + 1];
  strcpy (newStrBuf + sizeof (IDtype), str);
  return (csRegisteredString*)newStrBuf;
}

void csStringHash::csRegisteredString::Free (
  csStringHash::csRegisteredString* regStr)
{
  delete[] ((char*)regStr);
}

csStringHash::csStringHash (int size) : Registry (size)
{
}

csStringHash::~csStringHash ()
{
  Clear ();
}

const char* csStringHash::Register (const char *Name, csStringID id)
{
  csRegisteredString* itf;
  itf = Registry.Get (Name, 0);
  if (itf != 0)
  {
    itf->ID = id;
    return itf->GetString();
  }
  itf = csRegisteredString::Alloc (Name);
  itf->ID = id;
  Registry.Put (itf->GetString(), itf);
  return itf->GetString();
}

csStringID csStringHash::Request (const char *Name) const
{
  csRegisteredString *itf;

  itf = Registry.Get (Name, 0);
  if (itf != 0)
  {
    return itf->ID;
  }
  return csInvalidStringID;
}

const char* csStringHash::Request (csStringID id) const
{
  csRegisteredString* itf;

  csHash<csRegisteredString*, const char*, 
    csConstCharHashKeyHandler>::GlobalIterator it (Registry.GetIterator ());

  while (it.HasNext ())
  {
    itf = it.Next ();
    if (itf->ID == id)
      return itf->GetString();
  }

  return 0;
}

void csStringHash::Clear ()
{
  csHash<csRegisteredString*, const char*, 
    csConstCharHashKeyHandler>::GlobalIterator it (Registry.GetIterator ());

  while (it.HasNext ())
  {
    csRegisteredString* itf = it.Next ();
    csRegisteredString::Free (itf);
  }
  Registry.DeleteAll ();
}

csStringHashIterator::csStringHashIterator (csStringHash* hash) :
  hashIt (hash->Registry.GetIterator ())
{
}

csStringHashIterator::~csStringHashIterator ()
{
}

bool csStringHashIterator::HasNext ()
{
  return hashIt.HasNext();
}

csStringID csStringHashIterator::Next ()
{
  return (hashIt.Next())->ID;
}
