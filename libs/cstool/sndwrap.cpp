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

#include "cssysdef.h"
#include "isound/handle.h"
#include "cstool/sndwrap.h"
#include "csutil/csobject.h"

SCF_IMPLEMENT_IBASE (csSoundWrapper);
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSoundWrapper);
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundWrapper::SoundWrapper)
  SCF_IMPLEMENTS_INTERFACE (iSoundWrapper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSoundWrapper::csSoundWrapper (iSoundHandle *buf) : csObject()
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSoundWrapper);
  SoundHandle = buf;
}

csSoundWrapper::~csSoundWrapper ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiSoundWrapper);
  SCF_DESTRUCT_IBASE ();
}

iSoundHandle* csSoundWrapper::GetSound ()
{
  return SoundHandle;
}

iSoundHandle *csSoundWrapper::SoundWrapper::GetSound ()
{
  return scfParent->GetSound ();
}

iObject *csSoundWrapper::SoundWrapper::QueryObject ()
{
  return scfParent;
}
