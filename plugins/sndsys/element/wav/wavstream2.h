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
#include "csutil/scf_implementation.h"

#include "wavdata2.h"

// Advance declaration of external classes
namespace CS
{
  namespace Sound
  {
    class PCMSampleConverter;
    class SoundCyclicBuffer;
  }
}

class SndSysWavSoundStream : 
  public scfImplementation1<SndSysWavSoundStream, iSndSysStream>
{
 public:
  /// Construct a stream from the provided data.
  SndSysWavSoundStream(csRef<SndSysWavSoundData> data, char *WavData, 
    size_t WavDataLen, csSndSysSoundFormat *renderformat, int mode3d);
  virtual ~SndSysWavSoundStream();


  ////
  // Interface implementation
  ////

  //------------------------
  // iSndSysStream
  //------------------------
public:

  /// Retrieve the textual description of this stream
  virtual const char *GetDescription();

  /**
  * Get the format of the rendered sound data.  This is for informational 
  * purposes only.
  */
  virtual const csSndSysSoundFormat *GetRenderedFormat();

  /// Retrieve the 3D Mode the sound stream was created for
  virtual int Get3dMode();

  /**
  * Get length of this stream in rendered frames.
  * May return CS_SNDSYS_STREAM_UNKNOWN_LENGTH if the stream is of unknown 
  * length. For example, sound data being streamed from a remote host may not 
  * have a pre-determinable length.
  */
  virtual size_t GetFrameCount();


  /**
  * Returns the current position of this sound in rendered frames. 
  * This should return a valid value even if GetFrameCount() returns
  * CS_SNDSYS_STREAM_UNKNOWN_LENGTH since an object implementing
  * this interface should know its position relative to the beginning of the
  * data.  In the case where the beginning may be ambiguous
  * it should be considered to be at the point where the stream first
  * started.  In other words, where there is doubt, the position
  * should start at 0 and advance as the position advances.
  */
  virtual size_t GetPosition();

  /**
  * Resets the position of the stream to the beginning if possible.
  * FALSE may be returned if a reset operation is not permitted or not
  * possible.
  */
  virtual bool ResetPosition();

  /**
  * Sets the position of the stream to a particular frame.
  * FALSE may be returned if a set position operation is not permitted or
  * not possible.
  */
  virtual bool SetPosition (size_t newposition);

  /**
  * Pauses the stream at the current position.  
  * Data will not be provided through the GetData() call beyond the point of
  * pausing. AdvancePosition() will NOT adjust the position of the stream.
  *
  * If either of the above conditions cannot be met, this call fails and
  * returns FALSE.  The sound element should continue operation as though
  * this call were not made. For example, a stream coming from a remote
  * host may not be able to be stopped or advance-buffered properly, in
  * this case pausing the stream is not possible, and this function should
  * return FALSE.
  */
  virtual bool Pause();

  /**
  * Unpauses the stream and resumes providing data at the current position.
  * If the stream is not currently paused this function returns FALSE.
  */
  virtual bool Unpause();

  /**
  * Returns the PAUSE state of the stream:
  * - CS_SNDSYS_STREAM_PAUSED - The stream is paused.
  * - CS_SNDSYS_STREAM_UNPAUSED - The stream is not paused.  AdvancePosition
  *   is moving the stream position.
  */
  virtual int GetPauseState();

  /**
  * Sets the loop state of the stream. Current acceptable values are
  * CS_SNDSYS_STREAM_DONTLOOP and CS_SNDSYS_STREAM_LOOP
  * May return FALSE if looping is not supported
  */
  virtual bool SetLoopState(int loopstate);

  /**
  * Retrieves the loop state of the stream.  Current possible returns are
  * CS_SNDSYS_STREAM_DONTLOOP and CS_SNDSYS_STREAM_LOOP.
  */
  virtual int GetLoopState();

  /**
  * Set the playback rate adjustment factor in percent.  100 = 100% (normal
  * speed)
  */
  virtual void SetPlayRatePercent(int percent);

  /**
  * Retrieves the playback rate adjustment factor in percent.  100 = 100%
  * (normal speed)
  */
  virtual int GetPlayRatePercent();

  /**
  * If AutoUnregister is set, when the stream is paused it, and all sources
  * attached to it are removed from the sound engine
  */
  virtual void SetAutoUnregister(bool autounreg);

  /**
  * If AutoUnregister is set, when the stream is paused it, and all sources
  * attached to it are removed from the sound engine
  */
  virtual bool GetAutoUnregister();

  /**
  * Used by the sound renderer to determine if this stream needs to
  * be unregistered
  */
  virtual bool GetAutoUnregisterRequested();

  /**
  * NOT AN APPLICATION CALLABLE FUNCTION!   This function advances the stream
  * position based on the provided frame count value which is considered as an
  * elapsed frame count.
  *
  * A Sound Element will usually store the last advance frame internally.
  * When this function is called it will compare the last frame with the
  * frame presented and retrieve more data from the associated iSndSysData
  * container as needed.  It will then update its internal last advance
  * frame value.
  *  
  * This function is not necessarily thread safe and must be called ONLY
  * from the Sound System's main processing thread.
  */
  virtual void AdvancePosition(size_t frame_delta);

  /**
  * Used to retrieve pointers to properly formatted sound data.  
  *
  * Since a Stream may be attached to multiple Sources, it will be most
  * optimal to perform any decoded-data buffering at the stream level.
  * The parameters passed to GetDataPointers() should allow for proper
  * interface to a cyclic buffering method to store this decoded-data.
  *
  * Since the data in the Stream buffer(s) and data related to how the
  * buffers are accessed may change when AdvancePosition() is called, this
  * function is not safe to call while AdvancePosition() is in operation.
  * For this reason it is only safe to be called from the same thread that
  * calls AdvancePosition() - specifically the Sound System main processing
  * thread.
  *
  * \param position_marker Should point to a size_t initially filled by the
  *        Sound System internally when a Source is created - through a call
  *        to InitializeSourcePositionMarker().
  * \param max_requested_frames Should contain the maximum number of frames
  *        the calling source is interested in receiving.  On return,
  *        *buffer1_length + *buffer2_length must not exceed this value.
  * \param buffer1 should point to a (void *) that will be filled with a
  *        pointer to the first chunk of data on return or NULL (0) if no
  *        data is available
  * \param buffer1_frames should point to a long that will be filled with the
  *        length, in frames, of valid data in the buffer pointed to by 
  *        *buffer1 on return.
  * \param buffer2 should point to a (void *) that will be filled with a
  *        pointer to the second chunk of data on return, or NULL (0) if no
  *        second chunk is needed.
  * \param buffer2_frames should point to a long that will be filled with the
  *        length, in frames, of valid data in the buffer pointed to by 
  *        *buffer1 on return.
  *
  * \remarks Not intended to be called by an application.
  */
  virtual void GetDataPointers (size_t *position_marker, size_t max_requested_length,
    void **buffer1, size_t *buffer1_bytes, void **buffer2, size_t *buffer2_bytes);

  /**
  * Fill a size_t value that will be used to track a Source's position through 
  * calls to GetDataPointers().
  *
  * \remarks Not intended to be called by an application.
  */
  virtual void InitializeSourcePositionMarker (size_t* position_marker);

  ////
  // Internal functions
  ////
protected:
  /// Copies audio data from the decoding buffer into the cyclic sample buffer
  size_t CopyBufferBytes (size_t max_dest_bytes);


  ////
  //  Member variables
  ////
protected:
  /// The audio format in which sound data must be presented to the renderer
  csSndSysSoundFormat m_RenderFormat;

  /// Holds our reference to the underlying data element
  csRef<SndSysWavSoundData> m_SoundData;

  /// A pointer to our cyclic buffer object.
  //
  //  The cyclic buffer is used to hold recently decoded sound.  It 
  //   provides functionality which allows us to add new data at any point
  //   without a lot of excess copying.
  CS::Sound::SoundCyclicBuffer *m_pCyclicBuffer;

  /// Is this stream paused?
  bool m_bPaused;


  /// Is this stream set to loop (start over) when it reaches the end
  //   of the data?
  bool m_bLooping;

  /// Have we finished reading data from the underlying data element?
  //
  //  This is set to true once we have completed reading the underlying data
  //   only if we are not looping. If we are looping, then we'll just start
  //   back at the beginning of the data and we will never be finished reading.
  bool m_bPlaybackReadComplete;

  /// Holds our integer representation of the position of the source which is
  //   furthest ahead in reading.
  size_t m_MostAdvancedReadPointer;


  /**
  * If this value is !=positionInvalid then the next stream advancement will 
  * occur from the position requested. Used by ResetPosition()
  */
  size_t m_NewPosition;

  /**
  * Pointer to the PCM sample converter object that will handle our 
  * conversions.
  */
  CS::Sound::PCMSampleConverter *m_pPCMConverter;

  /// The buffer into which PCM audio is read in preparation
  //   for addition to the cyclic buffer. 
  char *m_pPreparedDataBuffer;

  /// Size (in bytes) of the buffer pointed to by m_pPreparedDataBuffer
  int m_PreparedDataBufferSize;

  /// The number of valid bytes of data remaining in the prepared data buffer
  size_t m_PreparedDataBufferUsage;

  /// The offset of the first byte of valid data remaining in the prepared 
  //   data buffer
  size_t m_PreparedDataBufferStart;

  /// Pointer to the base of the WAV audio data buffer
  char *m_pWavDataBase;

  /// Length in bytes of the audio data contained in the buffer
  size_t m_WavDataLength;

  /// Pointer to the next sample in the audio buffer
  char *m_pWavCurrentPointer;

  /// Number of bytes left in the audio buffer
  size_t m_WavBytesLeft;

  /// Stores wether a conversion is needed on the data
  bool m_bConversionNeeded;

  /// Stores the size in bytes of one frame of output data
  int m_RenderFrameSize;

  /// Stores the playback rate in percent
  int m_PlaybackPercent;

  /** 
  * Stores the output frequency which is 
  * (render_frequency * 100) / m_PlaybackPercent
  */
  int m_OutputFrequency;

  /**
  * When the output frequency is changed, the conversion output buffer will 
  * be resized
  */
  int m_NewOutputFrequency;

  /** 
  * The 3d mode that this stream was created for.  One of   SOUND3D_DISABLE, 
  * SOUND3D_RELATIVE, or SOUND3D_ABSOLUTE
  */
  int m_3DMode;

  /** 
  * Set to true if this stream and all tied sources should be removed from 
  * the sound system when it enters a paused state
  */
  bool m_bAutoUnregisterRequested;

  /// Set to true if this stream is ready for unregistering
  bool m_bAutoUnregisterReady;
};

#endif // #ifndef SNDSYS_STREAM_WAV_H


