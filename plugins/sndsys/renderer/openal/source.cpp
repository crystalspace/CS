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

#include "AL/al.h"

#include "source.h"

#include "isndsys/ss_stream.h"
#include "ivaria/reporter.h"

SndSysSourceOpenAL2D::SndSysSourceOpenAL2D (csRef<iSndSysStream> stream, iSndSysRendererOpenAL *renderer) :
  scfImplementationType(this),
  m_Gain (1.0), m_Stream (stream), m_Renderer (renderer), m_Update (true),
  m_CurrentBuffer (s_NumberOfBuffers-1), m_EmptyBuffer (0)
{
  // Get an OpenAL source
  alGenSources( 1, &m_Source );

  // Get the OpenAL buffers
  // TODO: Make the number of buffers configurable.
  alGenBuffers( s_NumberOfBuffers, m_Buffers );

  // Set some initial settings
  alSource3f (m_Source, AL_POSITION, 0, 0, -1);
  alSource3f (m_Source, AL_VELOCITY, 0, 0, 0);
  alSourcef (m_Source, AL_GAIN, m_Gain);
  alSourcei (m_Source, AL_SOURCE_RELATIVE, AL_TRUE);

  // Determine the OpenAL Format
  if (m_Stream->GetRenderedFormat()->Bits == 8)
    if (m_Stream->GetRenderedFormat()->Channels == 1)
      m_Format = AL_FORMAT_MONO8;
    else
      m_Format = AL_FORMAT_STEREO8;
  else
    if (m_Stream->GetRenderedFormat()->Channels == 1)
      m_Format = AL_FORMAT_MONO16;
    else
      m_Format = AL_FORMAT_STEREO16;

  // Claculate the number of bytes per frame
  m_BytesPerSample = m_Stream->GetRenderedFormat()->Bits / 8 * m_Stream->GetRenderedFormat()->Channels;

  // Save the sample rate locally
  m_SampleRate = m_Stream->GetRenderedFormat()->Freq;

  // Setup our position marker
  m_Stream->InitializeSourcePositionMarker (&m_PositionMarker);

  // Perform an initial update
  PerformUpdate();
}

SndSysSourceOpenAL2D::~SndSysSourceOpenAL2D ()
{
  // Release out OpenAL resources
  alDeleteBuffers( s_NumberOfBuffers, m_Buffers );
  alDeleteSources( 1, &m_Source );
}

/*
 * iSndSysSourceOpenAL interface
 */
void SndSysSourceOpenAL2D::PerformUpdate ()
{
  // Have we finished processing any buffers?
  ALint processedBuffers = 0;
  alGetSourcei (m_Source, AL_BUFFERS_PROCESSED, &processedBuffers);

  // Did OpenAL finish processing some buffers?
  if (processedBuffers > 0)
  {
    // Unqueue any processed buffers
    ALuint unqueued[s_NumberOfBuffers];
    alSourceUnqueueBuffers (m_Source, processedBuffers, unqueued);
    CS_ASSERT (unqueued[0] == m_Buffers[m_EmptyBuffer]);

    // Update the current buffer index.
    m_CurrentBuffer = (m_CurrentBuffer + processedBuffers) % s_NumberOfBuffers;
  }

  // Are there any empty buffers?
  ALint queuedBuffers = 0;
  alGetSourcei (m_Source, AL_BUFFERS_QUEUED, &queuedBuffers);
  if (queuedBuffers < s_NumberOfBuffers)
  {
    // Fill any emptied buffers
    do
    {
      if (FillBuffer (m_Buffers[m_EmptyBuffer]) == true)
      {
        // Queue the newly filled buffer.
        alSourceQueueBuffers (m_Source, 1, &m_Buffers[m_EmptyBuffer]);
        // Advance the empty pointer;
        m_EmptyBuffer = (m_EmptyBuffer + 1) % s_NumberOfBuffers;
      }
      else
      {
        // The buffer was not filled, stop filling buffers.
        break;
      }
    } while (m_EmptyBuffer != ( m_CurrentBuffer + 1 ) % s_NumberOfBuffers);
  }

  // Match the playing state of the stream
  // TODO: Optimise by registering for callbacks.
  int currentState;
  alGetSourcei (m_Source, AL_SOURCE_STATE, &currentState);
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

  // Do we need to update attributes?
  if (m_Update)
  {
    alSourcef (m_Source, AL_GAIN, m_Gain);
    m_Update = false;
  }
}

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
bool SndSysSourceOpenAL2D::FillBuffer (ALuint buffer) {
  // Advance the stream a bit
  m_Stream->AdvancePosition (16384);

  // Get som data from the stream
  void *data1, *data2;
  size_t length1, length2;
  m_Stream->GetDataPointers (&m_PositionMarker, 16384, &data1, &length1, &data2, &length2);

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
      alBufferData (buffer, m_Format, data2, length2, m_SampleRate);
    }
  }
  else
  {
    if (length2 == 0 )
    {
      // The first buffer has data
      alBufferData (buffer, m_Format, data1, length1, m_SampleRate);
    }
    else
    {
      // The both buffers have data
      // Combine the buffers
      ALchar tempbuffer[length1+length2];
      memcpy (tempbuffer, data1, length1);
      memcpy (tempbuffer+length1, data2, length2);

      // Add the data to the buffer
      alBufferData (buffer, m_Format, tempbuffer, length1+length2, m_SampleRate);
    }
  }
  return true;
}
