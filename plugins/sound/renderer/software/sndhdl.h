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

#ifndef __CS_SNDHDLSOFTWARE_H__
#define __CS_SNDHDLSOFTWARE_H__

#include "csplugincommon/soundrenderer/shdl.h"

class csSoundRenderSoftware;

class csSoundHandleSoftware : public csSoundHandle
{
public:
  csSoundRenderSoftware *SoundRender;

  csSoundHandleSoftware(csSoundRenderSoftware *srdr, iSoundData *snd);
  virtual ~csSoundHandleSoftware();

  void Unregister();
  virtual void vUpdate(void *buf, long NumSamples);
  virtual csPtr<iSoundSource> CreateSource(int Mode3d);

  virtual void IncSourceCount();
  virtual void DecSourceCount();

  virtual void ResetStream();

  void ProcessReset();

private:
  bool need_reset;
  int source_count;
};

#endif // __CS_SNDHDLSOFTWARE_H__
