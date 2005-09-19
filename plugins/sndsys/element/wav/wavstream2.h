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


#include "ss_stream.h"
#include "wavdata2.h"

class PCMSampleConverter;
class SoundCyclicBuffer;







class SndSysWavSoundStream : public iSndSysStream
{
 public:
  SCF_DECLARE_IBASE;

  SndSysWavSoundStream(csRef<SndSysWavSoundData> data, char *WavData, size_t WavDataLen, SndSysSoundFormat *renderformat, int mode3d);
  virtual ~SndSysWavSoundStream();

  /// Get the format of the rendered sound data.  This is for informational purposes only.
  virtual const SndSysSoundFormat *GetRenderedFormat();

  /// Retrieve the 3D Mode the sound stream was created for
  virtual int Get3dMode();

  /** Get length of this stream in rendered samples.
   *  May return ISOUND_STREAM_UNKNOWN_LENGTH if the stream is of unknown length.
   *  For example, sound data being streamed from a remote host may not have a pre-determinable length.
   */
  virtual long GetSampleCount();

  /** Returns the current position of this sound in rendered samples. 
   *  This should return a valid value even if GetSampleCount() returns ISOUND_STREAM_UNKNOWN_LENGTH since an object implementing
   *   this interface should know its position relative to the begining of the data.  In the case where the begining may be ambiguous
   *   it should be considered to be at the point where the stream first started.  In other words, where there is doubt, the position
   *   should start at 0 and advance as the position advances.
   */
  virtual long GetPosition();

  /** Resets the position of the stream to the begining if possible.
   *  FALSE may be returned if a reset operation is not permitted or not possible.
   */
  virtual bool ResetPosition();

  /** Sets the position of the stream to a particular sample.
   *  FALSE may be returned if a set position operation is not permitted or not possible.
   */
  virtual bool SetPosition(long newposition);

  /** Pauses the stream at the current position.  
   *    Data will not be provided through the GetData() call beyond the point of pausing.
   *    AdvancePosition() will NOT adjust the position of the stream.
   *
   *    If either of the above conditions cannot be met, this call fails and returns FALSE.  The sound element should continue
   *     operation as though this call were not made.
   *    For example, a stream coming from a remote host may not be able to be stopped or advance-buffered properly, in this case pausing the stream
   *      is not possible, and this function should return FALSE.
   */
  virtual bool Pause();


  /** Unpauses the stream and resumes providing data at the current position.
   *  If the stream is not currently paused this function returns FALSE.
   */
  virtual bool Unpause();

  /** Returns the PAUSE state of the stream:
   *  ISNDSYS_STREAM_PAUSED - The stream is paused.
   *  ISNDSYS_STREAM_UNPAUSED - The stream is not paused.  AdvancePosition is moving the stream position.
   */
  virtual int GetPauseState();

  /** Sets the loop state of the stream. Current acceptable values are ISNDSYS_STREAM_DONTLOOP and ISNDSYS_STREAM_LOOP
   *  May return FALSE if looping is not supported
   */
  virtual bool SetLoopState(int loopstate);

  /// Retrieves the loop state of the stream.  Current possible returns are ISNDSYS_STREAM_DONTLOOP and ISNDSYS_STREAM_LOOP
  virtual int GetLoopState();

  /// Set the playback rate adjustment factor in percent.  100 = 100% (normal speed)
  virtual void SetPlayRatePercent(int percent);

  /// Retrieves the playback rate adjustment factor in percent.  100 = 100% (normal speed)
  virtual int GetPlayRatePercent();

  /// If AutoUnregister is set, when the stream is paused it, and all sources attached to it are removed from the sound engine
  virtual void SetAutoUnregister(bool autounreg);

  /// If AutoUnregister is set, when the stream is paused it, and all sources attached to it are removed from the sound engine
  virtual bool GetAutoUnregister();

  /// Used by the sound renderer to determine if this stream needs to be unregistered
  virtual bool GetAutoUnregisterRequested();


  /** NOT AN APPLICATION CALLABLE FUNCTION!   This function advances the stream position based on the provided time value which is considered as a "current time".
   *   A Sound Element will usually store the last advance time internally.  When this function is called it will compare the last time with the time presented
   *   and retrieve more data from the associated iSndSysData container as needed.  It will then update its internal last advance time value.
   *  
   *   This function is not necessarily thread safe and must be called ONLY from the Sound System's main processing thread.
   *
   */
  virtual void AdvancePosition(csTicks current_time);

  /** NOT AN APPLICATION CALLABLE FUNCTION!  This function is used to retrieve pointers to properly formatted sound data.  
   *
   *  Since a Stream may be attached to multiple Sources, it will be most optimal to perform any decoded-data buffering at the stream level.  The parameters
   *   passed to GetDataPointers() should allow for proper interface to a cyclic buffering method to store this decoded-data.
   *
   *  Since the data in the Stream buffer(s) and data related to how the buffers are accessed may change when AdvancePosition() is called, this function is not
   *   safe to call while AdvancePosition() is in operation.  For this reason it is only safe to be called from the same thread that calls AdvancePosition() - specifically
   *   the Sound System main processing thread.
   *
   *   @param position_marker Should point to a long initially filled by the Sound System internally when a Source is created - through a call to InitializeSourcePositionMarker().
   *   @param max_requested_length Should contain the maximum number of bytes the calling source is interested in receiving.  On return, *buffer1_length + *buffer2_length must not exceed this value.
   *   @param buffer1 should point to a (void *) that will be filled with a pointer to the first chunk of data on return or NULL (0) if no data is available
   *   @param buffer1_length should point to a long that will be filled with the length of valid data in the buffer pointed to by *buffer1 on return.
   *   @param buffer2 should point to a (void *) that will be filled with a pointer to the second chunk of data on return, or NULL (0) if no second chunk is needed.
   *   @param buffer2_length should point to a long that will be filled with the length of valid data in the buffer pointed to by *buffer1 on return.
   *
   */
  virtual void GetDataPointers(long *position_marker,long max_requested_length,void **buffer1,long *buffer1_length,void **buffer2,long *buffer2_length);



  /// NOT AN APPLICATION CALLABLE FUNCTION!  This function fills a long value that will be used to track a Source's position through calls to GetDataPointers().
  virtual void InitializeSourcePositionMarker(long *position_marker);

  /// Retrieve a direct pointer to this object
  virtual iSndSysStream *GetPtr() { return this; }

 protected:
   long CopyBufferBytes(long max_dest_bytes);

 protected:
  SndSysSoundFormat render_format;
  csRef<SndSysWavSoundData> sound_data;

  SoundCyclicBuffer *p_cyclicbuffer;
  bool paused, looping;
  bool playback_read_complete;
  csTicks last_time;

  long most_advanced_read_pointer;

  /// If this value is >=0 then the next stream advancement will occur from the position requested
  //  Used by ResetPosition()
  long new_position;


  /// Pointer to the PCM sample converter object that will handle our conversions
  PCMSampleConverter *pcm_convert;

  char *prepared_data_buffer;
  int prepared_data_buffer_length;
  int prepared_buffer_usage, prepared_buffer_start;


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

  /// Stores the output frequency which is (render_frequency * 100) / playback_percent
  int output_frequency;

  /// When the output frequency is changed, the conversion output buffer will be resized
  int new_output_frequency;

  /// The 3d mode that this stream was created for.  One of   SOUND3D_DISABLE, SOUND3D_RELATIVE, or SOUND3D_ABSOLUTE
  int mode_3d;

  /// Set to true if this stream and all tied sources should be removed from the sound system when it enters a paused state
  bool auto_unregister;

  /// Set to true if this stream is ready for unregistering
  bool unregister;
};

#endif // #ifndef SNDSYS_STREAM_WAV_H


