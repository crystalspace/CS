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

#ifndef SNDSYS_STREAM_OGG_H
#define SNDSYS_STREAM_OGG_H


#include "isndsys/ss_stream.h"
#include "oggdata2.h"

namespace CrystalSpace
{
  class PCMSampleConverter;
  class SoundCyclicBuffer;
}


// This hack works around a build problem with some installations of Ogg/Vorbis
// on Cygwin where it attempts to #include the non-existent <_G_config.h>.  We
// must ensure that _WIN32 is not defined prior to #including the Vorbis
// headers.  This problem is specific to C++ builds; it does not occur with
// plain C builds (which explains why the CS configure check does not require
// this hack).
#ifdef __CYGWIN__
#undef _WIN32
#endif

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>


#ifdef CS_LITTLE_ENDIAN
  #define OGG_ENDIAN 0
#else
  #define OGG_ENDIAN 1
#endif





class SndSysOggSoundStream : public iSndSysStream
{
 public:
  SCF_DECLARE_IBASE;

  SndSysOggSoundStream (csRef<SndSysOggSoundData> data, OggDataStore *datastore, 
    csSndSysSoundFormat *renderformat, int mode3d);
  virtual ~SndSysOggSoundStream ();

  /**
   * Get the format of the rendered sound data.  This is for informational 
   * purposes only.
   */
  virtual const csSndSysSoundFormat *GetRenderedFormat();

  /// Retrieve the 3D Mode the sound stream was created for
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
  virtual void GetDataPointers (size_t *position_marker, 
    size_t max_requested_length, void **buffer1, size_t *buffer1_length, 
    void **buffer2, size_t *buffer2_length);
  virtual void InitializeSourcePositionMarker (size_t* position_marker);

  /// Retrieve a direct pointer to this object
  virtual iSndSysStream *GetPtr() { return this; }

 protected:
   size_t CopyBufferBytes (size_t max_dest_bytes);

 protected:
  OggVorbis_File vorbis_file;
  csSndSysSoundFormat render_format;
  OggStreamData stream_data;
  csRef<SndSysOggSoundData> sound_data;

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
   * conversions.
   */
  CrystalSpace::PCMSampleConverter *pcm_convert;

  char *prepared_data_buffer;
  int prepared_data_buffer_length;
  //char ogg_decode_buffer[4096];
  size_t prepared_buffer_usage, prepared_buffer_start;

  /// Numeric identifier of the current stream within the ogg file
  int current_ogg_stream;

  /// Format of the sound data in the current ogg stream
  vorbis_info *current_ogg_format_info;

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
   * SOUND3D_RELATIVE, or SOUND3D_ABSOLUTE
   */
  int mode_3d;

  /** 
   * Set to true if this stream and all tied sources should be removed from 
   * the sound system when it enters a paused state
   */
  bool auto_unregister;

  /// Set to true if this stream is ready for unregistering
  bool unregister;
};

#endif // #ifndef SNDSYS_STREAM_OGG_H


