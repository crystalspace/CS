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
#include "srdrcom.h"
#include "srdrsrc.h"

csSoundHandleSoftware::csSoundHandleSoftware(csSoundRenderSoftware *srdr, iSoundData *snd)
        : csSoundHandle(snd)
{
  SoundRender = srdr;
  SoundRender->IncRef();
  Registered = true;
}

csSoundHandleSoftware::~csSoundHandleSoftware()
{
  SoundRender->DecRef();
}

void csSoundHandleSoftware::Unregister()
{
  Registered = false;
  ReleaseSoundData();
}

csPtr<iSoundSource> csSoundHandleSoftware::CreateSource(int Mode3d)
{
  if (!Registered) return 0;
  return csPtr<iSoundSource> (new csSoundSourceSoftware(SoundRender, this, Mode3d));
}

void csSoundHandleSoftware::vUpdate(void *buf, long Num)
{
  for (long i=0; i<SoundRender->Sources.Length(); i++)
  {
    csSoundSourceSoftware *src = (csSoundSourceSoftware*)SoundRender->
      Sources.Get(i);
    if (src->SoundHandle==this && src->Active)
      src->WriteBuffer(buf, SoundRender->memory, Num);
  }
}

