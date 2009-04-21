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


#include "isndsys/ss_filter.h"
#include "isndsys/ss_source.h"
#include "isndsys/ss_stream.h"

#include "filterqueue.h"

using namespace CS::SndSys;


#define MAX_CHANNELS 18

#define SOURCE_INTEGER_VOLUME_MULTIPLE 1024

#define SOURCE_3D_BUFFER_TIME_MS 300


struct iSndSysStream;
class csSndSysRendererSoftware;


class csSourceParametersBasic
{
public:
  csSourceParametersBasic() {}
  csSourceParametersBasic(const csSourceParametersBasic *copyfrom)
  {
    Copy(copyfrom);
  }

  ~csSourceParametersBasic() {}

  void Copy(const csSourceParametersBasic *copyfrom)
  {
    volume=copyfrom->volume;
  }

public:
  float volume;
};


class csSourceParameters3D
{
public:
  csSourceParameters3D() {}
  csSourceParameters3D(const csSourceParameters3D *copyfrom) { Copy(copyfrom); }

  ~csSourceParameters3D() {}

  void Copy(const csSourceParameters3D *copyfrom)
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




class SndSysSourceSoftwareBasic :
  public scfImplementation2<SndSysSourceSoftwareBasic, 
                            scfFakeInterface<iSndSysSource>,
                            iSndSysSourceSoftware >
{
public:
  SndSysSourceSoftwareBasic(csRef<iSndSysStream> stream, 
    csSndSysRendererSoftware *rend);
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

  /**
   * Add an output filter at the specified location.
   * Output filters can only receive sound data and cannot modify it.
   * They will receive data from the same thread that the CS event handler
   * executes in, once per frame.
   *
   *  Valid Locations:  SS_FILTER_LOC_SOURCEOUT, SS_FILTER_LOC_SOURCEIN
   *  
   * Returns FALSE if the filter could not be added.
   */
  virtual bool AddOutputFilter(SndSysFilterLocation Location,
      iSndSysSoftwareOutputFilter *pFilter);

  /**
   * Remove an output filter from the registered list
   * 
   * Valid Locations:  SS_FILTER_LOC_SOURCEOUT, SS_FILTER_LOC_SOURCEIN
   * 
   * Returns FALSE if the filter is not in the list at the time of the call.
   */
  virtual bool RemoveOutputFilter(SndSysFilterLocation Location,
      iSndSysSoftwareOutputFilter *pFilter);


  virtual size_t MergeIntoBuffer(csSoundSample *frame_buffer,
    size_t frame_count);

  /**
   * Renderer convenience interface - Called to provide processing of
   * output filters
   */
  virtual void ProcessOutputFilters();

protected:
  void UpdateQueuedParameters();

protected:
  csSndSysRendererSoftware *renderer;
  csRef<iSndSysStream> sound_stream;
  size_t stream_position;

  csSourceParametersBasic active_parameters,queued_parameters;
  bool queued_updates;

  ////
  //  OutputFilter Queues
  ////
  SndSysOutputFilterQueue m_SourceOutFilterQueue;
  SndSysOutputFilterQueue m_SourceInFilterQueue;

};

class SndSysSourceSoftware3D :
  public scfImplementation4<SndSysSourceSoftware3D,
                            scfFakeInterface<iSndSysSource>,
                            iSndSysSource3D,
                            iSndSysSource3DDirectionalSimple,
                            iSndSysSourceSoftware >
{
public:

  SndSysSourceSoftware3D(csRef<iSndSysStream> stream, 
    csSndSysRendererSoftware *rend);
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

  /**
   * The directional radiation applies to sound that are oriented in a 
   * particular direction.
   * This value is expressed in radians and describes the half-angle of a cone 
   * spreading from the position of the source and opening in the direction of 
   * the source.
   * Set this value to 0.0f for an omni-directional sound. 
   */
  virtual void SetDirectionalRadiation(float rad);

  // Retrieves the current directional radiation 
  virtual float GetDirectionalRadiation();


  /** 
   * Set the greatest distance from a sound at which the sound plays at full 
   * amplitude. 
   * When a listener is closer than this distance, the amplitude is the volume 
   * of the sound.
   * When a listener is further than this distance, the amplitude follows the 
   * formula V = (volume / ((distance/minimum_distance) ^ rolloff_factor))
   */
  virtual void SetMinimumDistance (float distance);

  /** Set the greatest distance from a sound at which the sound can be heard.
   * If the distance to a listener is above this threshold, it will not be 
   * mixed into the output buffer at all.  This saves a tiny bit of processing.
   */
  virtual void SetMaximumDistance (float distance);

  /**
   * Retrieve the maximum distance for which a sound is heard at full volume.  
   * See SetMaximumDistance and SetMinimumDistance for distance notes.
   */
  virtual float GetMinimumDistance();

  /** Retrieve the maximum distance for which a sound can be heard.  
   * See SetMaximumDistance and SetMinimumDistance for distance notes.
   */
  virtual float GetMaximumDistance();

  /// Retrieve a direct pointer to this object
  virtual iSndSysSource *GetPtr() { return this; }

  /// Add an output filter at the specified location.
  //  Output filters can only receive sound data and cannot modify it.  They will receive data
  //   from the same thread that the CS event handler executes in, once per frame.
  //
  //  Valid Locations:  SS_FILTER_LOC_SOURCEOUT, SS_FILTER_LOC_SOURCEIN
  //  
  //  Returns FALSE if the filter could not be added.
  virtual bool AddOutputFilter(SndSysFilterLocation Location, iSndSysSoftwareOutputFilter *pFilter);

  /// Remove an output filter from the registered list
  //
  //  Valid Locations:  SS_FILTER_LOC_SOURCEOUT, SS_FILTER_LOC_SOURCEIN
  //
  // Returns FALSE if the filter is not in the list at the time of the call.
  virtual bool RemoveOutputFilter(SndSysFilterLocation Location, iSndSysSoftwareOutputFilter *pFilter);


  virtual size_t MergeIntoBuffer(csSoundSample *frame_buffer, 
    size_t frame_count);

  /// Renderer convenience interface - Called to provide processing of output filters
  virtual void ProcessOutputFilters();


protected:
  void UpdateQueuedParameters();

  /// Returns false if enough buffer space for a mix buffer can't be allocated
  //inline bool CheckMixBufferSize(long samples);

  /// Returns true if there are user defined filters attached to this source
  //inline bool HaveFilters() { return false; }

  //void ProcessFilterQueues();

  inline bool PrepareBuffer(csSoundSample **p_buf, size_t *p_buf_len, size_t required);
  inline void ClearBuffer(csSoundSample *p_buf, size_t p_buf_len);
  bool ProcessSoundChain(int channel, size_t buffer_samples);


  void SetupFilters();

protected:
  csSndSysRendererSoftware *renderer;

  csRef<iSndSysStream> sound_stream;
  size_t stream_position;

  csSourceParameters3D active_parameters,queued_parameters;
  bool queued_updates;

  /**
   * The working buffer is where the samples from one channel at a time are
   * manipulated
   */
  csSoundSample *clean_buffer;
  size_t clean_buffer_samples;

  /**
   * The working buffer is where the samples from one channel at a time are
   * manipulated
   */
  csSoundSample *working_buffer;
  size_t working_buffer_samples;

  /**
   * The historic buffer contains samples that were previously delivered for
   * use in effects
   */
  //csSoundSample *historic_buffer;
  //size_t historic_buffer_samples;

  /// The distance from the closest channel position to the source
  float closest_speaker;
  float speaker_distance[MAX_CHANNELS], speaker_direction_cos[MAX_CHANNELS];
  csRef<iSndSysSoftwareFilter3D> speaker_filter_chains[MAX_CHANNELS];
  bool filters_setup;

  ////
  //  OutputFilter Queues
  ////
  SndSysOutputFilterQueue m_SourceOutFilterQueue;
  SndSysOutputFilterQueue m_SourceInFilterQueue;


  //int debug_cycle;

  //Queue<iSndSysFilter> filter_add_queue,filter_remove_queue,filter_clear_queue;
  //csArray<iSndSysFilter *> filters;
};

#endif // #ifndef SNDSYS_RENDERER_SOFTWARE_SOURCE_H
