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

#include "cssysdef.h"
#include "isndldr.h"
#include "isystem.h"
#include "csutil/csvector.h"
#include "sndload.h"
#include "ivfs.h"

class csSoundLoader : public iSoundLoader {
private:
  csVector FormatLoaders;

public:
  DECLARE_IBASE;

  /// constructor
  csSoundLoader(iBase *iParent);

  /// destructor
  virtual ~csSoundLoader();

  /// Initialize the Sound Loader.
  virtual bool Initialize (iSystem *sys);

  /// Load a sound from the VFS.
  virtual iSoundData *LoadSound(UByte *Buffer, ULong Size,
    const csSoundFormat *Format, bool Streamed);

  /// register a sound loader
  void RegisterFormatLoader(csSoundFormatLoader *Loader);
};

IMPLEMENT_IBASE(csSoundLoader)
  IMPLEMENTS_INTERFACE(iSoundLoader)
  IMPLEMENTS_INTERFACE(iPlugIn)
IMPLEMENT_IBASE_END;

csSoundLoader::csSoundLoader(iBase *iParent) {
  CONSTRUCT_IBASE(iParent);
}

csSoundLoader::~csSoundLoader() {
  for (long i=0;i<FormatLoaders.Length();i++) {
    csSoundFormatLoader *ldr=(csSoundFormatLoader*)(FormatLoaders.Get(i));
    delete ldr;
  }
}

bool csSoundLoader::Initialize(iSystem *sys) {
  (void)sys;
#ifdef DO_AIFF
  RegisterFormatLoader(new csSoundLoader_AIFF());
#endif

#ifdef DO_AU
  RegisterFormatLoader(new csSoundLoader_AU());
#endif

#ifdef DO_IFF
  RegisterFormatLoader(new csSoundLoader_IFF());
#endif

#ifdef DO_WAV
  RegisterFormatLoader(new csSoundLoader_WAV());
#endif

  return true;
}

iSoundData *csSoundLoader::LoadSound(UByte *Buffer, ULong Size,
    const csSoundFormat *Format, bool Streamed) {
  iSoundData *SoundData;

  if (!Buffer || Size<1) return NULL;

  for (long i=0;i<FormatLoaders.Length();i++) {
    csSoundFormatLoader *Ldr=(csSoundFormatLoader*)(FormatLoaders.Get(i));
    SoundData=Ldr->Load(Buffer, Size, Format);
    if (SoundData) {
      if (!Streamed) {
        iSoundData *Unstreamed = SoundData->Decode();
        SoundData->DecRef();
        return Unstreamed;
      } else return SoundData;
    }
  }
  return NULL;
}

void csSoundLoader::RegisterFormatLoader(csSoundFormatLoader *Loader) {
  FormatLoaders.Push(Loader);
}

// i have stolen this from Olivier Langlois (olanglois@sympatico.ca) ;-)
short int csSndFunc::ulaw2linear(unsigned char ulawbyte)
{
  static int exp_lut[8]={0,132,396,924,1980,4092,8316,16764};
  short int sign, exponent, mantissa, sample;

  ulawbyte = ~ulawbyte;
  sign = (ulawbyte & 0x80);
  exponent = (ulawbyte >> 4) & 0x07;
  mantissa = ulawbyte & 0x0F;
  sample = exp_lut[exponent] + (mantissa << (exponent+3));
  if (sign != 0) sample = -sample;
  return(sample);
}

IMPLEMENT_FACTORY(csSoundLoader);

EXPORT_CLASS_TABLE (sndload)
EXPORT_CLASS (csSoundLoader, "crystalspace.sound.loader",
    "Sound Loader plug-in")
EXPORT_CLASS_TABLE_END;

