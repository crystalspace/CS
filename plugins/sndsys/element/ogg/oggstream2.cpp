/*
    Copyright (C) 2005 by Andrew Mann
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
#include "oggdata2.h"
#include "oggstream2.h"

#include "ivaria/reporter.h"

/* Keep these as integer values or the stream may feed portions of a sample to 
 * the higher layer which may cause problems
 * 1 , 1 will decode 1/1 or 1 second of audio in advance */
#define OGG_BUFFER_LENGTH_MULTIPLIER  1
#define OGG_BUFFER_LENGTH_DIVISOR     5

/**
 * The size in bytes of the buffer in which decoded ogg data is stored before 
 * copying/conversion
 */
#define OGG_DECODE_BUFFER_SIZE 4096

SCF_IMPLEMENT_IBASE (SndSysOggSoundStream)
  SCF_IMPLEMENTS_INTERFACE (iSndSysStream)
SCF_IMPLEMENT_IBASE_END

extern cs_ov_callbacks *GetCallbacks();


SndSysOggSoundStream::SndSysOggSoundStream (csRef<SndSysOggSoundData> data, 
					    OggDataStore *datastore, 
					    csSndSysSoundFormat *renderformat, 
					    int mode3d)
{
  SCF_CONSTRUCT_IBASE(0);

  // Copy render format information
  memcpy(&render_format,renderformat,sizeof(csSndSysSoundFormat));

  // Initialize the stream data
  stream_data.datastore=datastore;
  stream_data.position=0;

  sound_data=data;

  // Allocate an advance buffer
  p_cyclicbuffer = new CrystalSpace:: SoundCyclicBuffer (
    (render_format.Bits/8 * render_format.Channels) * 
      (render_format.Freq * OGG_BUFFER_LENGTH_MULTIPLIER / 
	OGG_BUFFER_LENGTH_DIVISOR));
  CS_ASSERT(p_cyclicbuffer!=0);

  // Initialize ogg file
  memset(&vorbis_file,0,sizeof(OggVorbis_File));
  ov_open_callbacks (&stream_data,&vorbis_file,0,0,
    *(ov_callbacks*)GetCallbacks());

  // Start the most advanced read pointer at offset 0
  most_advanced_read_pointer=0;

  // Set stream to paused, at the beginning, and not looping
  paused=true;
  last_time=0;
  looping=false;

  // A prepared data buffer will be allocated when we know the size needed
  prepared_data_buffer=0;
  prepared_data_buffer_length=0;
  prepared_buffer_start=0;

  // Set the render sample size
  render_sample_size=(renderformat->Bits/8) * renderformat->Channels;

  // No extra data left in the decode buffer since last call
  prepared_buffer_usage=0;

  // Playback rate is initially 100% (normal)
  playback_percent=100;

  /* Forces the output buffer to be resized and the converter to be created on 
   * the first pass */
  output_frequency=0;

  // Output frequency is initially the same as the render frequency
  new_output_frequency=renderformat->Freq;

  // Set to not a valid stream
  current_ogg_stream=-1;

  // Let the pcm converter get created on the first pass
  pcm_convert=0;

  // No new position 
  new_position=-1;

  // Store the 3d mode
  mode_3d=mode3d;

  // Auto unregister not requested
  auto_unregister=false;

  // Not ready to be unregistered
  unregister=false;

  // Playback not complete
  playback_read_complete=false;
}

SndSysOggSoundStream::~SndSysOggSoundStream ()
{
  delete p_cyclicbuffer;
  delete pcm_convert;
  delete[] prepared_data_buffer;

  SCF_DESTRUCT_IBASE();
}

const csSndSysSoundFormat *SndSysOggSoundStream::GetRenderedFormat()
{
  return &render_format;
}

int SndSysOggSoundStream::Get3dMode()
{
  return mode_3d;
}


long SndSysOggSoundStream::GetSampleCount()
{
  const csSndSysSoundFormat *data_format=sound_data->GetFormat();

  uint64 samplecount=sound_data->GetSampleCount();
  samplecount*=(render_format.Channels * render_format.Freq);
  samplecount/=(data_format->Channels * data_format->Freq);

  return (long)(samplecount & 0x7FFFFFFF);
}


long SndSysOggSoundStream::GetPosition()
{
  return most_advanced_read_pointer;

}

bool SndSysOggSoundStream::ResetPosition()
{
  new_position=0;
//  p_cyclicbuffer->Clear();
//  most_advanced_read_pointer=0;
  playback_read_complete=false;
  return true;
}

bool SndSysOggSoundStream::SetPosition(long newposition)
{
  new_position=newposition;
  //p_cyclicbuffer->Clear(newposition);
  playback_read_complete=false;
  return true;
}

bool SndSysOggSoundStream::Pause()
{
  paused=true;
  return true;
}


bool SndSysOggSoundStream::Unpause()
{
  paused=false;
  return true;
}

int SndSysOggSoundStream::GetPauseState()
{
  if (paused)
    return CS_SNDSYS_STREAM_PAUSED;
  return CS_SNDSYS_STREAM_UNPAUSED;
}

bool SndSysOggSoundStream::SetLoopState(int loopstate)
{
  switch (loopstate)
  {
    case CS_SNDSYS_STREAM_DONTLOOP:
      looping=false;
      break;
    case CS_SNDSYS_STREAM_LOOP:
      looping=true;
      break;
    default:
      return false; // Looping mode not supported
  }
  return true;
}

/** 
 * Retrieves the loop state of the stream.  Current possible returns are 
 * CS_SNDSYS_STREAM_DONTLOOP and CS_SNDSYS_STREAM_LOOP
 */
int SndSysOggSoundStream::GetLoopState()
{
  if (looping)
    return CS_SNDSYS_STREAM_LOOP;
  return CS_SNDSYS_STREAM_DONTLOOP;
}

void SndSysOggSoundStream::SetPlayRatePercent(int percent)
{
  playback_percent=percent;
  new_output_frequency = (render_format.Freq * 100) / playback_percent;
}

int SndSysOggSoundStream::GetPlayRatePercent()
{
  return playback_percent;
}

/** 
 * If AutoUnregister is set, when the stream is paused it, and all sources 
 * attached to it are removed from the sound engine
 */
void SndSysOggSoundStream::SetAutoUnregister(bool autounreg)
{
  auto_unregister=autounreg;
}

/** 
 * If AutoUnregister is set, when the stream is paused it, and all sources 
 * attached to it are removed from the sound engine
 */
bool SndSysOggSoundStream::GetAutoUnregister()
{
  return auto_unregister;
}

/// Used by the sound renderer to determine if this stream needs to be unregistered
bool SndSysOggSoundStream::GetAutoUnregisterRequested()
{
  return unregister;
}


void SndSysOggSoundStream::AdvancePosition(csTicks current_time)
{
  if (new_position>=0)
  {
    // Signal a full cyclic buffer flush
    last_time=0;

    // Flush the prepared samples too
    prepared_buffer_usage=0;
    prepared_buffer_start=0;

    // Seek the ogg stream to the requested position
    ov_raw_seek(&vorbis_file,new_position);

    new_position=-1;
    playback_read_complete=false;
  }
  if (paused || playback_read_complete)
  {
    last_time=current_time;
    return;
  }

 
  long bytes_read=0;
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
    needed_bytes=((elapsed_ms * render_format.Freq) / 1000) * 
      (render_format.Bits/8) * render_format.Channels ;

    /* If we need more space than is available in the whole cyclic buffer, 
     * then we already underbuffered, reduce to just 1 cycle full
     */
    if ((size_t)needed_bytes > p_cyclicbuffer->GetLength())
      needed_bytes=(long)(p_cyclicbuffer->GetLength() & 0x7FFFFFFF);
  }
  // Free space in the cyclic buffer if necessary
  if ((size_t)needed_bytes > p_cyclicbuffer->GetFreeBytes())
    p_cyclicbuffer->AdvanceStartValue(needed_bytes - 
      (long)(p_cyclicbuffer->GetFreeBytes() & 0x7FFFFFFF));

  // Fill in leftover decoded data if needed
  if (prepared_buffer_usage > 0)
    needed_bytes-=CopyBufferBytes(needed_bytes);

  last_time=current_time;

  while (needed_bytes > 0)
  {
    int last_ogg_stream=current_ogg_stream;
    char ogg_decode_buffer[OGG_DECODE_BUFFER_SIZE];

    bytes_read=0;

    while (bytes_read==0)
    {
      bytes_read = ov_read (&vorbis_file, ogg_decode_buffer,
	OGG_DECODE_BUFFER_SIZE, OGG_ENDIAN,
	(render_format.Bits==8)?1:2, (render_format.Bits==8)?0:1,
	&current_ogg_stream);
   
      // Assert on error
      CS_ASSERT(bytes_read >=0);

      if (bytes_read <= 0)
      {
        if (!looping)
        {
          // Seek back to the beginning for a restart.  Pause on the next call
          playback_read_complete=true;
          ov_raw_seek(&vorbis_file,0);
          return;
        }

        // Loop by resetting the position to the beginning and continuing
        ov_raw_seek(&vorbis_file,0);
      }
    }


    

    // If streams changed, the format may have changed as well
    if ((new_output_frequency != output_frequency) 
      || (last_ogg_stream != current_ogg_stream))
    {
      int needed_buffer,source_sample_size;

      output_frequency=new_output_frequency;

      current_ogg_format_info=ov_info(&vorbis_file,current_ogg_stream);

      // Create the pcm sample converter if it's not yet created
      if (pcm_convert == 0)
        pcm_convert = new CrystalSpace::PCMSampleConverter (
	  current_ogg_format_info->channels, render_format.Bits,
	  current_ogg_format_info->rate);

      // Calculate the size of one source sample
      source_sample_size=current_ogg_format_info->channels * render_format.Bits;

      // Calculate the needed buffer size for this conversion
      needed_buffer = (pcm_convert->GetRequiredOutputBufferMultiple (
	render_format.Channels,render_format.Bits,output_frequency) * 
	  (OGG_DECODE_BUFFER_SIZE + source_sample_size))/1024;

      // Allocate a new buffer if needed - this will only happen if the source rate changes
      if (prepared_data_buffer_length < needed_buffer)
      {
        delete[] prepared_data_buffer;
        prepared_data_buffer = new char[needed_buffer];
        prepared_data_buffer_length=needed_buffer;
      }
    }

    // If no conversion is necessary 
    if ((current_ogg_format_info->rate == output_frequency) &&
        (current_ogg_format_info->channels == render_format.Channels))
    {
      CS_ASSERT(bytes_read <= prepared_data_buffer_length);
      memcpy(prepared_data_buffer,ogg_decode_buffer,bytes_read);
      prepared_buffer_usage=bytes_read;
    }
    else
    {
      prepared_buffer_usage = pcm_convert->ConvertBuffer (ogg_decode_buffer,
	bytes_read, prepared_data_buffer, render_format.Channels,
	render_format.Bits,output_frequency);
    }

    if (prepared_buffer_usage > 0)
      needed_bytes-=CopyBufferBytes(needed_bytes);

  }
      
}


long SndSysOggSoundStream::CopyBufferBytes(long max_dest_bytes)
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




void SndSysOggSoundStream::GetDataPointers(long *position_marker, 
					    long max_requested_length,
					    void **buffer1, 
					    long *buffer1_length,
					    void **buffer2,
					    long *buffer2_length)
{
  p_cyclicbuffer->GetDataPointersFromPosition (position_marker,
    max_requested_length, (uint8 **)buffer1, buffer1_length, 
    (uint8 **)buffer2,buffer2_length);

  /* If read is finished and we've underbuffered here, then we can mark the 
   * stream as paused so no further advancement takes place */
  if ((!paused) && (playback_read_complete) 
      && ((*buffer1_length + *buffer2_length) < max_requested_length))
  {
    paused=true;
    if (auto_unregister)
      unregister=true;
    playback_read_complete=false;
  }

  if (*position_marker > most_advanced_read_pointer)
    most_advanced_read_pointer=*position_marker;
}



void SndSysOggSoundStream::InitializeSourcePositionMarker (
  long *position_marker)
{
  *position_marker=most_advanced_read_pointer;
}
