/*
    Copyright (C) 1998 by Jorrit Tyberghein    

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

#ifndef __SNDLOAD_H__
#define __SNDLOAD_H__

struct iSoundData;

// format-specific sound data loader
class csSoundDataLoader {
public:
  virtual iSoundData *Load (UByte *buf, ULong size) const = 0;
};

#define DECLARE_SOUNDDATA_LOADER(name)                                          \
  class name : public csSoundDataLoader {                                       \
  public:                                                                       \
    virtual iSoundData *Load(UByte*, ULong) const;                              \
  }

// all sound data loaders
DECLARE_SOUNDDATA_LOADER(csSoundLoader_AIFF);
DECLARE_SOUNDDATA_LOADER(csSoundLoader_AU);
DECLARE_SOUNDDATA_LOADER(csSoundLoader_IFF);
DECLARE_SOUNDDATA_LOADER(csSoundLoader_WAV);

// Some helper functions for the sound loaders
class csSndFunc {
public:
  static unsigned long makeWord(int b0, int b1)
  { return ((b0&0xff)<<8)|b1&0xff; }

  static unsigned long makeDWord(int b0, int b1, int b2, int b3)
  { return ((b0&0xff)<<24)|((b1&0xff)<<16)|((b2&0xff)<<8)|b3&0xff; }

  static short int ulaw2linear(unsigned char ulawbyte);
};

#endif // __SOUNDLOADER_H
