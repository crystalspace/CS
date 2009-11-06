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

#ifndef __CS_CSJOYWIN_H__
#define __CS_CSJOYWIN_H__

#include "csutil/scf_implementation.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/comp.h"
#include "csutil/cfgacc.h"
#include "csutil/dirtyaccessarray.h"

// Require DirectInput 8
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "dinputdefs.h"

CS_PLUGIN_NAMESPACE_BEGIN(JoystickWin)
{

/**
 * This plugin puts joystick events in the CS eventqueue.
 * Joystick data is gathered via the DirectInput joystick api.
 */
class csWindowsJoystick : 
  public scfImplementation3<csWindowsJoystick, 
                            iComponent,
                            iEventPlug,
                            iEventHandler>
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
    LPDIRECTINPUTDEVICE8W device; // DInput device
    int nButtons;		 // number of buttons
    uint nAxes;			 // number of axis
    joystate state[2];		 // joystick current & last state
    int nstate;			 // this is current state
	csDirtyAccessArray<int32> axesMapping;

    joydata() : number(0), device(0), nButtons(0), nAxes(0), nstate(0) {}
    joydata(joydata const& j) : number(j.number), device(j.device),
      nButtons(j.nButtons), nAxes(j.nAxes), nstate(j.nstate)
	{
	  axesMapping = j.axesMapping;
      for (int i = 0; i < 2; i++)
        state[i] = j.state[i];
    }
  };
  csEventID Frame;

 protected:
  iObjectRegistry *object_reg;
  csArray<joydata> joystick;
  csConfigAccess config;
  csRef<iEventQueue > eq;
  csRef<iEventOutlet> EventOutlet;
  LPDIRECTINPUT8W lpdin;

  bool Init ();
  bool Close ();
  void Report (int severity, const char* msg, ...);
  void LoadAxes(joystate& j, const joydata& jdata);

  void ReportDXError (HRESULT hr, const char* msg, ...);
 public:
  csWindowsJoystick (iBase *parent);
  virtual ~csWindowsJoystick ();

  virtual bool Initialize (iObjectRegistry *oreg);
  virtual bool CreateDevice (const DIDEVICEINSTANCEW* pdidInstance);

  /**\name iEventPlug implementation
   * @{ */
  virtual unsigned GetPotentiallyConflictingEvents ()
  { return CSEVTYPE_Joystick; }
  virtual unsigned QueryEventPriority (unsigned) { return 110; }
  /** @} */

  /**\name iEventHandler implementation
   * @{ */
  virtual bool HandleEvent (iEvent &Event);
  CS_EVENTHANDLER_PHASE_LOGIC("crystalspace.device.joystick")
  /** @} */
};

}
CS_PLUGIN_NAMESPACE_END(JoystickWin)

#endif // __CS_CSJOYWIN_H__
