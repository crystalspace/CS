/*
	Copyright (C) 2006 by Søren Bøg

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

#include "source.h"
#include "listener.h"
#include "renderer.h"

#include "isndsys/ss_stream.h"
#include "ivaria/reporter.h"

#if defined CS_HAVE_ALEXT_H
#if defined(CS_OPENAL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENAL_PATH,alext.h)
#else
#include <AL/alext.h>
#endif
#endif

// Sounds under this size are played using an OpenAL static buffer, not streamed
// When streaming is used, it's the length FillBuffer() will request.
#define ADVANCE_LENGTH 65532

/*
 * 2D Sound source
 */

SndSysSourceOpenAL2D::SndSysSourceOpenAL2D (csRef<iSndSysStream> stream, csSndSysRendererOpenAL *renderer) :
  scfImplementationType(this),
  m_Gain (1.0), m_Stream (stream), m_Renderer (renderer), m_Update (true),
  m_CurrentBuffer (s_NumberOfBuffers-1), m_EmptyBuffer (0)
{
  // Get an OpenAL source
  alGenSources( 1, &m_Source );

  // Calculate the number of bytes per frame
  m_BytesPerSample = m_Stream->GetRenderedFormat()->Bits / 8
    * m_Stream->GetRenderedFormat()->Channels;
  streamSize = m_Stream->GetFrameCount () * m_BytesPerSample;

  if (streamSize < ADVANCE_LENGTH && !m_Stream->AlwaysStream())
  {
    // This is a short sample, don't use streaming
    useStaticBuffer = true;
    isStaticBufferEmpty = true;
    requestedLength =  streamSize;
    // Allocate only one OpenAL buffer
    m_Buffers = new ALuint[1];
    alGenBuffers( 1, m_Buffers );
  }
  else
  {
    useStaticBuffer = isStaticBufferEmpty = false;
    requestedLength = ADVANCE_LENGTH;
    // Allocate and get the OpenAL buffers
    m_Buffers = new ALuint[s_NumberOfBuffers];
    alGenBuffers( s_NumberOfBuffers, m_Buffers );
  }

  // Set some initial settings
  alSource3f (m_Source, AL_POSITION, 0, 0, -1);
  alSource3f (m_Source, AL_VELOCITY, 0, 0, 0);
  alSourcef (m_Source, AL_GAIN, m_Gain);
  alSourcei (m_Source, AL_SOURCE_RELATIVE, AL_TRUE);

  // Determine the OpenAL Format
  switch (m_Stream->GetRenderedFormat()->Channels)
  {
  case 1:
    {
      switch (m_Stream->GetRenderedFormat()->Bits)
      {
      case 8:
        m_Format = AL_FORMAT_MONO8;
        break;
      case 16:
        m_Format = AL_FORMAT_MONO16;
        break;
      }
    }
    break;
  case 2:
    {
      switch (m_Stream->GetRenderedFormat()->Bits)
      {
      case 8:
        m_Format = AL_FORMAT_STEREO8;
        break;
      case 16:
        m_Format = AL_FORMAT_STEREO16;
        break;
      }
    }
    break;
  case 4:
  case 6:
  case 7:
  case 8:
    {
#ifdef AL_EXT_MCFORMATS
      if (renderer->extAL_EXT_MCFORMATS)
      {
	static const ALenum formats8[5] = { AL_FORMAT_QUAD8, 0,
	  AL_FORMAT_51CHN8, AL_FORMAT_61CHN8, AL_FORMAT_71CHN8 };
	static const ALenum formats16[5] = { AL_FORMAT_QUAD16, 0,
	  AL_FORMAT_51CHN16, AL_FORMAT_61CHN16, AL_FORMAT_71CHN16 };
	int fmtIndex = m_Stream->GetRenderedFormat()->Channels - 4;
	switch(m_Stream->GetRenderedFormat()->Bits)
	{
	case 8:
	  m_Format = formats8[fmtIndex];
	  break;
	case 16:
	  m_Format = formats16[fmtIndex];
	  break;
	}
      }
      else
#endif
      {
        // TODO: Some kind of fallback?
      }
    }
    break;
  default:
    {
      // Unsupported format
      break;
    }
  }

  // Save the sample rate locally
  m_SampleRate = m_Stream->GetRenderedFormat()->Freq;

  // Setup our position marker
  m_Stream->InitializeSourcePositionMarker (&m_PositionMarker);

  // Perform an initial update
  //PerformUpdate(m_Update);
}

SndSysSourceOpenAL2D::~SndSysSourceOpenAL2D ()
{
  // Release out OpenAL resources
  alDeleteSources( 1, &m_Source );
  if (useStaticBuffer)
    alDeleteBuffers( 1, m_Buffers);
  else
    alDeleteBuffers( s_NumberOfBuffers, m_Buffers );

  // Release the allocated buffers
  delete[] m_Buffers;
}

/*
 * iSndSysSourceOpenAL interface
 */

/*
 * iSndSysSource interface
 */

void SndSysSourceOpenAL2D::SetVolume (float volume)
{
  m_Gain = volume;
  m_Update = true;
}

float SndSysSourceOpenAL2D::GetVolume ()
{
  return m_Gain;
}

csRef<iSndSysStream> SndSysSourceOpenAL2D::GetStream()
{
  return m_Stream;
}

iSndSysSource *SndSysSourceOpenAL2D::GetPtr()
{
  return this;
}

/*
 * SndSysSourceOpenAL2D impementation
 */

void SndSysSourceOpenAL2D::PerformUpdate ( bool ExternalUpdates )
{
  // Have we finished processing any buffers?
  ALint processedBuffers = 0;
  alGetSourcei (m_Source, AL_BUFFERS_PROCESSED, &processedBuffers);

  // Did OpenAL finish processing some buffers? Are we going to seek in another position of the stream?
  if (!m_Stream->PendingSeek())
  {
    if (useStaticBuffer)
    {
      //According to the openal-soft developer AL_BUFFERS_PROCESSED for
      //AL_STATIC sources must be always 0 as we aren't queuing them.
      //So to support 1.13+ we need to check for the source state to know
      //if openal has finished playing the buffer.
      ALint currentState = 0;
      alGetSourcei (m_Source, AL_SOURCE_STATE, &currentState);
      if (currentState != AL_PLAYING)
      {
        alSourcei (m_Source, AL_BUFFER, 0);
        isStaticBufferEmpty = true;
      }
    }
    else if (processedBuffers > 0)
    {
      // Unqueue any processed buffers
      CS_ALLOC_STACK_ARRAY(ALuint, unqueued, s_NumberOfBuffers);
      alSourceUnqueueBuffers (m_Source, processedBuffers, unqueued);
      // (vk) This assert doesn't seem appropriate anymore now
      //      with static buffers being potentially used in parallel
      //CS_ASSERT (unqueued[0] == m_Buffers[m_EmptyBuffer]);
      // Update the current buffer index.
      m_CurrentBuffer = (m_CurrentBuffer + processedBuffers) % s_NumberOfBuffers;

    }
  }
  //we are going to seek in another position, so we have to prepare by flushing our buffers.
  else if(m_Stream->PendingSeek())
  {
    alSourceStop(m_Source); //Openal needs to stop the source before flushing it's buffers
    alSourcei(m_Source, AL_BUFFER, 0); //flush openal source from it's buffers
    //reset positions for buffering again
    m_CurrentBuffer = s_NumberOfBuffers-1;
    m_EmptyBuffer = 0;
  }

  // Are there any empty buffers?
  ALint queuedBuffers = 0;
  alGetSourcei (m_Source, AL_BUFFERS_QUEUED, &queuedBuffers);
  if (!useStaticBuffer && (queuedBuffers < static_cast<ALint>(s_NumberOfBuffers)))
  {
    // We're using streaming, fill any emptied buffers
    do
    {
      if (FillBuffer (m_Buffers[m_EmptyBuffer]) == true)
      {
        // Queue the newly filled buffer.
        alSourceQueueBuffers (m_Source, 1, &m_Buffers[m_EmptyBuffer]);
        // Advance the empty pointer;
        m_EmptyBuffer = (m_EmptyBuffer + 1) % s_NumberOfBuffers;
        // Increasing the number of queues
        queuedBuffers++;
      }
      else
      {
        // The buffer was not filled, stop filling buffers.
        break;
      }
    } while (m_EmptyBuffer != ( m_CurrentBuffer + 1 ) % s_NumberOfBuffers);
  }
  else if (useStaticBuffer && isStaticBufferEmpty)
  {
    // Just fill our static buffer
    if (FillBuffer (m_Buffers[0]) == true)
    {
      alSourcei (m_Source, AL_BUFFER, m_Buffers[0]);
      isStaticBufferEmpty = false;
    }
  }

  // Do we need to update attributes?
  if (m_Update)
  {
    alSourcef (m_Source, AL_GAIN, m_Gain);
    m_Update = false;
  }

  // Has anything external changed?
  if (ExternalUpdates) {
  }

  // Match the playing state of the stream
  // We check this every time, as the openal source may have gone through all
  // the attached buffers and set paused, while we still want it to play. 
  int currentState;
  alGetSourcei (m_Source, AL_SOURCE_STATE, &currentState);
  if (m_Stream->GetPauseState() == CS_SNDSYS_STREAM_COMPLETED)
  {
    if (queuedBuffers == 0)
    {
      // Set state to paused so that it will be
      // removed if auto unregistration is set
      m_Stream->Pause();
    }
  }

  if (m_Stream->GetPauseState() == CS_SNDSYS_STREAM_PAUSED)
  {
    if (currentState != AL_PAUSED || currentState != AL_INITIAL)
      alSourcePause (m_Source);
  }
  else
  {
    if (currentState != AL_PLAYING)
      alSourcePlay (m_Source);
  }
}

void SndSysSourceOpenAL2D::Configure( csConfigAccess config ) {
  s_NumberOfBuffers = config->GetInt("SndSys.OpenALBuffers", 4);
}
ALsizei SndSysSourceOpenAL2D::s_NumberOfBuffers = 4;

bool SndSysSourceOpenAL2D::FillBuffer (ALuint buffer) {

  // Advance the stream a bit
  m_Stream->AdvancePosition (requestedLength);

  // Get some data from the stream
  void *data1, *data2;
  size_t length1, length2;
  m_Stream->GetDataPointers (&m_PositionMarker, requestedLength, &data1, &length1, &data2, &length2);

  // Determine if/how the data must be combined.
  if (length1 == 0)
  {
    if (length2 == 0 )
    {
      // No data available
      return false;
    }
    else
    {
      // The second buffer has data
      alBufferData (buffer, m_Format, data2, (ALsizei)length2, m_SampleRate);
    }
  }
  else
  {
    if (length2 == 0 )
    {
      // The first buffer has data
      alBufferData (buffer, m_Format, data1, (ALsizei)length1, m_SampleRate);
    }
    else
    {
      // The both buffers have data
      // Combine the buffers
      CS_ALLOC_STACK_ARRAY(ALubyte, tempbuffer, length1+length2);
      memcpy (tempbuffer, data1, length1);
      memcpy (tempbuffer+length1, data2, length2);

      // Add the data to the buffer
      alBufferData (buffer, m_Format, tempbuffer, (ALsizei)(length1+length2), m_SampleRate);
    }
  }
  return true;
}



/*
 * 3D Sound source
 */

SndSysSourceOpenAL3D::SndSysSourceOpenAL3D (csRef<iSndSysStream> stream, csSndSysRendererOpenAL *renderer) :
  scfImplementationType(this, stream, renderer),
  m_MinDistance (1), m_MaxDistance (65536), m_InnerAngle (360), m_OuterAngle (360), m_OuterGain (0), m_Update (true)
{
  // Setup initial parameters
  m_Position.Set( 0, 0, 0 );
  m_Direction.Set( 0, 0, 0 );
  m_Velocity.Set( 0, 0, 0 );

  // Setup stream parameters
  if (stream->Get3dMode() == CS_SND3D_ABSOLUTE)
    alSourcei (GetSource(), AL_SOURCE_RELATIVE, AL_FALSE);
  else
    alSourcei (GetSource(), AL_SOURCE_RELATIVE, AL_TRUE);

  // Set the rolloff factor:
  alSourcef (GetSource(), AL_ROLLOFF_FACTOR, renderer->GetListener()->GetRollOffFactor());
}

SndSysSourceOpenAL3D::~SndSysSourceOpenAL3D ()
{
}

/*
 * iSndSysSourceOpenAL interface
 */

void SndSysSourceOpenAL3D::SetPosition (csVector3 pos)
{
  m_Position.Set (pos);
  m_Update = true;
}

csVector3 SndSysSourceOpenAL3D::GetPosition ()
{
  return m_Position;
}

void SndSysSourceOpenAL3D::SetMinimumDistance (float distance)
{
  if (distance < 0) {
    distance = 0;
  }

  m_MinDistance = distance;
  m_Update = true;
}

void SndSysSourceOpenAL3D::SetMaximumDistance (float distance)
{
  if (distance < 0) {
    distance = 0;
  }

  m_MaxDistance = distance;
  m_Update = true;
}

float SndSysSourceOpenAL3D::GetMinimumDistance ()
{
  return m_MinDistance;
}

float SndSysSourceOpenAL3D::GetMaximumDistance ()
{
  return m_MaxDistance;
}

/*
 * iSndSysSource3DDirectionalSimple interface
 */

void SndSysSourceOpenAL3D::SetDirection (csVector3 dir)
{
  m_Direction = dir;
  m_Update = true;
}

csVector3 SndSysSourceOpenAL3D::GetDirection ()
{
  return m_Direction;
}

void SndSysSourceOpenAL3D::SetDirectionalRadiation (float rad)
{
  if( rad == 0 ) {
    // OpenAL uses the direction vector to specify an omni directional source.
    m_Direction = csVector3( 0, 0, 0 );
    m_InnerAngle = 360;
    m_OuterAngle = 360;
  } else {
    m_InnerAngle = 0;
    // Note: OpenAL uses angles, not halfangles, hence I multiply by 360, not 180.
    m_OuterAngle = rad*360/PI;
  }
  m_OuterGain = 0;
  m_Update = true;
}

float SndSysSourceOpenAL3D::GetDirectionalRadiation ()
{
  // Note: OpenAL uses angles, not halfangles, hence I divide by 360, not 180.
  return m_OuterAngle*PI/360;
}

/*
 * iSndSysSource3DDirectional interface
 */
void SndSysSourceOpenAL3D::SetDirectionalRadiationInnerCone(float rad)
{
  // Note: OpenAL uses angles, not halfangles, hence I multiply by 360, not 180.
  m_InnerAngle = rad*360/PI;
  m_Update = true;
}

void SndSysSourceOpenAL3D::SetDirectionalRadiationOuterCone(float rad)
{
  // Note: OpenAL uses angles, not halfangles, hence I multiply by 360, not 180.
  m_OuterAngle = rad*360/PI;
  m_Update = true;
}

void SndSysSourceOpenAL3D::SetDirectionalRadiationOuterGain(float gain)
{
  m_OuterGain = gain;
  m_Update = true;
}

float SndSysSourceOpenAL3D::GetDirectionalRadiationInnerCone()
{
  return m_InnerAngle*PI/360;
}

float SndSysSourceOpenAL3D::GetDirectionalRadiationOuterCone()
{
  return m_OuterAngle*PI/360;
}

float SndSysSourceOpenAL3D::GetDirectionalRadiationOuterGain()
{
  return m_OuterGain;
}

/*
 * iSndSysSource3DDoppler interface
 */

void SndSysSourceOpenAL3D::SetVelocity (const csVector3 &Velocity)
{
  m_Velocity = Velocity;
  m_Update = true;
}

const csVector3 &SndSysSourceOpenAL3D::GetVelocity ()
{
  return m_Velocity;
}

/*
 * SndSysSourceOpenAL3D impementation
 */

void SndSysSourceOpenAL3D::PerformUpdate ( bool ExternalUpdates )
{
  // Do we need to update attributes?
  if (m_Update)
  {
    alSourcef (GetSource(), AL_REFERENCE_DISTANCE, m_MinDistance);
    alSourcef (GetSource(), AL_MAX_DISTANCE, m_MaxDistance);
    alSource3f (GetSource(), AL_POSITION, m_Position[0], m_Position[1], m_Position[2]);
    alSource3f (GetSource(), AL_DIRECTION, m_Direction[0], m_Direction[1], m_Direction[2]);
    alSourcef (GetSource(), AL_CONE_INNER_ANGLE, m_InnerAngle);
    alSourcef (GetSource(), AL_CONE_OUTER_ANGLE, m_OuterAngle);
    alSourcef (GetSource(), AL_CONE_OUTER_GAIN, m_OuterGain);
    alSource3f (GetSource(), AL_VELOCITY, m_Velocity[0], m_Velocity[1], m_Velocity[2]);
  }

  // Has anything external changed?
  if (ExternalUpdates) {
    alSourcef (GetSource(), AL_ROLLOFF_FACTOR, GetRenderer()->GetListener()->GetRollOffFactor());
  }

  // Let SndSysSourceOpenAL2D tak care of buffers and it's attributes:
  SndSysSourceOpenAL2D::PerformUpdate (ExternalUpdates);
}
