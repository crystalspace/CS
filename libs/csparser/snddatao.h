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

#if !defined(__CSSOUNDDATAOBJECT_H__)
#define __CSSOUNDDATAOBJECT_H__

#include "csobject/csobject.h"

struct iSoundHandle;

class csSoundDataObject : public csObject
{
protected:
  ///
  iSoundHandle* sndbuf;

public:
  ///
  csSoundDataObject(iSoundHandle* buf);
  ///
  ~csSoundDataObject();
  ///
  iSoundHandle* GetSound() const { return sndbuf; }
  ///
  static iSoundHandle* GetSound(csObject& csobj, const char* name);

  CSOBJTYPE;
};

#endif // __CSSOUNDDATAOBJECT_H__
