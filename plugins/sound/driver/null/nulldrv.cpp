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
#include "csutil/scf.h"
#include "cssnddrv/null/nulldrv.h"
#include "isystem.h"
#include "isndlstn.h"
#include "isndsrc.h"

IMPLEMENT_UNKNOWN_NODELETE (csSoundDriverNull)

BEGIN_INTERFACE_TABLE(csSoundDriverNull)
  IMPLEMENTS_INTERFACE(ISoundDriver)
END_INTERFACE_TABLE()

csSoundDriverNull::csSoundDriverNull(iSystem* piSystem)
{
  m_piSystem = piSystem;
  memorysize = 1024;
  memory = new unsigned char[memorysize];
  volume = 1.0;
}

csSoundDriverNull::~csSoundDriverNull()
{
  if(memory) delete [] memory;
}

STDMETHODIMP csSoundDriverNull::Open(ISoundRender* /*render*/, int frequency, bool bit16, bool stereo)
{
  SysPrintf (MSG_INITIALIZATION, "\nSoundDriver Null selected\n");

  m_bStereo = stereo;
  m_b16Bits = bit16;
  m_nFrequency = frequency;

  return S_OK;
}

STDMETHODIMP csSoundDriverNull::Close()
{
  return S_OK;
}

STDMETHODIMP csSoundDriverNull::SetVolume(float vol)
{
  volume = vol;

  return S_OK;
}

STDMETHODIMP csSoundDriverNull::GetVolume(float *vol)
{
  *vol = volume;

  return S_OK;
}

STDMETHODIMP csSoundDriverNull::LockMemory(void **mem, int *memsize)
{
  *mem = memory;
  *memsize = memorysize;

  return S_OK;
}

STDMETHODIMP csSoundDriverNull::UnlockMemory()
{
  return S_OK;
}

STDMETHODIMP csSoundDriverNull::IsBackground(bool *back)
{
  *back = true;

  return S_OK;
}

STDMETHODIMP csSoundDriverNull::Is16Bits(bool *bit)
{
  *bit = m_b16Bits;
  return S_OK;
}

STDMETHODIMP csSoundDriverNull::IsStereo(bool *stereo)
{
  *stereo = m_bStereo;
  return S_OK;
}

STDMETHODIMP csSoundDriverNull::GetFrequency(int *freq)
{
  *freq = m_nFrequency;
  return S_OK;
}

STDMETHODIMP csSoundDriverNull::IsHandleVoidSound(bool *handle)
{
  *handle = true;
  return S_OK;
}

void csSoundDriverNull::SysPrintf(int mode, char* szMsg, ...)
{
  char buf[1024];
  va_list arg;
  
  va_start (arg, szMsg);
  vsprintf (buf, szMsg, arg);
  va_end (arg);
  
  m_piSystem->Print(mode, buf);
}
