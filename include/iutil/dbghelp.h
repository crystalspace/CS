/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_IUTIL_DBGHELP_H__
#define __CS_IUTIL_DBGHELP_H__

/**\file
 * Debugging helper interface
 */
/**\addtogroup util
 * @{ */
#include "csutil/scf_interface.h"
#include "csutil/ref.h"
#include "iutil/string.h"

struct iGraphics3D;

/**\name iDebugHelper implementation support flags
 * @{ */
enum 
{
  /// Supports Benchmark()
  CS_DBGHELP_BENCHMARK = 0x1,
  /// Supports non graphical Dump().
  CS_DBGHELP_TXTDUMP = 0x2,
  /// Supports graphical Dump().
  CS_DBGHELP_GFXDUMP = 0x4,
  /// Supports StateTest().
  CS_DBGHELP_STATETEST = 0x8
};
/** @} */

/**
 * Some object that wants to implement unit testing, debugging and/or
 * benchmarking can implement this interface.
 */
struct iDebugHelper : public virtual iBase
{
  SCF_INTERFACE(iDebugHelper,3,0,0);
  /**
   * Return a bit field indicating what types of functions this specific
   * unit test implementation supports. This will return a combination of
   * the CS_DBGHELP_... flags:  
   * - #CS_DBGHELP_BENCHMARK
   * - #CS_DBGHELP_TXTDUMP
   * - #CS_DBGHELP_GFXDUMP
   * - #CS_DBGHELP_STATETEST
   */
  virtual int GetSupportedTests () const = 0;

  /**
   * Perform a state test. This function will test if the current state
   * of the object is ok. It will return 0 if it is ok.
   * Otherwise an iString  is returned containing some information about
   * the errors. DecRef() this returned string after using it.
   */
  virtual csPtr<iString> StateTest () = 0;

  /**
   * Perform a benchmark. This function will return a number indicating
   * how long the benchmark lasted in milliseconds.
   */
  virtual csTicks Benchmark (int num_iterations) = 0;

  /**
   * Do a text dump of the current state of this object. Returns 0 if
   * not supported or else a string which you should DecRef() after use.
   */
  virtual csPtr<iString> Dump () = 0;

  /**
   * Do a graphical dump of the current state of this object.
   */
  virtual void Dump (iGraphics3D* g3d) = 0;

  /**
   * Perform a debug command as defined by the module itself.
   * Returns 'false' if the command was not recognized.
   */
  virtual bool DebugCommand (const char* cmd) = 0;
};
/** @} */

#endif // __CS_IUTIL_DBGHELP_H__

