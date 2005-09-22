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

#ifndef SNDSYS_STREAM_WAV_H
#define SNDSYS_STREAM_WAV_H


#include "isndsys/ss_stream.h"
#include "wavdata2.h"

namespace CrystalSpace
{
  class PCMSampleConverter;
  class SoundCyclicBuffer;
}

class SndSysWavSoundStream : public iSndSysStream
{
 public:
  SCF_DECLARE_IBASE;

  SndSysWavSoundStream(csRef<SndSysWavSoundData> data, char *WavData, 
    size_t WavDataLen, csSndSysSoundFormat *renderformat, int mode3d);
  virtual ~SndSysWavSoundStream();

  virtual const csSndSysSoundFormat *GetRenderedFormat();
  virtual int Get3dMode();

  virtual size_t GetSampleCount();
  virtual size_t GetPosition();
  virtual bool ResetPosition();
  virtual bool SetPosition (size_t newposition);

  virtual bool Pause();
  virtual bool Unpause();
  virtual int GetPauseState();

  virtual bool SetLoopState(int loopstate);
  virtual int GetLoopState();

  virtual void SetPlayRatePercent(int percent);

  virtual int GetPlayRatePercent();

  virtual void SetAutoUnregister(bool autounreg);
  virtual bool GetAutoUnregister();
  virtual bool GetAutoUnregisterRequested();

  virtual void AdvancePosition(csTicks current_time);
  virtual void GetDataPointers (size_t* position_marker, 
    size_t max_requested_length, void **buffer1, size_t *buffer1_length, 
    void **buffer2, size_t *buffer2_length);
  virtual void InitializeSourcePositionMarker (size_t* position_marker);

  virtual iSndSysStream *GetPtr() { return this; }

 protected:
   size_t CopyBufferBytes(size_t max_dest_bytes);

 protected:
  csSndSysSoundFormat render_format;
  csRef<SndSysWavSoundData> sound_data;

  CrystalSpace::SoundCyclicBuffer *p_cyclicbuffer;
  bool paused, looping;
  bool playback_read_complete;
  csTicks last_time;

  size_t most_advanced_read_pointer;

  /** 
   * If this value is !=positionInvalid then the next stream advancement will 
   * occur from the position requested. Used by ResetPosition()
   */
  size_t new_position;


  /** 
   * Pointer to the PCM sample converter object that will handle our 
   * conversions
   */
  CrystalSpace::PCMSampleConverter *pcm_convert;

  char *prepared_data_buffer;
  int prepared_data_buffer_length;
  size_t prepared_buffer_usage, prepared_buffer_start;


  /// Pointers to the raw PCM data from the data source
  char *wav_data_start;
  size_t wav_bytes_total;
  char *wav_data;
  size_t wav_bytes_left;

  /// Stores wether a conversion is needed on the data
  bool conversion_needed;

  /// Stores the size in bytes of one sample of output data
  int render_sample_size;

  /// Stores the playback rate in percent
  int playback_percent;

  /**
   * Stores the output frequency which is 
   * (render_frequency * 100) / playback_percent
   */
  int output_frequency;

  /** 
   * When the output frequency is changed, the conversion output buffer will 
   * be resized
   */
  int new_output_frequency;

  /** 
   * The 3d mode that this stream was created for.  One of   SOUND3D_DISABLE, 
   * SOUND3D_RELATIVE, or SOUND3D_ABSOLUTE.
   */
  int mode_3d;

  /** 
   * Set to true if this stream and all tied sources should be removed from 
   * the sound system when it enters a paused state.
   */
  bool auto_unregister;

  /// Set to true if this stream is ready for unregistering
  bool unregister;
};

#endif // #ifndef SNDSYS_STREAM_WAV_H


