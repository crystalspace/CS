/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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

#ifndef __CS_ISOUND_WRAPPER_H__
#define __CS_ISOUND_WRAPPER_H__

#include "csutil/scf.h"

struct iSoundHandle;
struct iObject;

/**
 * This class is a csObject (iObject) wrapper for sound handles.
 */
struct iSoundWrapper : public virtual iBase
{
  SCF_INTERFACE(iSoundWrapper, 2,0,0);
  /// Return the sound handle
  virtual iSoundHandle *GetSound () = 0;

  /// Return the iObject for this sound wrapper
  virtual iObject *QueryObject () = 0;
};

#endif // __CS_ISOUND_WRAPPER_H__
