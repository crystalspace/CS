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

struct csRegisteredString
{
  csStringID ID;
  char *String;
  csRegisteredString()
  { String = 0; }
  ~csRegisteredString()
  { delete [] String; }
};

csStringHash::csStringHash (uint32 size) : Registry (size)
{
}

csStringHash::~csStringHash ()
{
  Clear ();
}

const char* csStringHash::Register (const char *Name, csStringID id)
{
  csRegisteredString *itf;
  csHashKey hkey = csHashCompute (Name);

  csHashIterator it (&Registry, hkey);
  while (it.HasNext ())
  {
    itf = (csRegisteredString*) it.Next ();
    if (strcmp (itf->String, Name)==0)
    {
      itf->ID = id;
      return itf->String;
    }
  }

  itf = new csRegisteredString ();
  itf->String = csStrNew (Name);
  itf->ID = id;

  Registry.Put (hkey, itf);
  return itf->String;
}

csStringID csStringHash::Request (const char *Name) const
{
  csRegisteredString *itf;
  csHashKey hkey = csHashCompute (Name);

  csHashIterator it (&Registry, hkey);
  while (it.HasNext ())
  {
    itf = (csRegisteredString*) it.NextConst ();
    if (strcmp (itf->String, Name)==0)
      return itf->ID;
  }

  return csInvalidStringID;
}

const char* csStringHash::Request (csStringID id) const
{
  csRegisteredString *itf;

  csGlobalHashIterator it (&Registry);
  while (it.HasNext ())
  {
    itf = (csRegisteredString*) it.NextConst ();
    if (itf->ID == id)
      return itf->String;
  }

  return 0;
}

void csStringHash::Clear ()
{
  csGlobalHashIterator it (&Registry);

  while (it.HasNext ())
  {
    csRegisteredString *s = (csRegisteredString*) it.Next ();
    delete s;
  }
  Registry.DeleteAll ();
}

csStringHashIterator::csStringHashIterator (csStringHash* hash)
{
  hashIt = new csGlobalHashIterator (&hash->Registry);
}

csStringHashIterator::~csStringHashIterator ()
{
  delete hashIt;
}

bool csStringHashIterator::HasNext ()
{
  return hashIt->HasNext();
}

csStringID csStringHashIterator::Next ()
{
  return ((csRegisteredString*)hashIt->Next())->ID;
}
