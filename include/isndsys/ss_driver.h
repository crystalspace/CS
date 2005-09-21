/*
    Copyright (C) 2004 by Andrew Mann

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

#ifndef SNDSYS_DRIVER_H
#define SNDSYS_DRIVER_H

#include "csutil/scf.h"

class SndSysRendererSoftware;
struct SndSysSoundFormat;

SCF_VERSION (iSndSysSoftwareDriver, 0, 1, 0);



// TODO:  Define a csSoundDevice structure that can be used to return information about playback devices available
//        for use by this driver.
//        Add interface functionality for enumerating and selecting particular playback devices.


/**
 * This is the interface for the low-level, system-dependent sound driver
 * that is used by the software sound renderer. The sound driver is
 * responsible for playing a single stream of samples.
 */
struct iSndSysSoftwareDriver : public iBase
{
  /**
   * Initialize the sound driver.
   *
   * The requested_format parameter may be modified during this function.
   */
  virtual bool Open(SndSysRendererSoftware *renderer,
  	SndSysSoundFormat *requested_format) = 0;

  /// Close the sound driver
  virtual void Close () = 0;


  /**
   * This function is called to start the driver thread.  
   */
  virtual bool StartThread() = 0;

  /// Stop the background thread
  virtual void StopThread() = 0;


  /// Candidate functions 

  // virtual vool DriverProvidesThread() = 0;
  //  The Sound System always needs its own background thread, but in some instances the low level interface to the OS based sound system
  //   can provide more accurate buffer fill requests through callbacks or some other mechanism.  Since only the driver code knows about this, and since
  //   we would like to use the most accurate method possible, 


  /// Functions below this point are likely to be removed due to redesign


  /// Lock and Get Sound Memory Buffer
  //virtual void LockMemory (void **mem, int *memsize) = 0;
  /// Unlock Sound Memory Buffer
  //virtual void UnlockMemory () = 0;
  /// Must the driver be updated manually or does it run in background?
  //virtual bool IsBackground () = 0;

  /**
   * Is the sound driver able to create silence without locking and
   * writing to the sound memory?
   */
  //virtual bool IsHandleVoidSound () = 0;
  // @@@ temporary
  //virtual bool ThreadAware (){ return false; }
};

#endif // #ifndef SNDSYS_DRIVER_H



