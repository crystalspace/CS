/*
    Copyright (C) 1998, 1999 by Nathaniel 'NooTe' Saint Martin
    Copyright (C) 1998, 1999 by Jorrit Tyberghein
    Written by Nathaniel 'NooTe' Saint Martin

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
#include "cssysdef.h"
#include "isound/handle.h"
#include "csparser/snddatao.h"
#include "csobject/csobject.h"

DECLARE_OBJECT_TYPE (csSoundWrapper);
DECLARE_OBJECT_TYPE (iSoundWrapper);

IMPLEMENT_CSOBJTYPE (csSoundWrapper, csObject);

IMPLEMENT_OBJECT_INTERFACE (csSoundWrapper)
  IMPLEMENTS_EMBEDDED_OBJECT_TYPE (iSoundWrapper)
  IMPLEMENTS_OBJECT_TYPE (csSoundWrapper)
IMPLEMENT_OBJECT_INTERFACE_END

IMPLEMENT_IBASE (csSoundWrapper);
  IMPLEMENTS_EMBEDDED_INTERFACE (iSoundWrapper);
IMPLEMENT_IBASE_END;

IMPLEMENT_EMBEDDED_IBASE (csSoundWrapper::SoundWrapper)
  IMPLEMENTS_INTERFACE (iSoundWrapper)
IMPLEMENT_EMBEDDED_IBASE_END

csSoundWrapper::csSoundWrapper (iSoundHandle *buf) : csObject(), SoundHandle(buf)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiSoundWrapper);

  if (SoundHandle) SoundHandle->IncRef();
}

csSoundWrapper::~csSoundWrapper ()
{
  if (SoundHandle) SoundHandle->DecRef ();
}

iSoundHandle* csSoundWrapper::GetSound ()
{
  return SoundHandle;
}

iSoundHandle* csSoundWrapper::GetSound (iObject* iobj, const char* name)
{
  csObject &csobj = *((csObject*)iobj);
  if (!name) return NULL;
  csObjIterator i = csobj.GetIterator (csObject::Type, true);
  while (!i.IsNull ())
  {
    csSoundWrapper *sw = QUERY_OBJECT_TYPE (i.GetObj (), csSoundWrapper);
    if (sw && strcmp (name, (*i).GetName ()) == 0)
      return sw->GetSound ();
    ++i;
  }
  return NULL;
}

iSoundHandle *csSoundWrapper::SoundWrapper::GetSound ()
{
  return scfParent->GetSound ();
}

iObject *csSoundWrapper::SoundWrapper::QueryObject ()
{
  return scfParent;
}
