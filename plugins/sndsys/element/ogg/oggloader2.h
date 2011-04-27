/*
    Copyright (C) 2005 by Andrew Mann
    Based in part on work by Norman Kraemer

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

#ifndef SNDSYS_LOADER_OGG_H
#define SNDSYS_LOADER_OGG_H

#include "isndsys/ss_loader.h"

#include "csutil/scf_implementation.h"


/// iSndSysLoader interface for Ogg Vorbis audio data
//
//  This also implements the iComponent interface for the entire sndsysogg plugin.
//  This interface was chosen because only one instance of the loader needs exist.
//  In contrast to the data and stream classes, which will have many object instances
//   in existence at once (usually).
class SndSysOggLoader : public 
  scfImplementation2<SndSysOggLoader, iSndSysLoader, iComponent>
{
public:
  /// A very un-busy constructor
  SndSysOggLoader (iBase *parent) :
      scfImplementationType(this, parent)
  {
  }

  // Sim
  virtual ~SndSysOggLoader ()
  {
  }

  ////
  // Interface implementation
  ////

  //------------------------
  // iComponent
  //------------------------
public:
  /// Initialize this component. Not much to do here.
  virtual bool Initialize (iObjectRegistry *){return true;}

  //------------------------
  // iSndSysLoader
  //------------------------
public:

  /// This function provides the caller with an iSndSysData interface with which
  //   they may access the Ogg Vorbis audio provided in the Buffer parameter.
  //
  //  \param pDescription This parameter is an optional description which will be attached
  //         to the newly created data element.  A filename is a good choice. This value 
  //         may be NULL (0).
  virtual csPtr<iSndSysData> LoadSound (iDataBuffer* Buffer, const char *pDescription)
  {
    SndSysOggSoundData *sd=0;

    // If the passed data is not Ogg Vorbis data, then we cannot use it
    if (SndSysOggSoundData::IsOgg (Buffer))
    {
      // Create the Data object.
      sd = new SndSysOggSoundData ((iBase*)this, Buffer);
      // Set the Data object desctiption to the passed value (may be NULL)
      sd->SetDescription(pDescription);
    }

    return csPtr<iSndSysData> (sd);
  }
};

#endif // #ifndef SNDSYS_LOADER_OGG_H




