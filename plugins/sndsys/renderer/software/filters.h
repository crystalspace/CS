/*
  Copyright (C) 2006 by Andrew Mann

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


#ifndef SNDSYS_RENDERER_SOFTWARE_FILTERS_H
#define SNDSYS_RENDERER_SOFTWARE_FILTERS_H

/*  Basic Filters for the Software Sound Renderer
 *
 *  These filters are not final filters.  They're just here until better ones are made.
 *
 *
*/
#include "csutil/scf_implementation.h"

#include "isndsys/ss_structs.h"
#include "isndsys/ss_filter.h"

#include "source.h"
#include "listener.h"

using namespace CS::SndSys;


/// A base class for software source filters.
//
//  This is a work in progress.  Expect rough edges.
//
//  Filters are intended to be a series of modification operations performed on a sound stream.
//  The theory is that filters might provide a more flexible way of defining both a series of 3d sound environments
//   from the source to the listener and an opportunity to insert other, non-traditional effects if desired.
//
//  The filter interface is not fully fleshed out, and the filters currently used by the 3D source are experimental
//
//
class SndSysSourceSoftwareFilter_Base :
  public scfImplementation1<SndSysSourceSoftwareFilter_Base, iSndSysSoftwareFilter3D>
{
public:

  SndSysSourceSoftwareFilter_Base() :
      scfImplementationType(this)
      {
      };
      virtual ~SndSysSourceSoftwareFilter_Base() 
      {
      };

      virtual void Apply(iSndSysSoftwareFilter3DProperties &properties)
      {
        if (next_filter)
          next_filter->Apply(properties);
      }

      /// Add a filter after this filter in the chain.
      //   The chain_idx parameter can be used to identify which sub-chain the filter
      //   should be added to.  It's ignored for the base class, since we have only
      //   one sub-chain.
      virtual bool AddSubFilter(iSndSysSoftwareFilter3D *filter, int /*chain_idx*/)
      {
        // Add the current next filter to the end of the passed filter chain
        if (next_filter)
        {
          // Find the furthest filter down the chain.
          iSndSysSoftwareFilter3D *deepest_filter=filter;
          while (deepest_filter->GetSubFilter() != 0)
            deepest_filter=deepest_filter->GetSubFilter();

          deepest_filter->AddSubFilter(next_filter);
        }

        next_filter=filter;
        return true;
      }

      virtual iSndSysSoftwareFilter3D *GetSubFilter(int /*chain_idx*/)
      {
        return next_filter;
      }

      virtual iSndSysSoftwareFilter3D *GetPtr() { return this; }


protected:
  csRef<iSndSysSoftwareFilter3D> next_filter;
};

// When used on an FIR floating average low pass filter, the number of
// samples determines the time over which the average is applied.  The
// frequency of the playback stream divided by the samples should yield
// the cutoff frequency. One downside of this is that the filter will need
// to process this many samples before it starts providing an even output.
// This should be avoidable by 'priming' the filter history with the first
// LOWPASS_SAMPLES passed in prior to generating the first
// output sample.  This presumes the first LOWPASS_SAMPLES*2 are relatively
// uniform, but the filter itself relies on the relative uniform
// distribution of sound waves anyway.
#define LOWPASS_SAMPLES 8

// 44100/128 = Thick muffle
// 44100/32 = Mild muffle
// 44100/8 = 


class SndSysSourceSoftwareFilter_LowPass :
  public SndSysSourceSoftwareFilter_Base
{
public:
  SndSysSourceSoftwareFilter_LowPass() :
      SndSysSourceSoftwareFilter_Base(), 
        sum(0), idx(0), primed(false)
      {
        int i;
        for (i=0;i<LOWPASS_SAMPLES;i++)
          history[i]=0;
      }
      virtual ~SndSysSourceSoftwareFilter_LowPass() 
      {
      }

      void PrimeFilter(csSoundSample *buffer, size_t sample_count)
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


      virtual iSndSysSoftwareFilter3D *GetPtr() 
      {
        return this; 
      }

protected:
  csSoundSample history[LOWPASS_SAMPLES];
  csSoundSample sum;
  size_t idx;
  bool primed;
};



class SndSysSourceSoftwareFilter_SplitPath :
  public SndSysSourceSoftwareFilter_Base
{
public:
  SndSysSourceSoftwareFilter_SplitPath() : SndSysSourceSoftwareFilter_Base(), 
    second_buffer(0), second_buffersize(0)
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
      second_buffer=new csSoundSample[properties.buffer_samples];
      second_buffersize=properties.buffer_samples;
    }
    // Copy to the second buffer
    if (second_filter)
      memcpy(second_buffer,properties.work_buffer,
      properties.buffer_samples * sizeof(csSoundSample));

    if (second_filter)
    {
      memcpy(&second_props, &properties,
        sizeof(iSndSysSoftwareFilter3DProperties));
      memcpy(second_buffer, properties.work_buffer,
        properties.buffer_samples * sizeof(csSoundSample));
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

  virtual bool AddSubFilter(iSndSysSoftwareFilter3D *filter, int chain_idx)
  {
    if (chain_idx==0)
    {
      iSndSysSoftwareFilter3D *deepest_filter=filter;
      while (deepest_filter->GetSubFilter() != 0)
        deepest_filter=deepest_filter->GetSubFilter();

      deepest_filter->AddSubFilter(next_filter);
      next_filter=filter;
      return true;
    }
    if (chain_idx==1)
    {
      iSndSysSoftwareFilter3D *deepest_filter=filter;
      while (deepest_filter->GetSubFilter() != 0)
        deepest_filter=deepest_filter->GetSubFilter();

      deepest_filter->AddSubFilter(second_filter);
      second_filter=filter;
      return true;
    }
    return false;
  }

  virtual iSndSysSoftwareFilter3D *GetSubFilter(int chain_idx)
  {
    if (chain_idx==0)
      return next_filter;
    if (chain_idx==1)
      return second_filter;
    return 0;
  }

protected:
  csSoundSample *second_buffer;
  size_t second_buffersize;
  csRef<iSndSysSoftwareFilter3D> second_filter;
};


class SndSysSourceSoftwareFilter_ITDDelay :
  public SndSysSourceSoftwareFilter_Base
{
public:
  SndSysSourceSoftwareFilter_ITDDelay() :
      SndSysSourceSoftwareFilter_Base(), history_buffer(0), history_samples(0)
      {
      }
      virtual ~SndSysSourceSoftwareFilter_ITDDelay()
      {
        delete[] history_buffer;
      }

      bool SetupHistoryBuffer(int frequency)
      {
        delete[] history_buffer;
        history_samples=frequency;
        history_buffer=new csSoundSample[history_samples];
        if (!history_buffer)
        {
          history_samples=0;
          return false;
        }
        memset(history_buffer,0,sizeof(csSoundSample) * history_samples);
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

        //Report(properties.reporter, CS_REPORTER_SEVERITY_DEBUG, 
        //"History shift is %u", history_shift);

        memcpy(history_buffer, &(history_buffer[history_shift]), 
          sizeof(csSoundSample) * (history_samples - history_shift));
        memcpy(&(history_buffer[history_samples-history_shift]), 
          properties.work_buffer, sizeof(csSoundSample) * history_shift);

        /* Calculate the delay for this channel, this is based off difference in 
        * distance between the closest channel and this channel */
        float delay_dist = 
          properties.speaker_distance[properties.channel] -
          properties.closest_speaker_distance;
        float time = delay_dist / 331.4f;
        float fsamples = time * properties.sound_format->Freq;
        size_t delay_samples = (size_t)fsamples;

        if (delay_samples<1)
          delay_samples=0;
        if (delay_samples>history_samples)
          delay_samples=history_samples;
        if (delay_samples>properties.buffer_samples)
          delay_samples=properties.buffer_samples;

        //Report(properties.reporter, CS_REPORTER_SEVERITY_DEBUG, 
        //"Delay_samples is %u", delay_samples);

        if (delay_samples>0)
        {
          if (history_shift + delay_samples > history_samples)
            delay_samples=history_samples- history_shift;
          if (delay_samples < properties.buffer_samples)
          {
            size_t idx;
            for (idx=properties.buffer_samples-1;idx>=delay_samples;idx--)
              properties.work_buffer[idx]=properties.work_buffer[idx-delay_samples];
            //memcpy(&(properties.work_buffer[delay_samples]), 
            //properties.work_buffer, 
            //(properties.buffer_samples- delay_samples) * sizeof(csSoundSample));
          }
          memcpy(properties.work_buffer, 
            &(history_buffer[history_samples-(history_shift + delay_samples)]), 
            delay_samples * sizeof(csSoundSample));
        }

        if (next_filter)
          next_filter->Apply(properties);
      }
protected:
  csSoundSample *history_buffer;
  size_t history_samples;
};

class SndSysSourceSoftwareFilter_Delay : public SndSysSourceSoftwareFilter_Base
{
public:
  SndSysSourceSoftwareFilter_Delay() : SndSysSourceSoftwareFilter_Base(), 
    history_buffer(0), history_samples(0)
  {
  }
  virtual ~SndSysSourceSoftwareFilter_Delay()
  {
    delete[] history_buffer;
  }

  void SetDelayTime(float sec)
  {
    delay_time=sec;
  }

  bool SetupHistoryBuffer(int frequency)
  {
    delete[] history_buffer;
    history_samples=frequency;
    history_buffer=new csSoundSample[history_samples];
    if (!history_buffer)
    {
      history_samples=0;
      return false;
    }
    memset(history_buffer,0,sizeof(csSoundSample) * history_samples);
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

    //Report(properties.reporter, CS_REPORTER_SEVERITY_DEBUG, 
    //"History shift is %u", history_shift);

    memcpy(history_buffer, &(history_buffer[history_shift]), 
      sizeof(csSoundSample) * (history_samples - history_shift));
    memcpy(&(history_buffer[history_samples-history_shift]), 
      properties.work_buffer, sizeof(csSoundSample) * history_shift);

    // Calculate the delay for this channel
    float fsamples = delay_time * properties.sound_format->Freq;
    size_t delay_samples = (size_t)fsamples;

    if (delay_samples<1)
      delay_samples=0;
    if (delay_samples>history_samples)
      delay_samples=history_samples;
    if (delay_samples>properties.buffer_samples)
      delay_samples=properties.buffer_samples;

    //Report(properties.reporter, CS_REPORTER_SEVERITY_DEBUG, 
    //"Delay_samples is %u", delay_samples);

    if (delay_samples>0)
    {
      if (history_shift + delay_samples > history_samples)
        delay_samples=history_samples- history_shift;
      if (delay_samples < properties.buffer_samples)
      {
        size_t idx;
        for (idx=properties.buffer_samples-1;idx>=delay_samples;idx--)
          properties.work_buffer[idx]=properties.work_buffer[idx-delay_samples];
        //memcpy(&(properties.work_buffer[delay_samples]), 
        //properties.work_buffer, 
        //(properties.buffer_samples- delay_samples) * sizeof(csSoundSample));
      }
      memcpy(properties.work_buffer, 
        &(history_buffer[history_samples-(history_shift + delay_samples)]), 
        delay_samples * sizeof(csSoundSample));
    }

    if (next_filter)
      next_filter->Apply(properties);
  }
protected:
  csSoundSample *history_buffer;
  size_t history_samples;
  float delay_time;
};

class SndSysSourceSoftwareFilter_Reverb : public SndSysSourceSoftwareFilter_Base
{
public:
  SndSysSourceSoftwareFilter_Reverb() : SndSysSourceSoftwareFilter_Base(), 
    history_buffer(0), history_samples(0)
  {
  }
  virtual ~SndSysSourceSoftwareFilter_Reverb()
  {
    delete[] history_buffer;
  }

  bool SetupHistoryBuffer(int frequency)
  {
    delete[] history_buffer;
    history_samples=frequency;
    history_buffer=new csSoundSample[history_samples];
    if (!history_buffer)
    {
      history_samples=0;
      return false;
    }
    memset(history_buffer,0,sizeof(csSoundSample) * history_samples);
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

    //Report(properties.reporter, CS_REPORTER_SEVERITY_DEBUG, 
    //"History shift is %u", history_shift);

    memcpy(history_buffer, &(history_buffer[history_shift]), 
      sizeof(csSoundSample) * (history_samples - history_shift));
    memcpy(&(history_buffer[history_samples-history_shift]), 
      properties.work_buffer, sizeof(csSoundSample) * history_shift);

    // Calculate the delay for this channel
    float delay_time=0.01f;
    float delay_intensity_factor=0.2f;

    while (delay_time<0.1f)
    {
      float fsamples=delay_time * properties.sound_format->Freq;
      size_t delay_samples = (size_t)fsamples;

      if (delay_samples<1)
        delay_samples=0;
      if (delay_samples>history_samples)
        delay_samples=history_samples;
      if (delay_samples>properties.buffer_samples)
        delay_samples=properties.buffer_samples;

      //Report(properties.reporter, CS_REPORTER_SEVERITY_DEBUG, 
      //"Delay_samples is %u", delay_samples);

      if (delay_samples>0)
      {
        if (history_shift + delay_samples > history_samples)
          delay_samples=history_samples- history_shift;
        if (delay_samples < properties.buffer_samples)
        {
          size_t idx;
          for (idx=1;idx<properties.buffer_samples;idx++)
          {
            float samp_vol = 
              history_buffer[history_samples-(delay_samples + history_shift)];
            samp_vol*=delay_intensity_factor;
            properties.work_buffer[idx] += (int)samp_vol;
          }
          //memcpy(&(properties.work_buffer[delay_samples]), 
          //properties.work_buffer, 
          //(properties.buffer_samples-delay_samples)*sizeof(csSoundSample));
        }
        //memcpy(properties.work_buffer, 
        //&(history_buffer[history_samples-(history_shift + delay_samples)]), 
        //delay_samples * sizeof(csSoundSample));
      }
      delay_time=delay_time*2.0f;
      delay_intensity_factor=delay_intensity_factor/2.0f;
    }

    if (next_filter)
      next_filter->Apply(properties);
  }
protected:
  csSoundSample *history_buffer;
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
    float iid_distance = 
      properties.speaker_distance[properties.channel]/minimum_distance;

    // The minimum distance is 1 minimum distance unit
    if (iid_distance < 1.0f) iid_distance=1.0f;

    // The rolloff factor is applied as a factor to the natural rolloff power
    float rollofffactor = properties.listener_parameters->rolloff_factor;

    // Adjustments based on the direction the sound is coming in compared to 
    // the direction of the receiving speaker
    // Temporary - no adjustment
    vol =  properties.source_parameters->volume;

    // Apply the rolloff factor to the volume
    if (rollofffactor != 1.0f)
      vol/=pow(iid_distance,rollofffactor);
    else
      vol/=iid_distance;

    int_vol = (int)(SOURCE_INTEGER_VOLUME_MULTIPLE * vol);

    /*
    if (debug_cycle[properties.channel]++ >=20)
    {
    properties.reporter->Report(CS_REPORTER_SEVERITY_DEBUG, 
    "crystalspace.SndSys.renderer.software", 
    "Channel %d has integer volume factor %d", 
    properties.channel, int_vol);
    debug_cycle[properties.channel]=0;
    }
    */

    for (i=0;i<properties.buffer_samples;i++)
      properties.work_buffer[i]=(properties.work_buffer[i] * int_vol)
      / SOURCE_INTEGER_VOLUME_MULTIPLE;

    if (next_filter)
      next_filter->Apply(properties);
  }

  //  int debug_cycle[16];
};


class SndSysSourceSoftwareFilter_DirectFade :
  public SndSysSourceSoftwareFilter_Base
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
      memset(properties.work_buffer, 0, 
        sizeof(csSoundSample) * properties.buffer_samples);
      return;
    }
    if (properties.speaker_direction_cos[properties.channel] <=cos_near)
    {
      float vol = 
        (properties.speaker_direction_cos[properties.channel]-cos_far) / range;

      size_t i;
      for (i=0;i<properties.buffer_samples;i++)
        properties.work_buffer[i] = 
        (int)((float)properties.work_buffer[i] * vol);
    }

    if (next_filter)
      next_filter->Apply(properties);
  }
protected:
  float cos_far, cos_near, range;
};









#endif // #ifndef SNDSYS_RENDERER_SOFTWARE_FILTERS_H
