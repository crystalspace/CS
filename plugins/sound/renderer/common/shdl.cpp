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
#include "shdl.h"
#include "isound/data.h"
#include "isound/source.h"

SCF_IMPLEMENT_IBASE(csSoundHandle);
  SCF_IMPLEMENTS_INTERFACE(iSoundHandle);
SCF_IMPLEMENT_IBASE_END;

csSoundHandle::csSoundHandle(iSoundData *s) {
  SCF_CONSTRUCT_IBASE(NULL);

  Data = s;
  Data->IncRef();
  Registered = false;
  ActiveStream = false;
  LoopStream = false;
}

csSoundHandle::~csSoundHandle() {
  CS_ASSERT(Registered == false);
  ReleaseSoundData();
}

void csSoundHandle::ReleaseSoundData() {
  if (Data) {
    Data->DecRef();
    Data = NULL;
  }
}

bool csSoundHandle::IsStatic() {
  return Data->IsStatic();
}

iSoundSource *csSoundHandle::Play(bool Loop) {
  iSoundSource *src = CreateSource(SOUND3D_DISABLE);
  if (src) {
    src->Play(Loop ? SOUND_LOOP : 0);
    src->DecRef();
  }
  return (Loop ? src : (iSoundSource *)NULL);
}

void csSoundHandle::StartStream(bool Loop)
{
  if (!Data->IsStatic()) {
    LoopStream = Loop;
    ActiveStream = true;
  }
}

void csSoundHandle::StopStream()
{
  ActiveStream = false;
}

void csSoundHandle::ResetStream()
{
  if (!Data->IsStatic())
    Data->ResetStreamed();
}

void csSoundHandle::Update_Time(csTicks Time)
{
  if (!ActiveStream) return;
  UpdateCount (Time * Data->GetFormat()->Freq / 1000);
}

void csSoundHandle::UpdateCount(long Num)
{
  if (!ActiveStream) return;
  while (Num > 0)
  {
    long n = Num;
    void *buf = Data->ReadStreamed(n);
    vUpdate(buf, n);
    Num -= n;
    if (Num > 0) {
      if (!LoopStream) break;
      Data->ResetStreamed();
    }
  }
}
