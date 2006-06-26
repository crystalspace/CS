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

#include "listener.h"

SndSysListenerOpenAL::SndSysListenerOpenAL () :
  scfImplementationType(this),
  Front (csVector3 (0, 0, -1)), Top (csVector3 (0, 1, 0)),
  Position (csVector3 (0, 0, 0)), Distance (1.0), RollOff (-1.0),
  Volume (1.0), update (true)
{
  alListener3f( AL_VELOCITY, 0, 0, 0 );
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
  this->Front = Front; this->Top = Top;
  update = true;
}

void SndSysListenerOpenAL::SetPosition (const csVector3 &pos)
{
  Position = pos;
  update = true;
}

void SndSysListenerOpenAL::SetDistanceFactor (float factor)
{
  Distance = factor;
  update = true;
}

void SndSysListenerOpenAL::SetRollOffFactor (float factor)
{
  RollOff = factor;
  update = true;
}

void SndSysListenerOpenAL::GetDirection (csVector3 &Front, csVector3 &Top)
{
  Front = this->Front; Top = this->Top;
}

const csVector3 &SndSysListenerOpenAL::GetPosition ()
{
  return this->Position;
}

float SndSysListenerOpenAL::GetDistanceFactor ()
{
  return this->Distance;
}

float SndSysListenerOpenAL::GetRollOffFactor ()
{
  return this->RollOff;
}

/*
  * SndSysListenerOpenAL implementation
  */
float SndSysListenerOpenAL::SetVolume (float vol)
{
  Volume = vol;
  update = true;
}

float SndSysListenerOpenAL::GetVolume ()
{
  return Volume;
}

void SndSysListenerOpenAL::Update ()
{
  if (update == true)
  {
    update = false;
    alListener3f (AL_POSITION, Position.x, Position.y, Position.z);
    alListenerf (AL_GAIN, Volume);
    float orientation[] = { Front.x, Front.y, Front.z, Top.x, Top.y, Top.z };
    alListenerfv (AL_ORIENTATION, orientation);
  }
}
