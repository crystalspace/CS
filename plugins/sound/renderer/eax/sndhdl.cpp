/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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
#include "sndhdl.h"
#include "sndrdr.h"
#include "sndsrc.h"

#define REFRESH_RATE    10

csSoundHandleEAX::csSoundHandleEAX(csSoundRenderEAX *srdr, iSoundData *snd)
        : csSoundHandle(snd)
{
  SoundRender = srdr;
  SoundRender->IncRef();
  Registered = true;
  NumSamples = Data->IsStatic() ? Data->GetStaticSampleCount() :
    (Data->GetFormat()->Freq/REFRESH_RATE);
}

csSoundHandleEAX::~csSoundHandleEAX() {
  SoundRender->DecRef();
}

void csSoundHandleEAX::Unregister() {
  Registered = false;
  ReleaseSoundData();
}

iSoundSource *csSoundHandleEAX::CreateSource(int Mode3d) {
  if (!Registered) return NULL;
  csSoundSourceEAX *src = new csSoundSourceEAX(NULL);
  if (src->Initialize(SoundRender, this, Mode3d, NumSamples)) return src;
  else {
    src->DecRef();
    return NULL;
  }
}

void csSoundHandleEAX::vUpdate(void *buf, long Num)
{
  long NumBytes = Num * Data->GetFormat()->Bits/8 * Data->GetFormat()->Channels;
  for (long i=0; i<SoundRender->ActiveSources.Length(); i++)
  {
    csSoundSourceEAX *src = (csSoundSourceEAX*)SoundRender->ActiveSources.Get(i);
    if (src->GetSoundHandle()==this && src->IsPlaying())
      src->Write(buf, NumBytes);
  }
}
