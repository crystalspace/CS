/*
    Copyright (C) 2005 by Andrew Mann

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

#ifndef SNDSYS_GENLOADER_H
#define SNDSYS_GENLOADER_H

#include "isndsys/ss_loader.h"
#include "csutil/scf_implementation.h"


/// A format-neutral 'loader' for sound files
class SndSysLoader : public 
  scfImplementation2<SndSysLoader,iSndSysLoader, iComponent>
{
public:

  SndSysLoader (iBase *parent) :
      scfImplementationType(this, parent)
  {
  }

  virtual ~SndSysLoader ()
  {
  }

  ////
  // Interface implementation
  ////

  //------------------------
  // iComponent
  //------------------------
public:

  /// Initialize this component.
  //
  //  We obtain interfaces for the various sound-type specific 
  //   loaders here.
  virtual bool Initialize (iObjectRegistry *reg);

  //------------------------
  // iSndSysLoader
  //------------------------
public:

  /// This function provides the caller with an iSndSysData interface with which
  //   they may access the audio provided in the Buffer parameter.
  //
  //  \param pDescription This parameter is an optional description which will be attached
  //         to the newly created data element.  A filename is a good choice. This value 
  //         may be NULL (0).
  virtual csPtr<iSndSysData> LoadSound (iDataBuffer* buffer, const char *pDescription);

protected:
  /// PCM Waveform audio loader interface
  csRef<iSndSysLoader> m_pWavLoader;

  /// Ogg Vorbis audio loader interface
  csRef<iSndSysLoader> m_pOggLoader;

  /// Speex audio loader interface
  csRef<iSndSysLoader> m_pSpeexLoader;
};

#endif // #ifndef SNDSYS_GENLOADER_H




