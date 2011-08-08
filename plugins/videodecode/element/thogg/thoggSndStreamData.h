/*
Copyright (C) 2011 by Alin Baciu

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

#ifndef __CS_THOGGSNDSTREAMDATA_H__
#define __CS_THOGGSNDSTREAMDATA_H__

/**\file
* Video Player: sound data
*/

#include <iutil/comp.h>
#include <ivideodecode/media.h>
#include <csutil/scf_implementation.h>

#include <iostream>
#include <stdio.h>


#include "thoggSndStream.h"
#include "csplugincommon/sndsys/snddata.h"
using namespace CS::SndSys;
using namespace std;

/// The implementation of iSndSysData for PCM Waveform audio
class SndSysTheoraSoundData : public SndSysBasicData
{
public:
  /// Construct this object from an iDataBuffer containing wav format PCM audio
  SndSysTheoraSoundData (iBase *parent, iDataBuffer* data);
  virtual ~SndSysTheoraSoundData();


  ////
  // Interface implementation
  ////

  //------------------------
  // iSndSysData
  //------------------------
public:

  /** 
   * Return the size of the data stored in bytes.  This is informational only 
   * and is not guaranteed to be a number usable for sound calculations.
   * For example, an audio file compressed with variable rate compression may 
   * result in a situation where FILE_SIZE is not equal to 
   * FRAME_COUNT * FRAME_SIZE since FRAME_SIZE may vary throughout the 
   * audio data.
   */
  virtual size_t GetDataSize();

  /**
   * Creates a stream associated with this sound data positioned at the
   * beginning of the sound data and initially paused if possible.
   */
  virtual iSndSysStream *CreateStream(csSndSysSoundFormat *renderformat,
  	int mode3d);


  ////
  // Internal functions
  ////
protected:


  /// Initializes member variables that must be calculated.
  //  This is called when information that must be calculated
  //  is requested for the first time 
  void Initialize();

  ////
  //  Member variables
  ////
protected:
};

#endif //__CS_THOGGSNDSTREAMDATA_H__