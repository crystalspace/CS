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
#include "cssysdef.h"
#include "alsadrv.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(csSoundDriverALSA)


SCF_IMPLEMENT_IBASE(csSoundDriverALSA)
  SCF_IMPLEMENTS_INTERFACE(iSoundDriver)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundDriverALSA::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSoundDriverALSA::AudioDevice::AudioDevice()
{
  audio = false;
  samplesize = 0;

  streamtype = SND_PCM_STREAM_PLAYBACK; // we want playback only

}

bool csSoundDriverALSA::AudioDevice::Open(int& frequency, bool& bit16, bool& stereo,
  int& fragments, int& fragmentsize)
{
  unsigned int rate = (unsigned int)frequency;
  int bytes_per_second;
  int dir;
  int err;

  // @@@ configurable via cmdline ?
  pcm_name = strdup("plughw:0,0");

  snd_pcm_hw_params_alloca(&hwparams);
  if (snd_pcm_open(&pcm_handle, pcm_name, streamtype, 0) < 0) 
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	      "crystalspace.sound.alsa", "Error opening PCM device %s\n", 
	      pcm_name);;
    return false;
  }

  
  if ((err=snd_pcm_hw_params_any(pcm_handle, hwparams)) < 0) 
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	      "crystalspace.sound.alsa", 
	      "Can not configure this PCM device (%s).\n %s !\n",  
	      pcm_name, snd_strerror(err));
    return false;
  }

  if (snd_pcm_hw_params_set_access(pcm_handle, hwparams, 
				   SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	      "crystalspace.sound.alsa", 
	      "Error setting INTERLEAVED access for PCM device (%s).\n", 
	      pcm_name);
    return false;
  }
  
  /* Set sample format */
  if (snd_pcm_hw_params_set_format(pcm_handle, hwparams, 
				   bit16 
				   ? SND_PCM_FORMAT_S16_LE
				   : SND_PCM_FORMAT_S8) < 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	      "crystalspace.sound.alsa", 
	      "Error setting sample format for PCM device (%s).\n", pcm_name);
    return false;
  }

  /* Set number of channels */
  if (snd_pcm_hw_params_set_channels(pcm_handle, hwparams, stereo ? 2 : 1) < 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	      "crystalspace.sound.alsa", 
	      "Error setting channels for PCM device (%s).\n", pcm_name);
    return false;
  }


  /* Set sample rate. If the exact rate is not supported */
  /* by the hardware, use nearest possible rate.         */
#ifndef ALSA_PCM_OLD_HW_PARAMS_API
  if (snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &rate, &dir) < 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
	      "crystalspace.sound.alsa", 
	      "The rate %d Hz is not available for your hardware (%s).\n",
	      frequency, pcm_name);
    return false;
  }
  if (rate != (unsigned int)frequency) 
#else
  rate = snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, frequency, &dir);
  if (dir != 0)
#endif
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
	      "crystalspace.sound.alsa", 
	      "The rate %d Hz is not supported by your hardware (%s).\n"
	      " ==> Using %u Hz instead.\n", 
	      frequency, pcm_name, rate);
    frequency = rate;
  }

  samplesize = (stereo ? 2 : 1) * (bit16 ? 2 : 1);
  bytes_per_second = frequency * samplesize;
  fragmentsize = 8192;
  fragments = (bytes_per_second + fragmentsize - 1) / fragmentsize;

  /* Set number of periods. Periods used to be called fragments. */ 
  if (snd_pcm_hw_params_set_periods(pcm_handle, hwparams, fragments, 0) < 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	      "crystalspace.sound.alsa", 
	      "Error setting fragments for PCM device (%s).\n", pcm_name);
    return false;
  }

  if (snd_pcm_hw_params_set_buffer_size(pcm_handle, hwparams, 
					(fragmentsize * fragments)>>samplesize) < 0) {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	      "crystalspace.sound.alsa", 
	      "Error setting buffersize for PCM device (%s).\n", pcm_name);
    return false;
  }
  
  /* commit our configuration */
  if (snd_pcm_hw_params(pcm_handle, hwparams) < 0) 
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	      "crystalspace.sound.alsa", 
	      "Error setting Hardwareparams for PCM device (%s).\n", pcm_name);
    return false;
  }

  audio = true;
  return true;
}

void csSoundDriverALSA::AudioDevice::Close()
{
  if (audio)
  {
    /* play pending sound fragments and then shut down */
    snd_pcm_drain(pcm_handle);
    audio = false;
  }
}

int csSoundDriverALSA::AudioDevice::recovery(int err)
{
  if (err == -EPIPE) /* under-run */
  {    
    err = snd_pcm_prepare(pcm_handle);
    if (err < 0)
      csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
		"crystalspace.sound.alsa", 
		"Can't recovery from underrun, prepare failed: %s\n", 
		snd_strerror(err));
    return 0;
  } else if (err == -ESTRPIPE) {
    while ((err = snd_pcm_resume(pcm_handle)) == -EAGAIN)
      sleep(1);       /* wait until the suspend flag is released */
    if (err < 0) 
    {
      err = snd_pcm_prepare(pcm_handle);
      if (err < 0)
	csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
		  "crystalspace.sound.alsa", 
		  "Can't recovery from suspend, prepare failed: %s\n", 
		  snd_strerror(err));
    }
    return 0;
  }
  return err;
}

void csSoundDriverALSA::AudioDevice::Play(unsigned char *snddata, int len)
{
  int samples = len / samplesize;
  snd_pcm_sframes_t written;

  while (audio && samples > 0) {
    written = snd_pcm_writei(pcm_handle, snddata, samples);
    if (written == -EAGAIN)
      continue;
    if (written < 0) {
      if (recovery(written) < 0) 
      {
	csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
		  "crystalspace.sound.alsa", 
		  "Write error: %s\n", snd_strerror(written));
	audio = false;
      }
      break;
    }
    snddata += written * samplesize;
    samples -= written;
  }
}

csSoundDriverALSA::csSoundDriverALSA(iBase *piBase)
{
  SCF_CONSTRUCT_IBASE(piBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  object_reg = 0;
  memorysize = 0;
  memory = 0;
  block_size=0;
  writeblock = 0;
  fragments = 0;
  soundbuffer = 0;
}

csSoundDriverALSA::~csSoundDriverALSA()
{
  Close ();
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csSoundDriverALSA::Open(iSoundRender *, int frequency, 
			     bool bit16, bool stereo)
{
  csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
  	"crystalspace.sound.alsa","SoundDriver ALSA selected");

  m_bStereo = stereo;
  m_b16Bits = bit16;
  m_nFrequency = frequency;

  if (device.Open(frequency,bit16,stereo, fragments, block_size))
  {
    soundbuffer = new unsigned char[fragments * block_size];
    if (soundbuffer == 0)
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
		"crystalspace.sound.alsa",
		"Not enough memory to allocate soundbuffer.");
      return false;
    }
    
  }
  else
    return false;

  return true;
}

void csSoundDriverALSA::Close()
{
  delete[] soundbuffer; 
  soundbuffer=0;
  device.Close();
  memory=0;
  memorysize=0;
}

void csSoundDriverALSA::LockMemory(void **mem, int *memsize)
{
  *mem = &soundbuffer[writeblock * block_size];
  *memsize = block_size;
}

void csSoundDriverALSA::UnlockMemory()
{
  device.Play(&soundbuffer[writeblock * block_size], block_size);
  writeblock = (writeblock+1) % fragments;
}

bool csSoundDriverALSA::IsHandleVoidSound() { return true; }
bool csSoundDriverALSA::IsBackground() { return true; }
bool csSoundDriverALSA::Is16Bits() { return  m_b16Bits; }
bool csSoundDriverALSA::IsStereo() { return m_bStereo; }
int csSoundDriverALSA::GetFrequency() { return m_nFrequency; }
