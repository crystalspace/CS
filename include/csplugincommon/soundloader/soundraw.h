/*
    Copyright (C) 1998, 1999 by Jorrit Tyberghein
    Written by Martin Geisse

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

#ifndef __CS_SOUNDRAW_H__
#define __CS_SOUNDRAW_H__

#include "csextern.h"
#include "isound/data.h"

class CS_CSPLUGINCOMMON_EXPORT csSoundDataRaw : public iSoundData 
{
public:
  SCF_DECLARE_IBASE;

  csSoundDataRaw(iBase *iParent, void *Data, long NumSamples,
    csSoundFormat Format);
  virtual ~csSoundDataRaw();

  virtual bool Initialize(const csSoundFormat *fmt);
  virtual const csSoundFormat *GetFormat();
  virtual bool IsStatic();
  virtual long GetStaticSampleCount();
  virtual void *GetStaticData();
  virtual void ResetStreamed();
  virtual void *ReadStreamed(long &NumSamples);

private:
  // the sample buffer
  void *Data;
  // number of samples
  long NumSamples;
  // format of this sound
  csSoundFormat Format;
};

#endif // __CS_SOUNDRAW_H__
