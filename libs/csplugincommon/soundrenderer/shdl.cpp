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
#include "csplugincommon/soundrenderer/shdl.h"
#include "isound/data.h"
#include "isound/source.h"

SCF_IMPLEMENT_IBASE(csSoundHandle);
  SCF_IMPLEMENTS_INTERFACE(iSoundHandle);
SCF_IMPLEMENT_IBASE_END;

csSoundHandle::csSoundHandle(iSoundData* s)
{
  SCF_CONSTRUCT_IBASE(0);

  Data = s;
  Registered = false;
  ActiveStream = false;
  LoopStream = false;
}

csSoundHandle::~csSoundHandle()
{
  CS_ASSERT(Registered == false);
  ReleaseSoundData();
  SCF_DESTRUCT_IBASE();
}

void csSoundHandle::ReleaseSoundData()
{
  if (Data)
  {
    Data = 0;
  }
}

bool csSoundHandle::IsStatic()
{
  return Data->IsStatic();
}

csPtr<iSoundSource> csSoundHandle::Play(bool Loop)
{
  // Looping the source on streaming sound handles makes no sense
  if (!IsStatic())
    Loop=false;

  csRef<iSoundSource> src (CreateSource(SOUND3D_DISABLE));
  if (src)
  {
    src->Play(Loop ? SOUND_LOOP : 0);
    return csPtr<iSoundSource> (src);
  }
  return 0;
}

void csSoundHandle::StartStream(bool Loop)
{
  if (!Data->IsStatic())
  {
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
    if (Num > 0)
    {
      if (!LoopStream) break;
      Data->ResetStreamed();
    }
  }
}
