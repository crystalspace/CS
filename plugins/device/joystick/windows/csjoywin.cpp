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
// #define CS_WINDOWS_JOYSTICK_CFG "/config/joystick.cfg"
// #define CS_WINDOWS_JOYSTICK_KEY "Device.Joystick." CS_PLATFORM_NAME "."

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
    joydata& jd = joystick[i];
    jd.device->Poll ();
    nstate = jd.nstate;
    hr = jd.device->GetDeviceState ((DWORD)sizeof (DIJOYSTATE2), 
      (LPVOID)&jd.state[nstate]);
    if (FAILED (hr))
    {
      jd.device->Acquire();  // try to reacquire
      // ... and try again
      hr = jd.device->GetDeviceState ((DWORD)sizeof (DIJOYSTATE2), 
	(LPVOID)&jd.state[nstate]);
    } 
    if (SUCCEEDED (hr))
    {
      last_state=1-nstate;
      for (int btn = 0; btn < 128; btn++) 
      {
        if (jd.state[nstate].rgbButtons[btn] != 
	  jd.state[last_state].rgbButtons[btn]) 
	{
	  int axdata[2] = { jd.state[nstate].lX, jd.state[nstat].lY };
          EventOutlet->Joystick (jd.number, btn + 1, 
				 jd.state[nstate].rgbButtons[btn] != 0,
				 axdata, 2);
        }
      }    
      if ((jd.state[nstate].lX != jd.state[last_state].lX) ||
        (jd.state[nstate].lY != jd.state[last_state].lY))
      {
	int axdata[2] = { jd.state[nstate].lX, jd.state[nstate].lY };
        EventOutlet->Joystick (jd.number, 0, 0, axdata, 2);
      }
      jd.nstate=last_state;
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
    data.number = (int)joystick.Length() + 1; // CS joystick numbers are 1-based.
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
    size_t const njoys = joystick.Length();
    for (i = 0; i < njoys; i++) 
    {
      joydata& jd = joystick[i];
      DIDEVICEINSTANCEA devInfo;
      memset (&devInfo, 0, sizeof (devInfo));
      devInfo.dwSize = sizeof (devInfo);
      hr = jd.device->GetDeviceInfo (&devInfo);
      if (FAILED (hr))
      {
	Report (CS_REPORTER_SEVERITY_WARNING, 
	  "Can't retrieve device information for #%zu: error %.8lx", i, hr);
      }
      else
      {
	wchar_t* devProduct = cswinAnsiToWide (devInfo.tszProductName);
        Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Found input device #%d: %s", jd.number, 
	  (const char*)csWtoC (devProduct));
	delete[] devProduct;
      }
    
#ifdef CS_DEBUG
      jd.device->SetCooperativeLevel (window,
        DISCL_EXCLUSIVE | DISCL_BACKGROUND);
#else
      jd.device->SetCooperativeLevel (window,
        DISCL_EXCLUSIVE | DISCL_FOREGROUND);
#endif  
         
      // according to DX SDK 4 joysticks
      jd.device->Acquire();	
      /*
        This is one of the crazy things in DInput: (Un)Acquire! 
	Who has to be shot for this?
       */
    }

    // hook into eventqueue
    if (njoys > 0)
    {
      eq = CS_QUERY_REGISTRY(object_reg, iEventQueue);
      if (eq)
      {
	eq->RegisterListener (&scfiEventHandler, CSMASK_Nothing);
	EventOutlet = eq->CreateEventOutlet (&scfiEventPlug);
      }
    }
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "DirectInput Joystick plugin loaded; %zu joysticks", njoys);
  } 
  else
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Joystick plugin: can't retrieve "
      "Direct Input interface: error %.8lx", hr);
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
