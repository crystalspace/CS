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

#include <stdarg.h>
#include <stdio.h>

#include "sysdef.h"
#include "cscom/com.h"
#include "cssndrdr/null/nrdrcom.h"
#include "cssndrdr/null/nrdrlst.h"
#include "cssndrdr/null/nrdrsrc.h"
#include "cssndrdr/null/nrdrbuf.h"
#include "isystem.h"
#include "isndlstn.h"
#include "isndsrc.h"
#include "isndbuf.h"

IMPLEMENT_UNKNOWN_NODELETE (csSoundRenderNull)

BEGIN_INTERFACE_TABLE(csSoundRenderNull)
  IMPLEMENTS_INTERFACE(ISoundRender)
END_INTERFACE_TABLE()

csSoundRenderNull::csSoundRenderNull(ISystem* piSystem) : m_pListener(NULL)
{
  m_piSystem = piSystem;

  CHK (m_pListener = new csSoundListenerNull ());
}

csSoundRenderNull::~csSoundRenderNull()
{
  CHK (delete m_pListener);
}

STDMETHODIMP csSoundRenderNull::GetListener(ISoundListener ** ppv )
{
  if (!m_pListener)
  {
    *ppv = NULL;
    return E_OUTOFMEMORY;
  }
  
  return m_pListener->QueryInterface (IID_ISoundListener, (void**)ppv);
}

STDMETHODIMP csSoundRenderNull::CreateSource(ISoundSource ** ppv, csSoundData* snd)
{
  ISoundBuffer* pNewBuffer=NULL;

  CreateSoundBuffer(&pNewBuffer, snd);

  if(!pNewBuffer)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }

  csSoundBufferNull *pNew = (csSoundBufferNull*)pNewBuffer;

  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }
  
  pNew->SetVolume (1.0);

  return pNew->CreateSource(ppv);
}

STDMETHODIMP csSoundRenderNull::CreateSoundBuffer(ISoundBuffer** ppv, csSoundData * /*snd*/)
{
  CHK (csSoundBufferNull* pNew = new csSoundBufferNull ());
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }
  pNew->SetVolume (1.0);

  return pNew->QueryInterface (IID_ISoundBuffer, (void**)ppv);
}

STDMETHODIMP csSoundRenderNull::Open()
{
  SysPrintf (MSG_INITIALIZATION, "\nSoundRender Null selected\n");

  return S_OK;
}

STDMETHODIMP csSoundRenderNull::Close()
{
  return S_OK;
}

STDMETHODIMP csSoundRenderNull::Update()
{
  return S_OK;
}

STDMETHODIMP csSoundRenderNull::SetVolume(float /*vol*/)
{
  return S_OK;
}

STDMETHODIMP csSoundRenderNull::GetVolume(float* /*vol*/)
{
  return S_OK;
}

STDMETHODIMP csSoundRenderNull::PlayEphemeral(csSoundData* /*snd*/)
{
  return S_OK;
}

void csSoundRenderNull::SysPrintf(int mode, char* szMsg, ...)
{
  char buf[1024];
  va_list arg;
  
  va_start (arg, szMsg);
  vsprintf (buf, szMsg, arg);
  va_end (arg);
  
  m_piSystem->Print(mode, buf);
}
