/*
    DOS support for Crystal Space 3D library
    Written by David N. Arnold <derek_arnold@fuse.net>
    Written by Andrew Zabolotny <bit@eltech.ru>

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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/nearptr.h>
#include <sys/farptr.h>
#include <crt0.h>

#include "cssysdef.h"
#include "cssys/djgpp/djgpp.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "inputq.h"
#include "djkeysys.h"
#include "djmousys.h"

static KeyboardHandler KH;
static MouseHandler MH;
bool EnablePrintf;

static unsigned short ScanCodeToChar[128] =
{
  0,        CSKEY_ESC,'1',      '2',      '3',      '4',      '5',      '6',    // 00..07
  '7',      '8',      '9',      '0',      '-',      '=',      '\b',     '\t',   // 08..0F
  'q',      'w',      'e',      'r',      't',      'y',      'u',      'i',    // 10..17
  'o',      'p',      '[',      ']',      '\n',     CSKEY_CTRL,'a',     's',    // 18..1F
  'd',      'f',      'g',      'h',      'j',      'k',      'l',      ';',    // 20..27
  39,       '`',      CSKEY_SHIFT,'\\',   'z',      'x',      'c',      'v',    // 28..2F
  'b',      'n',      'm',      ',',      '.',      '/',CSKEY_SHIFT,CSKEY_PADMULT,// 30..37
  CSKEY_ALT,' ',      0,        CSKEY_F1, CSKEY_F2, CSKEY_F3, CSKEY_F4, CSKEY_F5,// 38..3F
  CSKEY_F6,  CSKEY_F7, CSKEY_F8, CSKEY_F9, CSKEY_F10,0,       0,        CSKEY_HOME,// 40..47
  CSKEY_UP,  CSKEY_PGUP,CSKEY_PADMINUS,CSKEY_LEFT,CSKEY_CENTER,CSKEY_RIGHT,CSKEY_PADPLUS,CSKEY_END,// 48..4F
  CSKEY_DOWN,CSKEY_PGDN,CSKEY_INS,CSKEY_DEL,0,      0,        0,        CSKEY_F11,// 50..57
  CSKEY_F12,0,        0,        0,        0,        0,        0,        0,      // 58..5F
  0,        0,        0,        0,        0,        0,        0,        0,      // 60..67
  0,        0,        0,        0,        0,        0,        0,        CSKEY_PADDIV,// 68..6F
  0,        0,        0,        0,        0,        0,        0,        0,      // 70..77
  0,        0,        0,        0,        0,        0,        0,        0       // 78..7F
};

//================================================================= System ====

SCF_IMPLEMENT_IBASE_EXT (SysSystemDriver)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (SysSystemDriver::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SysSystemDriver::SysSystemDriver (iObjectRegistry* object_reg)
	: csSystemDriver (object_reg)
{
  // Sanity check
  if (sizeof (event_queue [0]) != 12)
  {
    printf ("ERROR! Your compiler does not handle packed structures!\n");
    printf ("sizeof (event_queue [0]) == %d instead of 12!\n",
      sizeof (event_queue [0]));
    exit (-1);
  }

  __dpmi_regs regs;
  regs.x.ax = 0x0000;
  __dpmi_int (0x33, &regs);
  MouseExists = !!(regs.x.ax);
  MouseOpened = false;
  EnablePrintf = true;

  DosHelper* doshelper = new DosHelper (this);
  if (!object_reg->Register (doshelper, "iDosHelper"))
  {
    printf ("Could not register iDosHelper!\n");
    exit (0);
  }

  EventOutlet = NULL;
}

SysSystemDriver::~SysSystemDriver ()
{
  if (EventOutlet)
    EventOutlet->DecRef ();
  iEventQueue* event_queue = CS_QUERY_REGISTRY (object_reg, iEventQueue);
  if (event_queue)
  {
    event_queue->RemoveListener (&scfiEventHandler);
    event_queue->DecRef ();
  }
}

bool SysSystemDriver::HandleEvent (iEvent& e)
{
  if (e.Type != csevBroadcast) return false;

  if (e.Command.Code == cscmdPreProcess)
  {
    if (!EventOutlet)
    {
      iEventQueue* q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
      if (q != NULL)
      {
        EventOutlet = q->CreateEventOutlet (this);
	q->DecRef ();
      }
    }

    bool ExtKey = false;
    // Fill in events ...
    while (event_queue_tail != event_queue_head)
    {
      switch (event_queue [event_queue_tail].Type)
      {
        case 1:
        {
          int ScanCode = event_queue [event_queue_tail].Keyboard.ScanCode;
          bool Down = (ScanCode < 0x80);

          if ((ScanCode == 0xe0) || (ScanCode == 0xe1))
            ExtKey = true;
          else
          {
            // handle keypad '/'
            if (ExtKey && (ScanCode == 0x35))
              ScanCode = 0x6f;

            ScanCode = ScanCodeToChar [ScanCode & 0x7F];
            if (ScanCode)
              EventOutlet->Key (ScanCode, -1, Down);
          }
          break;
        }
        case 2:
        {
          int Button = event_queue [event_queue_tail].Mouse.Button;
          bool Down = event_queue [event_queue_tail].Mouse.Down;
          int x = event_queue [event_queue_tail].Mouse.x;
          int y = event_queue [event_queue_tail].Mouse.y;

          EventOutlet->Mouse (Button, Down, x, y);
          break;
        }
      } /* endswitch */
      event_queue_tail = (event_queue_tail + 1) & EVENT_QUEUE_MASK;
    } /* endwhile */
    return true;
  }
  else if (e.Command.Code == cscmdSystemOpen)
  {
    // Initialize keyboard handler
    if (KH.install ())
      return false;
    // Give us exclusive access of the keyboard.
    KH.chain (0);
    KeyboardOpened = true;

    csConfigAccess cfg;
    cfg.AddConfig(GetObjectRegistry(), "/config/mouse.cfg");
    SensivityFactor = cfg->GetFloat ("MouseDriver.MouseSensivity", 1.0);

    if (MH.install ())
      return false;

    // Query screen size
    int FrameWidth, FrameHeight, Depth;
    bool FullScreen;
    //@@@ THIS IS BROKEN: Should get information from canvas!
    //@@@GetSettings (FrameWidth, FrameHeight, Depth, FullScreen);
    FrameWidth = 640;
    FrameHeight = 480;
    //@@@

    // Query mouse sensivity
    __dpmi_regs regs;
    regs.x.ax = 0x1B;
    __dpmi_int (0x33, &regs);
    mouse_sensivity_x = regs.x.bx;
    mouse_sensivity_y = regs.x.cx;
    mouse_sensivity_threshold = regs.x.dx;

    // Compute mouse sensivity
    regs.x.cx = 8;
    regs.x.dx = 8;
    regs.x.ax = 0x0F;
    __dpmi_int (0x33, &regs);
    regs.x.bx = (int) ((50 + FrameWidth / 32) * SensivityFactor);
    regs.x.cx = (int) ((50 + FrameHeight / 24) * SensivityFactor);
    regs.x.dx = MAX (regs.x.bx, regs.x.cx); // ? not sure about this
    regs.x.ax = 0x1A;
    __dpmi_int (0x33, &regs);

    // Set mouse range to full-screen
    regs.x.ax = 0x07;
    regs.x.cx = 0;
    regs.x.dx = FrameWidth - 1;
    __dpmi_int (0x33, &regs);
    regs.x.ax = 0x08;
    regs.x.cx = 0;
    regs.x.dx = FrameHeight - 1;
    __dpmi_int (0x33, &regs);

    // Set mouse to (0, 0)
    regs.x.ax = 0x04;
    regs.x.cx = 0;
    regs.x.dx = 0;
    __dpmi_int (0x33, &regs);

    MouseOpened = true;

    return true;
  }
  else if (e.Command.Code == cscmdSystemClose)
  {
    if (KeyboardOpened)
    {
      KH.uninstall ();
      KeyboardOpened = false;
    }

    if (MouseOpened)
    {
      MH.uninstall ();

      // Restore mouse sensivity
      __dpmi_regs regs;
      regs.x.cx = 8;
      regs.x.dx = 16;
      regs.x.ax = 0x0F;
      __dpmi_int (0x33, &regs);
      regs.x.ax = 0x1A;
      regs.x.bx = mouse_sensivity_x;
      regs.x.cx = mouse_sensivity_y;
      regs.x.dx = mouse_sensivity_threshold;
      __dpmi_int (0x33, &regs);

      MouseOpened = false;
    }
    return true;
  }
  return false;
}

bool SysSystemDriver::Initialize ()
{
  if (!csSystemDriver::Initialize ()) return false;

  iEventQueue* event_queue = CS_QUERY_REGISTRY (object_reg, iEventQueue);
  CS_ASSERT (event_queue != NULL);
  event_queue->RegisterListener (&scfiEventHandler,
  	CSMASK_Nothing | CSMASK_Broadcast);
  event_queue->DecRef ();

  return true;
}

void SysSystemDriver::SetMousePosition (int x, int y)
{
  if (MouseOpened)
  {
    __dpmi_regs regs;
    regs.x.cx = x;
    regs.x.dx = y;
    regs.x.ax = 0x04;
    __dpmi_int (0x33, &regs);
    }
  }
}

void SysSystemDriver::DoEnablePrintf (bool en)
{
  EnablePrintf = en;
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (DosHelper)
  SCF_IMPLEMENTS_INTERFACE (iDosHelper)
SCF_IMPLEMENT_IBASE_END

DosHelper::DosHelper (SysSystemDriver* sys)
{
  SCF_CONSTRUCT_IBASE (NULL);
  DosHelper::sys = sys;
}

DosHelper::~DosHelper ()
{
}

void DosHelper::SetMousePosition (int x, int y)
{
  sys->SetMousePosition (x, y);
}

void DosHelper::DoEnablePrintf (bool en)
{
  sys->DoEnablePrintf (en);
}
