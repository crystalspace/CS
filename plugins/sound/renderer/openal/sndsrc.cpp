/*
    Copyright (C) 2002 by Jorrit Tyberghein, Daniel Duhprey

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
#include "csutil/scf.h"

#include "sndsrc.h"
#include "sndhdl.h"
#include "sndrdr.h"

#include <AL/al.h>

SCF_IMPLEMENT_IBASE(csSoundSourceOpenAL)
  SCF_IMPLEMENTS_INTERFACE(iSoundSource)
SCF_IMPLEMENT_IBASE_END;

csSoundSourceOpenAL::csSoundSourceOpenAL(csSoundRenderOpenAL *rdr, 
	csSoundHandleOpenAL *hdl)
{
  SCF_CONSTRUCT_IBASE(hdl);

  parent = rdr;
  handle = hdl;

  alGenSources (1, &source);
  ALuint buffer = handle->GetID();
  alSourceQueueBuffers (source, 1, &buffer);
  alSourcei (source, AL_LOOPING, AL_FALSE);
  
  mode = SOUND3D_ABSOLUTE;
  alSourcei (source, AL_SOURCE_RELATIVE, AL_FALSE);
}

csSoundSourceOpenAL::~csSoundSourceOpenAL() 
{
}

void csSoundSourceOpenAL::SetPosition(csVector3 v)
{
  position[0] = v.x; position[1] = v.y; position[2] = v.z;
  alSourcefv (source, AL_POSITION, position);
}

void csSoundSourceOpenAL::SetVelocity(csVector3 v)
{
  velocity[0] = v.x; velocity[1] = v.y; velocity[2] = v.z;
  alSourcefv (source, AL_VELOCITY, velocity);
}

void csSoundSourceOpenAL::SetVolume(float vol)
{
  alSourcef (source, AL_GAIN, vol); 
}

float csSoundSourceOpenAL::GetVolume()
{
  float vol;
  alGetSourcef (source, AL_GAIN, &vol);
  return vol;
}

void csSoundSourceOpenAL::SetFrequencyFactor (float factor)
{
  alSourcef (source, AL_PITCH, factor);
}

float csSoundSourceOpenAL::GetFrequencyFactor ()
{
  float factor;
  alGetSourcef (source, AL_PITCH, &factor);
  return factor;
}

void csSoundSourceOpenAL::SetMode3D(int m) {
  mode = m;
  switch (mode) {
  case SOUND3D_ABSOLUTE:
    alSourcei (source, AL_SOURCE_RELATIVE, AL_FALSE);
    break;
  case SOUND3D_RELATIVE:
    alSourcei (source, AL_SOURCE_RELATIVE, AL_TRUE);
    break;
  }
}

void csSoundSourceOpenAL::Play(unsigned long PlayMethod)
{
  if (PlayMethod & SOUND_LOOP) {
  	alSourcei (source, AL_LOOPING, AL_TRUE);
  }
  alSourcePlay (source);
}

void csSoundSourceOpenAL::Stop()
{
  alSourceStop (source);
}
