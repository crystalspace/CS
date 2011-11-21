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


#include "cssysdef.h"

#include "csplugincommon/sndsys/sndstream.h"

using namespace CS::SndSys;

SndSysBasicStream::SndSysBasicStream(csSndSysSoundFormat *pRenderFormat, int Mode3D) : 
 scfImplementationType(this),
 m_pCyclicBuffer(0), m_pPCMConverter(0)
{
  // Copy render format information
  memcpy(&m_RenderFormat,pRenderFormat,sizeof(csSndSysSoundFormat));

  // Start the most advanced read pointer at offset 0
  m_MostAdvancedReadPointer=0;

  // Set stream to Paused, at the beginning, and not m_bLooping
  m_PauseState=CS_SNDSYS_STREAM_PAUSED;
  m_bLooping=false;
  
  //set the starting loop position to zero
  m_startLoopFrame=0;
  //set the rewind loop position to zero (rewind to end of stream)
  m_endLoopFrame = 0;

  // A prepared data buffer will be allocated when we know the size needed
  m_pPreparedDataBuffer=0;
  m_PreparedDataBufferSize=0;
  m_PreparedDataBufferStart=0;

  // Set the render sample size
  m_RenderFrameSize=(m_RenderFormat.Bits/8) * m_RenderFormat.Channels;

  // No extra data left in the decode buffer since last call
  m_PreparedDataBufferUsage=0;

  // Playback rate is initially 100% (normal)
  m_PlaybackPercent=100;

  /* Forces the output buffer to be resized and the converter to be created on 
  * the first pass */
  m_OutputFrequency=0;

  // Output frequency is initially the same as the render frequency
  m_NewOutputFrequency=m_RenderFormat.Freq;

  // Let the pcm converter get created on the first pass
  m_pPCMConverter=0;

  // No new position 
  m_NewPosition = InvalidPosition;

  // Store the 3d mode
  m_3DMode=Mode3D;

  // Auto m_bAutoUnregisterReady not requested
  m_bAutoUnregisterRequested=false;

  // Not ready to be unregistered
  m_bAutoUnregisterReady=false;

  // Playback not complete
  m_bPlaybackReadComplete=false;

}


SndSysBasicStream::~SndSysBasicStream()
{
  delete m_pCyclicBuffer;
  delete m_pPCMConverter;
  delete[] m_pPreparedDataBuffer;
}


const csSndSysSoundFormat *SndSysBasicStream::GetRenderedFormat()
{
  return &m_RenderFormat;
}

int SndSysBasicStream::Get3dMode()
{
  return m_3DMode;
}


size_t SndSysBasicStream::GetPosition()
{
  return m_MostAdvancedReadPointer;

}

bool SndSysBasicStream::ResetPosition()
{
  m_NewPosition=0;
  return true;
}

bool SndSysBasicStream::PendingSeek () 
{
  return (m_NewPosition != InvalidPosition);
}

bool SndSysBasicStream::SetPosition (size_t newposition)
{
  m_NewPosition=newposition;
  return true;
}

bool SndSysBasicStream::Pause()
{
  m_PauseState=CS_SNDSYS_STREAM_PAUSED;
  return true;
}


bool SndSysBasicStream::Unpause()
{
  m_PauseState=CS_SNDSYS_STREAM_UNPAUSED;
  return true;
}

int SndSysBasicStream::GetPauseState()
{
  return m_PauseState;
}

bool SndSysBasicStream::SetLoopState(int loopstate)
{
  switch (loopstate)
  {
  case CS_SNDSYS_STREAM_DONTLOOP:
    m_bLooping=false;
    break;
  case CS_SNDSYS_STREAM_LOOP:
    m_bLooping=true;
    break;
  default:
    return false; // Looping mode not supported
  }
  return true;
}

/** 
* Retrieves the loop state of the stream.  Current possible returns are 
* CS_SNDSYS_STREAM_DONTLOOP and CS_SNDSYS_STREAM_LOOP
*/
int SndSysBasicStream::GetLoopState()
{
  if (m_bLooping)
    return CS_SNDSYS_STREAM_LOOP;
  return CS_SNDSYS_STREAM_DONTLOOP;
}

size_t SndSysBasicStream::GetLoopStart()
{
    return m_startLoopFrame;
}

size_t SndSysBasicStream::GetLoopEnd()
{
    return m_endLoopFrame;
}

bool SndSysBasicStream::SetLoopBoundaries(size_t startPosition, size_t endPosition)
{
    //don't allow to set a loop start higher than loop end or we could have
    //some interesting effects
    if(endPosition != 0 && startPosition >= endPosition) return false;
    m_startLoopFrame = startPosition;
    m_endLoopFrame = endPosition;
    return true;
}

void SndSysBasicStream::SetPlayRatePercent(int percent)
{
  m_PlaybackPercent=percent;
  m_NewOutputFrequency = (m_RenderFormat.Freq * 100) / m_PlaybackPercent;
}

int SndSysBasicStream::GetPlayRatePercent()
{
  return m_PlaybackPercent;
}

/** 
* If AutoUnregister is set, when the stream is paused it, and all sources 
* attached to it are removed from the sound engine
*/
void SndSysBasicStream::SetAutoUnregister(bool autounreg)
{
  m_bAutoUnregisterRequested=autounreg;
}

/** 
* If AutoUnregister is set, when the stream is Paused it, and all sources 
* attached to it are removed from the sound engine
*/
bool SndSysBasicStream::GetAutoUnregister()
{
  return m_bAutoUnregisterRequested;
}

/// Used by the sound renderer to determine if this stream needs to be unregistered
bool SndSysBasicStream::GetAutoUnregisterRequested()
{
  return m_bAutoUnregisterReady;
}


size_t SndSysBasicStream::CopyBufferBytes(size_t max_dest_bytes)
{
  // If the entire prepared buffer can fit into the cyclic buffer, copy it
  //  there and reset the prepared buffer so we can use to it from the
  //  beginning again.
  if (max_dest_bytes >= m_PreparedDataBufferUsage)
  {
    max_dest_bytes=m_PreparedDataBufferUsage;
    m_pCyclicBuffer->AddBytes (&(m_pPreparedDataBuffer[m_PreparedDataBufferStart]),
      max_dest_bytes);
    m_PreparedDataBufferUsage=0;
    m_PreparedDataBufferStart=0;
    return max_dest_bytes;
  }

  // Otherwise copy what will fit and update the position and remaining byte
  //  indicators appropriately.
  m_pCyclicBuffer->AddBytes (&(m_pPreparedDataBuffer[m_PreparedDataBufferStart]),
    max_dest_bytes);
  m_PreparedDataBufferUsage-=max_dest_bytes;
  m_PreparedDataBufferStart+=max_dest_bytes;
  return max_dest_bytes;
}




void SndSysBasicStream::GetDataPointers (size_t* position_marker, 
                                            size_t max_requested_length,
                                            void **buffer1, 
                                            size_t *buffer1_length,
                                            void **buffer2,
                                            size_t *buffer2_length)
{
  m_pCyclicBuffer->GetDataPointersFromPosition (position_marker,
    max_requested_length, (uint8 **)buffer1, buffer1_length, 
    (uint8 **)buffer2,buffer2_length);

  /* If read is finished and we've underbuffered here, then we can mark the 
  * stream as paused so no further advancement takes place */
  if ((m_PauseState == CS_SNDSYS_STREAM_UNPAUSED) && (m_bPlaybackReadComplete) 
    && ((*buffer1_length + *buffer2_length) < max_requested_length))
  {
    m_PauseState=CS_SNDSYS_STREAM_COMPLETED;
    if (m_bAutoUnregisterRequested)
      m_bAutoUnregisterReady=true;
    m_bPlaybackReadComplete=false;
  }

  if (*position_marker > m_MostAdvancedReadPointer)
    m_MostAdvancedReadPointer=*position_marker;
}



void SndSysBasicStream::InitializeSourcePositionMarker (
  size_t *position_marker)
{
  *position_marker=m_MostAdvancedReadPointer;
}


bool SndSysBasicStream::RegisterCallback(iSndSysStreamCallback *pCallback)
{
  m_CallbackList.PushSmart(pCallback);
  return true;
}

bool SndSysBasicStream::UnregisterCallback(iSndSysStreamCallback *pCallback)
{
  return m_CallbackList.Delete(pCallback);
}

bool SndSysBasicStream::RegisterFrameNotification(size_t /*frame_number*/)
{
  /*
     Frame notification must be sent in the main application thread, but is
     triggered from the sound system's processing thread which is advancing
     the play cursor.  

     If we sort the frames that notification is desired for into a circular
     list then we can optimize the checks for notification to a single
     comparison against the 'next' frame.

     Adding notification must also pass through a threadsafe queue.
     Maintenance of the circular list should be handled in the sound
     system processing thread.
  */
  return false;
}

void SndSysBasicStream::QueueNotificationEvent(StreamNotificationType NotifyType, size_t FrameNum)
{
  StreamNotificationEvent *pEvent=new StreamNotificationEvent;

  if (!pEvent)
    return;

  pEvent->m_Type=NotifyType;
  pEvent->m_Frame=FrameNum;

  if (!m_NotificationQueue.QueueEntry(pEvent))
    delete pEvent;
}

void SndSysBasicStream::ProcessNotifications()
{
  StreamNotificationEvent *pEvent;
  size_t CallbackCount=m_CallbackList.GetSize ();

  // Empty the queue of notification events
  while ((pEvent=m_NotificationQueue.DequeueEntry()) != 0)
  {
    for (size_t CallbackIDX=0;CallbackIDX<CallbackCount;CallbackIDX++)
    {
      switch (pEvent->m_Type)
      {
      case STREAM_NOTIFY_PAUSED:
        m_CallbackList[CallbackIDX]->StreamPauseNotification();
        break;
      case STREAM_NOTIFY_UNPAUSED:
        m_CallbackList[CallbackIDX]->StreamUnpauseNotification();
        break;
      case STREAM_NOTIFY_LOOP:
        m_CallbackList[CallbackIDX]->StreamLoopNotification();
        break;
      case STREAM_NOTIFY_FRAME:
        m_CallbackList[CallbackIDX]->StreamFrameNotification(pEvent->m_Frame);
        break;
      default:
        break;
      }
    }
    delete pEvent;
  }
}


