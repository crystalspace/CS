/*
    Copyright (C) 2001 by Norman Krämer
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.
  
    You should have received a copy of the GNU General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _ARTSHANDLE_H_
#define _ARTSHANDLE_H_

#include "isound/handle.h"
#include "isound/source.h"
#include "isound/data.h"
#include "csarts.h"

class csArtsRenderer;

class csArtsHandle : public iSoundHandle, public iSoundSource
{
 protected:
  friend class csArtsRenderer;
  csArtsRenderer *pRend;
  iSoundData *sd;
  //  Arts::csSoundModule am;

  csArtsHandle (csArtsRenderer *pRend);

  bool UseData (iSoundData *sd);
  float frequencyfactor, volume;
  csVector3 speed, pos;
  int Mode3D;

  virtual void SetDirection (const csVector3 &Front, const csVector3 &Top);
  virtual void SetDistanceFactor (float factor);
  virtual void SetRollOffFactor (float factor);
  virtual void SetDopplerFactor (float factor);
  virtual void SetHeadSize (float size);

 public:

  SCF_DECLARE_IBASE;
  virtual ~csArtsHandle ();
  /// *************** iSoundHandle ****************  
  /// is this a static or streamed handle?
  virtual bool IsStatic();

  /// play an instance of this sound
  virtual void Play(bool Loop = false);
  /// create a sound source
  virtual iSoundSource *CreateSource(int Mode3d);

  /// Start playing the stream (only for streamed sound)
  virtual void StartStream(bool Loop);
  /// Stop playing the stream (only for streamed sound)
  virtual void StopStream();
  /// Reset the stream to the beginning (only for streamed sound)
  virtual void ResetStream();


  /// *************** iSoundSource ****************  
  /// Play the sound. PlayMethod can be set to any combination of SOUND_*
  virtual void Play (unsigned long playMethod = 0);
  /// Stop the sound
  virtual void Stop ();
  /// Set volume
  virtual void SetVolume (float volume);
  /// Get volume
  virtual float GetVolume ();
  /// Set frequency factor : 1 = normal, >1 faster, 0-1 slower
  virtual void SetFrequencyFactor (float factor);
  /// Get frequency factor
  virtual float GetFrequencyFactor ();

  /// return 3d mode
  virtual int GetMode3D();
  /// set 3d mode
  virtual void SetMode3D(int m);
  /// set position of this source
  virtual void SetPosition(csVector3 pos);
  /// get position of this source
  virtual csVector3 GetPosition();
  /// set velocity of this source
  virtual void SetVelocity(csVector3 spd);
  /// get velocity of this source
  virtual csVector3 GetVelocity();

};

#endif
