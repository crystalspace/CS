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
#include "cscom/com.h"
#include "isystem.h"

#include "dsound.h"

#include "cssndrdr/ds3d/sndrdr.h"
#include "cssndrdr/ds3d/sndlstn.h"

IMPLEMENT_UNKNOWN_NODELETE (csSoundListenerDS3D)

BEGIN_INTERFACE_TABLE(csSoundListenerDS3D)
  IMPLEMENTS_INTERFACE(ISoundListener)
END_INTERFACE_TABLE()

csSoundListenerDS3D::csSoundListenerDS3D()
{
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

int csSoundListenerDS3D::CreateListener(ISoundRender * render)
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

STDMETHODIMP csSoundListenerDS3D::SetPosition(float x, float y, float z)
{
  HRESULT hr;

  fPosX = x; fPosY = y; fPosZ = z;

  if ((hr = m_pDS3DListener->SetPosition(
    fPosX, fPosY, fPosZ,
    DS3D_IMMEDIATE)) != DS_OK)
  {
    return E_FAIL;
  }
  
  return S_OK;
}

STDMETHODIMP csSoundListenerDS3D::SetDirection(float fx, float fy, float fz, float tx, float ty, float tz)
{
  HRESULT hr;

  fDirFrontX = fx; fDirFrontY = fy; fDirFrontZ = fz;
  fDirTopX = tx; fDirTopY = ty; fDirTopZ = tz;

  if ((hr = m_pDS3DListener->SetOrientation(
    fDirFrontX, fDirFrontY, fDirFrontZ,
    fDirTopX, fDirTopY, fDirTopZ,
    DS3D_IMMEDIATE)) != DS_OK)
  {
    return E_FAIL;
  }

  return S_OK;
}

STDMETHODIMP csSoundListenerDS3D::SetHeadSize(float size)
{
  fHeadSize = size;

  return S_OK;
}

STDMETHODIMP csSoundListenerDS3D::SetVelocity(float x, float y, float z)
{
  HRESULT hr;
  
  fVelX = x; fVelY = y; fVelZ = z;

  if(!m_pDS3DListener) return E_FAIL;

  if ((hr = m_pDS3DListener->SetVelocity(
    fVelX, fVelY, fVelZ,
    DS3D_IMMEDIATE)) != DS_OK)
  {
    return E_FAIL;
  }

  return S_OK;
}

STDMETHODIMP csSoundListenerDS3D::SetDopplerFactor(float factor)
{
  HRESULT hr;

  fDoppler = factor;

  if(!m_pDS3DListener) return E_FAIL;

  if ((hr = m_pDS3DListener->SetDopplerFactor(fDoppler, DS3D_IMMEDIATE)) != DS_OK)
  {
    return E_FAIL;
  }

  return S_OK;
}

STDMETHODIMP csSoundListenerDS3D::SetDistanceFactor(float factor)
{
  HRESULT hr;

  fDistance = factor;

  if(!m_pDS3DListener) return E_FAIL;

  if ((hr = m_pDS3DListener->SetDistanceFactor(fDistance, DS3D_IMMEDIATE)) != DS_OK)
  {
    return E_FAIL;
  }
  
  return S_OK;
}

STDMETHODIMP csSoundListenerDS3D::SetRollOffFactor(float factor)
{
  HRESULT hr;

  fRollOff = factor;

  if(!m_pDS3DListener) return E_FAIL;

  if ((hr = m_pDS3DListener->SetRolloffFactor(fRollOff, DS3D_IMMEDIATE)) != DS_OK)
  {
    return E_FAIL;
  }
  
  return S_OK;
}

STDMETHODIMP csSoundListenerDS3D::SetEnvironment(SoundEnvironment env)
{
  Environment = env;

  return S_OK;
}

STDMETHODIMP csSoundListenerDS3D::GetPosition(float &x, float &y, float &z)
{
  x = fPosX; y = fPosY; z = fPosZ;

  return S_OK;
}

STDMETHODIMP csSoundListenerDS3D::GetDirection(float &fx, float &fy, float &fz, float &tx, float &ty, float &tz)
{
  fx = fDirFrontX; fy = fDirFrontY; fz = fDirFrontZ;
  tx = fDirTopX; ty = fDirTopY; tz = fDirTopZ;

  return S_OK;
}

STDMETHODIMP csSoundListenerDS3D::GetHeadSize(float &size)
{
  size = fHeadSize;

  return S_OK;
}

STDMETHODIMP csSoundListenerDS3D::GetVelocity(float &x, float &y, float &z)
{
  x = fVelX; y = fVelY; z = fVelZ;

  return S_OK;
}

STDMETHODIMP csSoundListenerDS3D::GetDopplerFactor(float &factor)
{
  factor = fDoppler;

  return S_OK;
}

STDMETHODIMP csSoundListenerDS3D::GetDistanceFactor(float &factor)
{
  factor = fDistance;

  return S_OK;
}

STDMETHODIMP csSoundListenerDS3D::GetRollOffFactor(float &factor)
{
  factor = fRollOff;

  return S_OK;
}

STDMETHODIMP csSoundListenerDS3D::GetEnvironment(SoundEnvironment &env)
{
  env = Environment;

  return S_OK;
}
