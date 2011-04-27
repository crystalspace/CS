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

#ifndef __CS_SNDSYS_DRIVER_H__
#define __CS_SNDSYS_DRIVER_H__

/**\file
 * Sound system: software driver
 */

#include "csutil/scf.h"

/**\addtogroup sndsys
 * @{ */

// \todo This should really be an interface
class csSndSysRendererSoftware; 
struct csSndSysSoundFormat;

/* TODO:  Define a csSoundDevice structure that can be used to return 
 *        information about playback devices available for use by this driver.
 *        Add interface functionality for enumerating and selecting particular 
 *	  playback devices.
 */

/**
 * This is the interface for the low-level, system-dependent sound driver
 * that is used by the software sound renderer. The sound driver is
 * responsible for playing a single stream of samples.
 */
struct iSndSysSoftwareDriver : public virtual iBase
{
  SCF_INTERFACE(iSndSysSoftwareDriver,0,2,0);

  /**
   * Initialize the sound driver.
   *
   * The requested_format parameter may be modified during this function.
   */
  virtual bool Open(csSndSysRendererSoftware *renderer,
  	csSndSysSoundFormat *requested_format) = 0;

  /// Close the sound driver
  virtual void Close () = 0;


  /**
   * This function is called to start the driver thread.  
   */
  virtual bool StartThread() = 0;

  /// Stop the background thread
  virtual void StopThread() = 0;
};

/** @} */

#endif // __CS_SNDSYS_DRIVER_H__
