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

#include "isndrdr.h"

extern const GUID IID_ISoundSource;

typedef struct
{
  /// Position of sound object
  float fPosX, fPosY, fPosZ;
  /// Velocity of sound object
  float fVelX, fVelY, fVelZ;
} csSoundSourceInfo;

interface ISoundSource : public IUnknown
{
public:
  /// Play a source
  STDMETHOD (PlaySource) (bool inLoop = false) PURE;
  /// Stop to play a source
  STDMETHOD (StopSource) () PURE;

  /// Set position of sound object
  STDMETHOD (SetPosition) (float x, float y, float z) PURE;
  /// Set velocity of sound object
  STDMETHOD (SetVelocity) (float x, float y, float z) PURE;

  /// Get all proprieties of sound object
  STDMETHOD (GetInfoSource) (csSoundSourceInfo *info) PURE;
};

#endif // __ISOUNDSOURCE_H__
