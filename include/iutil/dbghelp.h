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
 */
/**\addtogroup util
 * @{ */
#include "csutil/scf_interface.h"
#include "csutil/ref.h"

struct iGraphics3D;
struct iString;


/**\name iDebugHelper implementation support flags
 * @{ */
/**
 * supports UnitTest().
 */
#define CS_DBGHELP_UNITTEST 1
/**
 * supports Benchmark().
 */
#define CS_DBGHELP_BENCHMARK 2
/**
 *  supports non graphical Dump().
 */
#define CS_DBGHELP_TXTDUMP 4
/**
 * supports graphical Dump().
 */
#define CS_DBGHELP_GFXDUMP 8
/**
 supports StateTest().
 */
#define CS_DBGHELP_STATETEST 16
/** @} */

/**
 * Some object that wants to implement unit testing, debugging and/or
 * benchmarking can implement this interface.
 */
struct iDebugHelper : public virtual iBase
{
  SCF_INTERFACE(iDebugHelper,2,0,0);
  /**
   * Return a bit field indicating what types of functions this specific
   * unit test implementation supports. This will return a combination of
   * the CS_DBGHELP_... flags:
   * <ul>
   * <li> #CS_DBGHELP_UNITTEST
   * <li> #CS_DBGHELP_BENCHMARK
   * <li> #CS_DBGHELP_TXTDUMP
   * <li> #CS_DBGHELP_GFXDUMP
   * <li> #CS_DBGHELP_STATETEST
   * </ul>
   */
  virtual int GetSupportedTests () const = 0;

  /**
   * Perform a unit test. This function will try to test as much as possible
   * of the given module. This function returns 0 if the test succeeded.
   * Otherwise an iString  is returned containing some information about
   * the errors. DecRef() this returned string after using it.
   */
  virtual csPtr<iString> UnitTest () = 0;

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

