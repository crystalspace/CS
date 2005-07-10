/*
    Copyright (C) 2003 by TURBO J

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

#ifndef _CS_WINDOWS_JOYSTICK_
#define _CS_WINDOWS_JOYSTICK_

#include "csutil/scf.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/comp.h"
#include "csutil/cfgacc.h"
#include "csutil/dirtyaccessarray.h"

#define DIRECTINPUT_VERSION 0x0500
// @@@ Should be able build on older SDKs as well
#include <dinput.h>
#ifdef CS_COMPILER_BCC
#include "dinputdefs.h"
#endif

/**
 * This plugin puts joystick events in the CS eventqueue.
 * Joystick data is gathered via the DirectInput joystick api.
 */
class csWindowsJoystick : public iComponent
{
private:
  struct joystate
  {
    DIJOYSTATE2 di;			// DInput state structure
    csDirtyAccessArray<int32> axes;	// Axes in this state
    joystate() { memset(&di, 0, sizeof(di)); }
    joystate(joystate const& j) : axes(j.axes)
    { memcpy(&di, &j.di, sizeof(di)); }
  };
  struct joydata
  {
    int number;			 // joystick number; CS numbers are 1-based
    LPDIRECTINPUTDEVICE2 device; // DInput device
    int nButtons;		 // number of buttons
    uint nAxes;			 // number of axis
    joystate state[2];		 // joystick current & last state
    int nstate;			 // this is current state
    joydata() : number(0), device(0), nButtons(0), nAxes(0), nstate(0) {}
    joydata(joydata const& j) : number(j.number), device(j.device),
      nButtons(j.nButtons), nAxes(j.nAxes), nstate(j.nstate)
    {
      for (int i = 0; i < 2; i++)
        state[i] = j.state[i];
    }
  };

 protected:
  iObjectRegistry *object_reg;
  csArray<joydata> joystick;
  csConfigAccess config;
  csRef<iEventQueue > eq;
  csRef<iEventOutlet> EventOutlet;
  LPDIRECTINPUT lpdin;

  bool Init ();
  bool Close ();
  void Report (int severity, const char* msg, ...);
  void LoadAxes(joystate&);

 public:
  SCF_DECLARE_IBASE;

  csWindowsJoystick (iBase *parent);
  virtual ~csWindowsJoystick ();

  virtual bool Initialize (iObjectRegistry *oreg);
  virtual bool HandleEvent (iEvent &Event);
  virtual bool CreateDevice (const DIDEVICEINSTANCE* pdidInstance);

  struct eiEventPlug : public iEventPlug
  {
    SCF_DECLARE_EMBEDDED_IBASE (csWindowsJoystick);
    virtual unsigned GetPotentiallyConflictingEvents ()
    { return CSEVTYPE_Joystick; }
    virtual unsigned QueryEventPriority (unsigned) { return 110; }
  } scfiEventPlug;
  friend struct eiEventPlug;

  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE (csWindowsJoystick);
    virtual bool HandleEvent (iEvent &Event)
    { return scfParent->HandleEvent (Event); }
  } scfiEventHandler;
  friend struct eiEventHandler;
};

#endif
