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

#ifndef SNDSYS_LOADER_SPEEX_H
#define SNDSYS_LOADER_SPEEX_H

#include "csutil/scf_implementation.h"
#include "isndsys/ss_loader.h"
#include "iutil/comp.h"

/**
 * iSndSysLoader interface for Speex audio data.
 */
class SndSysSpeexLoader : public scfImplementation2<SndSysSpeexLoader, iSndSysLoader, iComponent>
{
public:
  SndSysSpeexLoader(iBase *parent);
  virtual ~SndSysSpeexLoader();

  virtual bool Initialize(iObjectRegistry*);
  virtual csPtr<iSndSysData> LoadSound(iDataBuffer* Buffer, const char *pDescription);
};

#endif // SNDSYS_LOADER_SPEEX_H
