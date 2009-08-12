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
#if defined(CS_OPENAL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENAL_PATH,al.h)
#else
#include <AL/al.h>
#endif

#include "listener.h"

SndSysListenerOpenAL::SndSysListenerOpenAL () :
  scfImplementationType(this),
  m_Front (csVector3 (0, 0, -1)), m_Top (csVector3 (0, 1, 0)),
  m_Position (csVector3 (0, 0, 0)), m_Distance (1.0), m_RollOff (1.0),
  m_Volume (1.0), m_Velocity (csVector3 (0, 0, 0)), m_DopplerFactor (1.0),
  m_SpeedOfSound(343.3f), m_Update (true), m_ExternalUpdate (true)
{
  alListener3f( AL_VELOCITY, 0, 0, 0 );
  alDistanceModel( AL_EXPONENT_DISTANCE_CLAMPED );

  Update ();
}

SndSysListenerOpenAL::~SndSysListenerOpenAL ()
{
}

/*
 * iSndSysListener interface
 */
void SndSysListenerOpenAL::SetDirection (const csVector3 &Front, const csVector3 &Top)
{
  m_Front = Front;
  m_Top = Top;
  m_Update = true;
}

void SndSysListenerOpenAL::SetPosition (const csVector3 &pos)
{
  m_Position = pos;
  m_Update = true;
}

void SndSysListenerOpenAL::SetDistanceFactor (float factor)
{
  m_Distance = factor;
  m_ExternalUpdate = true;
}

void SndSysListenerOpenAL::SetRollOffFactor (float factor)
{
  m_RollOff = factor;
  m_ExternalUpdate = true;
}

void SndSysListenerOpenAL::GetDirection (csVector3 &Front, csVector3 &Top)
{
  Front = m_Front;
  Top = m_Top;
}

const csVector3 &SndSysListenerOpenAL::GetPosition ()
{
  return m_Position;
}

float SndSysListenerOpenAL::GetDistanceFactor ()
{
  return m_Distance;
}

float SndSysListenerOpenAL::GetRollOffFactor ()
{
  return m_RollOff;
}

/*
 * iSndSysListenerDoppler interface
 */

void SndSysListenerOpenAL::SetVelocity (const csVector3 &Velocity)
{
  m_Velocity = Velocity;
  m_Update = true;
}

void SndSysListenerOpenAL::SetDopplerFactor (const float DopplerFactor)
{
  m_DopplerFactor = DopplerFactor;
  m_Update = true;
}

void SndSysListenerOpenAL::SetSpeedOfSound (const float SpeedOfSound)
{
  m_SpeedOfSound = SpeedOfSound;
  m_Update = true;
}

const csVector3 &SndSysListenerOpenAL::GetVelocity ()
{
  return m_Velocity;
}

float SndSysListenerOpenAL::GetDopplerFactor ()
{
  return m_DopplerFactor;
}

float SndSysListenerOpenAL::GetSpeedOfSound ()
{
  return m_SpeedOfSound;
}

/*
 * SndSysListenerOpenAL implementation
 */
void SndSysListenerOpenAL::SetVolume (float vol)
{
  m_Volume = vol;
  m_Update = true;
}

float SndSysListenerOpenAL::GetVolume ()
{
  return m_Volume;
}

bool SndSysListenerOpenAL::Update ()
{
  if (m_Update == true)
  {
    m_Update = false;
    alListener3f (AL_POSITION, m_Position.x, m_Position.y, m_Position.z);
    alListenerf (AL_GAIN, m_Volume);
    float orientation[] = { -m_Front.x, -m_Front.y, -m_Front.z, m_Top.x, m_Top.y, m_Top.z };
    alListenerfv (AL_ORIENTATION, orientation);
    alListener3f (AL_VELOCITY, m_Velocity.x, m_Velocity.y, m_Velocity.z);
    alDopplerFactor (m_DopplerFactor);
    alDopplerVelocity (m_SpeedOfSound);
  }
  if (m_ExternalUpdate == true)
  {
    m_ExternalUpdate = false;
    return true;
  }
  return false;
}
