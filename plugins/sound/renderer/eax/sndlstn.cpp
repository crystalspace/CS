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
#include "eax.h"

#include "cssndrdr/eax/sndrdr.h"
#include "cssndrdr/eax/sndlstn.h"

IMPLEMENT_UNKNOWN_NODELETE (csSoundListenerEAX)

BEGIN_INTERFACE_TABLE(csSoundListenerEAX)
  IMPLEMENTS_INTERFACE(ISoundListener)
END_INTERFACE_TABLE()

csSoundListenerEAX::csSoundListenerEAX()
{
  info.fPosX = info.fPosY = info.fPosZ = 0.0;
  info.fDirTopX = info.fDirTopY = info.fDirTopZ = 0.0;
  info.fDirFrontX = info.fDirFrontY = info.fDirFrontZ = 0.0;
  info.fVelX = info.fVelY = info.fVelZ = 0.0;
  info.fDoppler = 1.0;
  info.fDistance = 1.0;
  info.fRollOff = 1.0;

  m_p3DAudioRenderer = NULL;
  m_pDS3DPrimaryBuffer = NULL;
	m_pDS3DListener = NULL;
}

csSoundListenerEAX::~csSoundListenerEAX()
{
  DestroyListener();
}

int csSoundListenerEAX::CreateListener(ISoundRender * render)
{
  HRESULT hr;
  csSoundRenderEAX *renderEAX;

  if(!render) return E_FAIL;
  renderEAX = (csSoundRenderEAX *)render;

  m_p3DAudioRenderer = renderEAX->m_p3DAudioRenderer;
  
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
  
  if ((hr = m_pDS3DListener->SetDopplerFactor(info.fDoppler, DS3D_IMMEDIATE)) != DS_OK)
  {
    return E_FAIL;
  }
  
  if ((hr = m_pDS3DListener->SetDistanceFactor(info.fDistance, DS3D_IMMEDIATE)) != DS_OK)
  {
    return E_FAIL;
  }
  
  if ((hr = m_pDS3DListener->SetRolloffFactor(info.fRollOff, DS3D_IMMEDIATE)) != DS_OK)
  {
    return E_FAIL;
  }

  return S_OK;
}

int csSoundListenerEAX::DestroyListener()
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
  
  info.fDoppler = 1.0f;
  info.fDistance = 1.0f;
  info.fRollOff = 1.0f;

  return S_OK;
}

STDMETHODIMP csSoundListenerEAX::SetPosition(float x, float y, float z)
{
  HRESULT hr;

  info.fPosX = x; info.fPosY = y; info.fPosZ = z;

  if ((hr = m_pDS3DListener->SetPosition(
    info.fPosX, info.fPosY, info.fPosZ,
    DS3D_IMMEDIATE)) != DS_OK)
  {
    return E_FAIL;
  }
  
  return S_OK;
}

STDMETHODIMP csSoundListenerEAX::SetDirection(float fx, float fy, float fz, float tx, float ty, float tz)
{
  HRESULT hr;

  info.fDirFrontX = fx; info.fDirFrontY = fy; info.fDirFrontZ = fz;
  info.fDirTopX = tx; info.fDirTopY = ty; info.fDirTopZ = tz;

  if ((hr = m_pDS3DListener->SetOrientation(
    info.fDirFrontX, info.fDirFrontY, info.fDirFrontZ,
    info.fDirTopX, info.fDirTopY, info.fDirTopZ,
    DS3D_IMMEDIATE)) != DS_OK)
  {
    return E_FAIL;
  }

  return S_OK;
}

STDMETHODIMP csSoundListenerEAX::SetHeadSize(float size)
{
  info.fHeadSize = size;

  return S_OK;
}

STDMETHODIMP csSoundListenerEAX::SetVelocity(float x, float y, float z)
{
  HRESULT hr;
  
  info.fVelX = x; info.fVelY = y; info.fVelZ = z;

  if(!m_pDS3DListener) return E_FAIL;

  if ((hr = m_pDS3DListener->SetVelocity(
    info.fVelX, info.fVelY, info.fVelZ,
    DS3D_IMMEDIATE)) != DS_OK)
  {
    return E_FAIL;
  }

  return S_OK;
}

STDMETHODIMP csSoundListenerEAX::SetDopplerFactor(float factor)
{
  HRESULT hr;

  info.fDoppler = factor;

  if(!m_pDS3DListener) return E_FAIL;

  if ((hr = m_pDS3DListener->SetDopplerFactor(info.fDoppler, DS3D_IMMEDIATE)) != DS_OK)
  {
    return E_FAIL;
  }

  return S_OK;
}

STDMETHODIMP csSoundListenerEAX::SetDistanceFactor(float factor)
{
  HRESULT hr;

  info.fDistance = factor;

  if(!m_pDS3DListener) return E_FAIL;

  if ((hr = m_pDS3DListener->SetDistanceFactor(info.fDistance, DS3D_IMMEDIATE)) != DS_OK)
  {
    return E_FAIL;
  }
  
  return S_OK;
}

STDMETHODIMP csSoundListenerEAX::SetRollOffFactor(float factor)
{
  HRESULT hr;

  info.fRollOff = factor;

  if(!m_pDS3DListener) return E_FAIL;

  if ((hr = m_pDS3DListener->SetRolloffFactor(info.fRollOff, DS3D_IMMEDIATE)) != DS_OK)
  {
    return E_FAIL;
  }
  
  return S_OK;
}

#define MAX_s2eaxEnv 25
struct s2eaxEnv_type
{
  SoundEnvironment senv;
  EAX_REVERBPROPERTIES eaxenv;
} s2eaxEnv[MAX_s2eaxEnv]=
{
  {ENVIRONMENT_GENERIC,{EAX_PRESET_GENERIC}},
  {ENVIRONMENT_PADDEDCELL,{EAX_PRESET_PADDEDCELL}},
  {ENVIRONMENT_ROOM,{EAX_PRESET_ROOM}},
  {ENVIRONMENT_BATHROOM,{EAX_PRESET_BATHROOM}},
  {ENVIRONMENT_LIVINGROOM,{EAX_PRESET_LIVINGROOM}},
  {ENVIRONMENT_STONEROOM,{EAX_PRESET_STONEROOM}},
  {ENVIRONMENT_AUDITORIUM,{EAX_PRESET_AUDITORIUM}},
  {ENVIRONMENT_CONCERTHALL,{EAX_PRESET_CONCERTHALL}},
  {ENVIRONMENT_CAVE,{EAX_PRESET_CAVE}},
  {ENVIRONMENT_ARENA,{EAX_PRESET_ARENA}},
  {ENVIRONMENT_CARPETEDHALLWAY,{EAX_PRESET_CARPETEDHALLWAY}},
  {ENVIRONMENT_HALLWAY,{EAX_PRESET_HALLWAY}},
  {ENVIRONMENT_STONECORRIDOR,{EAX_PRESET_STONECORRIDOR}},
  {ENVIRONMENT_ALLEY,{EAX_PRESET_ALLEY}},
  {ENVIRONMENT_FOREST,{EAX_PRESET_FOREST}},
  {ENVIRONMENT_CITY,{EAX_PRESET_CITY}},
  {ENVIRONMENT_MOUNTAINS,{EAX_PRESET_MOUNTAINS}},
  {ENVIRONMENT_QUARRY,{EAX_PRESET_QUARRY}},
  {ENVIRONMENT_PLAIN,{EAX_PRESET_PLAIN}},
  {ENVIRONMENT_PARKINGLOT,{EAX_PRESET_PARKINGLOT}},
  {ENVIRONMENT_SEWERPIPE,{EAX_PRESET_SEWERPIPE}},
  {ENVIRONMENT_UNDERWATER,{EAX_PRESET_UNDERWATER}},
  {ENVIRONMENT_DRUGGED,{EAX_PRESET_DRUGGED}},
  {ENVIRONMENT_DIZZY,{EAX_PRESET_DIZZY}},
  {ENVIRONMENT_PSYCHOTIC,{EAX_PRESET_PSYCHOTIC}}
};

STDMETHODIMP csSoundListenerEAX::SetEnvironment(SoundEnvironment env)
{
  UINT ret = S_FALSE;
  info.Environment = env;

  LPKSPROPERTYSET pProperties;
  HRESULT hr = m_pDS3DPrimaryBuffer->QueryInterface(IID_IKsPropertySet, (void**) &pProperties);
  if(SUCCEEDED(hr) && pProperties)
  {
    ULONG support = 0;
    hr = pProperties->QuerySupport(DSPROPSETID_EAX_ReverbProperties, DSPROPERTY_EAX_ALL, &support);
    if(SUCCEEDED(hr))
    {
      if( (support&(KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET))
        == (KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET))
      {
        EAX_REVERBPROPERTIES preset={EAX_PRESET_GENERIC};
        
        for(int i=0; i<MAX_s2eaxEnv; i++)
          if(s2eaxEnv[i].senv==env)
          {
            preset = s2eaxEnv[i].eaxenv;
            break;
          }

        pProperties->Set(DSPROPSETID_EAX_ReverbProperties,
          DSPROPERTY_EAX_ALL,
          NULL,
          0,
          &preset,
          sizeof(EAX_REVERBPROPERTIES));
        ret = S_OK;
      }
      
      pProperties->Release();
      pProperties = NULL;
    }
  }

  return ret;
}

STDMETHODIMP csSoundListenerEAX::GetInfoListener(csSoundListenerInfo *i)
{
  *i = info;

  return S_OK;
}
