/*
    DOS support for Crystal Space 3D library
    Copyright (C) 1998 by Jorrit Tyberghein
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

/*
 * Written by David N. Arnold. <derek_arnold@fuse.net>
 * 13-07-98:  Andrew Zabolotny <bit@eltech.ru>
 *   - Rewritten video driver; no changes to Allegro support though
 *   - Rewritten keyboard handler
 *   - Rewritten mouse handler
 *   - Did I forgot to rewrite something? :-)
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

#include "sysdef.h"
#include "djgpp.h"
#include "csutil/inifile.h"
#include "isystem.h"
#include "inputq.h"
#include "djkeysys.h"
#include "djmousys.h"

static KeyboardHandler KH;
static MouseHandler MH;

static unsigned short ScanCodeToChar[128] =
{
  0,        CSKEY_ESC,'1',      '2',      '3',      '4',      '5',      '6',    // 00..07
  '7',      '8',      '9',      '0',      '-',      '=',      '\b',     '\t',   // 08..0F
  'q',      'w',      'e',      'r',      't',      'y',      'u',      'i',    // 10..17
  'o',      'p',      '[',      ']',      '\n',     CSKEY_CTRL,'a',     's',    // 18..1F
  'd',      'f',      'g',      'h',      'j',      'k',      'l',      ';',    // 20..27
  39,       '`',      CSKEY_SHIFT,'\\',   'z',      'x',      'c',      'v',    // 28..2F
  'b',      'n',      'm',      ',',      '.',      '/',      CSKEY_SHIFT,'*',  // 30..37
  CSKEY_ALT,' ',      0,        CSKEY_F1, CSKEY_F2, CSKEY_F3, CSKEY_F4, CSKEY_F5,// 38..3F
  CSKEY_F6,  CSKEY_F7, CSKEY_F8, CSKEY_F9, CSKEY_F10,0,       0,        CSKEY_HOME,// 40..47
  CSKEY_UP,  CSKEY_PGUP,'-',    CSKEY_LEFT,CSKEY_CENTER,CSKEY_RIGHT,'+',CSKEY_END,// 48..4F
  CSKEY_DOWN,CSKEY_PGDN,CSKEY_INS,CSKEY_DEL,0,      0,        0,        CSKEY_F11,// 50..57
  CSKEY_F12,0,        0,        0,        0,        0,        0,        0,      // 58..5F
  0,        0,        0,        0,        0,        0,        0,        0,      // 60..67
  0,        0,        0,        0,        0,        0,        0,        0,      // 68..6F
  0,        0,        0,        0,        0,        0,        0,        0,      // 70..77
  0,        0,        0,        0,        0,        0,        0,        0       // 78..7F
};

//================================================================== System ====

BEGIN_INTERFACE_TABLE (SysSystemDriver)
  IMPLEMENTS_COMPOSITE_INTERFACE (System)
  IMPLEMENTS_COMPOSITE_INTERFACE (DosSystemDriver)
END_INTERFACE_TABLE ()

IMPLEMENT_UNKNOWN_NODELETE (SysSystemDriver)

SysSystemDriver::SysSystemDriver () : csSystemDriver ()
{
  // Sanity check
  if (sizeof (event_queue [0]) != 12)
  {
    Printf (MSG_FATAL_ERROR, "ERROR! Your compiler does not handle packed structures!\n");
    Printf (MSG_FATAL_ERROR, "sizeof (event_queue [0]) == %d instead of 12!\n", sizeof (event_queue [0]));
    exit (-1);
  }
}

void SysSystemDriver::Loop ()
{
  while (!Shutdown && !ExitLoop)
  {
    static long prev_time = -1;
    long new_prev_time = Time ();
    NextFrame ((prev_time == -1) ? 0 : new_prev_time - prev_time, Time ());
    prev_time = new_prev_time;

    // Fill in events ...
    while (event_queue_tail != event_queue_head)
    {
      unsigned long time = Time ();
      switch (event_queue [event_queue_tail].Type)
      {
        case 1:
        {
          int ScanCode = event_queue [event_queue_tail].Keyboard.ScanCode;
          bool Down = (ScanCode < 0x80);

          ScanCode = ScanCodeToChar [ScanCode & 0x7F];
          if (ScanCode)
            if (Down)
              Keyboard->do_keypress (time, ScanCode);
            else
              Keyboard->do_keyrelease (time, ScanCode);
          break;
        }
        case 2:
        {
          int Button = event_queue [event_queue_tail].Mouse.Button;
          bool Down = event_queue [event_queue_tail].Mouse.Down;
          int x = event_queue [event_queue_tail].Mouse.x;
          int y = event_queue [event_queue_tail].Mouse.y;

          if (Button == 0)
            Mouse->do_mousemotion (time, x, y);
          else if (Down)
            Mouse->do_buttonpress (time, Button, x, y, Keyboard->Key.shift,
              Keyboard->Key.alt, Keyboard->Key.ctrl);
          else
            Mouse->do_buttonrelease (time, Button, x, y);
          break;
        }
      } /* endswitch */
      event_queue_tail = (event_queue_tail + 1) & EVENT_QUEUE_MASK;
    } /* endwhile */
  } // while (!Shutdown && !ExitLoop)
}

//== class XDosSystemDriver ====================================================

IMPLEMENT_COMPOSITE_UNKNOWN_AS_EMBEDDED (SysSystemDriver, DosSystemDriver)

STDMETHODIMP SysSystemDriver::XDosSystemDriver::EnablePrintf (bool Enable)
{
  extern void printf_Enable (bool Enable);
  printf_Enable (Enable);
  return S_OK;
}

STDMETHODIMP SysSystemDriver::XDosSystemDriver::SetMousePosition (int x, int y)
{
  METHOD_PROLOGUE (SysSystemDriver, DosSystemDriver);
  return ((SysMouseDriver *)pThis->Mouse)->SetMousePosition (x, y) ? S_OK : E_FAIL;
}

//================================================================ Keyboard ====

SysKeyboardDriver::SysKeyboardDriver() : csKeyboardDriver ()
{
  KeyboardOpened = false;
}

SysKeyboardDriver::~SysKeyboardDriver(void)
{
  Close();
}

bool SysKeyboardDriver::Open(csEventQueue* EvQueue)
{
  csKeyboardDriver::Open (EvQueue);

  // Initialize keyboard handler
  if (KH.install ())
    return false;
  // Give us exclusive access of the keyboard.
  KH.chain (0);

  KeyboardOpened = true;
  return true;
}

void SysKeyboardDriver::Close(void)
{
  if (KeyboardOpened)
  {
    KH.uninstall ();
    KeyboardOpened = false;
  }
}

//=================================================================== Mouse ====

SysMouseDriver::SysMouseDriver () : csMouseDriver ()
{
  __dpmi_regs regs;
  regs.x.ax = 0x0000;
  __dpmi_int (0x33, &regs);
  MouseExists = !!(regs.x.ax);
  MouseOpened = false;

  SensivityFactor = System->Config->GetFloat ("MouseDriver", "MouseSensivity", 1.0);
}

SysMouseDriver::~SysMouseDriver ()
{
  Close ();
}

bool SysMouseDriver::Open (ISystem* System, csEventQueue *EvQueue)
{
  if (!csMouseDriver::Open (System, EvQueue))
    return false;
  if (MH.install ())
    return false;

  // Query screen size from System object
  int FrameWidth, FrameHeight;
  System->GetWidthSetting (FrameWidth);
  System->GetHeightSetting (FrameHeight);

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

void SysMouseDriver::Close ()
{
  if (!MouseOpened)
    return;

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

bool SysMouseDriver::SetMousePosition (int x, int y)
{
  if (!MouseOpened)
    return false;

  __dpmi_regs regs;
  regs.x.cx = x;
  regs.x.dx = y;
  regs.x.ax = 0x04;
  __dpmi_int (0x33, &regs);
  return true;
}
