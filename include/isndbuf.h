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

#if !defined(__ISOUNDBUFFER_H__)
#define __ISOUNDBUFFER_H__

enum SoundBufferPlayMethod
{
  SoundBufferPlay_Normal = 0,
  SoundBufferPlay_Restart,
  SoundBufferPlay_InLoop,
  SoundBufferPlay_RestartInLoop,
  SoundBufferPlay_DestroyAtEnd,
};

scfInterface iSoundSource;

SCF_INTERFACE (iSoundBuffer, 0, 0, 1) : public iBase
{
  /// Play the sound
  virtual void Play (SoundBufferPlayMethod playMethod = SoundBufferPlay_Normal) = 0;
  /// Stop the sound
  virtual void Stop () = 0;
  /// Set volume
  virtual void SetVolume (float volume) = 0;
  /// Get volume
  virtual float GetVolume () = 0;
  /// Set frequency factor : 1 = normal, >1 more speed, <1 more slow
  virtual void SetFrequencyFactor (float factor) = 0;
  /// Get frequency factor
  virtual float GetFrequencyFactor () = 0;
  /// Create a 3d 
  virtual iSoundSource *CreateSource () = 0;
};

#endif // __ISOUNDBUFFER_H__
