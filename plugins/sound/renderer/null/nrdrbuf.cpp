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
#include "cscom/com.h"
#include "cssndrdr/null/nrdrsrc.h"
#include "cssndrdr/null/nrdrbuf.h"
#include "isystem.h"

IMPLEMENT_UNKNOWN_NODELETE (csSoundBufferNull)

BEGIN_INTERFACE_TABLE(csSoundBufferNull)
  IMPLEMENTS_INTERFACE(ISoundBuffer)
END_INTERFACE_TABLE()

csSoundBufferNull::csSoundBufferNull()
{
}

csSoundBufferNull::~csSoundBufferNull()
{
}

STDMETHODIMP csSoundBufferNull::CreateSource(ISoundSource** ppv)
{
  csSoundSourceNull* pNew = new csSoundSourceNull ();
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }

  pNew->pSoundBuffer = this;

  return pNew->QueryInterface (IID_ISoundSource, (void**)ppv);
}

STDMETHODIMP csSoundBufferNull::Play(SoundBufferPlayMethod /*playMethod*/)
{
  return S_OK;
}

STDMETHODIMP csSoundBufferNull::Stop()
{
  return S_OK;
}
