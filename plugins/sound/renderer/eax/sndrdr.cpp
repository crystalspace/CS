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
#include "cssysdef.h"

#include <stdio.h>

#include <initguid.h>
#include "dsound.h"
#include "eax.h"

#include "csutil/scf.h"
#include "sndrdr.h"
#include "sndlstn.h"
#include "sndsrc.h"
#include "sndbuf.h"
#include "isystem.h"
#include "isndlstn.h"
#include "isndsrc.h"

IMPLEMENT_FACTORY(csSoundRenderEAX)

EXPORT_CLASS_TABLE (sndrdrdseax)
	EXPORT_CLASS (csSoundRenderEAX, "crystalspace.sound.render.eax",
			  "EAX 3D Sound Driver for Crystal Space")
EXPORT_CLASS_TABLE_END;

IMPLEMENT_IBASE(csSoundRenderEAX)
	IMPLEMENTS_INTERFACE(iSoundRender)
	IMPLEMENTS_INTERFACE(iPlugIn)
IMPLEMENT_IBASE_END;

csSoundRenderEAX::csSoundRenderEAX(iBase *piBase)
{
	CONSTRUCT_IBASE(piBase);
	m_pListener = NULL;
	m_piSystem = NULL;
	m_p3DAudioRenderer = NULL;
}

bool csSoundRenderEAX::Initialize(iSystem *iSys)
{
	m_piSystem = iSys;
	m_pListener = new csSoundListenerEAX( NULL );
	return true;
}

csSoundRenderEAX::~csSoundRenderEAX()
{
	if(m_pListener)
		delete m_pListener;
}

iSoundListener *csSoundRenderEAX::GetListener()
{
	if (!m_pListener) return NULL;
	return QUERY_INTERFACE(m_pListener, iSoundListener);
}

iSoundSource *csSoundRenderEAX::CreateSource(csSoundData *snd)
{
	csSoundBufferEAX* pNew = new csSoundBufferEAX(NULL);
	if (!pNew) return NULL;
	pNew->CreateSoundBuffer(this, snd);
	pNew->SetVolume (1.0);
	return pNew->CreateSource();
}

iSoundBuffer *csSoundRenderEAX::CreateSoundBuffer(csSoundData *snd)
{
	csSoundBufferEAX* pNew = new csSoundBufferEAX (NULL);
	if (!pNew) return NULL;
	pNew->CreateSoundBuffer(this, snd);
	pNew->SetVolume (1.0);
	
	return QUERY_INTERFACE(pNew, iSoundBuffer);
}

void csSoundRenderEAX::PlayEphemeral(csSoundData *snd)
{
	iSoundBuffer *played = CreateSoundBuffer(snd);
	if(played)
	{
	    //to loop or not, defaults to not
	    if (loop == true) played->Play(SoundBufferPlay_InLoop);
	    if (!loop) played->Play(SoundBufferPlay_DestroyAtEnd);
	}
}

bool csSoundRenderEAX::Open()
{
	HRESULT hr;
	
	m_piSystem->Printf (MSG_INITIALIZATION, "\nSoundRender DirectSound3D with EAX selected\n");
	
	if (FAILED(hr = EAXDirectSoundCreate(NULL, &m_p3DAudioRenderer, NULL)))
	{
		m_piSystem->Printf(MSG_FATAL_ERROR, "Error : Cannot Initialize DirectSound3D !");
		Close();
		return false;
	}
	
	DWORD dwLevel = DSSCL_NORMAL;
	if (FAILED(hr = m_p3DAudioRenderer->SetCooperativeLevel(GetForegroundWindow(), dwLevel)))
	{
		m_piSystem->Printf(MSG_FATAL_ERROR, "Error : Cannot Set Cooperative Level!");
		Close();
		return(hr);
	}
	
	m_pListener->CreateListener(this);
	
	if(!m_pListener->m_pDS3DPrimaryBuffer)
	{
		m_piSystem->Printf(MSG_FATAL_ERROR, "Error : Listener isn't initialized !");
		Close();
		return false;
	}
	
	return true;
}

void csSoundRenderEAX::Close()
{
	HRESULT hr;
	
	if(m_pListener)
	{
		m_pListener->DestroyListener();
		m_pListener->DecRef();
	}
	
	if (m_p3DAudioRenderer)
	{
		if ((hr = m_p3DAudioRenderer->Release()) < DS_OK)
			return;
		
		m_p3DAudioRenderer = NULL;
	}
}

void csSoundRenderEAX::Update()
{
}

void csSoundRenderEAX::SetVolume(float vol)
{
	long dsvol = DSBVOLUME_MIN + (DSBVOLUME_MAX-DSBVOLUME_MIN)*vol;
	if (m_pListener)
	{
		m_pListener->m_pDS3DPrimaryBuffer->SetVolume(dsvol);
	}
}

float csSoundRenderEAX::GetVolume()
{
	long dsvol=DSBVOLUME_MIN;
	if (m_pListener)
	{
		m_pListener->m_pDS3DPrimaryBuffer->GetVolume(&dsvol);
	}
	return (float)(dsvol-DSBVOLUME_MIN)/(float)(DSBVOLUME_MAX-DSBVOLUME_MIN);
}

