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

#include "ia3dapi.h"

#include "cssndrdr/a3d/sndrdr.h"
#include "cssndrdr/a3d/sndlstn.h"

IMPLEMENT_UNKNOWN_NODELETE (csSoundListenerA3D)

BEGIN_INTERFACE_TABLE(csSoundListenerA3D)
  IMPLEMENTS_INTERFACE(ISoundListener)
END_INTERFACE_TABLE()

csSoundListenerA3D::csSoundListenerA3D()
{
  info.fPosX = info.fPosY = info.fPosZ = 0.0;
  info.fDirTopX = info.fDirTopY = info.fDirTopZ = 0.0;
  info.fDirFrontX = info.fDirFrontY = info.fDirFrontZ = 0.0;
  info.fVelX = info.fVelY = info.fVelZ = 0.0;
  info.fDoppler = 1.0;
  info.fDistance = 1.0;
  info.fRollOff = 1.0;

  m_p3DAudioRenderer = NULL;
	m_p3DListener = NULL;
}

csSoundListenerA3D::~csSoundListenerA3D()
{
  DestroyListener();
}

int csSoundListenerA3D::CreateListener(ISoundRender * render)
{
  HRESULT hr;
  csSoundRenderA3D *renderA3D;

  if(!render) return E_FAIL;
  renderA3D = (csSoundRenderA3D *)render;

  m_p3DAudioRenderer = renderA3D->m_p3DAudioRenderer;
  
  if (!m_p3DAudioRenderer)
    return(E_FAIL);
  
  if ((hr = m_p3DAudioRenderer->QueryInterface(IID_IA3dListener, (void **) &m_p3DListener)) != S_OK)
  {
    return E_FAIL;
  }

  if ((hr = m_p3DAudioRenderer->SetDopplerScale(info.fDoppler)) != S_OK)
  {
    return E_FAIL;
  }
  
  if ((hr = m_p3DAudioRenderer->SetDistanceModelScale(info.fDistance)) != S_OK)
  {
    return E_FAIL;
  }
  
  return S_OK;
}

int csSoundListenerA3D::DestroyListener()
{
  HRESULT hr;
  
  if (m_p3DListener)
  {
    if ((hr = m_p3DListener->Release()) < S_OK)
      return(hr);
    
    m_p3DListener = NULL;
  }
  
  info.fDoppler = 1.0f;
  info.fDistance = 1.0f;
  info.fRollOff = 1.0f;

  return S_OK;
}

STDMETHODIMP csSoundListenerA3D::SetPosition(float x, float y, float z)
{
  HRESULT hr;

  info.fPosX = x; info.fPosY = y; info.fPosZ = z;

  if ((hr = m_p3DListener->SetPosition3f(
    info.fPosX, info.fPosY, info.fPosZ)) != S_OK)
  {
    return E_FAIL;
  }
  
  return S_OK;
}

STDMETHODIMP csSoundListenerA3D::SetDirection(float fx, float fy, float fz, float tx, float ty, float tz)
{
  HRESULT hr;

  info.fDirFrontX = fx; info.fDirFrontY = fy; info.fDirFrontZ = fz;
  info.fDirTopX = tx; info.fDirTopY = ty; info.fDirTopZ = tz;

  if ((hr = m_p3DListener->SetOrientation6f(
    info.fDirFrontX, info.fDirFrontY, info.fDirFrontZ,
    info.fDirTopX, info.fDirTopY, info.fDirTopZ)) != S_OK)
  {
    return E_FAIL;
  }

  return S_OK;
}

STDMETHODIMP csSoundListenerA3D::SetHeadSize(float size)
{
  info.fHeadSize = size;

  return S_OK;
}

STDMETHODIMP csSoundListenerA3D::SetVelocity(float x, float y, float z)
{
  HRESULT hr;
  
  info.fVelX = x; info.fVelY = y; info.fVelZ = z;

  if(!m_p3DListener) return E_FAIL;

  if ((hr = m_p3DListener->SetVelocity3f(
    info.fVelX, info.fVelY, info.fVelZ)) != S_OK)
  {
    return E_FAIL;
  }

  return S_OK;
}

STDMETHODIMP csSoundListenerA3D::SetDopplerFactor(float factor)
{
  HRESULT hr;

  info.fDoppler = factor;

  if(!m_p3DAudioRenderer) return E_FAIL;

  if ((hr = m_p3DAudioRenderer->SetDopplerScale(info.fDoppler)) != S_OK)
  {
    return E_FAIL;
  }

  return S_OK;
}

STDMETHODIMP csSoundListenerA3D::SetDistanceFactor(float factor)
{
  HRESULT hr;

  info.fDistance = factor;

  if(!m_p3DAudioRenderer) return E_FAIL;

  if ((hr = m_p3DAudioRenderer->SetDistanceModelScale(info.fDistance)) != S_OK)
  {
    return E_FAIL;
  }
  
  return S_OK;
}

STDMETHODIMP csSoundListenerA3D::SetRollOffFactor(float factor)
{
  HRESULT hr;

  info.fRollOff = factor;

  return S_OK;
}

STDMETHODIMP csSoundListenerA3D::SetEnvironment(SoundEnvironment env)
{
  info.Environment = env;

  return S_OK;
}

STDMETHODIMP csSoundListenerA3D::GetInfoListener(csSoundListenerInfo *i)
{
  *i = info;

  return S_OK;
}
