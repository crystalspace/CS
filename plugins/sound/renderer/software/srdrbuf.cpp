/*
    Copyright (C) 1998, 1999 by Nathaniel 'NooTe' Saint Martin
    Copyright (C) 1998, 1999 by Jorrit Tyberghein
    Written by Nathaniel 'NooTe' Saint Martin

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

#include "sysdef.h"
#include "csutil/scf.h"
#include "cssndrdr/software/srdrbuf.h"
#include "cssndrdr/software/srdrsrc.h"
#include "isystem.h"

IMPLEMENT_FACTORY (csSoundBufferSoftware)

EXPORT_CLASS_TABLE (glwin32)
  EXPORT_CLASS (csSoundBufferSoftware, "crystalspace.sound.render.software",
    "Software sound driver for Crystal Space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE(csSoundBufferSoftware)
  IMPLEMENTS_INTERFACE(iSoundBuffer)
IMPLEMENT_IBASE_END;

csSoundBufferSoftware::csSoundBufferSoftware(iBase *piBase)
{
	CONSTRUCT_IBASE(piBase);
}

csSoundBufferSoftware::~csSoundBufferSoftware()
{
}

iSoundSource *csSoundBufferSoftware::CreateSource()
{
  csSoundSourceSoftware* pNew = new csSoundSourceSoftware (NULL);
  if(!pNew) return NULL;

  pNew->pSoundBuffer = this;

  return QUERY_INTERFACE(pNew, iSoundSource);
}

void csSoundBufferSoftware::Play(SoundBufferPlayMethod playMethod)
{
  setStarted(true);
  if(playMethod == SoundBufferPlay_InLoop)
    inLoop(true);
  else
    inLoop(false);
}

void csSoundBufferSoftware::Stop()
{
}

void csSoundBufferSoftware::SetVolume(float vol)
{
  fVolume = vol;
  setVolume(fVolume);
}

float csSoundBufferSoftware::GetVolume()
{
  return Channel::Volume;
}

void csSoundBufferSoftware::SetFrequencyFactor(float factor)
{
  fFrequencyFactor = factor;
}

float csSoundBufferSoftware::GetFrequencyFactor()
{
  return fFrequencyFactor;
}

