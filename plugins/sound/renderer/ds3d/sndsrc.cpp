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
#include "cssys/sysfunc.h"
#include "csutil/scf.h"

#include "dsound.h"

#include "sndrdr.h"
#include "sndsrc.h"
#include "sndhdl.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

SCF_IMPLEMENT_FACTORY(csSoundSourceDS3D)

SCF_IMPLEMENT_IBASE(csSoundSourceDS3D)
  SCF_IMPLEMENTS_INTERFACE(iSoundSource)
SCF_IMPLEMENT_IBASE_END;

csSoundSourceDS3D::csSoundSourceDS3D(iBase *piBase) {
  SCF_CONSTRUCT_IBASE(piBase);

  Buffer3D = NULL;
  Buffer2D = NULL;
  Renderer = NULL;
  SoundHandle = NULL;
  WriteCursor = -1;
}

csSoundSourceDS3D::~csSoundSourceDS3D() {
  if (Buffer3D) Buffer3D->Release();
  if (Buffer2D) {
    Buffer2D->Stop();
    Buffer2D->Release();
  }
  if (Renderer) Renderer->DecRef();
  if (SoundHandle) SoundHandle->DecRef();
}

void csSoundSourceDS3D::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (Renderer->object_reg,
  	iReporter);
  if (rep)
  {
    rep->ReportV (severity, "crystalspace.sound.ds3d", msg, arg);
    rep->DecRef ();
  }
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

bool csSoundSourceDS3D::Initialize(csSoundRenderDS3D *srdr,
        csSoundHandleDS3D *shdl, int mode3d, long NumSamples) {
  HRESULT r;

  srdr->IncRef();
  Renderer = srdr;
  shdl->IncRef();
  SoundHandle = shdl;

  Static = SoundHandle->Data->IsStatic();

  SampleBytes = SoundHandle->Data->GetFormat()->Channels *
                SoundHandle->Data->GetFormat()->Bits/8;
  BufferBytes = NumSamples * SampleBytes;

  DSBUFFERDESC dsbd;
  WAVEFORMATEX wfxFormat;

  dsbd.dwSize = sizeof(DSBUFFERDESC);
  dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN
    | DSBCAPS_CTRL3D | (Static ? DSBCAPS_STATIC : 0);
  dsbd.dwBufferBytes = BufferBytes;
  dsbd.dwReserved = 0;
  dsbd.lpwfxFormat = &wfxFormat;

  wfxFormat.wFormatTag = WAVE_FORMAT_PCM;
  wfxFormat.nChannels = SoundHandle->Data->GetFormat()->Channels;
  wfxFormat.nSamplesPerSec = SoundHandle->Data->GetFormat()->Freq;
  wfxFormat.wBitsPerSample = SoundHandle->Data->GetFormat()->Bits;
  wfxFormat.nBlockAlign = (wfxFormat.wBitsPerSample*wfxFormat.nChannels)/8;
  wfxFormat.nAvgBytesPerSec = wfxFormat.nBlockAlign*wfxFormat.nSamplesPerSec;
  wfxFormat.cbSize = 0;

  r = Renderer->AudioRenderer->CreateSoundBuffer(&dsbd, &Buffer2D, NULL);
  if (r != DS_OK) {
    Report (CS_REPORTER_SEVERITY_WARNING,
      "cannot create secondary sound buffer "
      "for sound source (%s).", srdr->GetError(r));
    return false;
  }

  r = Buffer2D->QueryInterface(IID_IDirectSound3DBuffer, (void **) &Buffer3D);
  if (r != DS_OK) {
    Report (CS_REPORTER_SEVERITY_WARNING,
    	"cannot query 3D buffer interface "
        "for sound source (%s).", srdr->GetError(r));
    return false;
  }

  SetMode3D(mode3d);
  BaseFrequency = SoundHandle->Data->GetFormat()->Freq;
  SetPosition(csVector3(0,0,0));
  SetVelocity(csVector3(0,0,0));
  Looped = false;

  if (Static) {
    void *buf = SoundHandle->Data->GetStaticData();
    Write(buf, BufferBytes);
  } else {
    ClearBuffer();
  }

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

void csSoundSourceDS3D::SetMode3D(int mode3D) {
  DWORD Mode = (mode3D == SOUND3D_ABSOLUTE) ? DS3DMODE_NORMAL :
    (mode3D == SOUND3D_RELATIVE) ? DS3DMODE_HEADRELATIVE :  DS3DMODE_DISABLE;
  
  HRESULT r = Buffer3D->SetMode(Mode, DS3D_DEFERRED);
  if (r != DS_OK) {
    Report (CS_REPORTER_SEVERITY_WARNING,
    	"cannot set secondary sound buffer 3d mode "
        "for sound source (%s).", Renderer->GetError(r));
  } else Renderer->SetDirty();
}

int csSoundSourceDS3D::GetMode3D() {
  DWORD Mode;
  HRESULT r = Buffer3D->GetMode(&Mode);
  if (r != DS_OK) {
    Report (CS_REPORTER_SEVERITY_ERROR,
    	"cannot get secondary sound buffer 3d mode "
        "for sound source (%s).", Renderer->GetError(r));
    return false;
  }
  if (Mode == DS3DMODE_NORMAL) return SOUND3D_ABSOLUTE;
  else if (Mode == DS3DMODE_HEADRELATIVE) return SOUND3D_RELATIVE;
  else return SOUND3D_DISABLE;
}

void csSoundSourceDS3D::Play(unsigned long PlayMethod)
{
  Looped = Static ? (PlayMethod & SOUND_LOOP) : true;
  Buffer2D->Stop();
  Renderer->AddSource(this);

  if (Static && (PlayMethod & SOUND_RESTART))
    Buffer2D->SetCurrentPosition(0);
  Buffer2D->Play(0, 0, Looped ? DSBPLAY_LOOPING : 0);
}

void csSoundSourceDS3D::Stop()
{
  Buffer2D->Stop();
  Renderer->RemoveSource(this);
  if (!Static) ClearBuffer();
}

void csSoundSourceDS3D::SetFrequencyFactor(float factor) {
  Buffer2D->SetFrequency(BaseFrequency * factor);
}

float csSoundSourceDS3D::GetFrequencyFactor() {
  DWORD frq;
  Buffer2D->GetFrequency(&frq);
  return (frq/BaseFrequency);
}

bool csSoundSourceDS3D::IsPlaying() {
  DWORD r;
  Buffer2D->GetStatus(&r);
  return (r & DSBSTATUS_PLAYING);
}

void csSoundSourceDS3D::Write(void *Data, unsigned long NumBytes) {
  void *Pointer1 = NULL, *Pointer2 = NULL;
  DWORD Length1, Length2;

  bool ResetPlayPosition = false;
  if (WriteCursor == -1) {
    ResetPlayPosition = true;
    WriteCursor = 0;
  }

  if (Buffer2D->Lock(WriteCursor, NumBytes, &Pointer1, &Length1,
	&Pointer2, &Length2, 0) != DS_OK) return;

  if (Pointer1) CopyMemory(Pointer1, Data, Length1);
  if (Pointer2) CopyMemory(Pointer2, (unsigned char *)Data+Length1, Length2);

  Buffer2D->Unlock(Pointer1, Length1, Pointer2, Length2);
  WriteCursor = (WriteCursor + NumBytes) % BufferBytes;
  if (ResetPlayPosition) Buffer2D->SetCurrentPosition(0);
}

void csSoundSourceDS3D::WriteMute(unsigned long NumBytes) {
  void *Pointer1 = NULL, *Pointer2 = NULL;
  DWORD Length1, Length2;

  bool ResetPlayPosition = false;
  if (WriteCursor == -1) {
    ResetPlayPosition = true;
    WriteCursor = 0;
  }

  if (Buffer2D->Lock(WriteCursor, NumBytes, &Pointer1, &Length1,
	&Pointer2, &Length2, 0) != DS_OK) return;

  unsigned char Byte = (SoundHandle->Data->GetFormat()->Bits==8)?128:0;

  if (Pointer1) FillMemory(Pointer1, Byte, Length1);
  if (Pointer2) FillMemory(Pointer2, Byte, Length2);

  Buffer2D->Unlock(Pointer1, Length1, Pointer2, Length2);
  WriteCursor = (WriteCursor + NumBytes) % BufferBytes;
  if (ResetPlayPosition) Buffer2D->SetCurrentPosition(0);
}

void csSoundSourceDS3D::ClearBuffer()
{
  WriteCursor = -1;
}

csSoundHandleDS3D *csSoundSourceDS3D::GetSoundHandle()
{
  return SoundHandle;
}
