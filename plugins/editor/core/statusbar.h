/*
    Copyright (C) 2007 by Seth Yastrov

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

#ifndef __STATUSBAR_H__
#define __STATUSBAR_H__

#include <csutil/scf_implementation.h>
#include <ivaria/pmeter.h>
#include <csutil/csstring.h>

#include <wx/statusbr.h>
#include <wx/event.h>
#include <wx/gauge.h>

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

/// Custom status bar which shows a progress gauge
class StatusBar : public wxStatusBar
{
public:
  StatusBar (wxWindow* parent);
  virtual ~StatusBar ();

  void OnSize (wxSizeEvent& event);

  wxGauge* GetGauge ()
  { return gauge; }
  
private:
  wxGauge* gauge;
  
  enum
  {
    Field_Text,
    Field_Gauge,
    Field_Null,
    Field_Max
  };
  
  DECLARE_EVENT_TABLE()
};

/// Implementation of iProgressMeter using StatusBar
class StatusBarProgressMeter : public scfImplementation1<StatusBarProgressMeter,iProgressMeter>
{
public:
  StatusBarProgressMeter (StatusBar* statusBar, int total = 100)
  : scfImplementationType (this), statusBar (statusBar),
                           granularity (1) // Default of 1 is more exciting
  {
    gauge = statusBar->GetGauge ();
    gauge->SetValue(0);
    gauge->Show();
  }
  
  virtual ~StatusBarProgressMeter () {}

  /**
   * Set the tick scale.  Valid values are 1-100, inclusive.  Default is 2.  A
   * value of 1 means that each printed tick represents one unit, thus a total
   * of 100 ticks will be printed.  A value of 2 means that each tick
   * represents two units, thus a total of 50 ticks will be printed, etc.
   */
  void SetTickScale (int scale)
  { tick_scale = scale; }
  
  /// Get the tick scale.
  int GetTickScale () const { return tick_scale; }

  /**
   * Set the id and description of what we are currently monitoring.
   * An id can be something like "crystalspace.engine.lighting.calculation".
   * \sa \ref FormatterNotes
   */
  virtual void CS_GNUC_PRINTF (3, 4)
      SetProgressDescription (const char* id, const char* description, ...)
  {
    va_list args;
    va_start (args, description);
    SetProgressDescriptionV (id, description, args);
    va_end (args);
  }
  
  virtual void CS_GNUC_PRINTF (3, 0)
      SetProgressDescriptionV (const char* id, const char* description, va_list args)
  {
    csString text;
    text.FormatV (description, args);
    statusBar->SetStatusText (wxString (text.GetData(), wxConvUTF8));
  }

  /// Increment the meter by n units (default 1) and print a tick mark.
  virtual void Step (unsigned int n = 1)
  {
    gauge->SetValue(gauge->GetValue() + n);
    if (GetCurrent() % granularity == 0) {
      gauge->Update();
      wxYield();
    }
  }
  
  /// Reset the meter to 0%.
  virtual void Reset () { gauge->SetValue(0); gauge->Refresh(); }
  /// Reset the meter and print the initial tick mark ("0%").
  virtual void Restart () { Reset (); }
  /// Abort the meter.
  virtual void Abort () { Reset (); }
  /// Finalize the meter (i.e. we completed the task sooner than expected).
  virtual void Finalize () { Reset (); }

  /// Set the total element count represented by the meter and perform a reset.
  virtual void SetTotal (int n) { gauge->SetRange(n); Reset(); }
  /// Get the total element count represented by the meter.
  virtual int GetTotal () const { return gauge->GetRange(); }
  /// Get the current value of the meter (<= total).
  virtual int GetCurrent () const { return gauge->GetValue(); }

  /**
   * Set the refresh granularity.  Valid values are 1-100, inclusive.  Default
   * is 10.  The meter is only refreshed after each "granularity" * number of
   * units have passed.  For instance, if granularity is 20, then * the meter
   * will only be updated at most 5 times, or every 20%.
   */
  virtual void SetGranularity (int n) { granularity = n; }
  /// Get the refresh granularity.
  virtual int GetGranularity () const { return granularity; }

private:
  StatusBar* statusBar;
  wxGauge* gauge;
  
  int granularity;
  int tick_scale;
};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
