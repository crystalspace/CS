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

#ifndef __SOUND_RENDER_SOFTWARE_H__
#define __SOUND_RENDER_SOFTWARE_H__

// SoundRender.H
// csSoundRenderSoftware class.

#include "csutil/scf.h"
#include "cssfxldr/common/snddata.h"
#include "cssndrdr/software/srdrchan.h"
#include "isndrdr.h"
#include "isnddrv.h"
#include "iplugin.h"

class csSoundListenerSoftware;

class csSoundRenderSoftware : public iSoundRender
{
public:
	DECLARE_IBASE;
	csSoundRenderSoftware(iBase *piBase);
	virtual ~csSoundRenderSoftware();
	
	//implementation of iPlugin
	virtual bool Initialize (iSystem *iSys);
	
	//implementation of iSoundRender
	virtual bool Open ();
	virtual void Close ();
	virtual void Update ();
	virtual void SetVolume (float vol);
	virtual float GetVolume ();
	virtual void PlayEphemeral (csSoundData *snd);
	virtual iSoundListener *GetListener ();
	virtual iSoundSource *CreateSource (csSoundData *snd);
	virtual iSoundBuffer *CreateSoundBuffer (csSoundData *snd);
	virtual void MixingFunction ();
	
public:
	void CalculEars3D();
	void CalculSound3D(Channel *c);
	iSystem* m_piSystem;
	iSoundDriver *m_piSoundDriver;
	
	/// print to the system's device
	void SysPrintf(int mode, char* str, ...);
private:
	/// list of all stored channels
	Channel *AllChannels;
	
	/// delete a channel of the list and destroy it
	bool delChannel (Channel_ID id);
	/// add a channel
	void addChannel(Channel *c);
	/// this function choice in the 'list' of channels which are the 'n' channels will be mixed 
	int setChannels();
	/// update the channels in the 'list' whose aren't be choiced by setChannels()
	void updateChannels();
	/// kill the channels in 'list' which are finish to be played
	void killChannels();
	
	/// the channels
	Channel ** Channels;
	int numberChannels;
	
	/// current priority function choice
	int PriorityFunc;
	
	/// is 16 bit mode device
	bool is16Bits();
	/// is a stereo device
	bool isStereo();
	/// return frequency
	int getFrequency();
	
	void *memory;
	int memorysize;
	
	float earL_x, earL_y, earL_z;
	float earR_x, earR_y, earR_z;
	float earL_DistanceFactor;
	float earR_DistanceFactor;
	
	csSoundListenerSoftware * m_pListener;
};

#endif	//__NETWORK_DRIVER_SOFTWARE_H__

