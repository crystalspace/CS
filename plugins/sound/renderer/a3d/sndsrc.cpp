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
#include "cssndrdr/a3d/sndbuf.h"
#include "cssndrdr/a3d/sndsrc.h"

IMPLEMENT_UNKNOWN_NODELETE (csSoundSourceA3D)

BEGIN_INTERFACE_TABLE(csSoundSourceA3D)
  IMPLEMENTS_INTERFACE(ISoundSource)
END_INTERFACE_TABLE()

csSoundSourceA3D::csSoundSourceA3D()
{
  fPosX = fPosY = fPosZ = 0.0;
  fVelX = fVelY = fVelZ = 0.0;

  m_p3DSource = NULL;
  m_p3DAudioRenderer = NULL;
}

csSoundSourceA3D::~csSoundSourceA3D()
{

}

STDMETHODIMP csSoundSourceA3D::GetSoundBuffer(ISoundBuffer **ppv)
{
  if(!pSoundBuffer)
  {
    return E_FAIL;
  }

  return pSoundBuffer->QueryInterface (IID_ISoundBuffer, (void**)ppv);
}

int csSoundSourceA3D::CreateSource(ISoundRender * render, csSoundData * sound)
{
  HRESULT           hr;
  csSoundRenderA3D *renderA3D;
  VOID             *pbWrite1 = NULL, *pbWrite2 = NULL;
  DWORD             cbLen1, cbLen2;

  if (!render) return E_FAIL;
  if (!sound)  return E_FAIL;

  renderA3D = (csSoundRenderA3D *)render;

  m_p3DAudioRenderer = renderA3D->m_p3DAudioRenderer;
  
  if (!m_p3DAudioRenderer)
    return(E_FAIL);
  
  if ((hr = m_p3DAudioRenderer->NewSource( A3DSOURCE_INITIAL_RENDERMODE_A3D , &m_p3DSource)) != S_OK)
  {
    if (m_p3DSource)
      m_p3DSource = NULL;

    return(hr);
  }
  
  WAVEFORMATEX    wfxFormat;
  wfxFormat.wFormatTag                    = WAVE_FORMAT_PCM;
  wfxFormat.nChannels                     = (sound->isStereo())?2:1;
  wfxFormat.nSamplesPerSec        = sound->getFrequency();
  wfxFormat.wBitsPerSample        = (sound->is16Bits())?16:8;
  wfxFormat.nBlockAlign           = (wfxFormat.wBitsPerSample*wfxFormat.nChannels)/8;
  wfxFormat.nAvgBytesPerSec       = wfxFormat.nBlockAlign*wfxFormat.nSamplesPerSec;
  wfxFormat.cbSize                                = 0;

  if ((hr = m_p3DSource->SetWaveFormat((void*)&wfxFormat)) != S_OK)
  {
    if (m_p3DSource != NULL)
    {
      m_p3DSource->Release();
      m_p3DSource = NULL;
    }

    return(hr);
  }

  if ((hr = m_p3DSource->AllocateWaveData(sound->getSize())) != S_OK)
  {
    if (m_p3DSource != NULL)
    {
      m_p3DSource->Release();
      m_p3DSource = NULL;
    }

    return(hr);
  }

  if ((hr = m_p3DSource->Lock(0, sound->getSize(), &pbWrite1, &cbLen1, &pbWrite2, &cbLen2, A3D_ENTIREBUFFER)) != S_OK)
  {
    if (m_p3DSource != NULL)
    {
      m_p3DSource->Release();
      m_p3DSource = NULL;
    }

    return(hr);
  }
  
  CopyMemory(pbWrite1, sound->getData(), sound->getSize());
    
  if ((hr = m_p3DSource->Unlock(pbWrite1, sound->getSize(), pbWrite2, 0)) != S_OK)
  {
    if (m_p3DSource != NULL)
    {
      m_p3DSource->Release();
      m_p3DSource = NULL;
    }
    
    return(hr);
  }


  
  return S_OK;
}

int csSoundSourceA3D::DestroySource()
{
  HRESULT hr;
  
  if (m_p3DSource)
  {
    if ((hr = m_p3DSource->Stop()) < S_OK)
      return(hr);
    
    if ((hr = m_p3DSource->FreeWaveData()) != S_OK)
      return(hr);

    if ((hr = m_p3DSource->Release()) < S_OK)
      return(hr);
    
    m_p3DSource = NULL;
  }

  return S_OK;
}

STDMETHODIMP csSoundSourceA3D::SetPosition(float x, float y, float z)
{
  fPosX = x; fPosY = y; fPosZ = z;

  if(!m_p3DSource)
    return E_FAIL;

  m_p3DSource->SetPosition3f(x, y, z);

  return S_OK;
}

STDMETHODIMP csSoundSourceA3D::SetVelocity(float x, float y, float z)
{
  fVelX = x; fVelY = y; fVelZ = z;

  if(!m_p3DSource)
    return E_FAIL;

  m_p3DSource->SetVelocity3f(x, y, z);

  return S_OK;
}

STDMETHODIMP csSoundSourceA3D::GetPosition(float &x, float &y, float &z)
{
  x = fPosX; y = fPosY; z = fPosZ;

  if(!m_p3DSource)
    return E_FAIL;

  m_p3DSource->GetPosition3f(&x, &y, &z);

  return S_OK;
}

STDMETHODIMP csSoundSourceA3D::GetVelocity(float &x, float &y, float &z)
{
  x = fVelX; y = fVelY; z = fVelZ;

  if(!m_p3DSource)
    return E_FAIL;

  m_p3DSource->GetVelocity3f(&x, &y, &z);

  return S_OK;
}
