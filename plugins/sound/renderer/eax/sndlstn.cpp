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
#include "csutil/scf.h"
#include "isystem.h"

#include "dsound.h"
#include "eax.h"

#include "sndrdr.h"
#include "sndlstn.h"

IMPLEMENT_IBASE(csSoundListenerEAX)
	IMPLEMENTS_INTERFACE(iSoundListener)
IMPLEMENT_IBASE_END;

csSoundListenerEAX::csSoundListenerEAX(iBase *piBase) {
  CONSTRUCT_IBASE(piBase);
  PrimaryBuffer = NULL;
  Listener = NULL;
  Renderer = NULL;

	EaxKsPropertiesSet = NULL;
}

csSoundListenerEAX::~csSoundListenerEAX() {

	if(EaxKsPropertiesSet)
	{
		EaxKsPropertiesSet->Release();		
		EaxKsPropertiesSet = NULL;
	}

  if (Renderer) Renderer->DecRef();
  if (Listener) Listener->Release();
  if (PrimaryBuffer) {
    PrimaryBuffer->Stop();
    PrimaryBuffer->Release();
  }
}

bool csSoundListenerEAX::Initialize(csSoundRenderEAX *srdr) {
  srdr->IncRef();
  Renderer = srdr;
	
  DSBUFFERDESC dsbd;
  dsbd.dwSize = sizeof(DSBUFFERDESC);
  dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME;
  dsbd.dwReserved = 0;
  dsbd.dwBufferBytes = 0;
  dsbd.lpwfxFormat = NULL;

  HRESULT r;
  r = Renderer->AudioRenderer->CreateSoundBuffer(&dsbd, &PrimaryBuffer, NULL);
  if (r != DS_OK) {
    Renderer->System->Printf(MSG_INITIALIZATION, "EAX listener: "
      "Cannot create primary sound buffer (%s).\n", Renderer->GetError(r));
    return false;
  }
	
  r = PrimaryBuffer->QueryInterface(IID_IDirectSound3DListener, (void **) &Listener);
  if (r != DS_OK) {
    Renderer->System->Printf(MSG_INITIALIZATION, "EAX listener: Cannot query listener"
      " interface from primary sound buffer (%s).\n", Renderer->GetError(r));
    return false;
  }

	r = PrimaryBuffer->QueryInterface(IID_IKsPropertySet, (void**) &EaxKsPropertiesSet);
	ULONG support = 0;
	if(SUCCEEDED(r) && EaxKsPropertiesSet)
	{    
		r = EaxKsPropertiesSet->QuerySupport(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ALLPARAMETERS, &support);
		if(!SUCCEEDED(r) || (support&(KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET))
				!= (KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET))
		{
			Renderer->System->Printf(MSG_INITIALIZATION, "EAX listener : this device don't support EAX 2.0\nSo only DirectSound3D will be used.");
			EaxKsPropertiesSet->Release();
			EaxKsPropertiesSet = NULL;
		}
	}
	else
		Renderer->System->Printf(MSG_INITIALIZATION, "EAX listener : cannot get properties, this device may not support EAX\nSo only DirectSound3D will be used.");

  SetPosition(csVector3(0,0,0));
  SetVelocity(csVector3(0,0,0));
  SetDirection(csVector3(0,0,1), csVector3(0,1,0));
  SetDistanceFactor(1.0);
  SetDopplerFactor(1.0);
  SetDistanceFactor(1.0);
  SetRollOffFactor(1.0);
  SetEnvironment(ENVIRONMENT_GENERIC);
  Prepare();

  return true;
}

void csSoundListenerEAX::SetPosition(csVector3 v) {
  Dirty = true;
  Position = v;
  Listener->SetPosition( v.x, v.y, v.z, DS3D_DEFERRED);
}

void csSoundListenerEAX::SetDirection(csVector3 f, csVector3 t) {
  Dirty = true;
  Front = f;
  Top = t;
  Listener->SetOrientation(f.x, f.y, f.z,t.x, t.y, t.z,DS3D_DEFERRED);
}

void csSoundListenerEAX::SetHeadSize(float size) {
//  Dirty = true;
  HeadSize = size;
// @@@
}

void csSoundListenerEAX::SetVelocity(csVector3 v) {
  Dirty = true;
  Velocity = v;
  Listener->SetVelocity(v.x, v.y, v.z, DS3D_DEFERRED);
}

void csSoundListenerEAX::SetDopplerFactor(float factor) {
  Dirty = true;
  Doppler = factor;
  Listener->SetDopplerFactor(Doppler, DS3D_DEFERRED);
}

void csSoundListenerEAX::SetDistanceFactor(float factor) {
  Dirty = true;
  DistanceFactor = factor;
  Listener->SetDistanceFactor(factor, DS3D_DEFERRED);
}

void csSoundListenerEAX::SetRollOffFactor(float factor) {
  Dirty = true;
  RollOff = factor;
  Listener->SetRolloffFactor(factor, DS3D_DEFERRED);
}

#define MAX_s2eaxEnv 25
struct s2eaxEnv_type
{
	SoundEnvironment senv;
	DWORD eaxenv;
} s2eaxEnv[MAX_s2eaxEnv]=
{
	{ENVIRONMENT_GENERIC,         {EAX_ENVIRONMENT_GENERIC}},
	{ENVIRONMENT_PADDEDCELL,      {EAX_ENVIRONMENT_PADDEDCELL}},
	{ENVIRONMENT_ROOM,            {EAX_ENVIRONMENT_ROOM}},
	{ENVIRONMENT_BATHROOM,        {EAX_ENVIRONMENT_BATHROOM}},
	{ENVIRONMENT_LIVINGROOM,      {EAX_ENVIRONMENT_LIVINGROOM}},
	{ENVIRONMENT_STONEROOM,       {EAX_ENVIRONMENT_STONEROOM}},
	{ENVIRONMENT_AUDITORIUM,      {EAX_ENVIRONMENT_AUDITORIUM}},
	{ENVIRONMENT_CONCERTHALL,     {EAX_ENVIRONMENT_CONCERTHALL}},
	{ENVIRONMENT_CAVE,            {EAX_ENVIRONMENT_CAVE}},
	{ENVIRONMENT_ARENA,           {EAX_ENVIRONMENT_ARENA}},
	{ENVIRONMENT_CARPETEDHALLWAY, {EAX_ENVIRONMENT_CARPETEDHALLWAY}},
	{ENVIRONMENT_HALLWAY,         {EAX_ENVIRONMENT_HALLWAY}},
	{ENVIRONMENT_STONECORRIDOR,   {EAX_ENVIRONMENT_STONECORRIDOR}},
	{ENVIRONMENT_ALLEY,           {EAX_ENVIRONMENT_ALLEY}},
	{ENVIRONMENT_FOREST,          {EAX_ENVIRONMENT_FOREST}},
	{ENVIRONMENT_CITY,            {EAX_ENVIRONMENT_CITY}},
	{ENVIRONMENT_MOUNTAINS,       {EAX_ENVIRONMENT_MOUNTAINS}},
	{ENVIRONMENT_QUARRY,          {EAX_ENVIRONMENT_QUARRY}},
	{ENVIRONMENT_PLAIN,           {EAX_ENVIRONMENT_PLAIN}},
	{ENVIRONMENT_PARKINGLOT,      {EAX_ENVIRONMENT_PARKINGLOT}},
	{ENVIRONMENT_SEWERPIPE,       {EAX_ENVIRONMENT_SEWERPIPE}},
	{ENVIRONMENT_UNDERWATER,      {EAX_ENVIRONMENT_UNDERWATER}},
	{ENVIRONMENT_DRUGGED,         {EAX_ENVIRONMENT_DRUGGED}},
	{ENVIRONMENT_DIZZY,           {EAX_ENVIRONMENT_DIZZY}},
	{ENVIRONMENT_PSYCHOTIC,       {EAX_ENVIRONMENT_PSYCHOTIC}}
};

void csSoundListenerEAX::SetEnvironment(SoundEnvironment env) {

  Environment = env;

	if(EaxKsPropertiesSet)
	{
		DWORD preset={EAX_ENVIRONMENT_GENERIC};
		
		for(int i=0; i<MAX_s2eaxEnv; i++)
		{
			if(s2eaxEnv[i].senv==env)
			{
				preset = s2eaxEnv[i].eaxenv;
				break;
			}
		}
		
		EaxKsPropertiesSet->Set(DSPROPSETID_EAX_ListenerProperties,
			DSPROPERTY_EAXLISTENER_ENVIRONMENT,
			NULL,
			0,
			&preset,
			sizeof(DWORD));
	}
}

csVector3 csSoundListenerEAX::GetPosition() {
  return Position;
}

void csSoundListenerEAX::GetDirection(csVector3 &f, csVector3 &t) {
  f = Front;
  t = Top;
}

float csSoundListenerEAX::GetHeadSize() {
  return HeadSize;
}

csVector3 csSoundListenerEAX::GetVelocity() {
  return Velocity;
}

float csSoundListenerEAX::GetDopplerFactor() {
  return Doppler;
}

float csSoundListenerEAX::GetDistanceFactor() {
  return DistanceFactor;
}

float csSoundListenerEAX::GetRollOffFactor() {
  return RollOff;
}

SoundEnvironment csSoundListenerEAX::GetEnvironment() {
  return Environment;
}

void csSoundListenerEAX::Prepare() {
  if (!Dirty) return;
  Listener->CommitDeferredSettings();
  Dirty = false;
}
