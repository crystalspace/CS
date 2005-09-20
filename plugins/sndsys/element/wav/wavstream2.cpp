/*
    Copyright (C) 2004 by Andrew Mann
    Based in part on work by Norman Kraemer

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "iutil/comp.h"
#include "isndsys/ss_stream.h"
#include "csplugincommon/sndsys/convert.h"
#include "csplugincommon/sndsys/cyclicbuf.h"
#include "wavdata2.h"
#include "wavstream2.h"






// Keep these as integer values or the stream may feed portions of a sample to the higher layer which may cause problems
// 1 , 1 will decode 1/1 or 1 second of audio in advance
#define WAV_BUFFER_LENGTH_MULTIPLIER  1
#define WAV_BUFFER_LENGTH_DIVISOR     5

/// The size in bytes of the buffer in which is used if PCM conversion is necessary
#define WAV_DECODE_BUFFER_SIZE 4096


SCF_IMPLEMENT_IBASE (SndSysWavSoundStream)
  SCF_IMPLEMENTS_INTERFACE (iSndSysStream)
SCF_IMPLEMENT_IBASE_END


SndSysWavSoundStream::SndSysWavSoundStream (csRef<SndSysWavSoundData> data, char *WavData, size_t WavDataLen, SndSysSoundFormat *renderformat, int mode3d)
{
  SCF_CONSTRUCT_IBASE(NULL);

  // Copy render format information
  memcpy(&render_format,renderformat,sizeof(SndSysSoundFormat));

  // Copy the wav data buffer start and length
  wav_data_start=WavData;
  wav_bytes_total=WavDataLen;

  // Current position is at the beginning
  wav_data=wav_data_start;
  wav_bytes_left=wav_bytes_total;


  sound_data=data;

  // Allocate an advance buffer
  p_cyclicbuffer = new SoundCyclicBuffer((render_format.Bits/8 * render_format.Channels) * (render_format.Freq * WAV_BUFFER_LENGTH_MULTIPLIER / WAV_BUFFER_LENGTH_DIVISOR));
  CS_ASSERT(p_cyclicbuffer!=NULL);

  // Start the most advanced read pointer at offset 0
  most_advanced_read_pointer=0;

  // Set stream to paused, at the beginning, and not looping
  paused=true;
  last_time=0;
  looping=false;

  // Set the render sample size
  render_sample_size=(renderformat->Bits/8) * renderformat->Channels;


  // The prepared data buffer will be initialized on first use
  prepared_data_buffer=NULL;
  prepared_data_buffer_length=0;
  prepared_buffer_start=0;

  // No extra data left in the decode buffer since last call
  prepared_buffer_usage=0;

  // Playback rate is initially 100% (normal)
  playback_percent=100;

  // Forces the output buffer to be resized and the converter to be created on the first pass
  output_frequency=0;

  // Output frequency is initially the same as the render frequency
  new_output_frequency=renderformat->Freq;

  // Let the pcm converter get created on the first pass
  pcm_convert=NULL;

  // No new position 
  new_position=-1;

  // Store the 3d mode
  mode_3d=mode3d;

  // Auto unregister not requested
  auto_unregister=false;

  // Not ready to be unregistered
  unregister=false;

  // Playback isn't complete
  playback_read_complete=false;
}

SndSysWavSoundStream::~SndSysWavSoundStream ()
{
  delete p_cyclicbuffer;
  delete pcm_convert;
  delete[] prepared_data_buffer;

  SCF_DESTRUCT_IBASE();
}

const SndSysSoundFormat *SndSysWavSoundStream::GetRenderedFormat()
{
  return &render_format;
}

int SndSysWavSoundStream::Get3dMode()
{
  return mode_3d;
}


long SndSysWavSoundStream::GetSampleCount()
{
  const SndSysSoundFormat *data_format=sound_data->GetFormat();

  uint64 samplecount=sound_data->GetSampleCount();
  samplecount*=(render_format.Channels * render_format.Freq);
  samplecount/=(data_format->Channels * data_format->Freq);

  return (long)(samplecount & 0x7FFFFFFF);
}


long SndSysWavSoundStream::GetPosition()
{
  return most_advanced_read_pointer;

}

bool SndSysWavSoundStream::ResetPosition()
{
  new_position=0;
//  p_cyclicbuffer->Clear();
//  most_advanced_read_pointer=0;
  playback_read_complete=false;
  return true;
}

bool SndSysWavSoundStream::SetPosition(long newposition)
{
  new_position=newposition;
//  p_cyclicbuffer->Clear(newposition);
  playback_read_complete=false;
  return true;
}

bool SndSysWavSoundStream::Pause()
{
  paused=true;
  return true;
}


bool SndSysWavSoundStream::Unpause()
{
  paused=false;
  return true;
}

int SndSysWavSoundStream::GetPauseState()
{
  if (paused)
    return ISNDSYS_STREAM_PAUSED;
  return ISNDSYS_STREAM_UNPAUSED;
}

bool SndSysWavSoundStream::SetLoopState(int loopstate)
{
  switch (loopstate)
  {
    case ISNDSYS_STREAM_DONTLOOP:
      looping=false;
      break;
    case ISNDSYS_STREAM_LOOP:
      looping=true;
      break;
    default:
      return false; // Looping mode not supported
  }
  return true;
}

/// Retrieves the loop state of the stream.  Current possible returns are ISNDSYS_STREAM_DONTLOOP and ISNDSYS_STREAM_LOOP
int SndSysWavSoundStream::GetLoopState()
{
  if (looping)
    return ISNDSYS_STREAM_LOOP;
  return ISNDSYS_STREAM_DONTLOOP;
}

void SndSysWavSoundStream::SetPlayRatePercent(int percent)
{
  playback_percent=percent;
  new_output_frequency = (render_format.Freq * 100) / playback_percent;
}

int SndSysWavSoundStream::GetPlayRatePercent()
{
  return playback_percent;
}


/// If AutoUnregister is set, when the stream is paused it, and all sources attached to it are removed from the sound engine
void SndSysWavSoundStream::SetAutoUnregister(bool autounreg)
{
  auto_unregister=autounreg;
}

/// If AutoUnregister is set, when the stream is paused it, and all sources attached to it are removed from the sound engine
bool SndSysWavSoundStream::GetAutoUnregister()
{
  return auto_unregister;
}

/// Used by the sound renderer to determine if this stream needs to be unregistered
bool SndSysWavSoundStream::GetAutoUnregisterRequested()
{
  return unregister;
}


void SndSysWavSoundStream::AdvancePosition(csTicks current_time)
{

  if (new_position>=0)
  {
    // Signal a full cyclic buffer flush
    last_time=0;

    // Flush the prepared samples too
    prepared_buffer_usage=0;
    prepared_buffer_start=0;

    // Move the wav read position to the requested position
    if (new_position >= wav_bytes_total)
      new_position=wav_bytes_total-1;
    wav_data=wav_data_start+new_position;
    wav_bytes_left=wav_bytes_total-new_position;

    new_position=-1;
    playback_read_complete=false;
  }
  if (paused || playback_read_complete)
  {
    last_time=current_time;
    return;
  }


  long needed_bytes;
  long elapsed_ms;
  
  if (last_time==0)
  {

    needed_bytes=p_cyclicbuffer->GetLength();
  }
  else
  {
    elapsed_ms=current_time - last_time;

    if (elapsed_ms==0)
      return;

    // Figure out how many bytes we need to fill for this advancement
    needed_bytes=((elapsed_ms * render_format.Freq) / 1000) * (render_format.Bits/8) * render_format.Channels ;

    // If we need more space than is available in the whole cyclic buffer, then we already underbuffered, reduce to just 1 cycle full
    if ((size_t)needed_bytes > p_cyclicbuffer->GetLength())
      needed_bytes=(long)(p_cyclicbuffer->GetLength() & 0x7FFFFFFF);

  }

  // Free space in the cyclic buffer if necessary
  if ((size_t)needed_bytes > p_cyclicbuffer->GetFreeBytes())
    p_cyclicbuffer->AdvanceStartValue(needed_bytes - (long)(p_cyclicbuffer->GetFreeBytes() & 0x7FFFFFFF));

  // Fill in leftover decoded data if needed
  if (prepared_buffer_usage > 0)
    needed_bytes-=CopyBufferBytes(needed_bytes);

  last_time=current_time;


  while (needed_bytes>0)
  {
    size_t available_bytes=WAV_DECODE_BUFFER_SIZE;

//    if (available_bytes > WAV_DECODE_BUFFER_SIZE)
//      available_bytes=WAV_DECODE_BUFFER_SIZE;

    if (available_bytes > wav_bytes_left)
      available_bytes=wav_bytes_left;


    // If the output frequency has changed, a new converter is necessary
    if (new_output_frequency != output_frequency)
    {
      int needed_buffer,source_sample_size;
      const SndSysSoundFormat *data_format=sound_data->GetFormat();

      output_frequency=new_output_frequency;

      // Create the pcm sample converter if it's not yet created
      if (pcm_convert == NULL)
      {
#ifdef CS_LITTLE_ENDIAN
        pcm_convert=new PCMSampleConverter(data_format->Channels,data_format->Bits,data_format->Freq);
#else
        // If we're running on a big endian system and the data is using 16 bit samples, endian conversion is necessary
        if (data_format->Bits>8)
          pcm_convert=new PCMSampleConverter(data_format->Channels,data_format->Bits,data_format->Freq, true);
        else
          pcm_convert=new PCMSampleConverter(data_format->Channels,data_format->Bits,data_format->Freq);
#endif
      }

      // Calculate the size of one source sample
      source_sample_size=data_format->Channels * data_format->Bits;

      // Calculate the needed buffer size for this conversion
      needed_buffer=(pcm_convert->GetRequiredOutputBufferMultiple(render_format.Channels,render_format.Bits,output_frequency) * (WAV_DECODE_BUFFER_SIZE + source_sample_size))/1024;

      // Allocate a new buffer if needed - this will only happen if the source rate changes
      if (prepared_data_buffer_length < needed_buffer)
      {
        delete[] prepared_data_buffer;
        prepared_data_buffer = new char[needed_buffer];
        prepared_data_buffer_length=needed_buffer;
      }

      
      if ((data_format->Bits == render_format.Bits) &&
          (data_format->Channels == render_format.Channels) &&
          (data_format->Freq == render_format.Freq))
      {
          conversion_needed=false;
      }
      else
        conversion_needed=true;

#ifdef CS_BIG_ENDIAN
      // WAV data is always stored in little endian format.  If we're running on a big endian 
      // system and we're reading 16 bit samples, then we need a converter
      if (data_format->Bits >8)
        conversion_needed=true;
#endif

    }

    // If no conversion is necessary 
    if (!conversion_needed)
    {
      memcpy(prepared_data_buffer,wav_data,available_bytes);
      prepared_buffer_usage=available_bytes;
    }
    else
      prepared_buffer_usage=pcm_convert->ConvertBuffer(wav_data,available_bytes,prepared_data_buffer,render_format.Channels,render_format.Bits,output_frequency);

    // Decrease the available bytes and move the buffer pointer ahead
    wav_data+=available_bytes;
    wav_bytes_left-=available_bytes;

    // Copy the data available into the destination buffer as requested
    if (prepared_buffer_usage > 0)
      needed_bytes-=CopyBufferBytes(needed_bytes);

    // At the end of the stream, do we loop or stop?
    if (wav_bytes_left<=0)
    {
      if (!looping)
      {
        playback_read_complete=true;
        wav_data=wav_data_start;
        wav_bytes_left=wav_bytes_total;
        break;
      }

      // Loop by resetting the position to the beginning and continuing
      wav_data=wav_data_start;
      wav_bytes_left=wav_bytes_total;
    }
  }


      
}


long SndSysWavSoundStream::CopyBufferBytes(long max_dest_bytes)
{
  if (max_dest_bytes >= prepared_buffer_usage)
  {
    max_dest_bytes=prepared_buffer_usage;
    p_cyclicbuffer->AddBytes(&(prepared_data_buffer[prepared_buffer_start]),max_dest_bytes);
    prepared_buffer_usage=0;
    prepared_buffer_start=0;
    return max_dest_bytes;
  }


  p_cyclicbuffer->AddBytes(&(prepared_data_buffer[prepared_buffer_start]),max_dest_bytes);
  prepared_buffer_usage-=max_dest_bytes;
  prepared_buffer_start+=max_dest_bytes;
  return max_dest_bytes;
}




void SndSysWavSoundStream::GetDataPointers(long *position_marker,long max_requested_length,void **buffer1,long *buffer1_length,void **buffer2,long *buffer2_length)
{
  p_cyclicbuffer->GetDataPointersFromPosition(position_marker,max_requested_length,(uint8 **)buffer1,buffer1_length,(uint8 **)buffer2,buffer2_length);

  // If read is finished and we've underbuffered here, then we can mark the stream as paused so no further advancement takes place
  if ((!paused) && (playback_read_complete) && ((*buffer1_length + *buffer2_length) < max_requested_length))
  {
    paused=true;
    if (auto_unregister)
      unregister=true;
    playback_read_complete=false;
  }

  if (*position_marker > most_advanced_read_pointer)
    most_advanced_read_pointer=*position_marker;
}



void SndSysWavSoundStream::InitializeSourcePositionMarker(long *position_marker)
{
  *position_marker=most_advanced_read_pointer;
}








