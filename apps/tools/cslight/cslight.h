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

#ifndef __CSLIGHT_H__
#define __CSLIGHT_H__

#include <stdarg.h>
#include "ivaria/pmeter.h"

struct iEngine;
struct iLoader;
struct iGraphics3D;
struct iGraphics2D;
struct iFont;
struct iTextureHandle;
struct iObjectRegistry;
struct iVirtualClock;
struct iEvent;

/**
 * Combined graphical and text progress meter.
 */
class csCsLightProgressMeter : public iProgressMeter
{
private:
  int granularity;
  int total;
  int current;
  csString cur_description;
  int tick_scale;
  int anchor;

public:
  /// Constructs a new progress meter.
  csCsLightProgressMeter (int total = 100);
  /// Destroys the progress meter.
  virtual ~csCsLightProgressMeter ();

  SCF_DECLARE_IBASE;

  /**
   * Set the tick scale.  Valid values are 1-100, inclusive.  Default is 2.  A
   * value of 1 means that each printed tick represents one unit, thus a total
   * of 100 ticks will be printed.  A value of 2 means that each tick
   * represents two units, thus a total of 50 ticks will be printed, etc.
   */
  void SetTickScale (int);
  /// Get the tick scale.
  int GetTickScale () const { return tick_scale; }

  /**
   * Set the id and description of what we are currently monitoring.
   * An id can be something like "crystalspace.engine.lighting.calculation".
   */
  virtual void SetProgressDescription (const char*, const char*, ...);
  virtual void SetProgressDescriptionV (const char*, const char*, va_list);

  /// Increment the meter by one unit and print a tick mark.
  virtual void Step (unsigned int i = 1);
  /// Reset the meter to 0%.
  virtual void Reset () { current = 0; anchor = 0; }
  /// Reset the meter and print the initial tick mark ("0%").
  virtual void Restart ();
  /// Abort the meter.
  virtual void Abort ();
  /// Finalize the meter (i.e. we completed the task sooner than expected).
  virtual void Finalize ();

  /// Set the total element count represented by the meter and perform a reset.
  virtual void SetTotal (int n) { total = n; Reset(); }
  /// Get the total element count represented by the meter.
  virtual int GetTotal () const { return total; }
  /// Get the current value of the meter (<= total).
  virtual int GetCurrent () const { return current; }

  /**
   * Set the refresh granularity.  Valid values are 1-100, inclusive.  Default
   * is 10.  The meter is only refreshed after each "granularity" * number of
   * units have passed.  For instance, if granularity is 20, then * the meter
   * will only be updated at most 5 times, or every 20%.
   */
  virtual void SetGranularity (int);
  /// Get the refresh granularity.
  virtual int GetGranularity () const { return granularity; }
};


/**
 * Main class.
 */
class Lighter
{
public:
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iGraphics2D> g2d;
  csRef<iFont> font;
  csRef<iTextureHandle> logo;
  int color_bg;
  int color_text;
  int color_done;
  int color_todo;
  iObjectRegistry* object_reg;
  csRef<iVirtualClock> vc;

  /**
   * Set the current VFS dir to the given map_dir.
   * This routine tries to be smart about mounting the dir.
   * Returns false on failure.
   */
  bool SetMapDir (const char* map_dir);

public:
  Lighter ();
  virtual ~Lighter ();

  bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);

  void Report (int severity, const char* msg, ...);
};

#endif // __CSLIGHT_H__
