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

#include "sysdef.h"
#include "csutil/scf.h"
#include "isystem.h"

#include "dsound.h"

#include "cssndrdr/ds3d/sndrdr.h"
#include "cssndrdr/ds3d/sndlstn.h"

IMPLEMENT_FACTORY(csSoundListenerDS3D)

IMPLEMENT_IBASE(csSoundListenerDS3D)
	IMPLEMENTS_INTERFACE(iSoundListener)
IMPLEMENT_IBASE_END;

csSoundListenerDS3D::csSoundListenerDS3D(iBase *piBase)
{
	CONSTRUCT_IBASE(piBase);

	fPosX = fPosY = fPosZ = 0.0;
	fDirTopX = fDirTopY = fDirTopZ = 0.0;
	fDirFrontX = fDirFrontY = fDirFrontZ = 0.0;
	fVelX = fVelY = fVelZ = 0.0;
	fDoppler = 1.0;
	fDistance = 1.0;
	fRollOff = 1.0;
	
	m_p3DAudioRenderer = NULL;
	m_pDS3DPrimaryBuffer = NULL;
	m_pDS3DListener = NULL;
}

csSoundListenerDS3D::~csSoundListenerDS3D()
{
	DestroyListener();
}

int csSoundListenerDS3D::CreateListener(iSoundRender * render)
{
	HRESULT hr;
	csSoundRenderDS3D *renderDS3D;
	
	if(!render) return E_FAIL;
	renderDS3D = (csSoundRenderDS3D *)render;
	
	m_p3DAudioRenderer = renderDS3D->m_p3DAudioRenderer;
	
	if (!m_p3DAudioRenderer)
		return(E_FAIL);
	
	DSBUFFERDESC    dsbd;
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;
	
	if ((hr = m_p3DAudioRenderer->CreateSoundBuffer(&dsbd, &m_pDS3DPrimaryBuffer, NULL)) != DS_OK)
	{
		return E_FAIL;
	}
	
	if ((hr = m_pDS3DPrimaryBuffer->QueryInterface(IID_IDirectSound3DListener, (void **) &m_pDS3DListener)) != DS_OK)
	{
		return E_FAIL;
	}
	
	if ((hr = m_pDS3DListener->SetDopplerFactor(fDoppler, DS3D_IMMEDIATE)) != DS_OK)
	{
		return E_FAIL;
	}
	
	if ((hr = m_pDS3DListener->SetDistanceFactor(fDistance, DS3D_IMMEDIATE)) != DS_OK)
	{
		return E_FAIL;
	}
	
	if ((hr = m_pDS3DListener->SetRolloffFactor(fRollOff, DS3D_IMMEDIATE)) != DS_OK)
	{
		return E_FAIL;
	}
	
	return S_OK;
}

int csSoundListenerDS3D::DestroyListener()
{
	HRESULT hr;
	
	if (m_pDS3DListener)
	{
		if ((hr = m_pDS3DListener->Release()) < DS_OK)
			return(hr);
		
		m_pDS3DListener = NULL;
	}
	
	if (m_pDS3DPrimaryBuffer)
	{
		if ((hr = m_pDS3DPrimaryBuffer->Stop()) < DS_OK)
			return(hr);
		
		if ((hr = m_pDS3DPrimaryBuffer->Release()) < DS_OK)
			return(hr);
		
		m_pDS3DPrimaryBuffer = NULL;
	}
	
	fDoppler = 1.0f;
	fDistance = 1.0f;
	fRollOff = 1.0f;
	
	return S_OK;
}

void csSoundListenerDS3D::SetPosition(float x, float y, float z)
{
	fPosX = x; fPosY = y; fPosZ = z;
	m_pDS3DListener->
		SetPosition( fPosX, fPosY, fPosZ,DS3D_IMMEDIATE );
}

void csSoundListenerDS3D::SetDirection(float fx, float fy, float fz, float tx, float ty, float tz)
{
	
	fDirFrontX = fx; fDirFrontY = fy; fDirFrontZ = fz;
	fDirTopX = tx; fDirTopY = ty; fDirTopZ = tz;
	
	m_pDS3DListener->SetOrientation(
		fDirFrontX, fDirFrontY, fDirFrontZ,
		fDirTopX, fDirTopY, fDirTopZ,
		DS3D_IMMEDIATE);
}

void csSoundListenerDS3D::SetHeadSize(float size)
{
	fHeadSize = size;
}

void csSoundListenerDS3D::SetVelocity(float x, float y, float z)
{
	fVelX = x; fVelY = y; fVelZ = z;
	
	if(!m_pDS3DListener) return;
	
	m_pDS3DListener->SetVelocity(
		fVelX, fVelY, fVelZ,
		DS3D_IMMEDIATE);
}

void csSoundListenerDS3D::SetDopplerFactor(float factor)
{
	fDoppler = factor;
	
	if(!m_pDS3DListener) return;
	
	m_pDS3DListener->SetDopplerFactor(fDoppler, DS3D_IMMEDIATE);
}

void csSoundListenerDS3D::SetDistanceFactor(float factor)
{
	fDistance = factor;
	
	if(!m_pDS3DListener) return;
	
	m_pDS3DListener->
		SetDistanceFactor(fDistance, DS3D_IMMEDIATE);
}

void csSoundListenerDS3D::SetRollOffFactor(float factor)
{
	fRollOff = factor;
	
	if(!m_pDS3DListener) return;
	
	m_pDS3DListener->SetRolloffFactor(fRollOff, DS3D_IMMEDIATE);
}

void csSoundListenerDS3D::SetEnvironment(SoundEnvironment env)
{
	Environment = env;
}

void csSoundListenerDS3D::GetPosition(float &x, float &y, float &z)
{
	x = fPosX; y = fPosY; z = fPosZ;
}

void csSoundListenerDS3D::GetDirection(float &fx, float &fy, float &fz, float &tx, float &ty, float &tz)
{
	fx = fDirFrontX; fy = fDirFrontY; fz = fDirFrontZ;
	tx = fDirTopX; ty = fDirTopY; tz = fDirTopZ;
}

float csSoundListenerDS3D::GetHeadSize()
{
	return fHeadSize;
}

void csSoundListenerDS3D::GetVelocity(float &x, float &y, float &z)
{
	x = fVelX; y = fVelY; z = fVelZ;
}

float csSoundListenerDS3D::GetDopplerFactor()
{
	return fDoppler;
}

float csSoundListenerDS3D::GetDistanceFactor()
{
	return fDistance;
}

float csSoundListenerDS3D::GetRollOffFactor()
{
	return fRollOff;
}

SoundEnvironment csSoundListenerDS3D::GetEnvironment()
{
	return Environment;
}
