/*
    Copyright (C) 2006 by Andrew Mann

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


#ifndef SNDDATA_H
#define SNDDATA_H

#include "iutil/databuff.h"
#include "isndsys/ss_structs.h"
#include "isndsys/ss_data.h"
#include "csutil/scf_implementation.h"

namespace CS
{
  namespace SndSys
  {


  class CS_CRYSTALSPACE_EXPORT SndSysBasicData :
    public scfImplementation1<SndSysBasicData, iSndSysData>
  {
  public:
    SndSysBasicData(iBase *pParent);
    virtual ~SndSysBasicData();

    ////
    // Internal functions
    ////
  protected:

    /// This is required to initialize the m_SampleCount and m_SoundFormat
    /// member variables. It is called internally.
    //    This is only called the first time that SoundFormat or SampleCount
    //    data is requested.
    virtual void Initialize() = 0;


    ////
    // Interface implementation
    ////

    //------------------------
    // iSndSysData
    //------------------------
  public:
    /// Get the format of the sound data.
    virtual const csSndSysSoundFormat *GetFormat();

    /// Get size of this sound in frames.
    virtual size_t GetFrameCount();

    /**
    * Return the size of the data stored in bytes.  This is informational only
    * and is not guaranteed to be a number usable for sound calculations.
    * For example, an audio file compressed with variable rate compression may
    * result in a situation where FILE_SIZE is not equal to
    * SAMPLE_COUNT * SAMPLE_SIZE since SAMPLE_SIZE may vary throughout the
    * audio data.
    */
    virtual size_t GetDataSize() = 0;

    /**
    * Creates a stream associated with this sound data positioned at the
    * begining of the sound data and initially paused if possible.
    */
    virtual iSndSysStream *CreateStream (csSndSysSoundFormat *pRenderFormat, int Mode3D) = 0;

    /// Set an optional description to be associated with this sound data
    //   A filename isn't a bad idea!
    virtual void SetDescription (const char *pDescription);

    /// Retrieve the description associated with this sound data
    //   This may return 0 if no description is set.
    virtual const char *GetDescription() { return m_pDescription; }

    ////
    //  Member variables
    ////
  protected:

    /// Flag indicating whether Initialize() has been called yet
    bool m_bInfoReady;

    /// The format that we're decoding the Ogg stream to. 
    //    Currently this is the default format that the ogg vorbis library
    //    returns for a given Ogg audio file.
    csSndSysSoundFormat m_SoundFormat;

    /// The number of frames in the decoded output
    size_t m_FrameCount;

    /// An optional brief description of the sound data
    char *m_pDescription;
  };



  }
  // END namespace CS::SndSys
}
// END namespace CS


#endif // #ifndef SNDDATA_H

