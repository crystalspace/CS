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
#include <stdio.h>
#include <windows.h>

#include <initguid.h>
#include "dsound.h"

#include "sysdef.h"
#include "cscom/com.h"
#include "cssndrdr/ds3d/sndrdr.h"
#include "cssndrdr/ds3d/sndlstn.h"
#include "cssndrdr/ds3d/sndsrc.h"
#include "isystem.h"
#include "isndlstn.h"
#include "isndsrc.h"


IMPLEMENT_UNKNOWN_NODELETE (csSoundRenderDS3D)

BEGIN_INTERFACE_TABLE(csSoundRenderDS3D)
  IMPLEMENTS_INTERFACE(ISoundRender)
END_INTERFACE_TABLE()

csSoundRenderDS3D::csSoundRenderDS3D(ISystem* piSystem) : m_pListener(NULL)
{
  m_p3DAudioRenderer = NULL;
  m_piSystem = piSystem;
  
  m_pListener = new csSoundListenerDS3D ();
}

csSoundRenderDS3D::~csSoundRenderDS3D()
{
  if(m_pListener)
    delete m_pListener;
}

STDMETHODIMP csSoundRenderDS3D::GetListener(ISoundListener** ppv )
{
  if (!m_pListener)
  {
    *ppv = NULL;
    return E_OUTOFMEMORY;
  }
  
  return m_pListener->QueryInterface (IID_ISoundListener, (void**)ppv);
}

STDMETHODIMP csSoundRenderDS3D::CreateSource(ISoundSource** ppv, csSoundBuffer *snd)
{
  csSoundSourceDS3D* pNew = new csSoundSourceDS3D ();
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }

  pNew->CreateSource(this, snd);
  
  return pNew->QueryInterface (IID_ISoundSource, (void**)ppv);
}

STDMETHODIMP csSoundRenderDS3D::Open()
{
  HRESULT hr;
  
  SysPrintf (MSG_INITIALIZATION, "\nSoundRender DirectSound3D selected\n");

  if (FAILED(hr = DirectSoundCreate(NULL, &m_p3DAudioRenderer, NULL)))
  {
    SysPrintf(MSG_FATAL_ERROR, "Error : Cannot Initialize DirectSound3D !");
    Close();
    return(hr);
  }
  
  
  DWORD dwLevel = DSSCL_NORMAL;
  if (FAILED(hr = m_p3DAudioRenderer->SetCooperativeLevel(GetForegroundWindow(), dwLevel)))
  {
    SysPrintf(MSG_FATAL_ERROR, "Error : Cannot Set Cooperative Level!");
    Close();
    return(hr);
  }
  
  m_pListener->CreateListener(this);

  return S_OK;
}

STDMETHODIMP csSoundRenderDS3D::Close()
{
  HRESULT hr;
  
  if(m_pListener)
  {
    m_pListener->DestroyListener();
    m_pListener->Release();
  }


  if (m_p3DAudioRenderer)
  {
    if ((hr = m_p3DAudioRenderer->Release()) < DS_OK)
      return(hr);
    
    m_p3DAudioRenderer = NULL;
  }

  return S_OK;
}

STDMETHODIMP csSoundRenderDS3D::Update()
{
  return S_OK;
}

STDMETHODIMP csSoundRenderDS3D::SetVolume(float vol)
{
  long dsvol;
  if (m_pListener)
  {
    m_pListener->m_pDS3DPrimaryBuffer->SetVolume(dsvol);
  }
  return S_OK;
}

STDMETHODIMP csSoundRenderDS3D::GetVolume(float *vol)
{
  long dsvol;
  if (m_pListener)
  {
    m_pListener->m_pDS3DPrimaryBuffer->GetVolume(&dsvol);
  }

  return S_OK;
}

STDMETHODIMP csSoundRenderDS3D::PlayEphemeral(csSoundBuffer *snd)
{
  return S_OK;
}

void csSoundRenderDS3D::SysPrintf(int mode, char* szMsg, ...)
{
  char buf[1024];
  va_list arg;
  
  va_start (arg, szMsg);
  vsprintf (buf, szMsg, arg);
  va_end (arg);
  
  m_piSystem->Print(mode, buf);
}
