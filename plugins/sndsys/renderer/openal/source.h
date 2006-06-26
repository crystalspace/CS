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

#ifndef SNDSYS_RENDERER_OPENAL_SOURCE_H
#define SNDSYS_RENDERER_OPENAL_SOURCE_H

#include "cssysdef.h"

#include "csutil/scf_implementation.h"

#include "AL/al.h"

#include "isndsys/ss_source.h"

class iSndSysRendererOpenAL;

class SndSysSourceOpenAL2D :
  public scfImplementation2<SndSysSourceOpenAL2D,
                            iSndSysSource,
                            iSndSysSourceOpenAL >
{
public:
  SndSysSourceOpenAL2D (csRef<iSndSysStream> stream, iSndSysRendererOpenAL *renderer);
  virtual ~SndSysSourceOpenAL2D ();

  /*
   * iSndSysSourceOpenAL interface
   */
  /**
   * Function to update the source, ie. perform any pending operations, and
   * possibly refill any pending buffers.
   *
   * @note Should only be called from the OpenAL renderer.
   * @note It is expected that the renderer has set the correct OpenAL context
   *       before calling the method.
   */
  virtual void PerformUpdate ();

  /*
   * iSndSysSource interface
   */
public:
  /// Set volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
  virtual void SetVolume (float volume);
  /// Get volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
  virtual float GetVolume ();

  /// Retrieve the iSoundStream attached to this source
  virtual csRef<iSndSysStream> GetStream();

  /// Retrieve a direct pointer to this object
  virtual iSndSysSource *GetPtr();

  /*
   * SndSysSourceOpenAL2D impementation
   */
private:
  /// Current gain (aka. volume) of the source
  float m_Gain;
  /// The stream attached to this source
  csRef<iSndSysStream> m_Stream;
  /// The renderer this source is attached to
  iSndSysRendererOpenAL *m_Renderer;
  /// Do we need to tell OpenAL about changes to this source
  bool m_Update;

  /// The number of OpenAL buffers to maintain.
  static const size_t s_NumberOfBuffers = 4;

  /// The OpenAL source
  ALuint m_Source;
  /// The OpenAL buffers
  ALuint m_Buffers[s_NumberOfBuffers];
  /// The index of the last empty buffer, ie. the currently playing one.
  size_t m_CurrentBuffer;
  /// The index of the first empty buffer.
  size_t m_EmptyBuffer;

  /// The position in the stream
  size_t m_PositionMarker;
  /// The OpenAL format of the stream
  ALenum m_Format;
  /// The sampling rate of the stream
  ALuint m_SampleRate;
  /// The number of bytes per sample in the stream
  size_t m_BytesPerSample;

  /// Helper function to fill an OpenAL buffer with data.
  bool FillBuffer (ALuint buffer);
};



class SndSysSourceOpenAL3D :
  public scfImplementation3<SndSysSourceOpenAL3D,
                            iSndSysSource,
                            iSndSysSourceOpenAL,
                            iSndSysSource3D >
{
public:
  SndSysSourceOpenAL3D (csRef<iSndSysStream> stream, iSndSysRendererOpenAL *renderer);
  virtual ~SndSysSourceOpenAL3D ();

  /*
   * iSndSysSourceOpenAL interface
   */
  /**
   * Function to update the source, ie. perform any pending operations, and
   * possibly refill any pending buffers.
   *
   * @note Should only be called from the OpenAL renderer.
   * @note It is expected that the renderer has set the correct OpenAL context
   *       before calling the method.
   */
  virtual void PerformUpdate () = 0;

  /*
   * iSndSysSource interface
   */
public:
  /// Set volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
  virtual void SetVolume (float volume) = 0;
  /// Get volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
  virtual float GetVolume () = 0;

  /// Retrieve the iSoundStream attached to this source
  virtual csRef<iSndSysStream> GetStream() = 0;

  /// Retrieve a direct pointer to this object
  virtual iSndSysSource *GetPtr();

  /*
   * iSndSysSource3D interface
   */
public:
  /// set position of this source
  virtual void SetPosition(csVector3 pos) = 0;
  /// get position of this source
  virtual csVector3 GetPosition() = 0;

  /**
   * Set the greatest distance from a sound at which the sound plays at full
   * amplitude. 
   * When a listener is closer than this distance, the amplitude is the volume
   * of the sound.
   * When a listener is further than this distance, the amplitude follows the
   * formula V = (volume * ((distance/minimum_distance) ^ rolloff_factor))
   */
  virtual void SetMinimumDistance (float distance) = 0;

  /**
   * Set the greatest distance from a sound at which the sound can be heard.
   * If the distance to a listener is above this threshold, it will not be
   * mixed into the output buffer at all.  This saves a tiny bit of processing.
   */
  virtual void SetMaximumDistance (float distance) = 0;

  /**
   * Retrieve the maximum distance for which a sound is heard at full volume.
   * See SetMaximumDistance and SetMinimumDistance for distance notes.
   */
  virtual float GetMinimumDistance () = 0;

  /**
   * Retrieve the maximum distance for which a sound can be heard.  See
   * SetMaximumDistance and SetMinimumDistance for distance notes.
   */
  virtual float GetMaximumDistance () = 0;
};

#endif // #ifndef SNDSYS_RENDERER_OPENAL_SOURCE_H
