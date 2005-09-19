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


#ifndef SNDSYS_RENDERER_SOFTWARE_SOURCE_H
#define SNDSYS_RENDERER_SOFTWARE_SOURCE_H


#include "ss_filter.h"
#include "ss_source.h"

#include "queue.h"

#define MAX_CHANNELS 18

#define SOURCE_INTEGER_VOLUME_MULTIPLE 1024

#define SOURCE_3D_BUFFER_TIME_MS 300


struct iSndSysStream;
//struct iSndSysSoftwareDriver;
//struct iConfigFile;
//class csSoundListener2Software;
//class SndSysSourceSoftware;


class SourceParametersBasic
{
public:
  SourceParametersBasic() {}
  SourceParametersBasic(const SourceParametersBasic *copyfrom) { Copy(copyfrom); }

  ~SourceParametersBasic() {}

  void Copy(const SourceParametersBasic *copyfrom)
  {
    volume=copyfrom->volume;
  }

public:
  float volume;
};


class SourceParameters3D
{
public:
  SourceParameters3D() {}
  SourceParameters3D(const SourceParameters3D *copyfrom) { Copy(copyfrom); }

  ~SourceParameters3D() {}

  void Copy(const SourceParameters3D *copyfrom)
  {
    volume=copyfrom->volume;
    minimum_distance=copyfrom->minimum_distance;
    maximum_distance=copyfrom->maximum_distance;
    position.Set(copyfrom->position);
    direction.Set(copyfrom->direction);
    directional_radiation=copyfrom->directional_radiation;
    directional_cos=copyfrom->directional_cos;
  }

public:
  float directional_radiation,directional_cos;
  float volume;
  float minimum_distance, maximum_distance;
  csVector3 position, direction;
};

class SndSysSourceSoftwareFilter_Base : public iSndSysSoftwareFilter3D
{
public:
  SCF_DECLARE_IBASE;

  SndSysSourceSoftwareFilter_Base() : next_filter(NULL) {};
  virtual ~SndSysSourceSoftwareFilter_Base() {};

  virtual void Apply(iSndSysSoftwareFilter3DProperties &properties)
  {
    if (next_filter)
      next_filter->Apply(properties);
  }

  virtual bool AddSubFilter(iSndSysSoftwareFilter3D *filter, int chain_idx)
  {
    // Add the current next filter to the end of the passed filter chain
    if (next_filter)
    {
      iSndSysSoftwareFilter3D *deepest_filter=filter;
      while (deepest_filter->GetSubFilter() != NULL)
        deepest_filter=deepest_filter->GetSubFilter();

      deepest_filter->AddSubFilter(next_filter);
    }

    next_filter=filter;
    return true;
  }

  virtual iSndSysSoftwareFilter3D *GetSubFilter(int chain_idx)
  {
    return next_filter;
  }

  virtual iSndSysSoftwareFilter3D *GetPtr() { return this; }

  void Report(iReporter *reporter, int severity, const char* msg, ...)
  {
    va_list arg;
    va_start (arg, msg);

    if (reporter)
      reporter->ReportV (severity, "crystalspace.sound2.renderer.software.filter", msg, arg);
    else
    {
      csPrintfV (msg, arg);
      csPrintf ("\n");
    }
    va_end (arg);
  }

protected:
  iSndSysSoftwareFilter3D *next_filter;
};

// When used on an FIR floating average low pass filter, the number of samples determines the time over which
//  the average is applied.  The frequency of the playback stream divided by the samples should yeild the cutoff frequency.
// One downside of this is that the filter will need to process this many samples before it starts providing an even output.
// This should be avoidable by 'priming' the filter history with the first LOWPASS_SAMPLES passed in prior to generating the first
//  output sample.  This presumes the first LOWPASS_SAMPLES*2 are relatively uniform, but the filter itself relies on the relative
//  uniform distribution of sound waves anyway.
#define LOWPASS_SAMPLES 8

// 44100/128 = Thick muffle
// 44100/32 = Mild muffle
// 44100/8 = 


class SndSysSourceSoftwareFilter_LowPass : public SndSysSourceSoftwareFilter_Base
{
public:
  SndSysSourceSoftwareFilter_LowPass() : SndSysSourceSoftwareFilter_Base(), sum(0), idx(0), primed(false)
  {
    int i;
    for (i=0;i<LOWPASS_SAMPLES;i++)
      history[i]=0;
  }
  virtual ~SndSysSourceSoftwareFilter_LowPass() 
  {
  }

  void PrimeFilter(SoundSample *buffer, size_t sample_count)
  {
    size_t i;
    if (sample_count < LOWPASS_SAMPLES)
      return;
    for (i=0;i<LOWPASS_SAMPLES;i++)
    {
      history[i]=buffer[i];
      sum+=buffer[i];
    }
  }

  virtual void Apply(iSndSysSoftwareFilter3DProperties &properties)
  {
    if (!primed)
    {
      PrimeFilter(properties.work_buffer, properties.buffer_samples);
      primed=true;
    }
    size_t i;
    for (i=0;i<properties.buffer_samples;i++)
    {
      sum-=history[idx];
      history[idx]=properties.work_buffer[i];
      idx++;
      if (idx >=LOWPASS_SAMPLES) idx=0;
      sum+=properties.work_buffer[i];
      properties.work_buffer[i]=sum/LOWPASS_SAMPLES;

      /*
      // Filter contstant (1 – 2^(-shift1))
      // lastout = input + (1 - 2^-2) * (lastout – input)
      lastout+=(buffer[i] - lastout) >> 1;
      buffer[i]=lastout;
      */
    }

    if (next_filter)
      next_filter->Apply(properties);
  }

  virtual bool AddSubFilter(iSndSysSoftwareFilter3D *filter, int chain_idx)
  {
    // Add the current next filter to the end of the passed filter chain
    if (next_filter)
    {
      iSndSysSoftwareFilter3D *deepest_filter=filter;
      while (deepest_filter->GetSubFilter() != NULL)
        deepest_filter=deepest_filter->GetSubFilter();

      deepest_filter->AddSubFilter(next_filter);
    }

    next_filter=filter;
    return true;
  }

  virtual iSndSysSoftwareFilter3D *GetSubFilter(int chain_idx)
  {
    return next_filter;
  }

  virtual iSndSysSoftwareFilter3D *GetPtr() 
  {
    return this; 
  }

protected:
  SoundSample history[LOWPASS_SAMPLES];
  SoundSample sum;
  size_t idx;
  bool primed;

};



class SndSysSourceSoftwareFilter_SplitPath : public SndSysSourceSoftwareFilter_Base
{
public:
  SndSysSourceSoftwareFilter_SplitPath() : SndSysSourceSoftwareFilter_Base(), 
    second_buffer(NULL), second_buffersize(0), second_filter(NULL)
  {
  }
  virtual ~SndSysSourceSoftwareFilter_SplitPath() 
  {
    delete[] second_buffer;
  }

  void Apply(iSndSysSoftwareFilter3DProperties &properties)
  {
    iSndSysSoftwareFilter3DProperties second_props;

    if (!second_buffer || second_buffersize<properties.buffer_samples)
    {
      delete[] second_buffer;
      second_buffer=new SoundSample[properties.buffer_samples];
      second_buffersize=properties.buffer_samples;
    }
    // Copy to the second buffer
    if (second_filter)
      memcpy(second_buffer,properties.work_buffer, properties.buffer_samples * sizeof(SoundSample));

    if (second_filter)
    {
      memcpy(&second_props, &properties, sizeof(iSndSysSoftwareFilter3DProperties));
      memcpy(second_buffer, properties.work_buffer, properties.buffer_samples * sizeof(SoundSample));
      second_props.work_buffer=second_buffer;
    }

    // Call each sub filter
    if (next_filter)
      next_filter->Apply(properties);
    if (second_filter)
    {
      second_filter->Apply(second_props);

      size_t i;
      for (i=0;i<properties.buffer_samples;i++)
        properties.work_buffer[i]+=second_buffer[i];
    }
  }

  bool AddSubFilterPtr(iSndSysSoftwareFilter3D *add, iSndSysSoftwareFilter3D **spot)
  {
    if (*spot)
    {
      iSndSysSoftwareFilter3D *deepest_filter=add;
      while (deepest_filter->GetSubFilter() != NULL)
        deepest_filter=deepest_filter->GetSubFilter();

      deepest_filter->AddSubFilter(*spot);
    }
    *spot=add;
    return true;
  }

  virtual bool AddSubFilter(iSndSysSoftwareFilter3D *filter, int chain_idx)
  {
    if (chain_idx==0)
      return AddSubFilterPtr(filter,&next_filter);
    if (chain_idx==1)
      return AddSubFilterPtr(filter,&second_filter);
    return false;
  }

  virtual iSndSysSoftwareFilter3D *GetSubFilter(int chain_idx)
  {
    if (chain_idx==0)
      return next_filter;
    if (chain_idx==1)
      return second_filter;
    return NULL;
  }

protected:
  SoundSample *second_buffer;
  size_t second_buffersize;
  iSndSysSoftwareFilter3D *second_filter;
};


class SndSysSourceSoftwareFilter_ITDDelay : public SndSysSourceSoftwareFilter_Base
{
public:
  SndSysSourceSoftwareFilter_ITDDelay() : SndSysSourceSoftwareFilter_Base(), history_buffer(NULL), history_samples(0)
  {
  }
  virtual ~SndSysSourceSoftwareFilter_ITDDelay()
  {
  }

  bool SetupHistoryBuffer(int frequency)
  {
    delete[] history_buffer;
    history_samples=frequency;
    history_buffer=new SoundSample[history_samples];
    if (!history_buffer)
    {
      history_samples=0;
      return false;
    }
    memset(history_buffer,0,sizeof(SoundSample) * history_samples);
    return true;
  }


  void Apply(iSndSysSoftwareFilter3DProperties &properties)
  {
    if (!history_buffer && !SetupHistoryBuffer(properties.sound_format->Freq))
      return;

    // shift the history buffer
    size_t history_shift=properties.buffer_samples;
    if (history_shift > history_samples)
      history_shift=history_samples;

    //Report(properties.reporter, CS_REPORTER_SEVERITY_DEBUG, "History shift is %u", history_shift);

    memcpy(history_buffer, &(history_buffer[history_shift]), sizeof(SoundSample) * (history_samples - history_shift));
    memcpy(&(history_buffer[history_samples-history_shift]), properties.work_buffer, sizeof(SoundSample) * history_shift);

    // Calculate the delay for this channel, this is based off difference in distance between the closest channel and this channel
    float delay_dist=properties.speaker_distance[properties.channel]-properties.closest_speaker_distance;
    float time=delay_dist / 331.4f;
    float fsamples=time * properties.sound_format->Freq;
    size_t delay_samples = fsamples;

    if (delay_samples<1)
      delay_samples=0;
    if (delay_samples>history_samples)
      delay_samples=history_samples;
    if (delay_samples>properties.buffer_samples)
      delay_samples=properties.buffer_samples;

    //Report(properties.reporter, CS_REPORTER_SEVERITY_DEBUG, "Delay_samples is %u", delay_samples);

    if (delay_samples>0)
    {
      if (history_shift + delay_samples > history_samples)
        delay_samples=history_samples- history_shift;
      if (delay_samples < properties.buffer_samples)
      {
        size_t idx;
        for (idx=properties.buffer_samples-1;idx>=delay_samples;idx--)
          properties.work_buffer[idx]=properties.work_buffer[idx-delay_samples];
        //memcpy(&(properties.work_buffer[delay_samples]), properties.work_buffer, (properties.buffer_samples- delay_samples) * sizeof(SoundSample));
      }
      memcpy(properties.work_buffer, &(history_buffer[history_samples-(history_shift + delay_samples)]), delay_samples * sizeof(SoundSample));
    }

    if (next_filter)
      next_filter->Apply(properties);
  }
protected:
  SoundSample *history_buffer;
  size_t history_samples;
};

class SndSysSourceSoftwareFilter_Delay : public SndSysSourceSoftwareFilter_Base
{
public:
  SndSysSourceSoftwareFilter_Delay() : SndSysSourceSoftwareFilter_Base(), history_buffer(NULL), history_samples(0)
  {
  }
  virtual ~SndSysSourceSoftwareFilter_Delay()
  {
  }

  void SetDelayTime(float sec)
  {
    delay_time=sec;
  }

  bool SetupHistoryBuffer(int frequency)
  {
    delete[] history_buffer;
    history_samples=frequency;
    history_buffer=new SoundSample[history_samples];
    if (!history_buffer)
    {
      history_samples=0;
      return false;
    }
    memset(history_buffer,0,sizeof(SoundSample) * history_samples);
    return true;
  }


  void Apply(iSndSysSoftwareFilter3DProperties &properties)
  {
    if (!history_buffer && !SetupHistoryBuffer(properties.sound_format->Freq))
      return;

    // shift the history buffer
    size_t history_shift=properties.buffer_samples;
    if (history_shift > history_samples)
      history_shift=history_samples;

    //Report(properties.reporter, CS_REPORTER_SEVERITY_DEBUG, "History shift is %u", history_shift);

    memcpy(history_buffer, &(history_buffer[history_shift]), sizeof(SoundSample) * (history_samples - history_shift));
    memcpy(&(history_buffer[history_samples-history_shift]), properties.work_buffer, sizeof(SoundSample) * history_shift);

    // Calculate the delay for this channel
    float fsamples=delay_time * properties.sound_format->Freq;
    size_t delay_samples = fsamples;

    if (delay_samples<1)
      delay_samples=0;
    if (delay_samples>history_samples)
      delay_samples=history_samples;
    if (delay_samples>properties.buffer_samples)
      delay_samples=properties.buffer_samples;

    //Report(properties.reporter, CS_REPORTER_SEVERITY_DEBUG, "Delay_samples is %u", delay_samples);

    if (delay_samples>0)
    {
      if (history_shift + delay_samples > history_samples)
        delay_samples=history_samples- history_shift;
      if (delay_samples < properties.buffer_samples)
      {
        size_t idx;
        for (idx=properties.buffer_samples-1;idx>=delay_samples;idx--)
          properties.work_buffer[idx]=properties.work_buffer[idx-delay_samples];
        //memcpy(&(properties.work_buffer[delay_samples]), properties.work_buffer, (properties.buffer_samples- delay_samples) * sizeof(SoundSample));
      }
      memcpy(properties.work_buffer, &(history_buffer[history_samples-(history_shift + delay_samples)]), delay_samples * sizeof(SoundSample));
    }

    if (next_filter)
      next_filter->Apply(properties);
  }
protected:
  SoundSample *history_buffer;
  size_t history_samples;
  float delay_time;
};

class SndSysSourceSoftwareFilter_Reverb : public SndSysSourceSoftwareFilter_Base
{
public:
  SndSysSourceSoftwareFilter_Reverb() : SndSysSourceSoftwareFilter_Base(), history_buffer(NULL), history_samples(0)
  {
  }
  virtual ~SndSysSourceSoftwareFilter_Reverb()
  {
  }

  bool SetupHistoryBuffer(int frequency)
  {
    delete[] history_buffer;
    history_samples=frequency;
    history_buffer=new SoundSample[history_samples];
    if (!history_buffer)
    {
      history_samples=0;
      return false;
    }
    memset(history_buffer,0,sizeof(SoundSample) * history_samples);
    return true;
  }


  void Apply(iSndSysSoftwareFilter3DProperties &properties)
  {
    if (!history_buffer && !SetupHistoryBuffer(properties.sound_format->Freq))
      return;

    // shift the history buffer
    size_t history_shift=properties.buffer_samples;
    if (history_shift > history_samples)
      history_shift=history_samples;

    //Report(properties.reporter, CS_REPORTER_SEVERITY_DEBUG, "History shift is %u", history_shift);

    memcpy(history_buffer, &(history_buffer[history_shift]), sizeof(SoundSample) * (history_samples - history_shift));
    memcpy(&(history_buffer[history_samples-history_shift]), properties.work_buffer, sizeof(SoundSample) * history_shift);

    // Calculate the delay for this channel
    float delay_time=0.01f;
    float delay_intensity_factor=0.2f;

    while (delay_time<0.1f)
    {
      float fsamples=delay_time * properties.sound_format->Freq;
      size_t delay_samples = fsamples;

      if (delay_samples<1)
        delay_samples=0;
      if (delay_samples>history_samples)
        delay_samples=history_samples;
      if (delay_samples>properties.buffer_samples)
        delay_samples=properties.buffer_samples;

      //Report(properties.reporter, CS_REPORTER_SEVERITY_DEBUG, "Delay_samples is %u", delay_samples);

      if (delay_samples>0)
      {
        if (history_shift + delay_samples > history_samples)
          delay_samples=history_samples- history_shift;
        if (delay_samples < properties.buffer_samples)
        {
          size_t idx;
          for (idx=1;idx<properties.buffer_samples;idx++)
          {
            float samp_vol=history_buffer[history_samples-(delay_samples + history_shift)];
            samp_vol*=delay_intensity_factor;
            properties.work_buffer[idx]+=samp_vol;
          }
          //memcpy(&(properties.work_buffer[delay_samples]), properties.work_buffer, (properties.buffer_samples- delay_samples) * sizeof(SoundSample));
        }
        //memcpy(properties.work_buffer, &(history_buffer[history_samples-(history_shift + delay_samples)]), delay_samples * sizeof(SoundSample));
      }
      delay_time=delay_time*2.0f;
      delay_intensity_factor=delay_intensity_factor/2.0f;
    }

    if (next_filter)
      next_filter->Apply(properties);
  }
protected:
  SoundSample *history_buffer;
  size_t history_samples;
};


class SndSysSourceSoftwareFilter_IID : public SndSysSourceSoftwareFilter_Base
{
public:
  SndSysSourceSoftwareFilter_IID() : SndSysSourceSoftwareFilter_Base()
  {
//    memset(debug_cycle, 0, sizeof(int) * 16);
  }
  virtual ~SndSysSourceSoftwareFilter_IID()
  {
  }


  void Apply(iSndSysSoftwareFilter3DProperties &properties)
  {
    float vol;
    int int_vol;
    size_t i;

    // Turn distance into units based off minimum distance
    float minimum_distance=properties.source_parameters->minimum_distance;
    if (minimum_distance < 0.000001f)
      minimum_distance=0.000001f;
    float iid_distance=properties.speaker_distance[properties.channel]/minimum_distance;

    // The minimum distance is 1 minimum distance unit
    if (iid_distance < 1.0f) iid_distance=1.0f;

    // The rolloff factor is applied as a factor to the natural rolloff power
    float rollofffactor=properties.listener_parameters->rolloff_factor;

    // Adjustments based on the direction the sound is coming in compared to the direction of the receiving speaker
    // Temporary - no adjustment
    vol =  properties.source_parameters->volume;

    // Apply the rolloff factor to the volume
    if (rollofffactor != 1.0f)
      vol/=pow(iid_distance,rollofffactor);
    else
      vol/=iid_distance;

    int_vol=(SOURCE_INTEGER_VOLUME_MULTIPLE * vol);

    /*
    if (debug_cycle[properties.channel]++ >=20)
    {
      properties.reporter->Report(CS_REPORTER_SEVERITY_DEBUG, "crystalspace.SndSys.renderer.software", "Channel %d has integer volume factor %d", 
                                  properties.channel, int_vol);
      debug_cycle[properties.channel]=0;
    }
    */

    for (i=0;i<properties.buffer_samples;i++)
      properties.work_buffer[i]=(properties.work_buffer[i] * int_vol) / SOURCE_INTEGER_VOLUME_MULTIPLE;
  
    if (next_filter)
      next_filter->Apply(properties);
  }

//  int debug_cycle[16];
};


class SndSysSourceSoftwareFilter_DirectFade : public SndSysSourceSoftwareFilter_Base
{
public:
  SndSysSourceSoftwareFilter_DirectFade() : SndSysSourceSoftwareFilter_Base()
  {
    cos_far=cos(7*PI / 16);
    cos_near=cos( PI / 4);

    range=cos_near-cos_far;
  }
  virtual ~SndSysSourceSoftwareFilter_DirectFade()
  {
  }


  void Apply(iSndSysSoftwareFilter3DProperties &properties)
  {

    if (properties.speaker_direction_cos[properties.channel] <=cos_far)
    {
      // Not a direct sound, clip, do not call further chains
      memset(properties.work_buffer, 0, sizeof(SoundSample) * properties.buffer_samples);
      return;
    }
    if (properties.speaker_direction_cos[properties.channel] <=cos_near)
    {
      float vol=(properties.speaker_direction_cos[properties.channel]-cos_far) / range;

      size_t i;
      for (i=0;i<properties.buffer_samples;i++)
        properties.work_buffer[i]*=vol;
    }

    if (next_filter)
      next_filter->Apply(properties);
  }
protected:
  float cos_far, cos_near, range;
};



class SndSysSourceSoftwareBasic : public iSndSysSourceSoftware
{
public:
  SCF_DECLARE_IBASE;

  SndSysSourceSoftwareBasic(csRef<iSndSysStream> stream, SndSysRendererSoftware *rend);
  virtual ~SndSysSourceSoftwareBasic();

  /// Set volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
  virtual void SetVolume (float volume);
  /// Get volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
  virtual float GetVolume ();

  /// Retrieve the iSoundStream attached to this source
  virtual csRef<iSndSysStream> GetStream() { return sound_stream; }

  /// Attach a filter to this source
//  virtual bool AttachFilter(csRef<iSndSysFilter> filter);

  /// Remove a filter from this source
//  virtual bool RemoveFilter(csRef<iSndSysFilter> filter);

  /// Retrieve a direct pointer to this object
  virtual iSndSysSource *GetPtr() { return this; }

  virtual size_t MergeIntoBuffer(SoundSample *channel_buffer, size_t buffer_samples);

protected:
  void UpdateQueuedParameters();

protected:
  SndSysRendererSoftware *renderer;
  csRef<iSndSysStream> sound_stream;
  long stream_position;

  SourceParametersBasic active_parameters,queued_parameters;
  bool queued_updates;

};

class SndSysSourceSoftware3D : public iSndSysSourceSoftware3D
{
public:
  SCF_DECLARE_IBASE;

  SndSysSourceSoftware3D(csRef<iSndSysStream> stream, SndSysRendererSoftware *rend);
  virtual ~SndSysSourceSoftware3D();

  /// Set volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
  virtual void SetVolume (float volume);
  /// Get volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
  virtual float GetVolume ();

  /// Retrieve the iSoundStream attached to this source
  virtual csRef<iSndSysStream> GetStream() { return sound_stream; }

  /// set position of this source
  virtual void SetPosition(csVector3 pos);
  /// get position of this source
  virtual csVector3 GetPosition();
  /// set velocity of this source

  /// set position of this source
  virtual void SetDirection(csVector3 dir);
  /// get position of this source
  virtual csVector3 GetDirection();

  /// The directional radiation applies to sound that are oriented in a particular direction.
  //   This value is expressed in radians and describes the half-angle of a cone spreading from the position of the source and opening
  //   in the direction of the source.
  //  Set this value to 0.0f for an omni-directional sound. 
  virtual void SetDirectionalRadiation(float rad);

  // Retrieves the current directional radiation 
  virtual float GetDirectionalRadiation();


  /** Set the greatest distance from a sound at which the sound plays at full amplitude. 
  *   When a listener is closer than this distance, the amplitude is the volume of the sound.
  *   When a listener is further than this distance, the amplitude follows the formula V = (volume / ((distance/minimum_distance) ^ rolloff_factor))
  */
  virtual void SetMinimumDistance (float distance);

  /** Set the greatest distance from a sound at which the sound can be heard.
  *   If the distance to a listener is above this threshold, it will not be mixed into the output buffer at all.  This saves a tiny bit of processing.
  */
  virtual void SetMaximumDistance (float distance);

  /// Retrieve the maximum distance for which a sound is heard at full volume.  See SetMaximumDistance and SetMinimumDistance for distance notes.
  virtual float GetMinimumDistance();

  /// Retrieve the maximum distance for which a sound can be heard.  See SetMaximumDistance and SetMinimumDistance for distance notes.
  virtual float GetMaximumDistance();


  /// Retrieve a direct pointer to this object
  virtual iSndSysSource *GetPtr() { return this; }

  virtual size_t MergeIntoBuffer(SoundSample *channel_buffer, size_t buffer_samples);


protected:
  void UpdateQueuedParameters();

  /// Returns false if enough buffer space for a mix buffer can't be allocated
  //inline bool CheckMixBufferSize(long samples);

  /// Returns true if there are user defined filters attached to this source
  //inline bool HaveFilters() { return false; }

  //void ProcessFilterQueues();

  inline bool PrepareBuffer(SoundSample **p_buf, size_t *p_buf_len, size_t required);
  inline void ClearBuffer(SoundSample *p_buf, size_t p_buf_len);
  bool ProcessSoundChain(int channel, size_t buffer_samples);


  void SetupFilters();

protected:
  SndSysRendererSoftware *renderer;

  csRef<iSndSysStream> sound_stream;
  long stream_position;

  SourceParameters3D active_parameters,queued_parameters;
  bool queued_updates;

  /// The working buffer is where the samples from one channel at a time are manipulated
  SoundSample *clean_buffer;
  size_t clean_buffer_samples;

  /// The working buffer is where the samples from one channel at a time are manipulated
  SoundSample *working_buffer;
  size_t working_buffer_samples;

  /// The historic buffer contains samples that were previously delivered for use in effects
  //SoundSample *historic_buffer;
  //size_t historic_buffer_samples;

  /// The distance from the closest channel position to the source
  float closest_speaker;
  float speaker_distance[MAX_CHANNELS], speaker_direction_cos[MAX_CHANNELS];
  iSndSysSoftwareFilter3D *speaker_filter_chains[MAX_CHANNELS];
  bool filters_setup;


  //int debug_cycle;

  //Queue<iSndSysFilter> filter_add_queue,filter_remove_queue,filter_clear_queue;
  //csArray<iSndSysFilter *> filters;
};








#endif // #ifndef SNDSYS_RENDERER_SOFTWARE_SOURCE_H




