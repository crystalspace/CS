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

extern const GUID IID_ISoundBuffer;

enum SoundBufferPlayMethod
{
  SoundBufferPlay_Normal = 0,
  SoundBufferPlay_Restart,
  SoundBufferPlay_InLoop,
  SoundBufferPlay_RestartInLoop,
  SoundBufferPlay_DestroyAtEnd,
};

interface ISoundSource;

interface ISoundBuffer : public IUnknown
{
public:
  /// Play the sound
  STDMETHOD (Play) (SoundBufferPlayMethod playMethod = SoundBufferPlay_Normal) PURE;
  /// Stop the sound
  STDMETHOD (Stop) () PURE;
  // Set volume
  STDMETHOD (SetVolume) (float volume) PURE;
  // Get volume
  STDMETHOD (GetVolume) (float &volume) PURE;
  // Set frequency factor : 1 = normal, >1 more speed, <1 more slow
  STDMETHOD (SetFrequencyFactor) (float factor) PURE;
  // Get frequency factor
  STDMETHOD (GetFrequencyFactor) (float &factor) PURE;
  // Create a 3d 
  STDMETHOD (CreateSource) (ISoundSource **source) PURE;
};

#endif // __ISOUNDBUFFER_H__
