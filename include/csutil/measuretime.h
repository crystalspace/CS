/*
    Copyright (C) 2004-2006 by Jorrit Tyberghein
              (C) 2004-2006 by Frank Richter

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

#ifndef __CS_CSUTIL_TIMEMEASURE_H__
#define __CS_CSUTIL_TIMEMEASURE_H__

/**\file
 * Simple helper class to measure execution time of a block.
 */

#include "csutil/csstring.h"
#include "csutil/util.h"
#include "csutil/sysfunc.h"

namespace CS
{

  /**
   * \addtogroup util
   * @{ */

  /**
   * Simple helper class to measure execution time of a block.
   * When destructed, csPrintf()s the time that has passed between
   * construction and destruction in microseconds.
   * 
   * Use like:
   * \code
   * void Foo (int x)
   * {
   *   CS::MeasureTime measureFoo ("Foo (%d) time", x);
   *   
   *   // ...
   * }
   * \endcode
   * This will print the total execution time of Foo().
   */ 
  class CS_CRYSTALSPACE_EXPORT MeasureTime
  {
  protected:
    csMicroTicks offsetTime;
    csString text;
    void PrintTime (const char* prefix, csMicroTicks time, const char* suffix);
  public:
    /**
     * Construct with a formatted description string.
     */
    MeasureTime (const char* format, ...)
    {
      va_list args;
      va_start (args, format);
      text.FormatV (format, args);
      va_end (args);

      offsetTime = csGetMicroTicks ();
    }
    
    ~MeasureTime ()
    {
      csMicroTicks endTime = csGetMicroTicks ();
      PrintTime ((text + ": ").GetData(), endTime - offsetTime, 
	" \xC2\xB5s\n");
    }
    
    /// Print an intermediate measurement.
    void PrintIntermediate (const char* descr, ...)
    {
      csMicroTicks currentTime = csGetMicroTicks ();

      csPrintf ("(%s)", text.GetData());
      va_list args;
      va_start (args, descr);
      csPrintfV (descr, args);
      va_end (args);
      PrintTime (": ", currentTime - offsetTime, " \xC2\xB5s\n");
      
      csMicroTicks currentTime2 = csGetMicroTicks ();
      // Correct difference from printing
      offsetTime += currentTime2 - currentTime;
    }
  };
 
/** @} */

} // namespace CS

#endif
