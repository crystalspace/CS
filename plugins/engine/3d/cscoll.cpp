/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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
#include "plugins/engine/3d/cscoll.h"
#include "plugins/engine/3d/engine.h"
#include "plugins/engine/3d/sector.h"

SCF_IMPLEMENT_IBASE(csCollection)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iCollection)
  SCF_IMPLEMENTS_INTERFACE(csCollection)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCollection::Collection)
  SCF_IMPLEMENTS_INTERFACE(iCollection)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csCollection::csCollection (
  csEngine *engine) :
    csObject(),
    objects(8, 8)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiCollection);

  csCollection::engine = engine;
}

csCollection::~csCollection ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiCollection);
  SCF_DESTRUCT_IBASE ();
}

iObject *csCollection::FindObject (char *name)
{
  int i;
  for (i = 0; i < objects.Length (); i++)
  {
    iObject *obj = objects[i];
    if (!strcmp (obj->GetName (), name)) return obj;
  }

  return 0;
}

