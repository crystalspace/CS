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

#include <dsound.h>

#include "csgeom/vector3.h"
#include "csplugincommon/directx/guids.h"
#include "csutil/scf.h"
#include "csutil/sysfunc.h"

#include "iutil/objreg.h"
#include "ivaria/reporter.h"

#include "sndrdr.h"
#include "sndsrc.h"
#include "sndhdl.h"

SCF_IMPLEMENT_FACTORY(csSoundSourceDS3D)

SCF_IMPLEMENT_IBASE(csSoundSourceDS3D)
  SCF_IMPLEMENTS_INTERFACE(iSoundSource)
SCF_IMPLEMENT_IBASE_END;


csSoundSourceDS3D::csSoundSourceDS3D(iBase *piBase) 
{
  SCF_CONSTRUCT_IBASE(piBase);

  Buffer3D = 0;
  Buffer2D = 0;
  Renderer = 0;
  SoundHandle = 0;
  WriteCursor = -1;
  MinimumDistance=1.0f;
  MaximumDistance=DS3D_DEFAULTMAXDISTANCE;
}

csSoundSourceDS3D::~csSoundSourceDS3D() 
{
  if (Renderer && Renderer->AudioRenderer && Buffer3D) Buffer3D->Release();
  if (Renderer && Renderer->AudioRenderer && Buffer2D) {
    Buffer2D->Stop();
    Buffer2D->Release();
  }
  Renderer = 0;
  SoundHandle = 0;
  SCF_DESTRUCT_IBASE();
}

void csSoundSourceDS3D::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep = CS_QUERY_REGISTRY (Renderer->object_reg,
    iReporter);
  if (rep)
    rep->ReportV (severity, "crystalspace.sound.ds3d", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

bool csSoundSourceDS3D::Initialize(csSoundRenderDS3D* srdr, 
				   csSoundHandleDS3D* shdl, 
				   int mode3d, long NumSamples) 
{
  HRESULT r;

  Renderer = srdr;
  SoundHandle = shdl;

  Static = SoundHandle->Data->IsStatic();

  SampleBytes = SoundHandle->Data->GetFormat()->Channels *
    SoundHandle->Data->GetFormat()->Bits/8;
  BufferBytes = NumSamples * SampleBytes;

  DSBUFFERDESC dsbd;
  WAVEFORMATEX wfxFormat;

  dsbd.dwSize = sizeof(DSBUFFERDESC);
  dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN
    | DSBCAPS_CTRL3D | (Static ? DSBCAPS_STATIC : 0) | 
    (Renderer->MuteInBackground ? 0 :DSBCAPS_GLOBALFOCUS  ) ;
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

  /* In some cases during shutdown, sound source objects can exist after the dsound interface
  *  has been released.
  */
  if (!Renderer->AudioRenderer)
    return false;

  r = Renderer->AudioRenderer->CreateSoundBuffer(&dsbd, &Buffer2D, 0);
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
    // If this source is playing static data, fill the buffer now.
    void *buf = SoundHandle->Data->GetStaticData();
    Write(buf, BufferBytes);
  } else {
    /* If this source is playing streaming data, clear the buffer.  
    *  Syncronization will occur on the Play() call.
    */
    ClearBuffer();
  }

  return true;
}

void csSoundSourceDS3D::SetPosition(csVector3 v)
{
  Renderer->SetDirty();
  Position = v;
  if (Renderer->AudioRenderer &&  Buffer3D) Buffer3D->SetPosition(v.x, v.y, v.z, DS3D_DEFERRED);
}

void csSoundSourceDS3D::SetVelocity(csVector3 v)
{
  Renderer->SetDirty();
  Velocity = v;
  if (Renderer->AudioRenderer && Buffer3D) Buffer3D->SetVelocity(v.x, v.y, v.z, DS3D_DEFERRED);
}

csVector3 csSoundSourceDS3D::GetPosition() 
{
  return Position;
}

csVector3 csSoundSourceDS3D::GetVelocity() 
{
  return Velocity;
}

void csSoundSourceDS3D::SetVolume(float vol)
{
  long dsvol = (long)(DSBVOLUME_MIN + (DSBVOLUME_MAX-DSBVOLUME_MIN)*vol);
  if (Renderer->AudioRenderer && Buffer2D) Buffer2D->SetVolume(dsvol);
}

float csSoundSourceDS3D::GetVolume()
{
  long dsvol=DSBVOLUME_MIN;
  if (Renderer->AudioRenderer && Buffer2D) Buffer2D->GetVolume(&dsvol);
  return (float)(dsvol-DSBVOLUME_MIN)/(float)(DSBVOLUME_MAX-DSBVOLUME_MIN);
}

void csSoundSourceDS3D::SetMinimumDistance (float distance)
{
  Renderer->SetDirty();
  if (distance < 0.0f)
    distance = 0.0f;
  if (Renderer->AudioRenderer && Buffer3D) Buffer3D->SetMinDistance(distance, DS3D_DEFERRED);
}

void csSoundSourceDS3D::SetMaximumDistance (float distance)
{
  Renderer->SetDirty();
  if (distance == SOUND_DISTANCE_INFINITE)
    distance = DS3D_DEFAULTMAXDISTANCE;
  else
  {
    if (distance < 0.000001f) distance = 0.000001f;
    if (distance < MinimumDistance) distance = MinimumDistance;
  }

  MaximumDistance=distance;
  if (Renderer->AudioRenderer && Buffer3D) Buffer3D->SetMaxDistance(distance, DS3D_DEFERRED);
}

float csSoundSourceDS3D::GetMinimumDistance ()
{
  return MinimumDistance;
}

float csSoundSourceDS3D::GetMaximumDistance ()
{
  return MaximumDistance;
}

void csSoundSourceDS3D::SetMode3D(int mode3D) 
{
  DWORD Mode = (mode3D == SOUND3D_ABSOLUTE) ? DS3DMODE_NORMAL :
(mode3D == SOUND3D_RELATIVE) ? DS3DMODE_HEADRELATIVE :  DS3DMODE_DISABLE;

if (Renderer->AudioRenderer && Buffer3D) 
{
  HRESULT r = Buffer3D->SetMode(Mode, DS3D_DEFERRED);
  if (r != DS_OK) {
    Report (CS_REPORTER_SEVERITY_WARNING,
      "cannot set secondary sound buffer 3d mode "
      "for sound source (%s).", Renderer->GetError(r));
  } else Renderer->SetDirty();
}
}

int csSoundSourceDS3D::GetMode3D() 
{
  DWORD Mode;

  if (!Renderer->AudioRenderer || !Buffer3D)
    return SOUND3D_DISABLE;

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
  if (Renderer->AudioRenderer && Buffer2D)
  {
    /* If we're playing static data, we may be looping the sound over and over
    *  in which case the buffer will loop.
    * If we're playing streaming data, the buffer is set to always loop since
    *  streams normally have a longer playtime than a single buffer fill.
    */
    Looped = Static ? (PlayMethod & SOUND_LOOP) : true;

    // Make sure the buffer is in a stop state
    Buffer2D->Stop();

    /* Set the buffer position to the beginning if this is a streaming data source
    *  or if this is a static data source and SOUND_RESTART is specified.
    */
    if (Static && (PlayMethod & SOUND_RESTART))
      Buffer2D->SetCurrentPosition(0);

    /*  If this is a static data source, the buffer is already filled from Initialize().
    *   In the case of a streaming source we need to be sure that we are synced up with
    *   the rest of the sources playing from this handle.
    */
    Renderer->mutex_ActiveSources->LockWait();
    SoundHandle->mutex_WriteCursor->LockWait();
    if (!Static)
    {
      if (SoundHandle->ActiveStream && SoundHandle->buffer)
      {
        // Fill our sound buffer with the data from the stream
        WriteCursor=0;
        Write(SoundHandle->buffer,SoundHandle->buffer_length);
        WriteCursor=SoundHandle->buffer_writecursor;

        Buffer2D->SetCurrentPosition(SoundHandle->GetPlayCursorPosition());
      }
      else
      {
        ClearBuffer();
        Buffer2D->SetCurrentPosition(SoundHandle->GetPlayCursorPosition());
      }
    }

    // Add this source to the renderer's source list
    Renderer->AddSource(this);

    Buffer2D->Play(0, 0, Looped ? DSBPLAY_LOOPING : 0);
    SoundHandle->mutex_WriteCursor->Release();
    Renderer->mutex_ActiveSources->Release();
  }
}

void csSoundSourceDS3D::Stop()
{
  if (Renderer->AudioRenderer && Buffer2D) Buffer2D->Stop();
  Renderer->RemoveSource(this);
}

void csSoundSourceDS3D::SetFrequencyFactor(float factor) 
{
  if (Renderer->AudioRenderer && Buffer2D)
    Buffer2D->SetFrequency((long)(BaseFrequency * factor));
}

float csSoundSourceDS3D::GetFrequencyFactor() {
  DWORD frq;
  if (!Renderer->AudioRenderer || !Buffer2D) 
    return 1.0f; // There's no right way to handle this

  Buffer2D->GetFrequency(&frq);
  return (frq/BaseFrequency);
}

bool csSoundSourceDS3D::IsPlaying() 
{
  DWORD r;
  if (!Renderer->AudioRenderer || !Buffer2D) 
    return false; // No dsound object or no buffer means nothing can be playing
  Buffer2D->GetStatus(&r);
  return (r & DSBSTATUS_PLAYING);
}

void csSoundSourceDS3D::Write(void *Data, unsigned long NumBytes) 
{
  void *Pointer1 = 0, *Pointer2 = 0;
  DWORD Length1, Length2;

  if (!Renderer->AudioRenderer || !Buffer2D) 
    return; // Don't crash


  bool ResetPlayPosition = false;
  if (WriteCursor == -1) {
    ResetPlayPosition = true;
    WriteCursor = 0;
  }

  if (Buffer2D->Lock(WriteCursor, NumBytes, &Pointer1, &Length1,
    &Pointer2, &Length2, 0 ) != DS_OK) return;

  if (Pointer1) CopyMemory(Pointer1, Data, Length1);
  if (Pointer2) CopyMemory(Pointer2, (unsigned char *)Data+Length1, Length2);

  Buffer2D->Unlock(Pointer1, Length1, Pointer2, Length2);
  WriteCursor = (WriteCursor + Length1 + Length2) % BufferBytes;
  if (ResetPlayPosition) Buffer2D->SetCurrentPosition(0);
}

void csSoundSourceDS3D::WriteMute(unsigned long NumBytes) 
{
  void *Pointer1 = 0, *Pointer2 = 0;
  DWORD Length1, Length2;

  if (!Renderer->AudioRenderer || !Buffer2D) 
    return; // Don't crash

  bool ResetPlayPosition = false;
  if (WriteCursor == -1) {
    ResetPlayPosition = true;
    WriteCursor = 0;
    /* If we're resetting the play cursor, stop the buffer during the write
     *  to avoid invalid data being played briefly before the position is
     *  reset.
     */
    Buffer2D->Stop();
  }

  if (Buffer2D->Lock(WriteCursor, NumBytes, &Pointer1, &Length1,
    &Pointer2, &Length2, 0) != DS_OK) return;

  unsigned char Byte = (SoundHandle->Data->GetFormat()->Bits==8)?128:0;

  if (Pointer1) FillMemory(Pointer1, Length1, Byte);
  if (Pointer2) FillMemory(Pointer2, Length2, Byte);

  Buffer2D->Unlock(Pointer1, Length1, Pointer2, Length2);
  WriteCursor = (WriteCursor + Length1 + Length2) % BufferBytes;
  if (ResetPlayPosition) 
  {
    Buffer2D->SetCurrentPosition(0);
    Buffer2D->Play(0, 0, Looped ? DSBPLAY_LOOPING : 0);
  }
}

void csSoundSourceDS3D::FillBufferWithSilence()
{
  void *Pointer1 = 0, *Pointer2 = 0;
  DWORD Length1, Length2;

  if (!Renderer->AudioRenderer || !Buffer2D) 
    return; // Don't crash

  if (Buffer2D->Lock(0, 0, &Pointer1, &Length1,
    &Pointer2, &Length2, DSBLOCK_ENTIREBUFFER) != DS_OK) return;

  unsigned char Byte = (SoundHandle->Data->GetFormat()->Bits==8)?128:0;
  if (Pointer1) FillMemory(Pointer1, Length1, Byte);
  if (Pointer2) FillMemory(Pointer2, Length2, Byte);

  Buffer2D->Unlock(Pointer1, Length1, Pointer2, Length2);
}

void csSoundSourceDS3D::ClearBuffer()
{
  if (!Renderer->AudioRenderer || !Buffer2D) 
    return; // Don't crash

  FillBufferWithSilence();

  Buffer2D->SetCurrentPosition(0);
  WriteCursor = -1;
}

void csSoundSourceDS3D::NotifyStreamEnd()
{
  /*  The sound handle is telling us that it handles a stream which is now done.
  *   We are given a chance to setup variables necessary for watching for the end of our playback buffer.
  */

  PlayEnd=WriteCursor;
}

// Here we check for the WriteCursor to overrun the PlayEnd position, signifying playback is complete.
void csSoundSourceDS3D::WatchBufferEnd()
{
  int32 bytes;
  void *Pointer1 = 0, *Pointer2 = 0;
  DWORD Length1, Length2;
  long old_cursor=WriteCursor;

  if (!Renderer->AudioRenderer || !Buffer2D) 
    return; // Don't crash

  // See if the write cursor can move (which probably means the read cursor has advanced)
  bytes=GetFreeBufferSpace();


  if (bytes)
  {
    /*  Since it's not practical to sit and wait for the play position to reach the end of the data,
    *   we begin filling the space past the end of the actual audio with silence (zeros).  At some point
    *   playback will move past the end of the legitimate audio, and we will be able to stop playback.
    *   However, it's unlikely we will catch this at the exact moment it hits the end of the real data.
    *   By adding silence to the end it doesn't matter if we are a bit late.
    */

    // Clear out the unused portion of the buffer
    if (Buffer2D->Lock(WriteCursor, bytes, &Pointer1, &Length1,
      &Pointer2, &Length2, 0) != DS_OK) return;

    unsigned char Byte = (SoundHandle->Data->GetFormat()->Bits==8)?128:0;
    if (Pointer1) FillMemory(Pointer1, Length1, Byte);
    if (Pointer2) FillMemory(Pointer2, Length2, Byte);



    Buffer2D->Unlock(Pointer1, Length1, Pointer2, Length2);
    WriteCursor = (WriteCursor + Length1 + Length2) % BufferBytes;

    /*  Check and see if the write cursor has overrun the position of the last byte of real audio.   
    *    If it has, we know the read cursor must also have passed this point and it's now safe to stop playback.
    *
    * Possibilities:
    *  Full buffer write (WriteCursor==old_cursor)  
    *     Note that we only reach here if bytes!=0 so we rule out the possibility of no movement and 
    *     leave only the conclusion that we just wrote data to the entire buffer, which means it must all be 
    *     empty now.
    *  Partial write without loop ( if (old_cursor<=PlayEnd && WriteCursor>PlayEnd && WriteCursor>old_cursor) )
    *  Partial write with loop (if (WriteCursor<old_cursor && (old_cursor<=PlayEnd || WriteCursor>PlayEnd)) )
    */
    if ((WriteCursor==old_cursor) ||
      (old_cursor<=PlayEnd && WriteCursor>=PlayEnd && WriteCursor>old_cursor) ||
      (WriteCursor<old_cursor && (old_cursor<=PlayEnd || WriteCursor>=PlayEnd)) )
    {
      // Done.
      Stop();
    }
  }

}

int32 csSoundSourceDS3D::GetFreeBufferSpace()
{
  DWORD playcursor;
  int32 freespace;

  if (!Renderer->AudioRenderer || !Buffer2D) 
    return 0; // Don't crash

  /*  We compare the Direct Sound playback cursor with our Write cursor to determine how much more
  *   of the buffer we can now fill.  Note that the Direct Sound internal write cursor has nothing
  *   to do with what has already been written, but only indicates the earliest position past the 
  *   playback cursor that is safe to write to (i.e. data before this point may have already been sent
  *   to the card).
  *   This is why we don't use the write cursor from Direct Sound, it's useless for what we do.
  */

  if (DS_OK!=Buffer2D->GetCurrentPosition(&playcursor,0))
    return 0;
  if (WriteCursor==-1)
  {
    // Fill entire buffer
    return BufferBytes;
  }
  else
  {
    freespace=playcursor-WriteCursor;
    if (freespace<0)
      freespace=freespace+BufferBytes;
  }

  return freespace;
}

csSoundHandleDS3D *csSoundSourceDS3D::GetSoundHandle()
{
  return SoundHandle;
}
