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

#if !defined(__CSSOUNDLISTENERA3D_H__)
#define __CSSOUNDLISTENERA3D_H__

#include "cssfxldr/common/snddata.h"
#include "isndrdr.h"
#include "isndlstn.h"

class csSoundListenerA3D  : public ISoundListener
{
public:
	csSoundListenerA3D();
	virtual ~csSoundListenerA3D();

	STDMETHODIMP SetDirection(float fx, float fy, float fz, float tx, float ty, float tz);
	STDMETHODIMP SetPosition(float x, float y, float z);
  STDMETHODIMP SetVelocity(float x, float y, float z);
	STDMETHODIMP SetDistanceFactor(float factor);
  STDMETHODIMP SetRollOffFactor(float factor);
  STDMETHODIMP SetDopplerFactor(float factor);
  STDMETHODIMP SetHeadSize(float size);
  STDMETHODIMP SetEnvironment(SoundEnvironment env);

	STDMETHODIMP GetDirection(float &fx, float &fy, float &fz, float &tx, float &ty, float &tz);
	STDMETHODIMP GetPosition(float &x, float &y, float &z);
  STDMETHODIMP GetVelocity(float &x, float &y, float &z);
	STDMETHODIMP GetDistanceFactor(float &factor);
  STDMETHODIMP GetRollOffFactor(float &factor);
  STDMETHODIMP GetDopplerFactor(float &factor);
  STDMETHODIMP GetHeadSize(float &size);
  STDMETHODIMP GetEnvironment(SoundEnvironment &env);

	DECLARE_IUNKNOWN()
	DECLARE_INTERFACE_TABLE(csSoundListenerA3D)

public:
  // Position
	float fPosX, fPosY, fPosZ;
  // Velocity
  float fVelX, fVelY, fVelZ;
  // Direction
  float fDirTopX, fDirTopY, fDirTopZ, fDirFrontX, fDirFrontY, fDirFrontZ;
  // Doppler
  float fDoppler;
  // Distance
  float fDistance;
  // RollOff
  float fRollOff;
  // HeadSize
  float fHeadSize;
  // Environment
  SoundEnvironment Environment;

  int CreateListener(ISoundRender *render);
	int DestroyListener();

	IA3dListener	*m_p3DListener;
  IA3d4 *m_p3DAudioRenderer;
};

#endif // __CSSOUNDLISTENERA3D_H__
