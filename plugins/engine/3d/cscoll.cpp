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


csCollection::csCollection ()
  : scfImplementationType (this), objects (8, 8)
{
}

csCollection::~csCollection ()
{
}

iObject *csCollection::FindObject (char *name) const
{
  size_t i;
  for (i = 0; i < objects.Length (); i++)
  {
    iObject *obj = objects[i];
    if (!strcmp (obj->GetName (), name)) return obj;
  }

  return 0;
}

