/*
    Alternate Windowing System: timer class
    Copyright (C) 2002 by Norman Kraemer

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

#ifndef __CS_AWS_TIMER_H__
#define __CS_AWS_TIMER_H__

#include "awsslot.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/virtclk.h"
#include "iutil/objreg.h"

class awsTimer : public awsSource
{
protected:
  /// Everything ready to go?
  bool bSetup;

  /// Flag to halt the ticker.
  bool stopped;

  csTicks start, nTicks;

  csRef<iVirtualClock> vc;
  iObjectRegistry *object_reg;

  /// Try to set up everything needed.
  bool Setup ();
public:
  static const int signalTick;

  SCF_DECLARE_IBASE_EXT (awsSource);

  awsTimer (iObjectRegistry *object_reg, iAwsComponent *comp);
  virtual ~awsTimer ();

  bool SetTimer (csTicks nTicks);
  void Stop ();
  bool Start ();
  bool IsRunning ();
  bool HandleEvent (iEvent &Event);

  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE (awsTimer);
    virtual bool HandleEvent (iEvent &Event)
    {
      return scfParent->HandleEvent (Event);
    }
  }
  scfiEventHandler;
};

#endif // __CS_AWS_TIMER_H__
