/*
	Copyright (C) 2004 by Norman Kraemer

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

#ifndef __CS_ALSADRV_H__
#define __CS_ALSADRV_H__

#include "csutil/scf.h"
#include "isound/driver.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

#include <alsa/version.h>
#if SND_LIB_MAJOR==0
#define ALSA_PCM_OLD_HW_PARAMS_API
#endif
#include <alsa/asoundlib.h>

class csSoundDriverALSA : public iSoundDriver
{

  /// Audio device.
  class AudioDevice
  {
  public:
    AudioDevice();

    /// Open device with specified parameters.
    bool Open(int& frequency, bool& bit16, bool& stereo, int& fragments,
	      int& block_size);

    /// Close device.
    void Close();
    /// Play sound.
    void Play(unsigned char *snddata, int len);
    /// recover from error if possible
    int recovery(int err);

    bool audio; // is audio on ?
    int samplesize; // size of a sample in bytes?
    iObjectRegistry *object_reg;

    snd_pcm_t *pcm_handle;// alsa handle
    snd_pcm_stream_t streamtype; // type : playback or capture
    snd_pcm_hw_params_t *hwparams; // to config device
    char *pcm_name; // hardware device name, ala plughw:0,0

  } device;


protected:
  iObjectRegistry* object_reg;
  void * memory;
  int memorysize;
  int m_nFrequency;
  bool m_b16Bits;
  bool m_bStereo;
  int fragments;
  int block_size;
  int writeblock;
  unsigned char *soundbuffer;

public:
  SCF_DECLARE_IBASE;
  csSoundDriverALSA(iBase *iParent);
  virtual ~csSoundDriverALSA();

  bool Initialize(iObjectRegistry *object_reg);

  bool Open(iSoundRender *render, int frequency, bool bit16, bool stereo);
  void Close();

  void LockMemory(void **mem, int *memsize);
  void UnlockMemory();
  bool IsBackground();
  bool Is16Bits();
  bool IsStereo();
  int GetFrequency();
  bool IsHandleVoidSound();
  bool ThreadAware (){ return true; }

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundDriverALSA);
    virtual bool Initialize (iObjectRegistry* p)
    { scfParent->object_reg = p; scfParent->device.object_reg = p; return true;}
  } scfiComponent;
  friend struct eiComponent;

};

#endif // __CS_ALSADRV_H__
