/*
    Copyright (C) 2002 by Norman Krämer
  
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
#include "csjoylin.h"
#include "ivaria/reporter.h"

#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>

#define CS_LINUX_JOYSTICK_CFG "/config/joystick.cfg"
#define CS_LINUX_JOYSTICK_KEY "Device.Joystick.Linux."

#define CS_LINUX_JOYSTICK_OLD_EVENTS // the values of the first two axis are sent only

CS_IMPLEMENT_PLUGIN;

SCF_IMPLEMENT_IBASE (csLinuxJoystick)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iEventPlug)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csLinuxJoystick::eiEventPlug)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csLinuxJoystick::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csLinuxJoystick::csLinuxJoystick (iBase *parent):
  object_reg(NULL),
  joystick(NULL),
  nJoy(0),
  eq(NULL),
  EventOutlet(NULL)
{
  SCF_CONSTRUCT_IBASE(parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventPlug);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventHandler);
}

csLinuxJoystick::~csLinuxJoystick ()
{
  Close ();
}

bool csLinuxJoystick::Initialize (iObjectRegistry *oreg)
{
  object_reg = oreg;
  return Init ();
}

bool csLinuxJoystick::HandleEvent (iEvent &)
{
  struct js_event js;

  for (int i=0; i < nJoy; i++)
  {
    while (read (joystick [i].fd, &js, sizeof(struct js_event)) == sizeof(struct js_event)) 
    {
      switch (js.type & ~JS_EVENT_INIT) 
      {
      case JS_EVENT_BUTTON:
        //        DEBUG_BREAK;
        joystick[i].button[js.number] = js.value;
        EventOutlet->Joystick (i, js.number+1, js.value, -1, 0);
        break;
      case JS_EVENT_AXIS:
        joystick[i].axis[js.number] = js.value;
#ifdef CS_LINUX_JOYSTICK_OLD_EVENTS
        if (js.number < 2)
          EventOutlet->Joystick (i, 0, 0, joystick[i].axis[0], (joystick[i].nAxes > 1 ? joystick[i].axis[1] : 0));
#else
        EventOutlet->Joystick (i, 0, 0, js.number, js.value);
#endif
        break;
      }
    }
  }
  return false;
}

bool csLinuxJoystick::Init ()
{
  // read list of devices
  config.AddConfig (object_reg, CS_LINUX_JOYSTICK_CFG);
  iConfigIterator *it = config->Enumerate (CS_LINUX_JOYSTICK_KEY);

  csVector h;
  int fd;

  nJoy=0;
  SCF_DEC_REF (eq);
  eq = NULL;
  SCF_DEC_REF (EventOutlet);
  EventOutlet = NULL;

  while (it->Next ())
  {
    if ((fd = open (it->GetStr (), O_RDONLY)) < 0) 
    {
      Report (CS_REPORTER_SEVERITY_NOTIFY, 
              "Failed to open joystick device %s - error: %s\n",
              it->GetStr (),
              strerror (errno));
    }
    else
      nJoy++;
    h.Push ((csSome)fd);
  }
  it->DecRef ();

  if (nJoy)
  {
    delete [] joystick;
    joystick = new joydata[nJoy];
    
    for (int i=0, n=0; i < h.Length (); i++)
    {
      fd = (int)h.Get (i);
      if (fd > -1)
      {
	unsigned char axes = 2;
	unsigned char buttons = 2;
	int version = 0x000800;
	char name[128] = "Unknown";

	ioctl (fd, JSIOCGVERSION, &version);
	ioctl (fd, JSIOCGAXES, &axes);
	ioctl (fd, JSIOCGBUTTONS, &buttons);
	ioctl (fd, JSIOCGNAME(128), name);

	Report (CS_REPORTER_SEVERITY_NOTIFY,
                "Joystick number %d (%s) has %d axes and %d buttons. Driver version is %d.%d.%d.\n",
		i, name, axes, buttons, version >> 16, (version >> 8) & 0xff, version & 0xff);
        joystick[n].number = i;
        joystick[n].fd = fd;
        joystick[n].nButtons = buttons;
        joystick[n].nAxes = axes;
        joystick[n].axis = new int16[axes];
        joystick[n].button = new int16[buttons];

        // make device non blocking
        fcntl(fd, F_SETFL, O_NONBLOCK);
        n++;
      }
    }

    // hook into eventqueue
    eq = CS_QUERY_REGISTRY(object_reg, iEventQueue);
    if (eq)
    {
      eq->RegisterListener (&scfiEventHandler, CSMASK_Nothing);
      EventOutlet = eq->CreateEventOutlet (&scfiEventPlug);
    }
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY,
            "No operable joystick found\n");
  }

  return eq && EventOutlet;
}

bool csLinuxJoystick::Close ()
{
  if (eq)
  {
    eq->RemoveListener (&scfiEventHandler);
    eq->DecRef ();
    eq = NULL;
  }

  SCF_DEC_REF (EventOutlet);
  EventOutlet = NULL;

  for (int i=0; i<nJoy; i++)
    close (joystick[i].fd);

  delete [] joystick;
  joystick = NULL;
  nJoy = 0;

  return true;
}

void csLinuxJoystick::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
  {
    rep->ReportV (severity, "crystalspace.device.joystick.linux", msg, arg);
    rep->DecRef ();
  }
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

SCF_IMPLEMENT_FACTORY (csLinuxJoystick);

SCF_EXPORT_CLASS_TABLE (csjoylin)
  SCF_EXPORT_CLASS (csLinuxJoystick, "crystalspace.device.joystick.linux", 
                    "Crystal Space Joystick plugin for Linux")
SCF_EXPORT_CLASS_TABLE_END
