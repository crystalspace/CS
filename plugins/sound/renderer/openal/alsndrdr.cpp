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
#include "cssys/sysfunc.h"

#include "csutil/scf.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/virtclk.h"
#include "ivaria/reporter.h"

#include "alsndrdr.h"
#include "alsndlst.h"
#include "alsndsrc.h"
#include "alsndhdl.h"

#include <AL/al.h>
#include <AL/alut.h>

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(csSoundRenderOpenAL);

SCF_EXPORT_CLASS_TABLE (sndoal)
SCF_EXPORT_CLASS (csSoundRenderOpenAL, "crystalspace.sound.render.openal",
        "OpenAL 3D Sound Driver for Crystal Space")
SCF_EXPORT_CLASS_TABLE_END;

SCF_IMPLEMENT_IBASE(csSoundRenderOpenAL)
  SCF_IMPLEMENTS_INTERFACE(iSoundRender)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundRenderOpenAL::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSoundRenderOpenAL::csSoundRenderOpenAL(iBase *parent)
{
  SCF_CONSTRUCT_IBASE(parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  listener = NULL;
  object_reg = NULL;

  roll = 1.0;
  dist = 1.0;
}

bool csSoundRenderOpenAL::Initialize(iObjectRegistry *r)
{
  object_reg = r;
  config.AddConfig(object_reg, "/config/sound.cfg");
  format.Freq = config->GetInt ("Sound.Software.Frequency", 44100);
  format.Bits = config->GetBool ("Sound.Software.16Bits", true) ? 16 : 8;
  format.Channels = config->GetBool ("Sound.Software.Stereo", true) ? 2 : 1;

  // OpenAL Initialization
  alutInit (0, NULL);
  alGetError ();

  listener = new csSoundListenerOpenAL (this);
  SetVolume (config->GetFloat ("Sound.Volume", 1.0));

  return true;
}

csSoundRenderOpenAL::~csSoundRenderOpenAL()
{
  alutExit ();
}

void csSoundRenderOpenAL::SetVolume(float vol)
{
  alListenerf (AL_GAIN, vol);
}

float csSoundRenderOpenAL::GetVolume()
{
  float vol;
  alGetListenerf (AL_GAIN, &vol);
  return vol;
}

csPtr<iSoundHandle> csSoundRenderOpenAL::RegisterSound(iSoundData *snd)
{
  CS_ASSERT (snd);
  if (!snd->Initialize (&format)) return NULL;
  csSoundHandleOpenAL *hdl = new csSoundHandleOpenAL (this, snd);
  handles.Push (hdl);
  hdl->IncRef ();
  return hdl;
}

void csSoundRenderOpenAL::UnregisterSound (iSoundHandle *hdl)
{
  handles.Delete(hdl);
}

void csSoundRenderOpenAL::MixingFunction()
{
}

void csSoundRenderOpenAL::AddSource(csSoundSourceOpenAL *src)
{
  alSourcef(src->GetID(), AL_REFERENCE_DISTANCE, dist);
  alSourcef(src->GetID(), AL_ROLLOFF_FACTOR, roll);
  sources.Push(src);
}

void csSoundRenderOpenAL::RemoveSource(csSoundSourceOpenAL *src)
{
  sources.Delete(src);
}

