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
#include "ia3d.h"

#include "cssndrdr/a3d/sndrdr.h"
#include "cssndrdr/a3d/sndsrc.h"

IMPLEMENT_UNKNOWN_NODELETE (csSoundSourceA3D)

BEGIN_INTERFACE_TABLE(csSoundSourceA3D)
  IMPLEMENTS_INTERFACE(ISoundSource)
END_INTERFACE_TABLE()

csSoundSourceA3D::csSoundSourceA3D()
{
  info.fPosX = info.fPosY = info.fPosZ = 0.0;
  info.fVelX = info.fVelY = info.fVelZ = 0.0;

  m_pDS3DBuffer2D = NULL;
  m_pDS3DBuffer3D = NULL;
}

csSoundSourceA3D::~csSoundSourceA3D()
{

}

int csSoundSourceA3D::CreateSource(ISoundRender * render, csSoundBuffer * sound)
{
  HRESULT                 hr;
  csSoundRenderA3D *renderA3D;
  DSBUFFERDESC    dsbd;
  VOID                            *pbWrite1 = NULL, *pbWrite2 = NULL;
  DWORD                           cbLen1, cbLen2;
  DWORD           dwMode;

  if (!render) return E_FAIL;
  if (!sound) return E_FAIL;

  renderA3D = (csSoundRenderA3D *)render;

  m_p3DAudioRenderer = renderA3D->m_p3DAudioRenderer;
  
  if (!m_p3DAudioRenderer)
    return(E_FAIL);
  
  ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
  dsbd.dwSize = sizeof(DSBUFFERDESC);
  
  dsbd.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLDEFAULT | DSBCAPS_CTRL3D;
  
  WAVEFORMATEX    wfxFormat;
  wfxFormat.wFormatTag                    = WAVE_FORMAT_PCM;
  wfxFormat.nChannels                     = (sound->isStereo())?2:1;
  wfxFormat.nSamplesPerSec        = sound->getFrequency();
  wfxFormat.wBitsPerSample        = (sound->is16Bits())?16:8;
  wfxFormat.nBlockAlign           = (wfxFormat.wBitsPerSample*wfxFormat.nChannels)/8;
  wfxFormat.nAvgBytesPerSec       = wfxFormat.nBlockAlign*wfxFormat.nSamplesPerSec;
  wfxFormat.cbSize                                = 0;
  dsbd.lpwfxFormat = &wfxFormat;
  dsbd.dwBufferBytes = sound->getSize();
  
  m_pDS3DBuffer2D = NULL;
  m_pDS3DBuffer3D = NULL;
  
  if ((hr = m_p3DAudioRenderer->CreateSoundBuffer(&dsbd, &m_pDS3DBuffer2D, NULL)) != DS_OK)
  {
    if (m_pDS3DBuffer2D)
      m_pDS3DBuffer2D = NULL;
    return(hr);
  }
  
  if ((hr = m_pDS3DBuffer2D->Lock(0, sound->getSize(), &pbWrite1, &cbLen1, &pbWrite2, &cbLen2, 0L)) != DS_OK)
  {
    if (pbWrite1 != NULL)
    {
      m_pDS3DBuffer2D->Unlock(pbWrite1, sound->getSize(), pbWrite2, 0);
      pbWrite1 = NULL;
    }
    
    if (m_pDS3DBuffer2D != NULL)
    {
      m_pDS3DBuffer2D->Release();
      m_pDS3DBuffer2D = NULL;
    }
    
    return(hr);
  }
  
  CopyMemory(pbWrite1, sound->getData(), sound->getSize());
    
  if ((hr = m_pDS3DBuffer2D->Unlock(pbWrite1, sound->getSize(), pbWrite2, 0)) != DS_OK)
  {
    if (pbWrite1 != NULL)
    {
      m_pDS3DBuffer2D->Unlock(pbWrite1, sound->getSize(), pbWrite2, 0);
      pbWrite1 = NULL;
    }
    
    if (m_pDS3DBuffer2D != NULL)
    {
      m_pDS3DBuffer2D->Release();
      m_pDS3DBuffer2D = NULL;
    }
    
    return(hr);
  }
  

  pbWrite1 = NULL;
  pbWrite2 = NULL;

  if((hr = m_pDS3DBuffer2D->QueryInterface(IID_IDirectSound3DBuffer, (void **) &m_pDS3DBuffer3D)) < DS_OK)
  {
  }
  else
  {
    dwMode = DS3DMODE_NORMAL;
    if ((hr = m_pDS3DBuffer3D->SetMode(dwMode, DS3D_IMMEDIATE)) < DS_OK)
    {
    }
  }
  
  return S_OK;
}

int csSoundSourceA3D::DestroySource()
{
  HRESULT hr;
  
  if (m_pDS3DBuffer3D)
  {
    if ((hr = m_pDS3DBuffer3D->Release()) < DS_OK)
      return(hr);
    
    m_pDS3DBuffer3D = NULL;
  }
  
  if (m_pDS3DBuffer2D)
  {
    if ((hr = m_pDS3DBuffer2D->Stop()) < DS_OK)
      return(hr);
    
    if ((hr = m_pDS3DBuffer2D->Release()) < DS_OK)
      return(hr);
    
    m_pDS3DBuffer2D = NULL;
  }

  return S_OK;
}

STDMETHODIMP csSoundSourceA3D::PlaySource(bool inLoop)
{
  if (m_pDS3DBuffer2D)
  {
    if (inLoop)
      m_pDS3DBuffer2D->Play(0, 0, DSBPLAY_LOOPING);
    else
      m_pDS3DBuffer2D->Play(0, 0, 0);
  }

  return S_OK;
}

STDMETHODIMP csSoundSourceA3D::StopSource()
{
  if (m_pDS3DBuffer2D)
    m_pDS3DBuffer2D->Stop();

  return S_OK;
}

STDMETHODIMP csSoundSourceA3D::SetPosition(float x, float y, float z)
{
  info.fPosX = x; info.fPosY = y; info.fPosZ = z;

  return S_OK;
}

STDMETHODIMP csSoundSourceA3D::SetVelocity(float x, float y, float z)
{
  return S_OK;
}

STDMETHODIMP csSoundSourceA3D::GetInfoSource(csSoundSourceInfo *i)
{
  *i=info;

  return S_OK;
}
