/*
    Copyright (C) 2002 by Jorrit Tyberghein, Daniel Duhprey

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

csSoundHandleOpenAL::csSoundHandleOpenAL(csSoundRenderOpenAL *srdr, iSoundData *snd)
        : csSoundHandle(snd)
{
  parent = srdr;
  alGenBuffers (1, &buffer);

  if (snd->IsStatic ()) {
    const csSoundFormat *f = snd->GetFormat ();
    ALenum format;
    int datalen = snd->GetStaticSampleCount () * f->Bits/8 * f->Channels;
    if (f->Bits == 8) {
      if (f->Channels == 2) {
        format = AL_FORMAT_STEREO8;
      } else {
        format = AL_FORMAT_MONO8;
      }
    } else {
      if (f->Channels == 2) {
        format = AL_FORMAT_STEREO16;
      } else {
        format = AL_FORMAT_MONO16;
      }
    }

    alBufferData (buffer, format, snd->GetStaticData (), datalen, f->Freq);
  }
}

csSoundHandleOpenAL::~csSoundHandleOpenAL() 
{
  alDeleteBuffers (1, &buffer);
}

csPtr<iSoundSource> csSoundHandleOpenAL::CreateSource(int mode) 
{
  iSoundSource *src = new csSoundSourceOpenAL(parent, this);
  src->SetMode3D (mode);
  parent->AddSource ((csSoundSourceOpenAL *)src);
  src->IncRef ();
  return csPtr<iSoundSource> (src);
}

void csSoundHandleOpenAL::vUpdate (void *, long) 
{
}
