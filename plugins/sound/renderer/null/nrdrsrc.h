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

#if !defined(__CSSOUNDSOURCENULL_H__)
#define __CSSOUNDSOURCENULL_H__

#include "isndbuf.h"
#include "isndsrc.h"

class csSoundBufferNull;

class csSoundSourceNull : public ISoundSource
{
public:
	csSoundSourceNull();
	virtual ~csSoundSourceNull();

	STDMETHODIMP SetPosition(float x, float y, float z);
	STDMETHODIMP SetVelocity(float x, float y, float z);

	STDMETHODIMP GetPosition(float &x, float &y, float &z);
	STDMETHODIMP GetVelocity(float &x, float &y, float &z);

  STDMETHODIMP GetSoundBuffer(ISoundBuffer **sound);

 	DECLARE_IUNKNOWN()
	DECLARE_INTERFACE_TABLE(csSoundSourceNull)

public:
  /// Position of sound object
  float fPosX, fPosY, fPosZ;
  /// Velocity of sound object
  float fVelX, fVelY, fVelZ;

  csSoundBufferNull* pSoundBuffer;
};

#endif // __CSSOUNDSOURCENULL_H__
