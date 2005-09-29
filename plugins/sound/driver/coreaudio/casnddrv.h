// casnddrv.h
// CoreAudio (MacOS/X) Sound Driver for Crystal Space
//
// Created by mreda on Sun Nov 11 2001.
// Copyright (c) 2001 Matt Reda. All rights reserved.

#ifndef __CS_CASNDDRV_H__
#define __CS_CASNDDRV_H__

#include <CoreAudio/CoreAudio.h>

#include "csutil/scf.h"
#include "isound/driver.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"


// CoreAudio driver for CrystalSpace
// This driver has a severe limitation - it forces 16 bit stereo mode, and
// the frequency requested is ignored and the frequency of the audio device
// is used.  I could find no documentation on how to request other formats
// It should be possible to fake mono but I haven't done that yet


class csSoundDriverCoreAudio : public iSoundDriver
{
public:
  SCF_DECLARE_IBASE;

  // Constructor/destructor
  csSoundDriverCoreAudio(iBase *base);
  virtual ~csSoundDriverCoreAudio();

  // Open the sound render
  virtual bool Open(iSoundRender *render, int freq, bool bit16, bool stereo);

  // Close the sound render
  virtual void Close();

  // Lock and Get Sound Memory Buffer
  virtual void LockMemory(void **mem, int *memsize);

  // Unlock Sound Memory Buffer
  virtual void UnlockMemory();

  // Must the driver be updated manually or does it run in background?
  virtual bool IsBackground();

  // Is the driver in 16 bits mode ?
  virtual bool Is16Bits();

  // Is the driver in stereo mode ?
  virtual bool IsStereo();

  // Get current frequency of driver
  virtual int GetFrequency();

  // Is the sound driver able to create silence without locking and
  // writing to the sound memory?
  virtual bool IsHandleVoidSound();

  // Create samples and copy them into buffer
  void CreateSamples(float *buffer);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundDriverCoreAudio);
    virtual bool Initialize(iObjectRegistry* object_reg)
    { scfParent->reg = object_reg; return true; }
  } scfiComponent;
  friend struct eiComponent;

protected:
  iObjectRegistry *reg;		// Object registry
  iSoundRender *soundRender;	// Associated sound renderer
  short *memory;		// Sound memory
  int memorySize;		// Size of memory buffer
  int frequency;		// Sound frequency
  bool is16Bit;			// true if sound is 16 bit
  bool isStereo;		// true if sound is stereo

  AudioDeviceID audioDevice;	// ID of audio output device to use

  bool isPlaying;		// Boolean to indicate if playback is going
};

#endif // __CS_CASNDDRV_H__
