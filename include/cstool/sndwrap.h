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

#ifndef __SNDWRAP_H__
#define __SNDWRAP_H__

#include "csutil/csobject.h"
#include "isound/wrapper.h"

class csSoundWrapper : public csObject
{
protected:
  ///
  iSoundHandle* SoundHandle;

public:

  ///
  csSoundWrapper(iSoundHandle* buf);
  ///
  ~csSoundWrapper();
  ///
  iSoundHandle* GetSound();

  SCF_DECLARE_IBASE;

  struct SoundWrapper : public iSoundWrapper
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundWrapper);

    virtual iSoundHandle *GetSound ();
    virtual iObject *QueryObject ();
  } scfiSoundWrapper;
};

#endif // __SNDWRAP_H__
