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

#include <objbase.h>
#include <stdlib.h>
#include <cguid.h>

/* the A3D include file */
#include <initguid.h>
#include "ia3dapi.h"
#include "ia3dutil.h"

#include "sysdef.h"
#include "csutil/scf.h"
#include "ia3dapi.h"
#include "cssndrdr/a3d/sndrdr.h"
#include "cssndrdr/a3d/sndlstn.h"
#include "cssndrdr/a3d/sndbuf.h"
#include "cssndrdr/a3d/sndsrc.h"
#include "isystem.h"
#include "isndlstn.h"
#include "isndsrc.h"
#include "isndbuf.h"

IMPLEMENT_FACTORY(csSoundRenderA3D);

EXPORT_CLASS_TABLE (sndrdra3d)
EXPORT_CLASS (csSoundRenderA3D, "crystalspace.sound.render.a3d",
			  "Aureal 3D Sound Driver for Crystal Space")
EXPORT_CLASS_TABLE_END;

IMPLEMENT_IBASE(csSoundRenderA3D)
	IMPLEMENTS_INTERFACE(iSoundRender)
	IMPLEMENTS_INTERFACE(iPlugIn)
IMPLEMENT_IBASE_END;

csSoundRenderA3D::csSoundRenderA3D(iBase *piBase)
{
	CONSTRUCT_IBASE(piBase);
	m_pListener = NULL;
	m_piSystem = NULL;
	m_pListener = NULL;
}

bool csSoundRenderA3D::Initialize(iSystem *iSys)
{
	if (!iSys->RegisterDriver ("iSoundRender", this))
		return false;
	
	m_p3DAudioRenderer = NULL;
	m_piSystem = iSys;
	m_pListener = new csSoundListenerA3D(NULL);
	return true;
}

csSoundRenderA3D::~csSoundRenderA3D()
{
	if(m_pListener)
		delete m_pListener;
}

iSoundListener *csSoundRenderA3D::GetListener()
{
	if (!m_pListener) return NULL;
	return QUERY_INTERFACE(m_pListener, iSoundListener);
}

iSoundSource *csSoundRenderA3D::CreateSource(csSoundData *snd)
{
	csSoundBufferA3D* pNew = new csSoundBufferA3D (NULL);
	if (!pNew) return NULL;
	pNew->CreateSoundBuffer(this, snd);
	pNew->SetVolume (1.0);
	return pNew->CreateSource();
}

iSoundBuffer *csSoundRenderA3D::CreateSoundBuffer(csSoundData *snd)
{
	csSoundBufferA3D* pNew = new csSoundBufferA3D (NULL);
	if (!pNew) return NULL;
	
	pNew->CreateSoundBuffer(this, snd);
	pNew->SetVolume (1.0);
	
	return QUERY_INTERFACE(pNew, iSoundBuffer);
}

bool csSoundRenderA3D::Open()
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
		return false;
	}
	
	if (FAILED(hr = m_p3DAudioRenderer->Init(NULL, NULL, A3DRENDERPREFS_DEFAULT)))
	{
		SysPrintf(MSG_FATAL_ERROR, "Error : Cannot Initialize Aureal3D !");
		Close();
		return false;
	}
	
	if (FAILED(hr = m_p3DAudioRenderer->SetCooperativeLevel(GetForegroundWindow(), A3D_CL_NORMAL)))
	{
		SysPrintf(MSG_FATAL_ERROR, "Error : Cannot Set Cooperative Level!");
		Close();
		return false;
	}
	
	m_pListener->CreateListener(this);
	
	return true;
}

void csSoundRenderA3D::Close()
{
	if(m_pListener)
	{
		m_pListener->DestroyListener();
		m_pListener->DecRef();
	}
	
	if (m_p3DAudioRenderer)
	{
		m_p3DAudioRenderer->Release();    
		m_p3DAudioRenderer = NULL;
	}
	
	CoUninitialize();
}

void csSoundRenderA3D::Update()
{
	if (m_p3DAudioRenderer)
		m_p3DAudioRenderer->Flush();
}

void csSoundRenderA3D::SetVolume(float vol)
{
	if (m_p3DAudioRenderer)
		m_p3DAudioRenderer->SetOutputGain(vol);
}

float csSoundRenderA3D::GetVolume()
{
	float vol = 0.0f;
	if (m_p3DAudioRenderer)
		m_p3DAudioRenderer->GetOutputGain(&vol);
	return vol;
}

void csSoundRenderA3D::PlayEphemeral(csSoundData *snd)
{
	iSoundBuffer *played = CreateSoundBuffer(snd);
	if(played)
	{
		played->Play(SoundBufferPlay_DestroyAtEnd);
	}
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
