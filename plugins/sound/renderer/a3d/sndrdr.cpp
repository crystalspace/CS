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

#include "ia3dapi.h"

#include "sysdef.h"
#include "cscom/com.h"
#include "cssndrdr/a3d/sndrdr.h"
#include "cssndrdr/a3d/sndlstn.h"
#include "cssndrdr/a3d/sndsrc.h"
#include "isystem.h"
#include "isndlstn.h"
#include "isndsrc.h"


IMPLEMENT_UNKNOWN_NODELETE (csSoundRenderA3D)

BEGIN_INTERFACE_TABLE(csSoundRenderA3D)
  IMPLEMENTS_INTERFACE(ISoundRender)
END_INTERFACE_TABLE()

csSoundRenderA3D::csSoundRenderA3D(ISystem* piSystem) : m_pListener(NULL)
{
  m_p3DAudioRenderer = NULL;
  m_piSystem = piSystem;
  
  m_pListener = new csSoundListenerA3D ();
}

csSoundRenderA3D::~csSoundRenderA3D()
{
  if(m_pListener)
    delete m_pListener;
}

STDMETHODIMP csSoundRenderA3D::GetListener(ISoundListener** ppv )
{
  if (!m_pListener)
  {
    *ppv = NULL;
    return E_OUTOFMEMORY;
  }
  
  return m_pListener->QueryInterface (IID_ISoundListener, (void**)ppv);
}

STDMETHODIMP csSoundRenderA3D::CreateSource(ISoundSource** ppv, csSoundBuffer *snd)
{
  csSoundSourceA3D* pNew = new csSoundSourceA3D ();
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }

  pNew->CreateSource(this, snd);
  
  return pNew->QueryInterface (IID_ISoundSource, (void**)ppv);
}

STDMETHODIMP csSoundRenderA3D::Open()
{
  HRESULT hr;
  
  SysPrintf (MSG_INITIALIZATION, "\nSoundRender Aureal3D selected\n");

	CoInitialize(NULL);

	hr = CoCreateInstance(CLSID_A3dApi, NULL, CLSCTX_INPROC_SERVER,
	                      IID_IA3d4, (void **)&m_p3DAudioRenderer);
  if (FAILED(hr))
  {
    SysPrintf(MSG_FATAL_ERROR, "Error : Cannot CoCreateInstance Aureal3D Api !");
    Close();
    return(hr);
  }

  if (FAILED(hr = m_p3DAudioRenderer->Init(NULL, NULL, A3DRENDERPREFS_DEFAULT)))
  {
    SysPrintf(MSG_FATAL_ERROR, "Error : Cannot Initialize Aureal3D !");
    Close();
    return(hr);
  }
  
  if (FAILED(hr = m_p3DAudioRenderer->SetCooperativeLevel(GetForegroundWindow(), A3D_CL_NORMAL)))
  {
    SysPrintf(MSG_FATAL_ERROR, "Error : Cannot Set Cooperative Level!");
    Close();
    return(hr);
  }
  
  m_pListener->CreateListener(this);

  return S_OK;
}

STDMETHODIMP csSoundRenderA3D::Close()
{
  HRESULT hr;
  
  if(m_pListener)
  {
    m_pListener->DestroyListener();
    m_pListener->Release();
  }


  if (m_p3DAudioRenderer)
  {
    if ((hr = m_p3DAudioRenderer->Release()) < S_OK)
      return(hr);
    
    m_p3DAudioRenderer = NULL;
  }

  CoUninitialize();

  return S_OK;
}

STDMETHODIMP csSoundRenderA3D::Update()
{
  m_p3DAudioRenderer->Flush();

  return S_OK;
}

STDMETHODIMP csSoundRenderA3D::SetVolume(float vol)
{
  return S_OK;
}

STDMETHODIMP csSoundRenderA3D::GetVolume(float *vol)
{
  return S_OK;
}

STDMETHODIMP csSoundRenderA3D::PlayEphemeral(csSoundBuffer *snd)
{
  return S_OK;
}

void csSoundRenderA3D::SysPrintf(int mode, char* szMsg, ...)
{
  char buf[1024];
  va_list arg;
  
  va_start (arg, szMsg);
  vsprintf (buf, szMsg, arg);
  va_end (arg);
  
  m_piSystem->Print(mode, buf);
}
