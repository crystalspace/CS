/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CS_IVARIA_PMETER_H__
#define __CS_IVARIA_PMETER_H__

/**\file
 * General progress meter interface
 */

#include "csutil/scf_interface.h"

/**
 * This is a general interface for a progress meter.
 * The engine needs an implementation of this to be able to report
 * progress on calculating lighting and other stuff.
 */
struct iProgressMeter : public virtual iBase
{
  SCF_INTERFACE(iProgressMeter, 2,0,0);
  /**
   * Set the id and description of what we are currently monitoring.
   * An id can be something like "crystalspace.engine.lighting.calculation".
   * \sa \ref FormatterNotes
   */
  virtual void SetProgressDescription (const char* id,
        const char* description, ...) CS_GNUC_PRINTF (3, 4) = 0;

  /**
   * Set the id and description of what we are currently monitoring.
   * An id can be something like "crystalspace.engine.lighting.calculation".
   * \sa \ref FormatterNotes
   */
  virtual void SetProgressDescriptionV (const char* id,
        const char* description, va_list) = 0;

  /**
   * Increment the meter by n units, default 1.
   * If the meter reaches 100% it should automatically stop itself.
   */
  virtual void Step (unsigned int n = 1) = 0;
  /// Reset the meter to 0%.
  virtual void Reset () = 0;
  /// Reset the meter and print the initial tick mark ("0%").
  virtual void Restart () = 0;
  /// Abort the meter.
  virtual void Abort () = 0;
  /// Finalize the meter (i.e. we completed the task sooner than expected).
  virtual void Finalize () = 0;

  /// Set the total element count represented by the meter and perform a reset.
  virtual void SetTotal (int n) = 0;
  /// Get the total element count represented by the meter.
  virtual int GetTotal () const = 0;
  /// Get the current value of the meter (<= total).
  virtual int GetCurrent () const = 0;

  /**
   * Set the refresh granularity.  Valid values are 1-100, inclusive.  Default
   * is 10.  The meter is only refreshed after each "granularity" * number of
   * units have passed.  For instance, if granularity is 20, then * the meter
   * will only be updated at most 5 times, or every 20%.
   */
  virtual void SetGranularity (int) = 0;
  /// Get the refresh granularity.
  virtual int GetGranularity () const = 0;
};

#endif // __CS_IVARIA_PMETER_H__

