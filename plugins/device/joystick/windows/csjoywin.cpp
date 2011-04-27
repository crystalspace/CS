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
#include "csplugincommon/directx/guids.h"

#include "csutil/csstring.h"
#include "csutil/event.h"
#include "csutil/eventnames.h"
#include "csutil/eventhandlers.h"
#include "csutil/util.h"
#include "csutil/win32/win32.h"
#include "csutil/win32/wintools.h"
#include "ivaria/reporter.h"
#include "ivideo/graph2d.h"

// using DirectInput -- included from csjoywin.h
#include "csjoywin.h"

#include "csplugincommon/directx/error.h"

// no config yet
// #define CS_WINDOWS_JOYSTICK_CFG "/config/joystick.cfg"
// #define CS_WINDOWS_JOYSTICK_KEY "Device.Joystick." CS_PLATFORM_NAME "."



CS_PLUGIN_NAMESPACE_BEGIN(JoystickWin)
{

SCF_IMPLEMENT_FACTORY (csWindowsJoystick);

csWindowsJoystick::csWindowsJoystick (iBase *parent) :
  scfImplementationType (this, parent),
  object_reg (0),
  joystick (0),
  eq (0),
  EventOutlet (0)
{
}

csWindowsJoystick::~csWindowsJoystick ()
{
  Close ();
}

bool csWindowsJoystick::Initialize (iObjectRegistry* oreg)
{
  object_reg = oreg;
  return Init ();
}

//  Pointer Math
// should be Win64 safe, though
#define JOYSTICK_ACCESS(afield,anofs) *(LONG*)(((ULONG_PTR)&afield)+anofs)

void csWindowsJoystick::LoadAxes(joystate& j, const joydata& jdata)
{
  size_t const n = j.axes.GetSize ();

  for (size_t i = 0; i < n; i++)
    j.axes[i] = JOYSTICK_ACCESS(j.di.lX,jdata.axesMapping[i]);
}

bool csWindowsJoystick::HandleEvent (iEvent& ev)
{
  if (ev.Name != Frame)
    return false;

  for (size_t i = 0; i < joystick.GetSize (); i++)
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
      LoadAxes(jd.state[nstate],jd);

      int last_state=1-nstate;
      for (int btn = 0; btn < 128; btn++) 
      {
    	if (jd.state[nstate].di.rgbButtons[btn] != 
            jd.state[last_state].di.rgbButtons[btn]) 
        {
          EventOutlet->Joystick (jd.number, btn, 
            jd.state[nstate].di.rgbButtons[btn] != 0,
            jd.state[nstate].axes.GetArray(), jd.nAxes);
        }
      }    
      for (uint a = 0; a < jd.nAxes; a++)
      {
        if (jd.state[nstate].axes[a] != jd.state[last_state].axes[a])
        {
          EventOutlet->Joystick (jd.number, -1, 0,
            jd.state[nstate].axes.GetArray(), jd.nAxes);
          break;
        }
      }
      jd.nstate=last_state;
    }
  }
  return false;
}

// a pointer to this struct is passed to axes_callback

struct JoyAxesInfo
{
  LPDIRECTINPUTDEVICE8W device;
  uint nAxes;
  csDirtyAccessArray<int32> axesMapping;
};

static BOOL CALLBACK axes_callback(LPCDIDEVICEOBJECTINSTANCEW i, LPVOID p)
{
  // Configure to return data in the range (-32767..32767)
  DIPROPRANGE diprg;
  JoyAxesInfo* pJoyAxisInfo = (JoyAxesInfo*)p;

  diprg.diph.dwSize = sizeof (diprg);
  diprg.diph.dwHeaderSize = sizeof (diprg.diph);
  diprg.diph.dwHow = DIPH_BYID;
  diprg.diph.dwObj = i->dwType;
  diprg.lMin = -32767;
  diprg.lMax =  32767;

  // use the GUID to set the offset of axis n
  // in the DIJOYSTATE2 struct
  // misUsing nAxes as Index variable
  if (i->guidType != GUID_Unknown) 
  {
    if (i->guidType == GUID_XAxis)
    { 
      pJoyAxisInfo->axesMapping[pJoyAxisInfo->nAxes++] = DIJOFS_X;
    }
    else if (i->guidType == GUID_YAxis) 
    { 
      pJoyAxisInfo->axesMapping[pJoyAxisInfo->nAxes++] = DIJOFS_Y;
    }
    else if (i->guidType == GUID_ZAxis)
    {   
      pJoyAxisInfo->axesMapping[pJoyAxisInfo->nAxes++] = DIJOFS_Z;
    }
    else if (i->guidType == GUID_RxAxis)
    { 
      pJoyAxisInfo->axesMapping[pJoyAxisInfo->nAxes++] = DIJOFS_RX;
    }
    else if (i->guidType == GUID_RyAxis)
    { 
      pJoyAxisInfo->axesMapping[pJoyAxisInfo->nAxes++] = DIJOFS_RY;
    }
    else if (i->guidType == GUID_RzAxis)
    { 
      pJoyAxisInfo->axesMapping[pJoyAxisInfo->nAxes++] = DIJOFS_RZ;
    }
    else if (i->guidType == GUID_Slider)
    { 
      //TODO check whether Slider0 or 1
      // haven't seen one with more than 1 Slider, though...
      pJoyAxisInfo->axesMapping[pJoyAxisInfo->nAxes++] = DIJOFS_SLIDER(0);
    }
  }
  else 
  {
    // I honestly don't know what to do in this case.
    // There seems to be no clear mapping between
    // axis # and DIJoystate2 member/offset
    //TODO BLACK MAGIC HERE
  }
  LPDIRECTINPUTDEVICE8W device = pJoyAxisInfo->device;
  device->SetProperty (DIPROP_RANGE, &diprg.diph);

  return DIENUM_CONTINUE;
}


bool csWindowsJoystick::CreateDevice (const DIDEVICEINSTANCEW*  pdidInstanc)
{ 
  LPDIRECTINPUTDEVICE8W device;
  lpdin->CreateDevice (pdidInstanc->guidInstance, &device, 0);
  bool const ok = (device != 0);
  if (ok) 
  {
    DIDEVCAPS caps;
    JoyAxesInfo jaxes;

    caps.dwSize = sizeof (caps);
    device->GetCapabilities (&caps);
    device->SetDataFormat (&c_dfDIJoystick2);

    jaxes.nAxes = 0;
    jaxes.axesMapping.SetSize(caps.dwAxes);
    if (caps.dwAxes >0) {
      DWORD i;
      for ( i = 0; i < caps.dwAxes; i++) {
        //TODO use a more sane default mapping
	jaxes.axesMapping[i] = 0; // default X Axis
      }
    }
    jaxes.device = device;
    if (SUCCEEDED(device->EnumObjects(axes_callback, (LPVOID)&jaxes,
      DIDFT_AXIS)))
    {
      joydata data;

      data.axesMapping = jaxes.axesMapping;
      data.number = (int)joystick.GetSize () + 0; // CS joystick numbers 1-based
      data.device = device;
      data.nButtons = caps.dwButtons;    
      data.nAxes = (uint)caps.dwAxes;
     
      for (int i = 0; i < 2; i++)
        data.state[i].axes.SetSize(data.nAxes);
      joystick.Push (data);
    }
  }
  return ok;
}

static BOOL CALLBACK dev_callback(LPCDIDEVICEINSTANCEW lpddi, LPVOID pvRef)
{
  csWindowsJoystick* po = (csWindowsJoystick*)pvRef;
  po->CreateDevice (lpddi);
  return DIENUM_CONTINUE;
}

bool csWindowsJoystick::Init ()
{
  csRef<iWin32Assistant> win32 = csQueryRegistry<iWin32Assistant> (object_reg);
  HRESULT hr = DirectInput8Create (win32->GetInstance (), DIRECTINPUT_VERSION, 
    IID_IDirectInput8W, (void**)&lpdin, 0);
  if (SUCCEEDED (hr))
  {
    csRef<iGraphics2D> g2d = csQueryRegistry<iGraphics2D> (object_reg);
    if (!g2d.IsValid())
    {
      Report(CS_REPORTER_SEVERITY_ERROR, 
        "A canvas is required");
      return false;
    }
    csRef<iWin32Canvas> canvas = scfQueryInterface<iWin32Canvas> (g2d);
    if (!canvas.IsValid())
    {
      Report(CS_REPORTER_SEVERITY_WARNING,
        "A window is required");
      return false;
    }

    HWND window = canvas->GetWindowHandle();
    // Only enum attached Joysticks, get Details of Devices
    lpdin->EnumDevices (DI8DEVCLASS_GAMECTRL, &dev_callback,
      (LPVOID)this, DIEDFL_ATTACHEDONLY);
    size_t i;
    size_t const njoys = joystick.GetSize ();
    for (i = 0; i < njoys; i++) 
    {
      joydata& jd = joystick[i];
      DIDEVICEINSTANCEW devInfo;
      memset (&devInfo, 0, sizeof (devInfo));
      devInfo.dwSize = sizeof (devInfo);
      hr = jd.device->GetDeviceInfo (&devInfo);
      if (FAILED (hr))
      {
        ReportDXError (hr,
          "Can't retrieve information for device %zu", i);
      }
      else
      {
        Report (CS_REPORTER_SEVERITY_NOTIFY,
          "Found input device %d: %ls", jd.number, devInfo.tszProductName);
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
    Frame = csevFrame (object_reg);
    if (njoys > 0)
    {
      eq = csQueryRegistry<iEventQueue> (object_reg);
      if (eq)
      {
	eq->RegisterListener (this, Frame);
	EventOutlet = eq->CreateEventOutlet (this);
      }
    }
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "DirectInput Joystick plugin loaded; %zu joysticks", njoys);
  } 
  else
  {
    ReportDXError (hr,
      "Joystick plugin: can't retrieve Direct Input interface");
  }

  return eq && EventOutlet;
}

bool csWindowsJoystick::Close ()
{
  if (eq)
  {
    eq->RemoveListener (this);
    eq = 0;
  }
  EventOutlet = 0;
  for (size_t i = 0; i < joystick.GetSize (); i++)
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

void csWindowsJoystick::ReportDXError (HRESULT hr, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csString s;
  s.FormatV (msg, arg);
  Report (CS_REPORTER_SEVERITY_ERROR, "%s: %s [%s]",
    s.GetData(), csDirectXError::GetErrorDescription (hr),
    csDirectXError::GetErrorSymbol (hr));
  va_end (arg);
}

}
CS_PLUGIN_NAMESPACE_END(JoystickWin)
