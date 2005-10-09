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

#ifndef __CS_SNDWRAP_H__
#define __CS_SNDWRAP_H__

#include "csextern.h"

#include "csutil/csobject.h"
#include "csutil/scf_implementation.h"
#include "isound/wrapper.h"

/**
 * Document me and possible move me to some sound manager! @@@
 * \deprecated
 */
class CS_CRYSTALSPACE_EXPORT csSoundWrapper : 
  public scfImplementationExt1<csSoundWrapper, csObject, iSoundWrapper>
{
protected:
  ///
  csRef<iSoundHandle> SoundHandle;

public:

  ///
  csSoundWrapper(iSoundHandle* buf);
  ///
  ~csSoundWrapper();

  virtual iSoundHandle *GetSound ();
  virtual iObject *QueryObject ();

};

#endif // __CS_SNDWRAP_H__
