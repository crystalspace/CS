/*
    Copyright (C) 2001 by Norman Krämer
  
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

#include <soundserver.h>
#include "cssysdef.h"
#include "artshndl.h"
#include "artsrend.h"

IMPLEMENT_IBASE (csArtsHandle)
  IMPLEMENTS_INTERFACE (iSoundHandle)
  IMPLEMENTS_INTERFACE (iSoundSource)
IMPLEMENT_IBASE_END

csArtsHandle::csArtsHandle (csArtsRenderer *pRend)
{
  CONSTRUCT_IBASE (NULL);
  this->pRend = pRend;
}

csArtsHandle::~csArtsHandle ()
{
}

bool csArtsHandle::IsStatic()
{
  return true;
}

bool csArtsHandle::UseData (iSoundData *sd)
{
  this->sd = sd;
  am = *pRend->CreateArtsModule ();
  if (!am.isNull())
  {
    // force a format we like
    csSoundFormat format;
    format.Freq = 44100;
    format.Bits = 16;
    format.Channels = 2;

    sd->Initialize (&format);

    std::vector<float> vData (sd->GetStaticNumSamples ()*2);
    short *data = (short*)sd->GetStaticData ();
    for (long i=0; i < sd->GetStaticNumSamples ()*2; i++)
      vData[i] = (float)data[i];
    
    am.SetData (vData);
    return true;
  }
  return false;
}
  
void csArtsHandle::Play(bool Loop)
{
  iSoundSource *ss = CreateSource (SOUND3D_DISABLE);
  if (ss)
  {
    ss->Play (Loop ? SOUND_LOOP : SOUND_RESTART);
    ss->DecRef ();
  }
}

iSoundSource *csArtsHandle::CreateSource(int Mode3d)
{
  return pRend->CreateSource (this, Mode3d);
}


void csArtsHandle::StartStream(bool Loop)
{
  (void)Loop;
}

void csArtsHandle::StopStream()
{
}

void csArtsHandle::ResetStream()
{
}

void csArtsHandle::Play (unsigned long playMethod)
{
  am.Play (playMethod);
}

void csArtsHandle::Stop ()
{
  am.Stop ();
}

void csArtsHandle::SetVolume (float volume)
{
  am.SetVolume (volume);
  this->volume = volume;
}

float csArtsHandle::GetVolume ()
{
  return volume;
}

void csArtsHandle::SetFrequencyFactor (float factor)
{
  am.SetFrequencyFactor (factor);
  this->frequencyfactor = factor;
}

float csArtsHandle::GetFrequencyFactor ()
{
  return frequencyfactor;
}

void csArtsHandle::SetMode3D(int m)
{
  am.Set3DType ((Arts::Sound3DType)m);
  Mode3D = m;
}

int csArtsHandle::GetMode3D()
{
  return Mode3D;
}

void csArtsHandle::SetPosition(csVector3 thePos)
{
  am.SetSoundPosition (thePos.x, thePos.y, thePos.z);
  pos = thePos;
}

csVector3 csArtsHandle::GetPosition()
{
  return pos;
}

void csArtsHandle::SetVelocity(csVector3 spd)
{
  am.SetVelocity (spd.x, spd.y, spd.z);
  speed = spd;
}

csVector3 csArtsHandle::GetVelocity()
{
  return speed;
}


void csArtsHandle::SetDirection (const csVector3 &Front, const csVector3 &Top)
{
  am.SetOrientation (Front.x, Front.y, Front.z, Top.x, Top.y, Top.z);
}

void csArtsHandle::SetDistanceFactor (float factor)
{
  am.SetDistanceFactor (factor);
}

void csArtsHandle::SetRollOffFactor (float factor)
{
  am.SetRollOffFactor (factor);
}

void csArtsHandle::SetDopplerFactor (float factor)
{
  am.SetDopplerFactor (factor);
}

void csArtsHandle::SetHeadSize (float size)
{
  am.SetHeadSize (size);
}


