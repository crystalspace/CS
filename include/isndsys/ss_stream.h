/*
    Copyright (C) 2004 by Andrew Mann
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#ifndef __CS_SNDSYS_STREAM_H__
#define __CS_SNDSYS_STREAM_H__

/**\file
 * Sound system: stream
 */

#include "csutil/scf.h"

/**\addtogroup sndsys
 * @{ */

struct csSndSysSoundFormat;
struct iSndSysData;
struct iSndSysStreamCallback;

const size_t CS_SNDSYS_STREAM_UNKNOWN_LENGTH = (size_t)-1;

#define CS_SNDSYS_STREAM_PAUSED     0
#define CS_SNDSYS_STREAM_UNPAUSED   1
#define CS_SNDSYS_STREAM_COMPLETED  2

#define CS_SNDSYS_STREAM_DONTLOOP   0
#define CS_SNDSYS_STREAM_LOOP       1

/// Every sound stream must be created with one of these 3d modes.
enum
{
  /// Disable 3d effect.
  CS_SND3D_DISABLE=0,
  /// Position of the sound is relative to the listener.
  CS_SND3D_RELATIVE,
  /// Position of the sound is absolute.
  CS_SND3D_ABSOLUTE
};

/// The primary interface for a sound stream used by the audio system.
//
//  All audio is presented to the sound system in the form of a stream.
//  This interface is the minimal interface that all audio providing 
//  objects must implement in order to produce sound.
//
struct iSndSysStream : public virtual iBase
{
  SCF_INTERFACE(iSndSysStream,1,2,0);

  /// Retrieve a description of this stream.  
  //  This is not guaranteed to be useful for any particular purpose, different,
  //  or even accurate.  This is mainly for diagnostic purposes.
  virtual const char *GetDescription() = 0;

  /**
   * Get the format of the rendered sound data.  This is for informational
   * purposes only.
   */
  virtual const csSndSysSoundFormat *GetRenderedFormat() = 0;

  /// Retrieve the 3D Mode the sound stream was created for.
  virtual int Get3dMode() = 0;

  /**
   * Get length of this stream in rendered frames.
   * May return CS_SNDSYS_STREAM_UNKNOWN_LENGTH if the stream is of unknown 
   * length. For example, sound data being streamed from a remote host may not 
   * have a pre-determinable length.
   */
  virtual size_t GetFrameCount() = 0;

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
  virtual size_t GetPosition() = 0;

  /**
   * Resets the position of the stream to the beginning if possible.
   * FALSE may be returned if a reset operation is not permitted or not
   * possible.
   */
  virtual bool ResetPosition() = 0;

  /**
   * Sets the position of the stream to a particular frame.
   * FALSE may be returned if a set position operation is not permitted or
   * not possible.
   */
  virtual bool SetPosition (size_t newposition) = 0;

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
  virtual bool Pause() = 0;

  /**
   * Unpauses the stream and resumes providing data at the current position.
   * If the stream is not currently paused this function returns FALSE.
   */
  virtual bool Unpause() = 0;

  /**
   * Returns the PAUSE state of the stream:
   * - CS_SNDSYS_STREAM_PAUSED - The stream is paused.
   * - CS_SNDSYS_STREAM_UNPAUSED - The stream is not paused.  AdvancePosition
   *   is moving the stream position.
   * - CS_SNDSYS_STREAM_COMPLETED - The stream's data has been processed but
   *   not rendered yet. In order to keep the state consistent, the Sound
   *   System's main processing thread should call Pause() once the data has
   *   been entirely rendered.
   */
  virtual int GetPauseState() = 0;

  /**
   * Sets the loop state of the stream. Current acceptable values are
   * CS_SNDSYS_STREAM_DONTLOOP and CS_SNDSYS_STREAM_LOOP
   * May return FALSE if looping is not supported
   */
  virtual bool SetLoopState(int loopstate) = 0;

  /**
   * Retrieves the loop state of the stream.  Current possible returns are
   * CS_SNDSYS_STREAM_DONTLOOP and CS_SNDSYS_STREAM_LOOP.
   */
  virtual int GetLoopState() = 0;

  /**
   * Set the playback rate adjustment factor in percent.  100 = 100% (normal
   * speed)
   */
  virtual void SetPlayRatePercent(int percent) = 0;

  /**
   * Retrieves the playback rate adjustment factor in percent.  100 = 100%
   * (normal speed)
   */
  virtual int GetPlayRatePercent() = 0;

  /**
   * If AutoUnregister is set, when the stream is paused it, and all sources
   * attached to it are removed from the sound engine
   */
  virtual void SetAutoUnregister(bool autounreg) = 0;

  /**
   * If AutoUnregister is set, when the stream is paused it, and all sources
   * attached to it are removed from the sound engine
   */
  virtual bool GetAutoUnregister() = 0;

  /**
   * Used by the sound renderer to determine if this stream needs to
   * be unregistered
   */
  virtual bool GetAutoUnregisterRequested() = 0;


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
  virtual void AdvancePosition(size_t frame_delta) = 0;

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
  virtual void GetDataPointers(size_t* position_marker, size_t max_requested_frames,
  	void **buffer1, size_t *buffer1_frames, void **buffer2,	size_t *buffer2_frames) = 0;

  /**
   * Fill a size_t value that will be used to track a Source's position through 
   * calls to GetDataPointers().
   *
   * \remarks Not intended to be called by an application.
   */
  virtual void InitializeSourcePositionMarker (size_t* position_marker) = 0;

  /** Called by the sound system to allow a stream time to process pending notifications.
  * 
  * \remarks Not intended to be called by an application.  This is called from the
  *          main application thread.
  */
  virtual void ProcessNotifications() = 0;

  /// Register a component to receive notification of stream events
  virtual bool RegisterCallback(iSndSysStreamCallback *pCallback) = 0;

  /// Unregister a previously registered callback component 
  virtual bool UnregisterCallback(iSndSysStreamCallback *pCallback) = 0;

  /// Register a particular frame number which will trigger a callback notification when
  //   it's crossed.
  virtual bool RegisterFrameNotification(size_t frame_number) = 0;

  /// Whether this stream always needs to be treated as a stream regardless of size.
  virtual bool AlwaysStream() const = 0;
    
  /**
   * Gets where the audio should rewind to loop.
   * 
   * \return The position where to restart playing when looping in frames.
   */
  virtual size_t GetLoopStart() = 0;

  /**
   * Gets when the audio should rewind to loop.
   * 
   * \return The position where to rewind to loop start when looping in frames.
   */
  virtual size_t GetLoopEnd() = 0;
    
  /**
   * Sets the loop start and end bounduaries. The start position defines the position in frames
   * where the loop will restart when the stream reaches the end of the stream, in case
   * endPosition is 0 or the frame defined in endPosition.
   * 
   * \note The endPosition is exclusive while the start position is inclusive, so for example to get
   *       a loop from frame 0 to frame 0 you should do startPosition 0 and endPosition 1
   * \param startPosition The position in frames where to restart playing when looping.
   * \param endPosition The position in frames where to rewind to loop start when looping.
   * \return false if the parameters are out of bound or the audio format plugin doesn't support this.
   */
  virtual bool SetLoopBoundaries(size_t startPosition, size_t endPosition) = 0;
  
  /**
   * Check if the stream is pending position replacement.
   * Usually in this case it might be a good idea to flush buffers and
   * rebuffer.
   * \return TRUE if the position is being changed
   */
  virtual bool PendingSeek () = 0;

};

/// Sound System stream interface for callback notification
//
//  A component wishing to receive notification of Sound Stream events
//  should implement this interface, and call iSndSysStream::RegisterCallback()
//  to register with the stream of interest.
//
struct iSndSysStreamCallback : public virtual iBase
{
  SCF_INTERFACE(iSndSysStreamCallback,0,1,0);

  /// Called when this stream loops
  virtual void StreamLoopNotification() = 0;

  /// Called when this stream transitions from playing to paused state
  virtual void StreamPauseNotification() = 0;

  /// Called when this stream transitions from paused to playing state
  virtual void StreamUnpauseNotification() = 0;

  /// Called when this stream passes a frame previously registered for notification
  virtual void StreamFrameNotification(size_t frame_number) = 0;
};





/** @} */

#endif // __CS_SNDSYS_STREAM_H__
