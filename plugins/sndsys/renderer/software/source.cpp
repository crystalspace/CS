/*
	Copyright (C) 2004 by Andrew Mann

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
#include "csutil/sysfunc.h"
#include "csgeom.h"
#include "iutil/objreg.h"

#include "isndsys/ss_structs.h"

#include "isndsys/ss_data.h"
#include "isndsys/ss_stream.h"
#include "isndsys/ss_filter.h"
#include "isndsys/ss_source.h"
#include "isndsys/ss_listener.h"
#include "isndsys/ss_renderer.h"

#include "ivaria/reporter.h"

#include "renderer.h"
#include "listener.h"

#include "source.h"


SCF_IMPLEMENT_IBASE(SndSysSourceSoftwareBasic)
	SCF_IMPLEMENTS_INTERFACE(iSndSysSourceSoftware)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_IBASE(SndSysSourceSoftware3D)
  SCF_IMPLEMENTS_INTERFACE(iSndSysSourceSoftware3D)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_IBASE(SndSysSourceSoftwareFilter_Base)
  SCF_IMPLEMENTS_INTERFACE(SndSysSourceSoftwareFilter_Base)
SCF_IMPLEMENT_IBASE_END;




//////////////////////////////////////////////////////////////////////////
//
//  SndSysSourceSoftwareBasic
//
//////////////////////////////////////////////////////////////////////////
SndSysSourceSoftwareBasic::SndSysSourceSoftwareBasic(csRef<iSndSysStream> stream, SndSysRendererSoftware *rend)
: renderer(rend), sound_stream(stream)
{
  SCF_CONSTRUCT_IBASE(NULL);

  active_parameters.volume=0.0f;
  queued_parameters.volume=1.0f;

  queued_parameters.Copy(&active_parameters);
  queued_updates=false;
}

SndSysSourceSoftwareBasic::~SndSysSourceSoftwareBasic()
{
  // Clear out filters
  //while (!filters.IsEmpty())
    //RemoveFilter(filters[0]);

  SCF_DESTRUCT_IBASE();
}


/// Set volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
void SndSysSourceSoftwareBasic::SetVolume (float vol) 
{
  queued_parameters.volume=vol;
  queued_updates=true;
}

/// Get volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
float SndSysSourceSoftwareBasic::GetVolume () 
{
  return active_parameters.volume;
}


void SndSysSourceSoftwareBasic::UpdateQueuedParameters()
{
  // The most simple of cases for updating queued parameters is a straight copy.
  if (queued_updates)
  {
    // If we're 'starting' the source, synchronize with the stream play position
    if ((active_parameters.volume == 0.0f) &&
      (queued_parameters.volume != 0.0f))
    {
      sound_stream->InitializeSourcePositionMarker(&stream_position);
    }

    active_parameters.Copy(&queued_parameters);
    queued_updates=false;
  }
}

size_t SndSysSourceSoftwareBasic::MergeIntoBuffer(SoundSample *channel_buffer, size_t buffer_samples)
{
  float source_volume;
  void *buf1,*buf2;
  long buf1_len,buf2_len;
  int int_volume;
  long request_bytes;
  int bytes_per_sample;
  //size_t original_buffer_samples=buffer_samples;
  //bool fake_full_return=false;

  UpdateQueuedParameters();

  //ProcessFilterQueues();

  // If the source is at 0.0 volume, we won't spend any more time on it
  if (active_parameters.volume == 0.0f)
  {
    //renderer->Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System: Source is muted.");
    return buffer_samples;
  }

  // If the stream associated with the source is paused, the source generates no audio
  if ((sound_stream->GetPauseState() == ISNDSYS_STREAM_PAUSED) && 
      (sound_stream->GetPosition() == stream_position))
  {
    //renderer->Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System: Stream is paused.");
    return buffer_samples;
  }

  // Translate samples into bytes for the stream
  bytes_per_sample=renderer->render_format.Bits/8;
  request_bytes=buffer_samples * bytes_per_sample;
  buf1_len=0;
  buf2_len=0;

  //renderer->Report(CS_REPORTER_SEVERITY_DEBUG, "Stream position is %u.", stream_position);

  sound_stream->GetDataPointers(&stream_position,request_bytes,&buf1,&buf1_len,&buf2,&buf2_len);

  CS_ASSERT((buf1_len + buf2_len) <= request_bytes);

  //renderer->Report(CS_REPORTER_SEVERITY_DEBUG, "Read %d bytes from stream of length %d samples.", buf1_len + buf2_len, sound_stream->GetSampleCount());

  // Update the number of samples to those actually used
  buffer_samples=(buf1_len+buf2_len)/bytes_per_sample;

  if (buffer_samples == 0)
  {
    //renderer->Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System: Stream reports 0 length buffers.");
    return 0;
  }


  // Calculate the volume factor to apply to this buffer
  source_volume=active_parameters.volume;
  if (source_volume <0.00001f)
  {
    //renderer->Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System: Source volume below play threshold.");
    return buffer_samples;
  }

  //renderer->Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System: Source merge beginning.");

  // Calculate integer volume factor for faster calculations below
  int_volume = (int)(SOURCE_INTEGER_VOLUME_MULTIPLE * source_volume);

  /*
  if (HaveFilters())
  {
    // Allocate and clear a mix buffer
    if (!CheckMixBufferSize(buffer_samples))
      return buffer_samples;

    // Use the mix buffer as the mix destination
    dst_ptr=source_mix_buffer;
  }
  */

  /* renderer->render_format contains the output render format, which is
   *  also the format that the stream must present data in.
   * Here we convert from 8 or 16 bit stream data into channelized 16 bit 
   *  signed buffers stored in signed 32 bit accumulators
   *
   * The renderer stores samples for each channel individually:
   * [S0C0][S1C0]...[SnC0][S0C1]...[SnC1] .... [SnCn]
   *  not interleaved:
   * [S0C0][S0C1]...[S0Cn][S1C0]...[S1Cn] .... [SnCn]
   *
   * The stream also stores samples this way.  
  */
  if (renderer->render_format.Bits==8)
  {
    int i, buffer_idx;
    SoundSample mix;
    unsigned char *src_ptr;
    size_t second_half_offset=buffer_samples/2;

    buffer_idx=0;

    buf1_len/=2;
    buf2_len/=2;

    if (buf1_len)
    {
      src_ptr =(unsigned char *)buf1;
      for (i=0;i<buf1_len;i++, buffer_idx++)
      {
        // Convert to a signed value
        mix=src_ptr[i*2]-128;
        // Convert to 16 bit signed values
        mix*=256;
        // Apply volume
        mix=(mix * int_volume) / SOURCE_INTEGER_VOLUME_MULTIPLE;
        // Add into the destination sample buffer
        channel_buffer[buffer_idx]+=mix;

        // Convert to a signed value
        mix=src_ptr[i*2+1]-128;
        // Convert to 16 bit signed values
        mix*=256;
        // Apply volume
        mix=(mix * int_volume) / SOURCE_INTEGER_VOLUME_MULTIPLE;
        // Add into the destination sample buffer
        channel_buffer[second_half_offset+buffer_idx]+=mix;
        CS_ASSERT(second_half_offset+buffer_idx < buffer_samples);
      }
    }
    if (buf2_len)
    {
      src_ptr =(unsigned char *)buf2;
      for (i=0;i<buf2_len;i++, buffer_idx++)
      {
        // Convert to a signed value
        mix=src_ptr[i*2]-128;
        // Convert to 16 bit signed values
        mix*=256;
        // Apply volume
        mix=(mix * int_volume) / SOURCE_INTEGER_VOLUME_MULTIPLE;
        // Add into the destination sample buffer
        channel_buffer[buffer_idx]+=mix;

        // Convert to a signed value
        mix=src_ptr[i*2+1]-128;
        // Convert to 16 bit signed values
        mix*=256;
        // Apply volume
        mix=(mix * int_volume) / SOURCE_INTEGER_VOLUME_MULTIPLE;
        // Add into the destination sample buffer
        channel_buffer[second_half_offset+buffer_idx]+=mix;
        CS_ASSERT(second_half_offset+buffer_idx < buffer_samples);
      }
    }
  }
  else
  {
    // 16 bit samples
    int i, buffer_idx;
    SoundSample mix;
    short *src_ptr;
    size_t second_half_offset=buffer_samples/2;

    buffer_idx=0;

    // Convert the lengths from byte lengths to word lengths
    //  We'll read 2 samples at once, so use half lengths
    buf1_len/=4;
    buf2_len/=4;

    if (buf1_len)
    {
      src_ptr =(short *)buf1;
      for (i=0;i<buf1_len;i++,buffer_idx++)
      {
        // Convert to a signed integer
        mix=src_ptr[i*2];
        // Apply volume
        mix=(mix * int_volume) / SOURCE_INTEGER_VOLUME_MULTIPLE;
        // Add into the destination sample buffer
        channel_buffer[buffer_idx]+=mix;

        // Convert to a signed integer
        mix=src_ptr[i*2+1];
        // Apply volume
        mix=(mix * int_volume) / SOURCE_INTEGER_VOLUME_MULTIPLE;
        // Add into the destination sample buffer
        channel_buffer[second_half_offset+buffer_idx]+=mix;
        CS_ASSERT(second_half_offset+buffer_idx < buffer_samples);
      }
    }
    if (buf2_len)
    {
      src_ptr =(short *)buf2;
      for (i=0;i<buf2_len;i++, buffer_idx++)
      {
        // Convert to a signed integer
        mix=src_ptr[i*2];
        // Apply volume
        mix=(mix * int_volume) / SOURCE_INTEGER_VOLUME_MULTIPLE;
        // Add into the destination sample buffer
        channel_buffer[buffer_idx]+=mix;

        // Convert to a signed integer
        mix=src_ptr[i*2+1];
        // Apply volume
        mix=(mix * int_volume) / SOURCE_INTEGER_VOLUME_MULTIPLE;
        // Add into the destination sample buffer
        channel_buffer[second_half_offset+buffer_idx]+=mix;
        CS_ASSERT(second_half_offset+buffer_idx < buffer_samples);
      }
    }
  }

  /*
  if (HaveFilters())
  {
    // Run filter chain
    size_t filter,filter_max;
    filter_max=filters.Length();
    for (filter=0;filter<filter_max;filter++)
    {
      filters[filter]->Apply(source_mix_buffer, buffer, buffer_samples, &(renderer->render_format));
    }

    // Add the post-filter mixed buffer into the renderer buffer
    for (long sample=0;sample<buffer_samples;sample++)
      buffer[sample]+=source_mix_buffer[sample];
  }
  */

  return buffer_samples;
}




//////////////////////////////////////////////////////////////////////////
//
//  SndSysSourceSoftware3D
//
//////////////////////////////////////////////////////////////////////////
SndSysSourceSoftware3D::SndSysSourceSoftware3D(csRef<iSndSysStream> stream, SndSysRendererSoftware *rend)
: renderer(rend), sound_stream(stream), clean_buffer(NULL), clean_buffer_samples(0), working_buffer(NULL), working_buffer_samples(0),
  filters_setup(false)
{
  SCF_CONSTRUCT_IBASE(NULL);

  active_parameters.maximum_distance=ISNDSYS_SOURCE_DISTANCE_INFINITE;
  active_parameters.minimum_distance=1.0;
  active_parameters.position.Set(0,0,0);

  active_parameters.volume=0.0f;
  queued_parameters.volume=1.0f;


  //const SndSysSoundFormat *fmt=stream->GetRenderedFormat();
  // Allocate the history buffer
 // historic_buffer_samples=(fmt->Freq * SOURCE_3D_BUFFER_TIME_MS) / 1000;
 // historic_buffer=new SoundSample[historic_buffer_samples];

 // ClearBuffer(historic_buffer,historic_buffer_samples);

  queued_parameters.Copy(&active_parameters);
  queued_updates=false;
}

SndSysSourceSoftware3D::~SndSysSourceSoftware3D()
{
  // Clear out filters
  //while (!filters.IsEmpty())
  //RemoveFilter(filters[0]);
  SCF_DESTRUCT_IBASE();
}


/// Set volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
void SndSysSourceSoftware3D::SetVolume (float vol) 
{
  queued_parameters.volume=vol;
  queued_updates=true;
}

/// Get volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
float SndSysSourceSoftware3D::GetVolume () 
{
  return active_parameters.volume;
}

/// set position of this source
void SndSysSourceSoftware3D::SetPosition(csVector3 pos) 
{
  queued_parameters.position.Set(pos);
  queued_updates=true;
}

/// get position of this source
csVector3 SndSysSourceSoftware3D::GetPosition() 
{
  return active_parameters.position;
}

/// set position of this source
void SndSysSourceSoftware3D::SetDirection(csVector3 dir)
{
  queued_parameters.direction=dir;
  queued_updates=true;
}

/// get position of this source
csVector3 SndSysSourceSoftware3D::GetDirection()
{
  return active_parameters.direction;
}

void SndSysSourceSoftware3D::SetDirectionalRadiation(float rad)
{
  while (rad> PI)
    rad-=PI;
  while (rad<-PI)
    rad+=PI;

  queued_parameters.directional_radiation=rad;
  queued_parameters.directional_cos=cos(rad);
  queued_updates=true;
}

// Retrieves the current directional radiation 
float SndSysSourceSoftware3D::GetDirectionalRadiation()
{
  return active_parameters.directional_radiation;
}


void SndSysSourceSoftware3D::SetMinimumDistance (float distance) 
{
  if (distance < 0.000001f)
    distance=0.000001f;

  queued_parameters.minimum_distance=distance;
  queued_updates=true;
}

void SndSysSourceSoftware3D::SetMaximumDistance (float distance) 
{
  if (distance != ISNDSYS_SOURCE_DISTANCE_INFINITE && 
    distance < 0.000001f)
    distance=0.0f; // Don't play at all

  queued_parameters.maximum_distance=distance;
  queued_updates=true;
}

/// Retrieve the maximum distance for which a sound is heard at full volume.  See SetMaximumDistance and SetMinimumDistance for distance notes.
float SndSysSourceSoftware3D::GetMinimumDistance () 
{
  return active_parameters.minimum_distance;
}

/// Retrieve the maximum distance for which a sound can be heard.  See SetMaximumDistance and SetMinimumDistance for distance notes.
float SndSysSourceSoftware3D::GetMaximumDistance () 
{
  return active_parameters.maximum_distance;
}


void SndSysSourceSoftware3D::UpdateQueuedParameters()
{
  // The most simple of cases for updating queued parameters is a straight copy.
  if (queued_updates)
  {
    // If we're 'starting' the source, synchronize with the stream play position
    if ((active_parameters.volume == 0.0f) &&
      (queued_parameters.volume != 0.0f))
    {
      sound_stream->InitializeSourcePositionMarker(&stream_position);
    }

    active_parameters.Copy(&queued_parameters);
    queued_updates=false;
  }
}


void SndSysSourceSoftware3D::SetupFilters()
{
  size_t i;
  for (i=0;i<MAX_CHANNELS;i++)
  {
    // Apply the ITD delay for the channel
    // Apply the IID intensity for the channel


    iSndSysSoftwareFilter3D *last_filt, *next_filt;
    next_filt=new SndSysSourceSoftwareFilter_ITDDelay();
    speaker_filter_chains[i]=next_filt;
    last_filt=next_filt;
    next_filt=new SndSysSourceSoftwareFilter_IID();
    last_filt->AddSubFilter(next_filt);
    last_filt=next_filt;



    /*
    // Clip the direct path if it's blocked
    iSndSysSoftwareFilter3D *clip_filt= new SndSysSourceSoftwareFilter_DirectOnly();
    last_filt->AddSubFilter(clip_filt);
    */

    // Split the channel into two subchannels - we'll treat one as the direct and one as the reverb
    next_filt=new SndSysSourceSoftwareFilter_SplitPath();
    last_filt->AddSubFilter(next_filt);
    last_filt=next_filt;

    // Clip the direct path if it's blocked
    iSndSysSoftwareFilter3D *clip_filt= new SndSysSourceSoftwareFilter_DirectFade();
    last_filt->AddSubFilter(clip_filt);


    // Second path is indirect - apply a low pass filter
    next_filt=new SndSysSourceSoftwareFilter_LowPass();
    last_filt->AddSubFilter(next_filt,1);
    last_filt=next_filt;

    // Apply a 0.05 second delay to the second path as well
    /*
    SndSysSourceSoftwareFilter_Delay *delay=new SndSysSourceSoftwareFilter_Delay();
    delay->SetDelayTime(0.05);
    next_filt=delay;
    last_filt->AddSubFilter(next_filt);
    last_filt=next_filt;
    */

    // Add some reverb to the indirect path too
    next_filt= new SndSysSourceSoftwareFilter_Reverb();
    last_filt->AddSubFilter(next_filt);
  }

  filters_setup=true;
}


size_t SndSysSourceSoftware3D::MergeIntoBuffer(SoundSample *channel_buffer, size_t buffer_samples)
{
  void *buf1,*buf2;
  long buf1_len,buf2_len;
  long request_bytes;
  int bytes_per_sample;
  size_t per_channel_samples;

  // TODO: Can we calculate listener/source dependant delay and intensity here only if the source and/or listener properties have changed?


  UpdateQueuedParameters();

  if (!filters_setup)
    SetupFilters();

  //ProcessFilterQueues();

  // If the source is at 0.0 volume, we won't spend any more time on it
  if (active_parameters.volume == 0.0f)
  {
    //renderer->Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System: Source is muted.");
    return buffer_samples;
  }

  // If the stream associated with the source is paused, the source generates no audio
  if ((sound_stream->GetPauseState() == ISNDSYS_STREAM_PAUSED) && 
    (sound_stream->GetPosition() == stream_position))
  {
    //renderer->Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System: Stream is paused.");
    return buffer_samples;
  }


  // The samples count given by the renderer is the samples for all channels combined.
  //  Translate into the samples for each channel
  per_channel_samples = buffer_samples / renderer->render_format.Channels;


  //renderer->Report(CS_REPORTER_SEVERITY_DEBUG, "Stream position is %u.", stream_position);



  // Translate samples into bytes for the stream
  bytes_per_sample=renderer->render_format.Bits/8;
  request_bytes=per_channel_samples * bytes_per_sample;
  sound_stream->GetDataPointers(&stream_position,request_bytes,&buf1,&buf1_len,&buf2,&buf2_len);

  CS_ASSERT((buf1_len + buf2_len) <= request_bytes);
  //renderer->Report(CS_REPORTER_SEVERITY_DEBUG, "Read %d bytes from stream of length %d samples.", buf1_len + buf2_len, sound_stream->GetSampleCount());

  // Update the number of samples to those actually used
  per_channel_samples=(buf1_len+buf2_len)/bytes_per_sample;
  buffer_samples = per_channel_samples * renderer->render_format.Channels;

  if (buffer_samples == 0)
  {
    //renderer->Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System: Stream reports 0 length buffers.");
    return 0;
  }

  // Prepare buffers
  if (!PrepareBuffer(&working_buffer,&working_buffer_samples, per_channel_samples))
  {
    //renderer->Report(CS_REPORTER_SEVERITY_ERROR, "Sound System: Failed to allocate a working buffer.");
    return buffer_samples;
  }
  if (!PrepareBuffer(&clean_buffer,&clean_buffer_samples, per_channel_samples))
  {
    //renderer->Report(CS_REPORTER_SEVERITY_ERROR, "Sound System: Failed to allocate a clean buffer.");
    return buffer_samples;
  }

  // Convert the read samples into the clean buffer
  if (renderer->render_format.Bits==8)
  {
    int i, buffer_idx;
    SoundSample mix;
    unsigned char *src_ptr;

    buffer_idx=0;

    if (buf1_len)
    {
      src_ptr =(unsigned char *)buf1;
      for (i=0;i<buf1_len;i++)
      {
        // Convert to a signed value
        mix=src_ptr[i]-128;

        // Convert to 16 bit signed values
        mix*=256;

        // Place in the clean buffer
        clean_buffer[buffer_idx++]=mix;
      }
    }
    if (buf2_len)
    {
      src_ptr =(unsigned char *)buf2;
      for (i=0;i<buf2_len;i++)
      {
        // Convert to a signed value
        mix=src_ptr[i]-128;

        // Convert to 16 bit signed values
        mix*=256;

        // Place in the clean buffer
        clean_buffer[buffer_idx++]=mix;
      }
    }
  }
  else
  {
    // 16 bit samples
    int i, buffer_idx;
    short *src_ptr;

    buffer_idx=0;

    // Convert the lengths from byte lengths to word lengths
    buf1_len/=2;
    buf2_len/=2;

    if (buf1_len)
    {
      src_ptr =(short *)buf1;
      for (i=0;i<buf1_len;i++)
        // Convert to a signed integer
        clean_buffer[buffer_idx++]=src_ptr[i];
    }
    if (buf2_len)
    {
      src_ptr =(short *)buf2;
      for (i=0;i<buf2_len;i++)
        // Convert to a signed integer
        clean_buffer[buffer_idx++]=src_ptr[i];
    }
  }


  int channel,total_channels;
  total_channels=renderer->render_format.Channels;
  csVector3 listener_to_source;
  
  if (sound_stream->Get3dMode() == SND3D_RELATIVE)
    listener_to_source=active_parameters.position;
  else
  {
    // Translate absolute coordinates into relative listener coordinates
    listener_to_source=renderer->Listener->active_properties.world_to_listener.Other2This(active_parameters.position);
  }

  /*
  if (debug_cycle++ >= 20)
  {
    renderer->Report(CS_REPORTER_SEVERITY_DEBUG, "3D sound source has relative vector (%0.4f,%0.4f,%0.4f)", 
      listener_to_source.x, listener_to_source.y, listener_to_source.z);
    debug_cycle=0;
  }
  */



  // For each output channel, calculate the distance
  closest_speaker=-1.0f;
  for (channel=0;channel<total_channels;channel++)
  {
    csVector3 speaker_to_source = listener_to_source - renderer->Speakers[channel].RelativePosition;
    // Calculate the distance between the listener and the source
    float distance=speaker_to_source.Norm();

    // Clamp the distance at the max
    if (active_parameters.maximum_distance != ISNDSYS_SOURCE_DISTANCE_INFINITE)
    {
      if (distance > active_parameters.maximum_distance)
        distance=active_parameters.maximum_distance;
    }

    if ((closest_speaker < 0.0f) || (distance < closest_speaker))
      closest_speaker=distance;
    speaker_distance[channel]=distance;

    // Translate speaker position from starting coordinate system to directional coordinate system
    speaker_to_source.Normalize();

    speaker_direction_cos[channel]=(renderer->Speakers[channel].Direction * speaker_to_source);
    /*
    // Approximate the effect of direction as a 40 percent variance in volume
    speaker_scale[channel] = 0.40f * (listener_to_source * speaker_to_source);
    if (speaker_scale[channel] < 0.0f)
      speaker_scale[channel]=0.0f;
    speaker_scale[channel]+=0.60f;
    */
  }

  // For each output channel, calculate the sound
  for (channel=0;channel<total_channels;channel++)
  {
    if (ProcessSoundChain(channel, per_channel_samples))
    {
      // If ProcessSoundChain returns true, merge the samples into the renderer buffer
      SoundSample *channel_base=&(channel_buffer[channel * per_channel_samples]);
      for (size_t i=0;i<per_channel_samples;i++)
        channel_base[i]+=working_buffer[i];
    }
  }


  // Update the historic buffer
  /*
  if (per_channel_samples > historic_buffer_samples)
    memcpy(historic_buffer,&(clean_buffer[per_channel_samples-historic_buffer_samples]), sizeof(SoundSample) * historic_buffer_samples);
  else
  {
    memcpy(historic_buffer,&(historic_buffer[per_channel_samples]), sizeof(SoundSample) * per_channel_samples);
    memcpy(&(historic_buffer[historic_buffer_samples-per_channel_samples]), clean_buffer, sizeof(SoundSample) * per_channel_samples);
  }
  */



  /*
  if (HaveFilters())
  {
  // Run filter chain
  size_t filter,filter_max;
  filter_max=filters.Length();
  for (filter=0;filter<filter_max;filter++)
  {
  filters[filter]->Apply(source_mix_buffer, buffer, buffer_samples, &(renderer->render_format));
  }

  // Add the post-filter mixed buffer into the renderer buffer
  for (long sample=0;sample<buffer_samples;sample++)
  buffer[sample]+=source_mix_buffer[sample];
  }
  */


  return buffer_samples;
}

bool SndSysSourceSoftware3D::ProcessSoundChain(int channel, size_t buffer_samples)
{

  /*
  float vol;
  int int_vol;



  // Calculate the intensity for this channel based off actual distance

  // Turn distance into units based off minimum distance
  float minimum_distance=active_parameters.minimum_distance;
  if (minimum_distance < 0.000001f)
    minimum_distance=0.000001f;
  float iid_distance=speaker_distance[channel]/minimum_distance;

  // The minimum distance is 1 minimum distance unit
  if (iid_distance < 1.0f) iid_distance=1.0f;

  // The rolloff factor is applied as a factor to the natural rolloff power   - 2.0?
  float rollofffactor=renderer->Listener->active_properties.rolloff_factor * 1.0f;

  // Calculate the intensity for this channel using listener channel direction
  vol = speaker_scale[channel] * active_parameters.volume;

  // Apply the rolloff factor to the volume
  if (rollofffactor != 1.0f)
    vol/=pow(iid_distance,rollofffactor);

  int_vol=(SOURCE_INTEGER_VOLUME_MULTIPLE * vol);


  // TODO: use the source direction to determine if the listener is in the direct projection cone of the sound


  // Calculate the delay for this channel, this is based off difference in distance between the closest channel and this channel
  float delay_dist=speaker_distance[channel]-closest_speaker;
  float time=delay_dist / 331.4f;
  float fsamples=time * renderer->render_format.Freq;
  size_t samples = fsamples;

  if (samples<1)
    samples=0;
  if (samples>historic_buffer_samples)
    samples=historic_buffer_samples;
  if (samples>buffer_samples)
    samples=buffer_samples;


  // Create the output buffer
  size_t s,d;
  d=0;
  // Process history buffer first
  for (s=historic_buffer_samples-samples;s<historic_buffer_samples;s++, d++)
    working_buffer[d]=(historic_buffer[s] * int_vol) / SOURCE_INTEGER_VOLUME_MULTIPLE ;

  // Process the real buffer now
  if (buffer_samples>samples)
  {
    samples=buffer_samples-samples;
    for (s=0;s<samples;s++, d++)
      working_buffer[d]=(clean_buffer[s] * int_vol) / SOURCE_INTEGER_VOLUME_MULTIPLE ;
  }
  */

  // Test - apply a low pass filter to the buffer
  iSndSysSoftwareFilter3DProperties filter_props;
  filter_props.buffer_samples=buffer_samples;
  filter_props.channel=channel;
  filter_props.clean_buffer=clean_buffer;
  filter_props.closest_speaker_distance=closest_speaker;
  filter_props.listener_parameters=&(renderer->Listener->active_properties);
  filter_props.sound_format=&(renderer->render_format);
  filter_props.source_parameters=&(active_parameters);
  filter_props.work_buffer=working_buffer;
  filter_props.speaker_distance=speaker_distance;
  filter_props.speaker_direction_cos=speaker_direction_cos;
  filter_props.reporter=renderer->reporter;

  memcpy(working_buffer, clean_buffer, sizeof(SoundSample) * buffer_samples);


  speaker_filter_chains[channel]->Apply(filter_props);


  return true;
}



inline bool SndSysSourceSoftware3D::PrepareBuffer(SoundSample **p_buf, size_t *p_buf_len, size_t required)
{
  if (*p_buf_len < required)
  {
    delete[] (*p_buf);
    *p_buf=new SoundSample[required];
    if (!*p_buf)
    {
      *p_buf_len=0;
      return false;
    }
    *p_buf_len=required;
  }
  //memset(*p_buf, 0, sizeof(SoundSample) * required);
  return true;
}

inline void SndSysSourceSoftware3D::ClearBuffer(SoundSample *p_buf, size_t p_buf_len)
{
  memset(p_buf,0, sizeof(SoundSample) * p_buf_len);
}


