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

#ifndef __CHANNEL_H
#define __CHANNEL_H

#include "cssfxldr/common/snddata.h"

typedef unsigned long Channel_ID;

/**
 * channel class :
 * store a SoundBuffer, set its volume, activity, pan, priority...
 */
class Channel
{
public:
  ///
  Channel();
  ///
  ~Channel();

  ///
  void setVolume(float Volume);
  ///
  void setPan(float Pan);
  ///
  void setVolume(float left, float right);

  ///
  bool isActive();

  ///
  bool is3D();
  void set3D(bool set = true);

  ///
  bool getEphemeral();
  ///
	void setEphemeral(bool e = true);

  /// return current sample and go to next step
  int getSample();
  ///
  void inLoop(bool loop);
  ///
  void toStep(unsigned long s);
  ///
  void addStep(unsigned long s);
  ///
  void * getUserData();
  ///
  void setUserData(void *data);

  ///
  void setStarted(bool sm=true);
  ///
  bool isStarted();

  ///
  int getPriority();
  ///
  void setPriority(int priority);
  ///
  bool isStereo();

  ///
  bool setSoundData(csSoundData *snd, bool toLoop = false);
  csSoundData *pSoundData;

  float Volume;
  float Pan;
  float Volume_left;
  float Volume_right;
  unsigned long Step;
  bool Loop;
  int Priority;
  bool Is3D;

  bool Used;

  Channel *next;

  ///
  Channel_ID getID();
  bool Active;
  
private:
	bool Ephemeral;
  bool started;

  void * user_data;

  ///
  void computeVolume(float volume, float pan);
  Channel_ID ID;
  static Channel_ID lastID;
};

#endif //__CHANNEL_H
