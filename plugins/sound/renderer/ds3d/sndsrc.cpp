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

#include "dsound.h"

#include "sndrdr.h"
#include "sndsrc.h"
#include "isystem.h"

IMPLEMENT_FACTORY(csSoundSourceDS3D)

IMPLEMENT_IBASE(csSoundSourceDS3D)
  IMPLEMENTS_INTERFACE(iSoundSource)
IMPLEMENT_IBASE_END;

csSoundSourceDS3D::csSoundSourceDS3D(iBase *piBase) {
  CONSTRUCT_IBASE(piBase);

  Buffer3D = NULL;
  Buffer2D = NULL;
  Renderer = NULL;
}

csSoundSourceDS3D::~csSoundSourceDS3D() {
  if (Buffer3D) Buffer3D->Release();
  if (Buffer2D) {
    Buffer2D->Stop();
    Buffer2D->Release();
  }
  if (Renderer) Renderer->DecRef();
}

bool csSoundSourceDS3D::Initialize(csSoundRenderDS3D *srdr,
        iSoundStream *Data, bool is3d) {
  srdr->IncRef();
  Renderer = srdr;

  // if number of samples is unknown this should be a streamed sound
  if (Data->GetNumSamples()==-1) return false;

  unsigned long BufferBytes = Data->GetNumSamples() *
    Data->GetFormat()->Channels * Data->GetFormat()->Bits/8;

  DSBUFFERDESC dsbd;
  ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
  dsbd.dwSize = sizeof(DSBUFFERDESC);
  dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN  | DSBCAPS_CTRL3D;
	
  WAVEFORMATEX wfxFormat;
  wfxFormat.wFormatTag = WAVE_FORMAT_PCM;
  wfxFormat.nChannels = Data->GetFormat()->Channels;
  wfxFormat.nSamplesPerSec = Data->GetFormat()->Freq;
  wfxFormat.wBitsPerSample = Data->GetFormat()->Bits;
  wfxFormat.nBlockAlign = (wfxFormat.wBitsPerSample*wfxFormat.nChannels)/8;
  wfxFormat.nAvgBytesPerSec = wfxFormat.nBlockAlign*wfxFormat.nSamplesPerSec;
  wfxFormat.cbSize = 0;
  dsbd.lpwfxFormat = &wfxFormat;
  dsbd.dwBufferBytes = BufferBytes;
	
  if (Renderer->AudioRenderer->CreateSoundBuffer(&dsbd, &Buffer2D, NULL) != DS_OK)
    return false;
	
  void *pbWrite1 = NULL, *pbWrite2 = NULL;
  DWORD cbLen1, cbLen2;

  if (Buffer2D->Lock(0, BufferBytes, &pbWrite1, &cbLen1,
        &pbWrite2, &cbLen2, 0L) != DS_OK) {
    if (pbWrite1) Buffer2D->Unlock(pbWrite1, BufferBytes, pbWrite2, 0);
    return false;
  }

  // this can possibly be optimized without CopyMemory()
  long num = Data->GetNumSamples();
  void *d = Data->Read(num);
  CopyMemory(pbWrite1, d, BufferBytes);
  Data->DiscardBuffer(d);
    
  if (Buffer2D->Unlock(pbWrite1, BufferBytes, pbWrite2, 0) != DS_OK) {
    if (pbWrite1) Buffer2D->Unlock(pbWrite1, BufferBytes, pbWrite2, 0);
    return false;
  }

  if (is3d) {
    if (Buffer2D->QueryInterface(IID_IDirectSound3DBuffer,
      (void **) &Buffer3D) < DS_OK) return false;

    DWORD dwMode = DS3DMODE_NORMAL;
    if (Buffer3D->SetMode(dwMode, DS3D_IMMEDIATE) < DS_OK) return false;
  }

  BaseFrequency = Data->GetFormat()->Freq;
  SetPosition(csVector3(0,0,0));
  SetVelocity(csVector3(0,0,0));

  return true;
}

void csSoundSourceDS3D::SetPosition(csVector3 v)
{
  Renderer->SetDirty();
  Position = v;
  if (Buffer3D) Buffer3D->SetPosition(v.x, v.y, v.z, DS3D_DEFERRED);
}

void csSoundSourceDS3D::SetVelocity(csVector3 v)
{
  Renderer->SetDirty();
  Velocity = v;
  if (Buffer3D) Buffer3D->SetVelocity(v.x, v.y, v.z, DS3D_DEFERRED);
}

csVector3 csSoundSourceDS3D::GetPosition() {
  return Position;
}

csVector3 csSoundSourceDS3D::GetVelocity() {
  return Velocity;
}

void csSoundSourceDS3D::SetVolume(float vol)
{
  long dsvol = DSBVOLUME_MIN + (DSBVOLUME_MAX-DSBVOLUME_MIN)*vol;
  Buffer2D->SetVolume(dsvol);
}

float csSoundSourceDS3D::GetVolume()
{  
  long dsvol=DSBVOLUME_MIN;
  Buffer2D->GetVolume(&dsvol);
  return (float)(dsvol-DSBVOLUME_MIN)/(float)(DSBVOLUME_MAX-DSBVOLUME_MIN);
}

void csSoundSourceDS3D::Play(unsigned long PlayMethod)
{
  Buffer2D->Stop();
  if (PlayMethod & SOUND_RESTART) Buffer2D->SetCurrentPosition(0);
  Buffer2D->Play(0, 0, (PlayMethod & SOUND_LOOP) ? DSBPLAY_LOOPING : 0);
  IncRef(); // @@@
}

void csSoundSourceDS3D::Stop()
{
  Buffer2D->Stop();
}

bool csSoundSourceDS3D::Is3d() {
  return (Buffer3D != NULL);
}

void csSoundSourceDS3D::SetFrequencyFactor(float factor) {
  Buffer2D->SetFrequency(BaseFrequency * factor);
}

float csSoundSourceDS3D::GetFrequencyFactor() {
  DWORD frq;
  Buffer2D->GetFrequency(&frq);
  return (frq/BaseFrequency);
}
