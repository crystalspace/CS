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

#ifndef __CS_THOGGSNDSTREAM_H__
#define __CS_THOGGSNDSTREAM_H__

/**\file
* Video Player: sound stream
*/

#include <iutil/comp.h>
#include <ivideodecode/media.h>
#include <csutil/scf_implementation.h>

#include <iostream>
#include <stdio.h>


#include "csplugincommon/sndsys/sndstream.h"
using namespace CS::SndSys;
using namespace std;



class SndSysTheoraStream : public SndSysBasicStream
{
public:
  SndSysTheoraStream (csSndSysSoundFormat *pRenderFormat, int Mode3D);
  virtual ~SndSysTheoraStream ();


  ////
  // Interface implementation
  ////

  //------------------------
  // iSndSysStream
  //------------------------
public:


  /// Retrieve the textual description of this stream
  virtual const char *GetDescription() ;

  /**
  * Get length of this stream in rendered frames.
  * May return CS_SNDSYS_STREAM_UNKNOWN_LENGTH if the stream is of unknown 
  * length. For example, sound data being streamed from a remote host may not 
  * have a pre-determinable length.
  */
  virtual size_t GetFrameCount() ;

  /**
  * NOT AN APPLICATION CALLABLE FUNCTION!   This function advances the stream
  * position based on the provided frame count value which is considered as an
  * elapsed frame count.
  *
  * A Sound Element will usually store the last advance frame internally.
  * When this function is called it will compare the last frame with the
  * frame presented and retrieve more data from the associated iSndSysData
  * container as needed.  It will then update its internal last advance
  * frame value.
  *  
  * This function is not necessarily thread safe and must be called ONLY
  * from the Sound System's main processing thread.
  */
  virtual void AdvancePosition(size_t frame_delta) ;

  ////
  //  Member variables
  ////
protected:
  csSndSysSoundFormat pRenderFormat;
};
/** @} */

#endif // __CS_THOGGSNDSTREAM_H__