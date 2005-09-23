/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

#ifndef __CS_SNDSYS_MANAGER_H__
#define __CS_SNDSYS_MANAGER_H__

#include "csutil/scf.h"
#include "csutil/scf_implementation.h"

struct iObject;
struct iSndSysStream;

/**
 * A wrapper/holder for a loaded sound.
 */
struct iSndSysWrapper : public virtual iBase
{
  SCF_INTERFACE (iSndSysWrapper, 0, 0, 1);

  /// Get the iObject which represents this wrapper.
  virtual iObject* QueryObject () = 0;

  /// Get the sound stream associated with this object.
  virtual iSndSysStream* GetStream () = 0;

  /// Set the sound stream associated with this object.
  virtual void SetStream (iSndSysStream* stream) = 0;
};

/**
 * This is the sound manager for Crystal Space. Its only purpose is to keep
 * track of loaded sounds.
 */
struct iSndSysManager : public virtual iBase
{
  SCF_INTERFACE (iSndSysManager, 0, 0, 1);

  /// Create a new sound wrapper.
  virtual iSndSysWrapper* CreateSound (const char* name) = 0;

  /// Remove a sound wrapper from the sound manager.
  virtual void RemoveSound (iSndSysWrapper* snd) = 0;

  /// Remove a sound wrapper by index from the sound manager.
  virtual void RemoveSound (size_t idx) = 0;

  /// Remove all sound wrappers.
  virtual void RemoveSounds () = 0;

  /// Return the number of sounds.
  virtual size_t GetSoundCount () const = 0;

  /// Get the specified sound.
  virtual iSndSysWrapper* GetSound (size_t idx) = 0;

  /// Find a sound wrapper by name.
  virtual iSndSysWrapper* FindSoundByName (const char* name) = 0;
};

#endif // __CS_SNDSYS_MANAGER_H__

