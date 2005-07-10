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
#include "csutil/csstring.h"
#include "csutil/event.h"
#include "csutil/util.h"
#include "csutil/win32/win32.h"
#include "csutil/win32/wintools.h"
#include "ivaria/reporter.h"

#define INITGUID
#include <windows.h>
// using DirectInput -- included from csjoywin.h
#include "csjoywin.h"

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

void csWindowsJoystick::LoadAxes(joystate& j)
{
  size_t const n = j.axes.Length();
  if (n > 0) j.axes[0] = j.di.lX;
  if (n > 1) j.axes[1] = j.di.lY;
  if (n > 2) j.axes[2] = j.di.lZ;
  if (n > 3) j.axes[3] = j.di.lRx;
  if (n > 4) j.axes[4] = j.di.lRy;
  if (n > 5) j.axes[5] = j.di.lRz;
  if (n > 6) j.axes[6] = j.di.rglSlider[0];
  if (n > 7) j.axes[7] = j.di.rglSlider[1];
}

bool csWindowsJoystick::HandleEvent (iEvent& ev)
{
  if (ev.Type != csevBroadcast ||
      csCommandEventHelper::GetCode(&ev) != cscmdPreProcess)
    return false;

  for (size_t i = 0; i < joystick.Length (); i++)
  {
    joydata& jd = joystick[i];
    jd.device->Poll ();
    int nstate = jd.nstate;
    HRESULT hr = jd.device->GetDeviceState ((DWORD)sizeof (DIJOYSTATE2), 
      (LPVOID)&jd.state[nstate].di);
    if (FAILED (hr))
    {
      jd.device->Acquire();  // try to reacquire
      // ... and try again
      hr = jd.device->GetDeviceState ((DWORD)sizeof (DIJOYSTATE2), 
	(LPVOID)&jd.state[nstate].di);
    } 
    if (SUCCEEDED (hr))
    {
      LoadAxes(jd.state[nstate]);

      int last_state=1-nstate;
      for (int btn = 0; btn < 128; btn++) 
      {
        if (jd.state[nstate].di.rgbButtons[btn] != 
	    jd.state[last_state].di.rgbButtons[btn]) 
          EventOutlet->Joystick (jd.number, btn + 1, 
				 jd.state[nstate].di.rgbButtons[btn] != 0,
				 jd.state[nstate].axes.GetArray(), jd.nAxes);
      }    
      for (uint a = 0; a < jd.nAxes; a++)
      {
        if (jd.state[nstate].axes[a] != jd.state[last_state].axes[a])
	{
          EventOutlet->Joystick (jd.number, 0, 0,
	    jd.state[nstate].axes.GetArray(), jd.nAxes);
	  break;
	}
      }
      jd.nstate=last_state;
    }
  }
  return false;
}

static BOOL CALLBACK axes_callback(LPCDIDEVICEOBJECTINSTANCE i, LPVOID p)
{
  // Configure to return data in the range (-32767..32767)
  DIPROPRANGE diprg;
  diprg.diph.dwSize = sizeof (diprg);
  diprg.diph.dwHeaderSize = sizeof (diprg.diph);
  diprg.diph.dwHow = DIPH_BYID;
  diprg.diph.dwObj = i->dwType;
  diprg.lMin = -32767;
  diprg.lMax =  32767;

  LPDIRECTINPUTDEVICE2 device = (LPDIRECTINPUTDEVICE2)p;
  device->SetProperty (DIPROP_RANGE, &diprg.diph);

  return DIENUM_CONTINUE;
}

bool csWindowsJoystick::CreateDevice (const DIDEVICEINSTANCE*  pdidInstanc)
{ 
  LPDIRECTINPUTDEVICE device;
  lpdin->CreateDevice (pdidInstanc->guidInstance, &device, 0);
  bool const ok = (device != 0);
  if (ok) 
  {
    LPDIRECTINPUTDEVICE2 device2 = (LPDIRECTINPUTDEVICE2)device;

    DIDEVCAPS caps;
    caps.dwSize = sizeof (caps);
    device2->GetCapabilities (&caps);
    device2->SetDataFormat (&c_dfDIJoystick2);

    if (SUCCEEDED(device2->EnumObjects(axes_callback, (LPVOID)device2,
      DIDFT_AXIS)))
    {
      joydata data;
      data.number = (int)joystick.Length() + 1; // CS joystick numbers 1-based
      data.device = device2;
      data.nButtons = caps.dwButtons;    
      data.nAxes = (uint)caps.dwAxes;
      for (int i = 0; i < 2; i++)
        data.state[i].axes.SetSize(data.nAxes);
      joystick.Push (data);
    }
  }
  return ok;
}

static BOOL CALLBACK dev_callback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
  csWindowsJoystick* po = (csWindowsJoystick*)pvRef;
  po->CreateDevice (lpddi);
  return DIENUM_CONTINUE;
}

bool csWindowsJoystick::Init ()
{
  csRef<iWin32Assistant> win32 = CS_QUERY_REGISTRY(object_reg, iWin32Assistant);
  HRESULT hr = DirectInputCreate (win32->GetInstance (), DIRECTINPUT_VERSION, 
    &lpdin, NULL);
  if (SUCCEEDED (hr))
  {
    HWND window = win32->GetApplicationWindow ();
    // Only enum attached Joysticks, get Details of Devices
    lpdin->EnumDevices (DIDEVTYPE_JOYSTICK, &dev_callback,
      (LPVOID)this, DIEDFL_ATTACHEDONLY);
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
	eq->RegisterListener (&scfiEventHandler, CSMASK_FrameProcess);
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
