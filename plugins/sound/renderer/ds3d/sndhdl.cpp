/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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
#include "csplugincommon/directx/guids.h"
#include "sndhdl.h"
#include "sndrdr.h"
#include "sndsrc.h"


csSoundHandleDS3D::csSoundHandleDS3D(csSoundRenderDS3D* srdr, iSoundData* snd, 
				     float BufferLengthSeconds, 
				     bool LocalBuffer) : csSoundHandle(snd)
{
  SoundRender = srdr;
  Registered = true;
  buffer=0;  

  NumSamples = Data->IsStatic() ? Data->GetStaticSampleCount() :
  (long)(Data->GetFormat()->Freq * BufferLengthSeconds);

  buffer_length=(NumSamples* Data->GetFormat()->Bits * Data->GetFormat()->Channels)/8;
  if (LocalBuffer)
    buffer=malloc(buffer_length);

  buffer_writecursor=0;

  // We need recursive locking capabilities
  mutex_WriteCursor=csMutex::Create(true);

}

csSoundHandleDS3D::~csSoundHandleDS3D() 
{
  SoundRender = 0;
  if (buffer)
    free(buffer);
  buffer=0;
}

void csSoundHandleDS3D::Unregister() 
{
  Registered = false;
  ReleaseSoundData();
}

csPtr<iSoundSource> csSoundHandleDS3D::CreateSource(int Mode3d) 
{
  if (!Registered) return 0;
  csSoundSourceDS3D *src = new csSoundSourceDS3D(0);


  if (src->Initialize(SoundRender, this, Mode3d, NumSamples))
    return csPtr<iSoundSource> (src);
  else
  {
    src->DecRef();
    return 0;
  }


}

void csSoundHandleDS3D::StartStream(bool Loop)
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

// Only overridden because it calls UpdateCount which is altered
void csSoundHandleDS3D::Update_Time(csTicks Time)
{
  UpdateCount (Time * Data->GetFormat()->Freq / 1000);
}

void csSoundHandleDS3D::UpdateCount(long NumSamples)
{
  int32 freespace;
  csSoundSourceDS3D *src;
  long Num;
  long bytespersample;
  bool noneplaying=true;


  // If the stream is not active, allow the source to check for buffer end
  if (!ActiveStream)
  {
    // Check to be sure this is not a static buffer
    if (!Data->IsStatic())
    {
      // Find any sources associated with us
      for (size_t i=0; i<SoundRender->ActiveSources.Length(); i++)
      {
        src = (csSoundSourceDS3D*)SoundRender->ActiveSources.Get(i);
        if (src->GetSoundHandle()==this && src->IsPlaying())
          src->WatchBufferEnd();
      }
    }
    return;
  }


  /* Calculate the smallest amount of free space in the Direct Sound buffers that is no longer valid (has been played but not yet written with new data)
  *   If the sources aren't well syncronized, this will reduce the amount the advance buffer is used.
  *
  */
  mutex_WriteCursor->LockWait();
  freespace=0;
  for (size_t i=0; i<SoundRender->ActiveSources.Length(); i++)
  {
    src = (csSoundSourceDS3D*)SoundRender->ActiveSources.Get(i);
    if (src->GetSoundHandle()==this && src->IsPlaying())
    {
      noneplaying=false;
      int32 srcfree=src->GetFreeBufferSpace();
      if (srcfree && (freespace==0 || srcfree<freespace))
        freespace=srcfree;
    }
  }


  bytespersample=(Data->GetFormat()->Bits * Data->GetFormat()->Channels)/8;



  // Translate free space into samples for calling an iSoundData interface
  Num=freespace/bytespersample;

  // If no sources are playing, use the passed reference sample count to advance time
  if (noneplaying)
    Num=NumSamples;


  /* If the amount to be read is greater than the total buffer length, we have underbuffered.
  *  All we can do is skip.  This should only be possible if we are using our csGetTicks() calculated
  *  time delta since the directsound cursor method is circular.
  */
  if (Num*bytespersample>buffer_length)
    Num=buffer_length/bytespersample;


  // Read until we've read a full buffer's worth
  while (Num > 0)
  {
    long n =1;

    // Read data until the end of the stream, or the buffer is full
    while (n>0) 
    {
      n=Num;
      /* Some SoundData formats allocate the internal buffer for decoded data in a 
      *  brain-dead manner.  They simply allocate as much space as we pass even
      *  if the data is much smaller.  In the case of Ogg, the codec simply
      *  won't provide more than 1 Ogg frame per read call, so with a 500k
      *  maximum buffer, Ogg ends up allocating 500k and never putting more than 
      *  512 bytes into it.
      * We provide a memory-saving sane maximum value to process here.
      */
      if (n>32768)
        n=32768;

      void *buf = Data->ReadStreamed(n);
      // Add the data that we did get to the buffer
      vUpdate(buf, n);
      // If the local buffer is valid, copy the data to the local buffer too
      if (buffer)
      {
        long position1;
        long length1,length2;
        long readlength=n*bytespersample;

        length2=0;
        position1=buffer_writecursor;
        length1=readlength;;
        if (buffer_writecursor+readlength > buffer_length)
        {
          length1=buffer_length-buffer_writecursor;
          length2=(buffer_writecursor+readlength) % buffer_length;
        }
        if (length1) CopyMemory((unsigned char *)buffer+position1,buf,length1);
        // Position 2 is always 0, even if valid since it will be at the start of the buffer
        if (length2) CopyMemory(buffer,(unsigned char *)buf+length1,length2);
      }


      // Advance the local write cursor
      buffer_writecursor=(buffer_writecursor+n*bytespersample) % buffer_length;

      Num -= n;
    }
    // If the buffer isn't full, we must have reached the end of the stream. Reset to the beginning and continue filling.
    if (Num > 0)
    {
      // If this should not loop, we are done.
      if (!LoopStream) 
      {
        if (!Data->IsStatic())
        {
          ActiveStream=false; // Stream is done playing
          // Notify all sources that this stream has ended
          for (size_t i=0; i<SoundRender->ActiveSources.Length(); i++)
          {
            src = (csSoundSourceDS3D*)SoundRender->ActiveSources.Get(i);
            if (src->GetSoundHandle()==this && src->IsPlaying())
              src->NotifyStreamEnd(); 
          }
        }
        break;
      }
      Data->ResetStreamed(); // Reset the data stream to the begining and pull more data out for looping
    }
  }

  mutex_WriteCursor->Release();

}


long csSoundHandleDS3D::GetPlayCursorPosition()
{
  /*
  * (WriteCursor+BytesFree)%BufferSize = PlayCursor
  *
  */
  long bytesfree;
  csSoundSourceDS3D *src;

  if (ActiveStream)
  {
    // Find a source playing our stream
    for (size_t i=0; i<SoundRender->ActiveSources.Length(); i++)
    {
      src = (csSoundSourceDS3D*)SoundRender->ActiveSources.Get(i);
      if (src->GetSoundHandle()==this && src->IsPlaying())
      {
        bytesfree=src->GetFreeBufferSpace();
        return (buffer_writecursor+bytesfree) % buffer_length;
      }
    }
  }

  return buffer_writecursor;
}

void csSoundHandleDS3D::vUpdate(void *buf, long Num)
{
  long NumBytes = Num * Data->GetFormat()->Bits/8 * Data->GetFormat()->Channels;
  for (size_t i=0; i<SoundRender->ActiveSources.Length(); i++)
  {
    csSoundSourceDS3D *src = (csSoundSourceDS3D*)SoundRender->ActiveSources.Get(i);
    if (src->GetSoundHandle()==this && src->IsPlaying())
      src->Write(buf, NumBytes);
  }
}
