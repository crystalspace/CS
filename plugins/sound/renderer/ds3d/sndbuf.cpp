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
#include "cssndrdr/ds3d/sndbuf.h"
#include "cssndrdr/ds3d/sndsrc.h"
#include "isystem.h"

IMPLEMENT_FACTORY(csSoundBufferDS3D)

IMPLEMENT_IBASE(csSoundBufferDS3D)
	IMPLEMENTS_INTERFACE(iSoundBuffer)
IMPLEMENT_IBASE_END;

csSoundBufferDS3D::csSoundBufferDS3D(iBase *piBase)
{
	CONSTRUCT_IBASE(piBase);
	m_pDS3DBuffer2D = NULL;
	m_p3DAudioRenderer = NULL;
}

csSoundBufferDS3D::~csSoundBufferDS3D()
{
	DestroySoundBuffer();
}

int csSoundBufferDS3D::CreateSoundBuffer(iSoundRender * render, csSoundData * sound)
{
	HRESULT                 hr;
	csSoundRenderDS3D *renderDS3D;
	DSBUFFERDESC    dsbd;
	VOID                            *pbWrite1 = NULL, *pbWrite2 = NULL;
	DWORD                           cbLen1, cbLen2;
	
	if (!render) return E_FAIL;
	if (!sound) return E_FAIL;
	
	renderDS3D = (csSoundRenderDS3D *)render;
	
	m_p3DAudioRenderer = renderDS3D->m_p3DAudioRenderer;
	fFrequencyOrigin = sound->getFrequency();
	
	if (!m_p3DAudioRenderer)
		return(E_FAIL);
	
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	
	dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN  | DSBCAPS_CTRL3D;  
	
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
	
	return S_OK;
}

int csSoundBufferDS3D::DestroySoundBuffer()
{
	HRESULT hr;
	
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

iSoundSource *csSoundBufferDS3D::CreateSource()
{
	csSoundSourceDS3D* pNew = new csSoundSourceDS3D(NULL);
	if (!pNew) return NULL;
	
	pNew->m_p3DAudioRenderer = m_p3DAudioRenderer;
	pNew->m_pDS3DBuffer2D = m_pDS3DBuffer2D;
	pNew->pSoundBuffer = this;
	
	return QUERY_INTERFACE(pNew,iSoundSource);
}

void csSoundBufferDS3D::Play(SoundBufferPlayMethod playMethod)
{
	if (m_pDS3DBuffer2D)
	{
		switch(playMethod)
		{
		case SoundBufferPlay_Normal:
			m_pDS3DBuffer2D->Stop();
			m_pDS3DBuffer2D->Play(0, 0, 0);
			break;
			
		case SoundBufferPlay_InLoop:
			m_pDS3DBuffer2D->Stop();
			m_pDS3DBuffer2D->Play(0, 0, DSBPLAY_LOOPING);
			break;
			
		case SoundBufferPlay_RestartInLoop:
			m_pDS3DBuffer2D->Stop();
			m_pDS3DBuffer2D->SetCurrentPosition(0);
			m_pDS3DBuffer2D->Play(0, 0, DSBPLAY_LOOPING);
			break;
			
		case SoundBufferPlay_Restart:
			m_pDS3DBuffer2D->Stop();
			m_pDS3DBuffer2D->SetCurrentPosition(0);
			m_pDS3DBuffer2D->Play(0, 0, 0);
			break;
			
		case SoundBufferPlay_DestroyAtEnd:
			break;
			
		default:
			break;
		}
	}
}

void csSoundBufferDS3D::Stop()
{
	if (m_pDS3DBuffer2D)
		m_pDS3DBuffer2D->Stop();
}

void csSoundBufferDS3D::SetVolume(float vol)
{
	fVolume = vol;
	
	if (m_pDS3DBuffer2D)
	{
		long dsvol = DSBVOLUME_MIN + (DSBVOLUME_MAX-DSBVOLUME_MIN)*vol;
		m_pDS3DBuffer2D->SetVolume(dsvol);
	}
}

float csSoundBufferDS3D::GetVolume()
{  
	float returnVal = fVolume;
	
	if (m_pDS3DBuffer2D)
	{
		long dsvol=DSBVOLUME_MIN;
		m_pDS3DBuffer2D->GetVolume(&dsvol);
		returnVal = (float)(dsvol-DSBVOLUME_MIN)/(float)(DSBVOLUME_MAX-DSBVOLUME_MIN);
	}

	return returnVal;
}

void csSoundBufferDS3D::SetFrequencyFactor(float factor)
{
	fFrequencyFactor = factor;
	
	if (m_pDS3DBuffer2D)
	{
		unsigned long dsfreq = fFrequencyOrigin*factor;
		m_pDS3DBuffer2D->SetFrequency(dsfreq);
	}
}

float csSoundBufferDS3D::GetFrequencyFactor()
{
	float factor = fFrequencyFactor;
	
	if (m_pDS3DBuffer2D)
	{
		unsigned long dsfreq;
		m_pDS3DBuffer2D->GetFrequency(&dsfreq);
		factor = (float)(dsfreq)/fFrequencyOrigin;
	}

	return factor;
}