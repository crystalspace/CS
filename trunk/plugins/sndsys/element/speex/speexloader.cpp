/*
    Copyright (C) 2008 by Mike Gist

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

#include "speexdata.h"
#include "speexloader.h"

SCF_IMPLEMENT_FACTORY (SndSysSpeexLoader);

SndSysSpeexLoader::SndSysSpeexLoader(iBase *parent) : scfImplementationType(this, parent)
{
}

SndSysSpeexLoader::~SndSysSpeexLoader()
{
}

bool SndSysSpeexLoader::Initialize(iObjectRegistry*)
{
  return true;
}

csPtr<iSndSysData> SndSysSpeexLoader::LoadSound(iDataBuffer* Buffer, const char* pDescription)
{
  SndSysSpeexSoundData* data = 0;

  // If the data is Speex then load, else return 0;
  if (SndSysSpeexSoundData::IsSpeex(Buffer))
  {
    data = new SndSysSpeexSoundData((iBase*)this, Buffer);
    data->SetDescription(pDescription);
  }

  return csPtr<iSndSysData> (data);
}
