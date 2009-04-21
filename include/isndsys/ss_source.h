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

#ifndef __CS_SNDSYS_ISNDSYS_SOURCE_H__
#define __CS_SNDSYS_ISNDSYS_SOURCE_H__

/**\file
 * Sound system: source
 */

#include "csutil/scf.h"
#include "csgeom/vector3.h"
#include "isndsys/ss_structs.h"
#include "isndsys/ss_filter.h"

/**\addtogroup sndsys
 * @{ */


struct iSndSysFilter;
struct iSndSysStream;

#ifndef CS_SNDSYS_SOURCE_DISTANCE_INFINITE
#define CS_SNDSYS_SOURCE_DISTANCE_INFINITE -1.0f
#endif

/**
 * A sound source is the origin of a sound in Crystal Space. It is the object
 * through which a sound is played. Just like a speaker, only it isn't in
 * space.
 */
struct iSndSysSource : public virtual iBase
{
  SCF_INTERFACE(iSndSysSource,2,0,0);

  /// Set volume (0.0 = silence, 1.0 = as provided, 2.0 = twice as loud)
  virtual void SetVolume (float volume) = 0;
  /// Get volume (0.0 = silence, 1.0 = as provided, 2.0 = twice as loud)
  virtual float GetVolume () = 0;

  /// Retrieve the iSoundStream attached to this source
  virtual csRef<iSndSysStream> GetStream() = 0;

  /// Retrieve a direct pointer to this object
  virtual iSndSysSource *GetPtr() = 0;
};

/**
 * Sound source extensions for the software renderer.
 */
struct iSndSysSourceSoftware : public iSndSysSource
{
  SCF_INTERFACE(iSndSysSourceSoftware,2,0,0);

  /**
   * Renderer convenience interface - requests the source to fill the
   * supplied buffers
   *
   * @param frame_buffer - A pointer to an array of csSoundSample sample buffers
   *                       to be filled with sound sample data
   * @param frame_count  - The size of the buffer pointed to by frame_buffer in
   *                       frames.
   * @return - The number of frames that were actually filled.
   */
  virtual size_t MergeIntoBuffer(csSoundSample *frame_buffer, size_t frame_count) = 0;


  /// Renderer convenience interface - Called to provide processing of output filters
  virtual void ProcessOutputFilters() = 0;

  /// Add an output filter at the specified location.
  //  Output filters can only receive sound data and cannot modify it.  They will receive data
  //   from the same thread that the CS event handler executes in, once per frame.
  //
  //  Valid Locations:  SS_FILTER_LOC_SOURCEOUT, SS_FILTER_LOC_SOURCEIN
  //  
  //  Returns FALSE if the filter could not be added.
  virtual bool AddOutputFilter(SndSysFilterLocation Location, iSndSysSoftwareOutputFilter *pFilter) = 0;

  /// Remove an output filter from the registered list
  //
  //  Valid Locations:  SS_FILTER_LOC_SOURCEOUT, SS_FILTER_LOC_SOURCEIN
  //
  // Returns FALSE if the filter is not in the list at the time of the call.
  virtual bool RemoveOutputFilter(SndSysFilterLocation Location, iSndSysSoftwareOutputFilter *pFilter) = 0;
};

/**
 * Interface for OpenAL sound sources
 *
 * @note May be extended later.
 */
struct iSndSysSourceOpenAL : public virtual iBase
{
  SCF_INTERFACE(iSndSysSourceOpenAL,1,0,0);
};

/**
 * Extension to the iSndSysSource interface, allowing sources to be positioned
 * in space. 3D sound sources are attunated according to the formula:
 * V = (volume / ((distance/minimum_distance) ^ rolloff_factor))
 */
struct iSndSysSource3D : public virtual iBase
{
  SCF_INTERFACE(iSndSysSource3D,2,0,0);

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
   * formula V = (volume / ((distance/minimum_distance) ^ rolloff_factor))
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

  /// Attach a filter to this source
//  virtual bool AttachFilter(csRef<iSound2Filter> filter) = 0;

  /// Remove a filter from this source
//  virtual bool RemoveFilter(csRef<iSound2Filter> filter)= 0;
};

/**
 * Extension to the iSndSysSource3D interface, allowing simple directional
 * orientation of sound sources. The concept is a cone with it's apex at the
 * source and extendning in a given direction, with a given half-angle. Within
 * this cone the source is audible. Outside the cone, it is not.
 *
 * @todo The interface should probably be phased out and the software renderer
 *       updated to use iSndSysSource3DDirectional.
 */
struct iSndSysSource3DDirectionalSimple : public virtual iBase
{
  SCF_INTERFACE(iSndSysSource3DDirectionalSimple,2,0,0);

  /// set direction of this source
  virtual void SetDirection(csVector3 dir) = 0;
  /// get direction of this source
  virtual csVector3 GetDirection() = 0;

  /**
   * The directional radiation applies to sound that are oriented in a
   * particular direction.
   * This value is expressed in radians and describes the half-angle of a
   * cone spreading from the position of the source and opening
   * in the direction of the source.
   * Set this value to 0.0f for an omni-directional sound. 
   */
  virtual void SetDirectionalRadiation(float rad) = 0;

  /// Retrieves the current directional radiation 
  virtual float GetDirectionalRadiation() = 0;

};

/**
 * Extension to the iSndSysSource3D interface, allowing directional orientation
 * of sound sources. The concept is a set of two cones with apices at the
 * source, called the inner and outer cones. The inner cone should be contained
 * completely within the outer cone. Within the inner cone the source will be
 * at full volume (attunated according to distance). Outside the outer cone the
 * source's volume will be multiplied by the outer gain. Between the two cones
 * the volume will be interpolated between the two extremes.
 */
struct iSndSysSource3DDirectional : public virtual iBase
{
  SCF_INTERFACE(iSndSysSource3DDirectional,2,0,0);

  /**
   * set direction of this source
   * Set this to (0, 0, 0) for a omni-directional sound.
   */
  virtual void SetDirection(csVector3 dir) = 0;
  /// get direction of this source
  virtual csVector3 GetDirection() = 0;

  /**
   * This value is expressed in radians and describes the half-angle of the
   * inner cone.
   *
   * @see iSndSysSource3DDirectional
   * @note Unlike iSndSysSource3DDirectionalSimple setting the to 0.0f will not
   *       make an omni directional source
   */
  virtual void SetDirectionalRadiationInnerCone(float rad) = 0;

  /**
   * This value is expressed in radians and describes the half-angle of the
   * outer cone.
   *
   * @see iSndSysSource3DDirectional
   * @note Unlike iSndSysSource3DDirectionalSimple setting the to 0.0f will not
   *       make an omni directional source
   */
  virtual void SetDirectionalRadiationOuterCone(float rad) = 0;

  /**
   * This value describes the gain outside of the outer cone.
   *
   * @see iSndSysSource3DDirectional
   */
  virtual void SetDirectionalRadiationOuterGain(float gain) = 0;

  /// Retrieves the current half-angle of the inner cone
  virtual float GetDirectionalRadiationInnerCone() = 0;
  /// Retrieves the current half-angle of the outer cone
  virtual float GetDirectionalRadiationOuterCone() = 0;
  /// Retrieves the current gain/volume outside the outer cone
  virtual float GetDirectionalRadiationOuterGain() = 0;

};

/**
 * Extension to the iSndSysSource3D interface, allowing Doppler shift effects.
 * The Doppler effect that causes sound sources the change in pitch as their
 * relative velocities change. As an example the siren of an ambulance will
 * increase in pitch as it approaches you, and decrease once it has passed you.
 *
 * The pitch of a source is multiplied by the value
 *   doppler_factor * (speed_of_sound - listener_velocity) / (speed_of_sound + source_velocity)
 * Where the two velocities are the projections of the source and listener
 * velocities, onto the vector between them.
 */
struct iSndSysSource3DDoppler : public virtual iBase
{
  SCF_INTERFACE (iSndSysSource3DDoppler,2,0,0);

  /// Set velocity (speed) of the source
  virtual void SetVelocity (const csVector3 &Velocity) = 0;

  /// Get velocity (speed) of the source
  virtual const csVector3 &GetVelocity () = 0;
};

/** @} */

#endif // __CS_SNDSYS_ISNDSYS_SOURCE_H__
