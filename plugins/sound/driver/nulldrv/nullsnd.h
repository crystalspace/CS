/*
	NULL sound driver by Norman Krämer <normank@lycosmail.com>

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

#ifndef __SOUND_DRIVER_NULL_H__
#define __SOUND_DRIVER_NULL_H__

#include "csutil/scf.h"
#include "isnddrv.h"

class csSoundDriverNULL : public iSoundDriver
{
protected:
  int freq;
public:
  DECLARE_IBASE;
  csSoundDriverNULL(iBase *iParent) { CONSTRUCT_IBASE (iParent); }
  virtual ~csSoundDriverNULL(){};

  virtual bool Initialize (iSystem *){ return true; }
  /// Open the sound render
  virtual bool Open (iSoundRender *, int frequency, bool , bool ){ freq=frequency; return true; }
  /// Close the sound render
  virtual void Close () {}
  /// Set Volume [0, 1]
  virtual void SetVolume (float ){};
  /// Get Volume [0, 1]
  virtual float GetVolume () {return 0.0;}
  /// Lock and Get Sound Memory Buffer
  virtual void LockMemory (void **, int *) {}
  /// Unlock Sound Memory Buffer
  virtual void UnlockMemory (){}
  /// Is driver need to be updated 'manually' ?
  virtual bool IsBackground () {return true;}
  /// Do driver is in 16 bits mode ?
  virtual bool Is16Bits () {return false;}
  /// Do driver is in stereo mode ?
  virtual bool IsStereo () {return false;}
  /// get current frequency of driver
  virtual int GetFrequency (){return freq;}
  /// Is driver have it's own handler for no sound data else soundrender fill memory
  virtual bool IsHandleVoidSound () {return true;}
  
};

#endif	//__SOUND_DRIVER_NULL_H__

