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

#include <stdarg.h>
#include <stdio.h>

#include "sysdef.h"
#include "qint.h"
#include "cscom/com.h"
#include "cssndrdr/software/srdrcom.h"
#include "cssndrdr/software/srdrlst.h"
#include "cssndrdr/software/srdrsrc.h"
#include "cssndrdr/software/srdrbuf.h"
#include "csutil/inifile.h"
#include "isystem.h"
#include "isndlstn.h"
#include "isndsrc.h"
#include "isndbuf.h"

csIniFile* configsndsoft;

IMPLEMENT_UNKNOWN_NODELETE (csSoundRenderSoftware)

BEGIN_INTERFACE_TABLE(csSoundRenderSoftware)
  IMPLEMENTS_INTERFACE(ISoundRender)
END_INTERFACE_TABLE()

// some defines for priority function
#define Low_Priority -50
#define Normal_Priority 0
#define High_Priority 50

/// list of priority algorythm used
enum PRIORITY_FUNC
{
  Priority_None=1,
  Priority_Up,
  Priority_Down
};

csSoundRenderSoftware::csSoundRenderSoftware(ISystem* piSystem):m_pListener(NULL)
{
  HRESULT hRes;
  m_piSystem = piSystem;

  configsndsoft = new csIniFile ("sndsoft.cfg");

#ifdef SOUND_DRIVER
  char *szSoundDriver = SOUND_DRIVER;	// "crystalspace.sound.driver.xxx"
#else
  char *szSoundDriver = "crystalspace.sound.driver.null";
#endif
  ISoundDriverFactory* piFactory = NULL;
  CLSID clsidSoundDriver;
  
  //ASSERT( piSystem );
  m_piSystem = piSystem;
  hRes = csCLSIDFromProgID( &szSoundDriver, &clsidSoundDriver );

  if (FAILED(hRes))
  {	  
    SysPrintf(MSG_FATAL_ERROR, "Error! SoundDriver DLL with ProgID \"%s\" not found on this system.", szSoundDriver);
    exit(0);
  }

  hRes = csCoGetClassObject( clsidSoundDriver, CLSCTX_INPROC_SERVER, NULL, (REFIID)IID_ISoundDriverFactory, (void**)&piFactory );
  if (FAILED(hRes))
  {
    SysPrintf(MSG_FATAL_ERROR, "Error! Couldn't create sound driver instance.");
    exit(0);
  }

  hRes = piFactory->CreateInstance( (REFIID)IID_ISoundDriver, m_piSystem, (void**)&m_piSoundDriver );
  if (FAILED(hRes))
  {
    SysPrintf(MSG_FATAL_ERROR, "Error! Couldn't create sound driver instance.");
    exit(0);
  }

  FINAL_RELEASE( piFactory );

  AllChannels = NULL;

  numberChannels = configsndsoft->GetInt("SoundRender.software", "MAX_CHANNELS", 16);
  CHK (Channels = new Channel* [numberChannels]);
  for (int i=0; i<numberChannels; i++)
    Channels [i]=NULL;

  earL_x = earL_y = earL_z = 0.0;
  earR_x = earR_y = earR_z = 0.0;

  (void)new csSoundListenerSoftware ();
}

csSoundRenderSoftware::~csSoundRenderSoftware()
{
  FINAL_RELEASE (m_piSoundDriver);

  if (Channels)
  {
    CHKB (delete [] Channels);
    Channels = NULL;
  }

  if(m_pListener)
    delete m_pListener;
}

STDMETHODIMP csSoundRenderSoftware::GetListener(ISoundListener ** ppv )
{
  if (!m_pListener)
  {
    *ppv = NULL;
    return E_OUTOFMEMORY;
  }

  return m_pListener->QueryInterface (IID_ISoundListener, (void**)ppv);
}

STDMETHODIMP csSoundRenderSoftware::CreateSource(ISoundSource ** ppv, csSoundData *snd)
{
  ISoundBuffer* pNewBuffer=NULL;

  CreateSoundBuffer(&pNewBuffer, snd);

  if(!pNewBuffer)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }

  csSoundBufferSoftware *pNew = (csSoundBufferSoftware*)pNewBuffer;

  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }
  
  pNew->set3D ();

  return pNew->CreateSource(ppv);
}

STDMETHODIMP csSoundRenderSoftware::CreateSoundBuffer(ISoundBuffer** ppv, csSoundData *snd)
{
  if (snd == NULL)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }

  snd->convertFrequencyTo(getFrequency());
  snd->convert16bitTo(is16Bits());

  CHK (csSoundBufferSoftware* pNew = new csSoundBufferSoftware ());
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }

  pNew->setSoundData (snd, false);
  pNew->setPriority (Normal_Priority);

  addChannel(pNew);
  
  return pNew->QueryInterface (IID_ISoundBuffer, (void**)ppv);
}

bool csSoundRenderSoftware::is16Bits()
{
  bool ret = true;
  m_piSoundDriver->Is16Bits(&ret);
  return ret;
}

bool csSoundRenderSoftware::isStereo()
{
  bool ret = true;
  m_piSoundDriver->IsStereo(&ret);
  return ret;
}

int csSoundRenderSoftware::getFrequency()
{
  int ret = 22050;
  m_piSoundDriver->GetFrequency(&ret);
  return ret;

}

STDMETHODIMP csSoundRenderSoftware::Open()
{
  SysPrintf (MSG_INITIALIZATION, "\nSoundRender Software selected\n");

  m_piSoundDriver->Open(this,
    configsndsoft->GetInt("SoundRender.software", "FREQUENCY", 22050),
    configsndsoft->GetInt("SoundRender.software", "16BITS", true),
    configsndsoft->GetInt("SoundRender.software", "STEREO", true));

  SysPrintf (MSG_INITIALIZATION, " Use %d mixing channels maximum\n", numberChannels);

  return S_OK;
}

STDMETHODIMP csSoundRenderSoftware::Close()
{
  m_piSoundDriver->Close();
  return S_OK;
}

STDMETHODIMP csSoundRenderSoftware::Update()
{
  bool res = false;

  // Is current sound driver work in a background thread ?
  m_piSoundDriver->IsBackground(&res);
  
  if(!res) // No !
  {
    // update sound data
    MixingFunction();
  }

  return S_OK;
}

STDMETHODIMP csSoundRenderSoftware::SetVolume(float vol)
{
  m_piSoundDriver->SetVolume(vol);

  return S_OK;
}

STDMETHODIMP csSoundRenderSoftware::GetVolume(float *vol)
{
  m_piSoundDriver->GetVolume(vol);

  return S_OK;
}

STDMETHODIMP csSoundRenderSoftware::PlayEphemeral(csSoundData *snd)
{
  if (snd == NULL)
    return 0;

  snd->convertFrequencyTo(getFrequency());
  snd->convert16bitTo(is16Bits());
  
  CHK (Channel *c = new Channel ());

  c->setSoundData (snd, false);
  c->setEphemeral ();
  c->setPriority (Normal_Priority);
  c->setStarted ();

  addChannel (c);

  return S_OK;
}

STDMETHODIMP csSoundRenderSoftware::MixingFunction()
{
  if(!m_piSoundDriver)
    return E_FAIL;

  CalculEars3D();

  int numberUsedChannels = setChannels ();
  
  if(numberUsedChannels == 0)
  {
    bool handlevoidsound;
    m_piSoundDriver->IsHandleVoidSound(&handlevoidsound);

    if(handlevoidsound) return S_OK;
  }

  if(m_piSoundDriver->LockMemory(&memory, &memorysize)!=S_OK)
    return E_FAIL;

  if(!memory || memorysize<1)
    return E_FAIL;

  if (!is16Bits ())
  {
    unsigned char *ptr = (unsigned char *)memory;
    long ptr_size = memorysize;
    
    if (!isStereo ())
    {
      // 8 bit mono
      for (int p = 0; p < ptr_size; p++)
      {
        float sample = 0;
        numberUsedChannels = 0;
        
        for(int i=0; i<numberChannels; i++)
        {
          if (Channels[i] && Channels [i]->isActive ())
          {
            int smp = Channels [i]->getSample ();
            if (Channels [i]->isStereo ())
              smp = (smp + Channels [i]->getSample ()) / 2;
            
            sample += smp * Channels [i]->Volume;
            numberUsedChannels++;
          }
        }
        
        if (numberUsedChannels > 0)
          *ptr++ = QInt (sample / numberUsedChannels);
        //*ptr++ = sample;
        else
          *ptr++ = 128;
      }
    }
    else // if (isStereo ())
    {
      // 8 bit stereo
      for (int p = 0; p < ptr_size; p += 2)
      {
        float left = 0, right = 0;
        
        numberUsedChannels = 0;
        for (int i = 0; i < numberChannels; i++)
        {
          if (Channels [i] && Channels [i]->isActive ())
          {
            int samplel, sampler;
            samplel = sampler = Channels [i]->getSample ();
            if (Channels [i]->isStereo ())
              sampler = Channels [i]->getSample ();
            
            left += samplel * Channels[i]->Volume_left;
            right += sampler * Channels[i]->Volume_right;
            numberUsedChannels++;
          }
        }
        
        if (numberUsedChannels > 0)
        {
          *ptr++ = QInt (left / numberUsedChannels);
          *ptr++ = QInt (right / numberUsedChannels);
          //*ptr++ = left;
          //*ptr++ = right;
        }
        else
        {
          *ptr++ = 128;
          *ptr++ = 128;
        }
      }
    }
  }
  else // if(is16Bits ())
  {
    short *ptr = (short *)memory;
    long ptr_size = memorysize/2;
    
    if (!isStereo ())
    {
      // 16 bit mono
      for (int p = 0; p < ptr_size; p++)
      {
        float sample = 0;
        
        numberUsedChannels = 0;
        for (int i = 0; i < numberChannels; i++)
        {
          if (Channels [i] && Channels [i]->isActive ())
          {
            int smp = Channels [i]->getSample ();
            if (Channels [i]->isStereo ())
              smp = (smp + Channels [i]->getSample ()) / 2;
            
            sample += smp * Channels [i]->Volume;
            numberUsedChannels++;
          }
        }
        
        if (numberUsedChannels > 0)
          *ptr++ = QInt (sample / numberUsedChannels);
        //*ptr++ = sample;
        else
          *ptr++ = 0;
      }
    }
    else // if (isStereo ())
    {
      // 16 bit stereo
      for (int p = 0; p < ptr_size; p += 2)
      {
        float left = 0, right = 0;
        
        numberUsedChannels = 0;
        for (int i = 0; i < numberChannels; i++)
        {
          if (Channels [i] && Channels [i]->isActive ())
          {
            int samplel, sampler;
            samplel = sampler = Channels [i]->getSample ();
            if (Channels [i]->isStereo ())
              sampler = Channels [i]->getSample ();
            
            left += samplel * Channels [i]->Volume_left;
            right += sampler * Channels [i]->Volume_right;
            numberUsedChannels++;
          }
        }
        
        if (numberUsedChannels > 0)
        {
          *ptr++ = QInt (left / numberUsedChannels);
          *ptr++ = QInt (right / numberUsedChannels);
          //*ptr++ = left;
          //*ptr++ = right;
        }
        else
        {
          *ptr++ = 0;
          *ptr++ = 0;
        }
      }
    }
  }

  updateChannels ();
  killChannels ();

  m_piSoundDriver->UnlockMemory();

  return S_OK;
}

void csSoundRenderSoftware::SysPrintf(int mode, char* szMsg, ...)
{
  char buf[1024];
  va_list arg;
  
  va_start (arg, szMsg);
  vsprintf (buf, szMsg, arg);
  va_end (arg);
  
  m_piSystem->Print(mode, buf);
}

void csSoundRenderSoftware::addChannel (Channel *c)
{
  if (c == NULL)
    return;

  c->next = NULL;
  if (AllChannels)
  {
    Channel *old = AllChannels;
    while (old->next)
      old = old->next;
    old->next = c;
  }
  else
    AllChannels = c;
}

bool csSoundRenderSoftware::delChannel (Channel_ID id)
{
  if (AllChannels)
  {
    Channel *c = AllChannels;
    Channel *old = NULL;

    if (c && c->getID () == id)
    {
      if (c->next)
        AllChannels = c->next; 
      else
        AllChannels = NULL;

      CHK (delete c);
      return true;
    }
    else
    {
      while (c)
      {
        old = c;
        c = c->next;
        if (c && c->getID () == id)
        {
          if (c->next)
            old->next = c->next;
          else
            old->next = NULL;

          CHK (delete c);
          return true;
        }
      }
    }
  }

  return false;
}

int csSoundRenderSoftware::setChannels ()
{
  Channel *c = AllChannels;
  int n = 0, i;
  
  for(i = 0; i < numberChannels; i++)
    Channels [i] = NULL;
  
  while (c)
  {
    c->Used = false;
    if (c->isActive () && c->isStarted ())
    {
      if (n >= numberChannels)
      {
        if(PriorityFunc == Priority_Up)
        {
          for (i = 0; i < numberChannels; i++)
            if (Channels [i]->getPriority () < c->getPriority ())
            {
              if(c->is3D())
              {
                CalculSound3D(c);
              }
              if(c->Volume > 0.0)
              {
                Channels [i] = c;
                c->Used = true;
                break;
              }
            }
        }
        else if (PriorityFunc == Priority_Down)
        {
          for (i = 0; i < numberChannels; i++)
            if (Channels [i]->getPriority () > c->getPriority ())
            {
              if(c->is3D())
              {
                CalculSound3D(c);
              }
              if(c->Volume > 0.0)
              {
                Channels [i] = c;
                c->Used = true;
                break;
              }
            }
        }
        else //if(PriorityFunc == Priority_None)
          break;
      }
      else
      {
        if(c->is3D())
        {
          CalculSound3D(c);
        }
        if(c->Volume > 0.0)
        {
          Channels [n] = c;
          c->Used = true;
          n++;
        }
      }
    }
    c = c->next;
  }

  return n;
}

void csSoundRenderSoftware::updateChannels ()
{
  Channel *c = AllChannels;
  
  while (c)
  {
    if (c->isActive () && !c->Used)
      c->addStep ((is16Bits ()) ? memorysize / 2 : memorysize);
    
    c = c->next;
  }
}

void csSoundRenderSoftware::killChannels ()
{
  Channel *c = AllChannels;
  Channel *old;
  
  while (c)
  {
    old = c;
    c = c->next;
    if (!old->isActive ())
      delChannel (old->getID ());
  }
}

void csSoundRenderSoftware::CalculSound3D(Channel * c)
{
  if(c!= NULL && c->is3D())
  {
    csSoundSourceSoftware *src =(csSoundSourceSoftware *)c;

    if(src != NULL)
    {
      float fPosX, fPosY, fPosZ;
      src->GetPosition(fPosX, fPosY, fPosZ);

      // calcul distance from ears to sound
      float x, y ,z, distance_l, distance_r;
      x = earL_x - fPosX;
      y = earL_y - fPosY;
      z = earL_z - fPosZ;
      distance_l = sqrt( x*x + y*y + z*z);
      x = earR_x - fPosX;
      y = earR_y - fPosY;
      z = earR_z - fPosZ;
      distance_r = sqrt( x*x + y*y + z*z);

      // yes, I know I need some thing better :-)
      float vol_l = distance_l*earL_DistanceFactor/100.0;
      float vol_r = distance_r*earR_DistanceFactor/100.0;

      // set channel volume level
      c->setVolume(vol_l, vol_r);
    }
  }
}

void csSoundRenderSoftware::CalculEars3D()
{
  if(m_pListener)
  {
    // Calcul of ears
    // need to be done with the position and rotation of listener
    float fPosX, fPosY, fPosZ;
    m_pListener->GetPosition(fPosX, fPosY, fPosZ);
    earL_x = fPosX;
    earL_y = fPosY;
    earL_z = fPosZ;

    earR_x = fPosX;
    earR_y = fPosY;
    earR_z = fPosZ;

    float fDistance;
    m_pListener->GetDistanceFactor(fDistance);
    earL_DistanceFactor = fDistance;
    earR_DistanceFactor = fDistance;
  }
  else
  {
    earL_x = earL_y = earL_z = 0.0;
    earR_x = earR_y = earR_z = 0.0;

    earL_DistanceFactor = 1.0;
    earR_DistanceFactor = 1.0;
  }
}
