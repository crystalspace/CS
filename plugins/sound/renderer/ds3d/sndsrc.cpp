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

#include "dsound.h"

#include "cssndrdr/ds3d/sndrdr.h"
#include "cssndrdr/ds3d/sndsrc.h"
#include "cssndrdr/ds3d/sndbuf.h"
#include "isystem.h"

IMPLEMENT_FACTORY(csSoundSourceDS3D)

IMPLEMENT_IBASE(csSoundSourceDS3D)
  IMPLEMENTS_INTERFACE(iSoundSource)
IMPLEMENT_IBASE_END;

csSoundSourceDS3D::csSoundSourceDS3D(iBase *piBase)
{
  CONSTRUCT_IBASE(piBase);

  fPosX = fPosY = fPosZ = 0.0;
  fVelX = fVelY = fVelZ = 0.0;
  m_pDS3DBuffer3D = NULL;
}

csSoundSourceDS3D::~csSoundSourceDS3D()
{
  DestroySource();
}

int csSoundSourceDS3D::CreateSource()
{
  HRESULT                 hr;
  DWORD           dwMode;

  if (!m_p3DAudioRenderer
    || !m_pDS3DBuffer2D)
    return(E_FAIL);
  
  if((hr = m_pDS3DBuffer2D->QueryInterface(IID_IDirectSound3DBuffer, (void **) &m_pDS3DBuffer3D)) < DS_OK)
  {
    return(E_FAIL);
  }
  else
  {
    dwMode = DS3DMODE_NORMAL;
    if ((hr = m_pDS3DBuffer3D->SetMode(dwMode, DS3D_IMMEDIATE)) < DS_OK)
    {
      return(E_FAIL);
    }
  }

  return S_OK;
}

int csSoundSourceDS3D::DestroySource()
{
  HRESULT hr;
  
  if (m_pDS3DBuffer3D)
  {
    if ((hr = m_pDS3DBuffer3D->Release()) < DS_OK)
      return(hr);
    
    m_pDS3DBuffer3D = NULL;
  }
  
  return S_OK;
}

iSoundBuffer *csSoundSourceDS3D::GetSoundBuffer()
{
  if(!pSoundBuffer) return NULL;
  return QUERY_INTERFACE(pSoundBuffer, iSoundBuffer);
}

void csSoundSourceDS3D::SetPosition(float x, float y, float z)
{
  fPosX = x; fPosY = y; fPosZ = z;

  if (m_pDS3DBuffer3D)
    m_pDS3DBuffer3D->SetPosition(x, y ,z, DS3D_IMMEDIATE);
}

void csSoundSourceDS3D::SetVelocity(float x, float y, float z)
{
  fVelX = x; fVelY = y; fVelZ = z;

  if (m_pDS3DBuffer3D)
    m_pDS3DBuffer3D->SetVelocity(x, y ,z, DS3D_IMMEDIATE);
}

void csSoundSourceDS3D::GetPosition(float &x, float &y, float &z)
{
  x = fPosX; y = fPosY; z = fPosZ;

  if (m_pDS3DBuffer3D)
  {
    D3DVECTOR v;
    m_pDS3DBuffer3D->GetPosition(&v);
    x = v.x; y = v.y; z = v.z;
  }
}

void csSoundSourceDS3D::GetVelocity(float &x, float &y, float &z)
{
  x = fVelX; y = fVelY; z = fVelZ;

  if (m_pDS3DBuffer3D)
  {
    D3DVECTOR v;
    m_pDS3DBuffer3D->GetVelocity(&v);
    x = v.x; y = v.y; z = v.z;
  }
}
