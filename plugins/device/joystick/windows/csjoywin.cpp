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

#include "cssysdef.h"
#include "csver.h"
#include "ivaria/reporter.h"
#include "csutil/util.h"
#include "csutil/csstring.h"
#define INITGUID
#include <windows.h>

// using DirectInput -- included from csjoywin.h
#include "csjoywin.h"

#include "csutil/win32/wintools.h"
#include "csutil/win32/win32.h"

// no config yet
// #define CS_WINDOWS_JOYSTICK_CFG "/config/joywin.cfg"
// #define CS_WINDOWS_JOYSTICK_KEY "Device.JoyWin."

// #define CS_LINUX_JOYSTICK_OLD_EVENTS // the values of the first two axis are sent only

CS_IMPLEMENT_PLUGIN;

SCF_IMPLEMENT_FACTORY (csWindowsJoystick);

SCF_IMPLEMENT_IBASE (csWindowsJoystick)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iEventPlug)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csWindowsJoystick::eiEventPlug)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csWindowsJoystick::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csWindowsJoystick::csWindowsJoystick (iBase *parent) :
  object_reg (0),
  joystick (0),
  eq (0),
  EventOutlet (0)
{
  SCF_CONSTRUCT_IBASE(parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventPlug);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventHandler);
}

csWindowsJoystick::~csWindowsJoystick ()
{
  Close ();
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiEventHandler);
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiEventPlug);
  SCF_DESTRUCT_IBASE();
}

bool csWindowsJoystick::Initialize (iObjectRegistry* oreg)
{
  object_reg = oreg;
  return Init ();
}

bool csWindowsJoystick::HandleEvent (iEvent &)
{
  HRESULT hr;
  int nstate, last_state;
  for (size_t i = 0; i < joystick.Length (); i++)
  {
    joystick[i].device->Poll ();
    nstate = joystick[i].nstate;
    hr = joystick[i].device->GetDeviceState ((DWORD)sizeof (DIJOYSTATE2), 
      (LPVOID)&joystick[i].state[nstate]);
    if (FAILED (hr))
    {
      joystick[i].device->Acquire();  // try to reacquire
      // ... and try again
      hr = joystick[i].device->GetDeviceState ((DWORD)sizeof (DIJOYSTATE2), 
	(LPVOID)&joystick[i].state[nstate]);
    } 
    if (SUCCEEDED (hr))
    {
      last_state=1-nstate;
      for (int btn = 0; btn < 128; btn++) 
      {
        if (joystick[i].state[nstate].rgbButtons[btn] != 
	  joystick[i].state[last_state].rgbButtons[btn]) 
	{
          EventOutlet->Joystick(i, btn + 1, 
	    joystick[i].state[nstate].rgbButtons[btn] != 0,
            joystick[i].state[nstate].lX, joystick[i].state[nstate].lY);
        }
      }    
      if ((joystick[i].state[nstate].lX != joystick[i].state[last_state].lX) ||
        (joystick[i].state[nstate].lY != joystick[i].state[last_state].lY))
      {
        EventOutlet->Joystick (i, 0, 0, joystick[i].state[nstate].lX,
	joystick[i].state[nstate].lY);
      }
      joystick[i].nstate=last_state;
    }
  }
  return false;
}

// DirectInputDeviceEnumerator Function

BOOL CALLBACK DIEnumDevicesCallback(
  LPCDIDEVICEINSTANCE lpddi,  
  LPVOID pvRef)
{
  csWindowsJoystick* po;
  po = (csWindowsJoystick*) pvRef;
  po->CreateDevice (lpddi);
  return DIENUM_CONTINUE;
}

bool csWindowsJoystick::CreateDevice (const DIDEVICEINSTANCE*  pdidInstanc)
{ 
  DIDEVCAPS caps;
  LPDIRECTINPUTDEVICE device;
  lpdin->CreateDevice (pdidInstanc->guidInstance, &device, 0);
  if (device) 
  {
    joydata data;
    data.number = joystick.Length ();
    caps.dwSize = sizeof (caps);
    data.device = (LPDIRECTINPUTDEVICE2)device;
    data.device->GetCapabilities (&caps);
    data.nAxes = caps.dwAxes;
    data.nButtons = caps.dwButtons;    
    data.device->SetDataFormat (&c_dfDIJoystick2);

    // Configure to return data in the range (-32767..32767)
    DIPROPRANGE diprg;
    diprg.diph.dwSize = sizeof (diprg);
    diprg.diph.dwHeaderSize = sizeof (diprg.diph);
    diprg.diph.dwHow = DIPH_BYOFFSET;
    diprg.lMin = -32767;
    diprg.lMax = 32767;  

    diprg.diph.dwObj = 0; // lX
    data.device->SetProperty (DIPROP_RANGE, &diprg.diph);
    diprg.diph.dwObj = 1 * sizeof(LONG); //lY
    data.device->SetProperty (DIPROP_RANGE, &diprg.diph);

    joystick.Push (data);
  }
  return true;
}

bool csWindowsJoystick::Init ()
{
  csRef<iWin32Assistant> win32 = CS_QUERY_REGISTRY (object_reg, 
    iWin32Assistant);
  HRESULT hr = DirectInputCreate (win32->GetInstance (), DIRECTINPUT_VERSION, 
    &lpdin, NULL);
  if (SUCCEEDED (hr))
  {
    HWND window = win32->GetApplicationWindow ();
    // Only enum attached Joysticks, get Details of Devices
    lpdin->EnumDevices (DIDEVTYPE_JOYSTICK, &DIEnumDevicesCallback,
      (LPVOID)this,  DIEDFL_ATTACHEDONLY);
    size_t i;
    for (i = 0; i < joystick.Length (); i++) 
    {
      DIDEVICEINSTANCEA devInfo;
      memset (&devInfo, 0, sizeof (devInfo));
      devInfo.dwSize = sizeof (devInfo);
      hr = joystick[i].device->GetDeviceInfo (&devInfo);
      if (FAILED (hr))
      {
	Report (CS_REPORTER_SEVERITY_WARNING, 
	  "Can't retrieve device information for #%d: error %.8x", i, hr);
      }
      else
      {
	wchar_t* devProduct = cswinAnsiToWide (devInfo.tszProductName);
        Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Found input device #%d: %s", i + 1, 
	  (const char*)csWtoC (devProduct));
	delete[] devProduct;
      }
    
#ifdef CS_DEBUG
      joystick[i].device->SetCooperativeLevel (window,
        DISCL_EXCLUSIVE | DISCL_BACKGROUND);
#else
      joystick[i].device->SetCooperativeLevel (window,
        DISCL_EXCLUSIVE | DISCL_FOREGROUND);
#endif  
         
      // according to DX SDK 4 joysticks
      joystick[i].device->Acquire();	
      /*
        This is one of the crazy things in DInput: (Un)Acquire! 
	Who has to be shot for this?
       */
    }
   // hook into eventqueue
    eq = CS_QUERY_REGISTRY(object_reg, iEventQueue);
    if (eq)
    {
      eq->RegisterListener (&scfiEventHandler, CSMASK_Nothing);
      EventOutlet = eq->CreateEventOutlet (&scfiEventPlug);
    }
    Report (CS_REPORTER_SEVERITY_NOTIFY, "DirectInput Joystick plugin loaded!");
  } 
  else
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 
      "Can't retrieve Direct Input interface: error %.8x", hr);
  }

  return eq && EventOutlet;
}


bool csWindowsJoystick::Close ()
{
  if (eq)
  {
    eq->RemoveListener (&scfiEventHandler);
    eq = 0;
  }
  EventOutlet = 0;
  for (size_t i = 0; i < joystick.Length (); i++)
  {  
    joystick[i].device->Unacquire ();
    joystick[i].device->Release ();
  }
  lpdin->Release();
  lpdin = 0;
  return true;
}

void csWindowsJoystick::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csReportV (object_reg, severity, "crystalspace.device.joystick.windows", 
    msg, arg);
  va_end (arg);
}
