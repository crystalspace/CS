/*
    Copyright (C) 2006 by Andrew Mann

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



#ifndef SNDSTREAM_H
#define SNDSTREAM_H

#include "iutil/databuff.h"
#include "isndsys/ss_structs.h"
#include "isndsys/ss_stream.h"
#include "csplugincommon/sndsys/convert.h"
#include "csplugincommon/sndsys/cyclicbuf.h"
#include "csplugincommon/sndsys/queue.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"


namespace CS
{
  namespace SndSys
  {

  class CS_CRYSTALSPACE_EXPORT SndSysBasicStream :
    public scfImplementation1<SndSysBasicStream, iSndSysStream>
  {
  public:
    SndSysBasicStream(csSndSysSoundFormat *pRenderFormat, int Mode3D);
    virtual ~SndSysBasicStream();

    /// A constant value used to indicate an invalid position for the stream
    static const size_t InvalidPosition = (size_t)~0;

    ////
    // Structure/enum definitions
    ////

    /// Types of notification events
    typedef enum
    {
      STREAM_NOTIFY_PAUSED=0,
      STREAM_NOTIFY_UNPAUSED,
      STREAM_NOTIFY_LOOP,
      STREAM_NOTIFY_FRAME
    } StreamNotificationType;

    /// Structure containing the data for each notification event
    struct StreamNotificationEvent
    {
      /// The type of this notification event
      StreamNotificationType m_Type;
      /// The frame number for this event
      //    This is currently only valid for 
      //    STREAM_NOTIFY_FRAME event types
      size_t m_Frame;
    };


    ////
    // Interface implementation
    ////

    //------------------------
    // iSndSysStream
    //------------------------
  public:

    /// Retrieve the textual description of this stream
    virtual const char *GetDescription() = 0;

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
    * Check if the stream is pending position replacement.
    * Usually in this case it might be a good idea to flush buffers and
    * rebuffer.
    * \return TRUE if the position is being changed
    */
    virtual bool PendingSeek ();

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
    * - CS_SNDSYS_STREAM_COMPLETED - The stream's data has been processed but
    *   not rendered yet. In order to keep the state consistent, the Sound
    *   System's main processing thread should call Pause() once the data has
    *   been entirely rendered.
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
    * Gets where the audio should rewind to loop.
    * 
    * \return The position where to restart playing when looping in frames.
    */
    virtual size_t GetLoopStart();

   /**
    * Gets when the audio should rewind to loop.
    * 
    * \return The position where to rewind to loop start when looping in frames.
    */
    virtual size_t GetLoopEnd();
    
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
    virtual bool SetLoopBoundaries(size_t startPosition, size_t endPosition);

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
    * \param max_requested_length Should contain the maximum number of bytes
    *        the calling source is interested in receiving.  On return,
    *        *buffer1_bytes + *buffer1_bytes must not exceed this value.
    * \param buffer1 Should point to a (void *) that will be filled with a
    *        pointer to the first chunk of data on return or NULL (0) if no
    *        data is available
    * \param buffer1_bytes Should point to a long that will be filled with the
    *        length, in bytes, of valid data in the buffer pointed to by 
    *        *buffer1 on return.
    * \param buffer2 Should point to a (void *) that will be filled with a
    *        pointer to the second chunk of data on return, or NULL (0) if no
    *        second chunk is needed.
    * \param buffer2_bytes Should point to a long that will be filled with the
    *        length, in bytes, of valid data in the buffer pointed to by 
    *        *buffer1 on return.
    *
    * \remarks Not intended to be called by an application.
    */
    virtual void GetDataPointers (size_t *position_marker, size_t max_requested_length,
      void **buffer1, size_t *buffer1_bytes, void **buffer2, size_t *buffer2_bytes);

    /**
    * Fill a size_t value that will be used to track a Source's position
    * through calls to GetDataPointers().
    *
    * \remarks Not intended to be called by an application.
    */
    virtual void InitializeSourcePositionMarker (size_t* position_marker);

    /**
    * Called by the sound system to allow a stream time to process pending
    * notifications.
    * 
    * \remarks Not intended to be called by an application.
    *   This is called from the main application thread.
    */
    virtual void ProcessNotifications();


    /// Register a component to receive notification of stream events
    virtual bool RegisterCallback(iSndSysStreamCallback *pCallback);

    /// Unregister a previously registered callback component 
    virtual bool UnregisterCallback(iSndSysStreamCallback *pCallback);

    /// Register a particular frame number which will trigger a callback
    /// notification when it's crossed.
    virtual bool RegisterFrameNotification(size_t frame_number);

    /// Whether this stream always needs to be treated as a stream regardless of size.
    virtual bool AlwaysStream() const { return false; }

    ////
    // Internal functions
    ////
  protected:
    /// Copies audio data from the ogg decoding buffer into the cyclic
    /// sample buffer
    //
    size_t CopyBufferBytes (size_t max_dest_bytes);

    /// Called to queue a notification event
    void QueueNotificationEvent(StreamNotificationType NotifyType, size_t FrameNum);


    ////
    //  Member variables
    ////
  protected:
    /// The audio format in which sound data must be presented to the renderer
    csSndSysSoundFormat m_RenderFormat;

    /// A pointer to our cyclic buffer object.
    //
    //  The cyclic buffer is used to hold recently decoded sound.  It 
    //   provides functionality which allows us to add new data at any point
    //   without a lot of excess copying.
    SoundCyclicBuffer *m_pCyclicBuffer;

    /// Is this stream paused?
    int16 m_PauseState;

    /// Is this stream set to loop (start over) when it reaches the end
    //   of the data?
    bool m_bLooping;
    
    ///The frame where the loop will start when it happens
    size_t m_startLoopFrame;
    
    ///The frame where the audio will loop, even if there are still frames
    size_t m_endLoopFrame;
    

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
    PCMSampleConverter *m_pPCMConverter;

    /// The buffer into audio is decoded in preparation
    //   for addition to the cyclic buffer.
    char *m_pPreparedDataBuffer;

    /// Size (in bytes) of the buffer pointed to by m_pPreparedDataBuffer
    int m_PreparedDataBufferSize;

    /// The number of valid bytes of data remaining in the prepared data buffer
    size_t m_PreparedDataBufferUsage;

    /// The offset of the first byte of valid data remaining in the prepared 
    //   data buffer
    size_t m_PreparedDataBufferStart;

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

    /// The list of registered callback interfaces (with references held)
    csRefArray<iSndSysStreamCallback> m_CallbackList;

    /// Threadsafe queue of pending notification events
    Queue<StreamNotificationEvent> m_NotificationQueue;
  };


 }
 // END namespace CS::SndSys
}
// END namespace CS




#endif // #ifndef SNDSTREAM_H

