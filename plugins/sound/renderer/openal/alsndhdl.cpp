/*
    Copyright (C) 2002 by Jorrit Tyberghein, Daniel Duhprey

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

#include "alsndrdr.h"
#include "alsndhdl.h"
#include "alsndsrc.h"

csSoundHandleOpenAL::csSoundHandleOpenAL(csSoundRenderOpenAL *srdr, iSoundData *snd,float BufferLengthSeconds,bool LocalBuffer)
: csSoundHandle(snd)
{
  SoundRender = srdr;
  Data=snd;


  local_buffer=0;  

  NumSamples = snd->IsStatic() ? snd->GetStaticSampleCount() :
  (long int)(((float)(snd->GetFormat()->Freq)) * BufferLengthSeconds);

  buffer_length=(NumSamples* snd->GetFormat()->Bits * snd->GetFormat()->Channels)/8;
  if (LocalBuffer)
    local_buffer=malloc(buffer_length);

  buffer_writecursor=0;

  mutex_WriteCursor=csMutex::Create(true);
  ActiveStream=false;

}

csSoundHandleOpenAL::~csSoundHandleOpenAL() 
{
  if (local_buffer)
    free(local_buffer);
  local_buffer=0;
}

void csSoundHandleOpenAL::StartStream(bool Loop)
{
  if (!Data->IsStatic() && !ActiveStream)
  {
    SoundRender->mutex_ActiveSources->LockWait();
    mutex_WriteCursor->LockWait();
    LoopStream = Loop;
    ActiveStream = true;
    // Fill our local buffer if we have one
    UpdateCount(NumSamples);
    mutex_WriteCursor->Release();
    SoundRender->mutex_ActiveSources->Release();
  }
}

csPtr<iSoundSource> csSoundHandleOpenAL::CreateSource(int mode) 
{
  iSoundSource *src = new csSoundSourceOpenAL(SoundRender, this);
  src->SetMode3D (mode);
  return csPtr<iSoundSource> (src);
}


// Only overridden because it calls UpdateCount which is altered
void csSoundHandleOpenAL::Update_Time(csTicks Time)
{
  UpdateCount (Time * Data->GetFormat()->Freq / 1000);
}

void csSoundHandleOpenAL::UpdateCount(long NumSamples)
{
  csSoundSourceOpenAL *src;
  long Num;
  long bytespersample;

  if (NumSamples<=0)
    return;

  // If the stream is not active, allow the source to check for buffer end
  if (!ActiveStream)
  {
    // Check to be sure this is not a static buffer
    if (!Data->IsStatic())
    {
      // Find any sources associated with us
      for (long i=0; i<SoundRender->ActiveSources.Length(); i++)
      {
        src = (csSoundSourceOpenAL*)SoundRender->ActiveSources.Get(i);
        if (src->GetSoundHandle()==this && src->IsPlaying())
          src->WatchBufferEnd();
      }
    }
    return;
  }


  mutex_WriteCursor->LockWait();

  bytespersample=(Data->GetFormat()->Bits * Data->GetFormat()->Channels)/8;


  if (NumSamples*bytespersample>buffer_length)
    NumSamples=buffer_length/bytespersample;

  Num=NumSamples;


  // Read data until the end of the stream, or the buffer is full

  void *buf = Data->ReadStreamed(Num);
  if (Num && buf)
  {

    // Add the data that we did get to the buffer
    vUpdate(buf, Num);

    // If the local buffer is valid, copy the data to the local buffer too
    if (local_buffer)
    {
      long position1;
      long length1,length2;
      long readlength=Num*bytespersample;

      length2=0;
      position1=buffer_writecursor;
      length1=readlength;;
      if (buffer_writecursor+readlength > buffer_length)
      {
        length1=buffer_length-buffer_writecursor;
        length2=(buffer_writecursor+readlength) % buffer_length;
      }
      if (length1) memcpy((unsigned char *)local_buffer+position1,buf,length1);
      // Position 2 is always 0, even if valid since it will be at the start of the buffer
      if (length2) memcpy(local_buffer,(unsigned char *)buf+length1,length2);
    }


    // Advance the local write cursor
    buffer_writecursor=(buffer_writecursor+Num*bytespersample) % buffer_length;
  }


  // If the buffer isn't full, we must have reached the end of the stream. Reset to the beginning and continue filling.
  if (Num < NumSamples)
  {
    // If this should not loop, we are done.
    if (!LoopStream) 
    {
      if (!Data->IsStatic())
      {
        ActiveStream=false; // Stream is done playing
        // Notify all sources that this stream has ended
        for (long i=0; i<SoundRender->ActiveSources.Length(); i++)
        {
          src = (csSoundSourceOpenAL*)SoundRender->ActiveSources.Get(i);
          if (src->GetSoundHandle()==this && src->IsPlaying())
            src->NotifyStreamEnd(); 
        }
      }
    }
    else
      Data->ResetStreamed(); // Reset the data stream to the begining and pull more data out for looping
  }

  mutex_WriteCursor->Release();

}


long csSoundHandleOpenAL::GetPlayCursorPosition()
{
  return buffer_writecursor;
}

void csSoundHandleOpenAL::vUpdate(void *buf, long Num)
{
  long NumBytes = Num * Data->GetFormat()->Bits/8 * Data->GetFormat()->Channels;
  for (long i=0; i<SoundRender->ActiveSources.Length(); i++)
  {
    csSoundSourceOpenAL *src = (csSoundSourceOpenAL*)SoundRender->ActiveSources.Get(i);
    if (src->GetSoundHandle()==this && src->IsPlaying())
      src->Write(buf, NumBytes);
  }
}

