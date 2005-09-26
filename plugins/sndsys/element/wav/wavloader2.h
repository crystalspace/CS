/*
    Copyright (C) 2005 by Andrew Mann
    Based in part on work by Norman Kraemer

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef SNDSYS_LOADER_WAV_H
#define SNDSYS_LOADER_WAV_H



// Waveform audio loader

class SndSysWavLoader : public iSndSysLoader
{
public:
  SCF_DECLARE_IBASE;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (SndSysWavLoader);
    virtual bool Initialize (iObjectRegistry *){return true;}
  } scfiComponent;

  SndSysWavLoader (iBase *parent)
  {
    SCF_CONSTRUCT_IBASE (parent);
    SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  }

  virtual ~SndSysWavLoader ()
  {
    SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
    SCF_DESTRUCT_IBASE();
  }

  virtual csPtr<iSndSysData> LoadSound (iDataBuffer* Buffer)
  {
    SndSysWavSoundData *sd=0;
    if (SndSysWavSoundData::IsWav (Buffer))
      sd = new SndSysWavSoundData ((iBase*)this, Buffer);
				      
    return csPtr<iSndSysData> (sd);
  }
};

#endif // #ifndef SNDSYS_LOADER_WAV_H


