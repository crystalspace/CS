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

#if !defined(__ISOUNDSOURCE_H__)
#define __ISOUNDSOURCE_H__

struct iSoundBuffer;

SCF_VERSION (iSoundSource, 0, 0, 1);

struct iSoundSource : public iBase
{
  /// Set position of sound object
  virtual void SetPosition (float x, float y, float z) = 0;
  /// Set velocity of sound object
  virtual void SetVelocity (float x, float y, float z) = 0;
  /// Get position of sound object
  virtual void GetPosition (float &x, float &y, float &z) = 0;
  /// Get velocity of sound object
  virtual void GetVelocity (float &x, float &y, float &z) = 0;

  /// Get sound buffer
  virtual iSoundBuffer *GetSoundBuffer () = 0;
};

#endif // __ISOUNDSOURCE_H__
