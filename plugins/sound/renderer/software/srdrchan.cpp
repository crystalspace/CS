/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Nathaniel Saint Martin <noote@bigfoot.com>

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

#include <stdio.h>
#include <math.h>

#include "sysdef.h"
#include "cssndrdr/software/srdrchan.h"

Channel_ID Channel::lastID=1;

Channel::Channel()
{
  Active = false;
  pSoundData = NULL;
  Step = 0;
  Loop = false;
  Used = false;
  Is3D = false;
  Priority = 0;
  Ephemeral = false;
  started = false;
  user_data = NULL;

  next = NULL;

  lastID++;
  ID = lastID;
}

Channel::~Channel()
{
}

Channel_ID Channel::getID()
{
  return ID;
}

bool Channel::setSoundData(csSoundData *snd, bool toLoop)
{
  if(snd == NULL) return false;

  Step = 0;
  Loop = 0;
  Volume = 1.0;
  Pan = 0.0;
  pSoundData = snd;
  Loop = toLoop;
  computeVolume(Volume, Pan);

  Active = true;

  return true;
}

void Channel::toStep(unsigned long s)
{
  if(isActive())
  {
    if(s>=(unsigned long)pSoundData->getSize())
    {
      if(Loop)
      {
        while(s>=(unsigned long)pSoundData->getSize())
          s-=pSoundData->getSize();
        Step=s;
      }
      else
      {
        if(Ephemeral)
        {
          Active=false;
          pSoundData = NULL;
        }
        started = false;
        Step=0;
      }
    }
    else Step = s;
  }
}

void Channel::addStep(unsigned long s)
{
  toStep(Step+s);
}

void Channel::inLoop(bool loop)
{
  Loop=loop;
}

bool Channel::isActive()
{
  return Active;
}

bool Channel::is3D()
{
  return Is3D;
}

void Channel::set3D(bool set)
{
  Is3D = set;
}

int Channel::getSample()
{
  int ret = 64;

  if(pSoundData)
  {
    if(isActive() && isStarted())
    {
      if(pSoundData->is16Bits())
      {
        short *ptr = (short *) pSoundData->getData();
        ret = ptr[Step];
        addStep(1);
      }
      else //if(!pSoundData->is16Bits())
      {
        unsigned char *ptr = (unsigned char *) pSoundData->getData();
        ret = ptr[Step];
        addStep(1);
      }
    }
    else
    {
      if(pSoundData->is16Bits())
        ret = 0;
      else //if(!pSoundData->is16bits)
        ret = 128;
    }
  }

  return ret;
}

bool Channel::isStereo()
{
  if(pSoundData) return pSoundData->isStereo();
  return false;
}

void Channel::setVolume(float volume)
{
  computeVolume(volume, Pan);
}

void Channel::setVolume(float left, float right)
{
  Volume_left  = left;
  Volume_right = right;
  if(Volume_left>1) Volume_left=1.0;
  else if(Volume_left<0) Volume_left=0.0;
  if(Volume_right>1) Volume_right=1.0;
  else if(Volume_right<0) Volume_right=0.0;

  Volume = (Volume_left + Volume_right) / 2.0;
}

void Channel::setPan(float pan)
{
  computeVolume(Volume, pan);
}

void Channel::computeVolume(float volume, float pan)
{
  Volume = volume;
  if(Volume<0.0) Volume = 0.0;
  if(Volume>1.0) Volume = 1.0;
  Pan = pan;
  if(Pan<-1.0) Pan = -1.0;
  if(Pan>1.0) Pan = 1.0;

  float vl=1.0;
  float vr=1.0;
  if(Pan>0)
  {
    // sorry I don't know how calcul this
  }
  else if(Pan<0)
  {
    // sorry I don't know how calcul this
  }

  Volume_left  = Volume*vl;
  Volume_right = Volume*vr;
  if(Volume_left>1) Volume_left=1.0;
  else if(Volume_left<0) Volume_left=0.0;
  if(Volume_right>1) Volume_right=1.0;
  else if(Volume_right<0) Volume_right=0.0;
}

void Channel::setPriority(int priority)
{
  Priority = priority;
}

int Channel::getPriority()
{
  return Priority;
}

bool Channel::isStarted()
{
  return started;
}

void Channel::setStarted(bool s)
{
  started = s;
}

void Channel::setUserData(void * data)
{
  user_data=data;
}

void * Channel::getUserData()
{
  return user_data;
}

void Channel::setEphemeral(bool e)
{
  Ephemeral = e;
}

bool Channel::getEphemeral()
{
  return Ephemeral;
}
