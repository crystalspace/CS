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

#ifndef __SOUNDRAW_H__
#define __SOUNDRAW_H__

#include "isound/data.h"

class csSoundDataRaw : public iSoundData {
friend class csSoundStreamRaw;
public:
  DECLARE_IBASE;

  csSoundDataRaw(iBase *iParent, void *Data, long NumSamples,
    csSoundFormat Format);
  virtual ~csSoundDataRaw();

  void Convert(const csSoundFormat *NewFormat);

  virtual long GetNumSamples();
  virtual const csSoundFormat *GetFormat();
  virtual iSoundStream *CreateStream();

private:
  // the sample buffer
  void *Data;
  // number of samples
  long NumSamples;
  // format of this sound
  csSoundFormat Format;
};

class csSoundStreamRaw : public iSoundStream {
public:
  DECLARE_IBASE;
  
  // constructor
  csSoundStreamRaw(csSoundDataRaw *sndData);
  // destructor
  virtual ~csSoundStreamRaw();

  // return the format descriptor
  virtual const csSoundFormat *GetFormat();
  // return true if the samples may be precached
  virtual bool MayPrecache();
  // reset the stream to the beginning
  virtual void Restart();
  // read the sound buffer
  virtual void *Read(long &NumSamples);
  // discard a buffer returned by Read()
  virtual void DiscardBuffer(void *buf);

private:
  // the sound data
  csSoundDataRaw *Data;
  // current position
  long Position;
};

#endif
