/*
    Copyright (C) 1998,1999 by Jorrit Tyberghein
    Written by Xavier Planet.
    Overhauled and re-engineered by Eric Sunshine <sunshine@sunshineco.com>
  
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
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/param.h>
#include <Application.h>
#include <Beep.h>

#include "sysdef.h"
#include "cssys/system.h"
#include "csbe.h"
#include "csutil/inifile.h"

#define CSBE_MOUSE_BUTTON_COUNT 3

// The System driver ////////////////

IMPLEMENT_IBASE (SysSystemDriver)
  IMPLEMENTS_INTERFACE (iSystem)
  IMPLEMENTS_INTERFACE (iBeLibSystemDriver)
IMPLEMENT_IBASE_END

class CrystApp : public BApplication
{
public:
  CrystApp(iSystem*);
  ~CrystApp();
  bool QuitRequested();
  void MessageReceived(BMessage*);
  void HideMouse();
  void ShowMouse();
  bool SetMouse(csMouseCursorID shape);
  void checkMouseMoved();

private:
  iSystem* driver;
  bool shift_down;
  bool alt_down;
  bool ctrl_down;
  bool button_state[CSBE_MOUSE_BUTTON_COUNT];
  bool real_mouse;
  bool mouse_moved;
  BPoint mouse_point;

  void doMouseMotion(BMessage*);
  void doMouseAction(BMessage*);
  void doKeyAction(BMessage*);
  void queueMouseEvent(int button, bool down, BPoint);
  void checkButton(int button, int32 buttons, int32 mask, BPoint);
  void checkButtons(BMessage*);
  void checkModifiers(BMessage*);
  void checkModifier(long flags, long mask, int tag, bool& state) const;
  int classifyFunctionKey(BMessage*) const;
  int classifyAsciiKey(int) const;
};

CrystApp::CrystApp(iSystem* isys) :
  BApplication("application/x-vnd.xsware-crystal"),
  driver (isys), shift_down(false), alt_down(false), ctrl_down(false),
  real_mouse(true), mouse_moved(false), mouse_point(0,0)
{
  driver->IncRef();
  for (int i = CSBE_MOUSE_BUTTON_COUNT; i-- > 0; )
    button_state[i] = false;
};

CrystApp::~CrystApp()
{
  driver->DecRef ();
}

bool CrystApp::QuitRequested()
{
  printf("Terminating rendering loop.\n");
  driver->StartShutdown();
  snooze(200000); // @@@FIXME: Necessary?
  status_t err_code, exit_value;
  err_code = wait_for_thread(find_thread("LoopThread"), &exit_value);
  printf("LoopThread termination code: %lx\n", err_code);
  return true;
}

void CrystApp::HideMouse()
{
  if (!IsCursorHidden())
    HideCursor();
}

void CrystApp::ShowMouse()
{
  if (IsCursorHidden())
    ShowCursor();
}

bool CrystApp::SetMouse(csMouseCursorID shape)
{
  real_mouse = false;
  if (shape == csmcArrow)
  {
    SetCursor(B_HAND_CURSOR);
    real_mouse = true;
  }

  if (real_mouse)
    ShowMouse();
  else
    HideMouse();

  return real_mouse;
}

void CrystApp::MessageReceived(BMessage* m)
{	
  switch(m->what)
  {
    case B_KEY_DOWN:
    case B_KEY_UP:
      doKeyAction(m);
      break;
    case B_MOUSE_DOWN:
    case B_MOUSE_UP:
      doMouseAction(m);
      break;
    case B_MOUSE_MOVED:
      doMouseMotion(m);
      break;
    default:
      BApplication::MessageReceived(m);
      break;
  }
}

void CrystApp::queueMouseEvent(int button, bool down, BPoint where)
{
  if (where.x >= 0 && where.y >= 0)
  {
    driver->QueueMouseEvent(button, down, where.x, where.y,
      (shift_down ? CSMASK_SHIFT : 0) |
      (alt_down   ? CSMASK_ALT   : 0) |
      (ctrl_down  ? CSMASK_CTRL  : 0));
  }
}

void CrystApp::checkMouseMoved()
{
  if (mouse_moved)
  {
    queueMouseEvent(0, false, mouse_point);
    mouse_moved = false;
  }
}

void CrystApp::checkButton(int button, int32 buttons, int32 mask, BPoint where)
{
  if (button < CSBE_MOUSE_BUTTON_COUNT)
  {
    bool const state = (buttons & mask) != 0;
    if (state != button_state[button])
    {
      button_state[button] = state;
      checkMouseMoved();
      queueMouseEvent(button + 1, state, where);
    }
  }
}

void CrystApp::checkButtons(BMessage* m)
{
  BPoint where;
  int32 buttons;
  if (m->FindPoint("where",   &where  ) == B_OK &&
      m->FindInt32("buttons", &buttons) == B_OK)
  {
    checkButton(0, buttons, B_PRIMARY_MOUSE_BUTTON,   where);
    checkButton(1, buttons, B_SECONDARY_MOUSE_BUTTON, where);
    checkButton(2, buttons, B_TERTIARY_MOUSE_BUTTON,  where);
  }
}

void CrystApp::checkModifier(long flags, long mask, int tag, bool& state) const
{
  bool const new_state = (flags & mask) != 0;
  if (state != new_state)
  {
    state = new_state;
    driver->QueueKeyEvent(tag, state);
  }
}

void CrystApp::checkModifiers(BMessage* m)
{
  int32 flags;
  if (m->FindInt32("modifiers", &flags) == B_OK)
  {
    checkModifier(flags, B_SHIFT_KEY,   CSKEY_SHIFT, shift_down);
    checkModifier(flags, B_OPTION_KEY,  CSKEY_ALT,   alt_down  );
    checkModifier(flags, B_CONTROL_KEY, CSKEY_CTRL,  ctrl_down );
  }
}

void CrystApp::doMouseAction(BMessage* m)
{
  checkModifiers(m);
  checkButtons(m);
}

void CrystApp::doMouseMotion(BMessage* m)
{
  checkModifiers(m);
  int32 transit;
  if (m->FindInt32("be:transit", &transit) == B_OK)
  {
    if (transit == B_EXITED_VIEW)
      ShowMouse();
    else if (transit == B_ENTERED_VIEW)
    {
      if (!real_mouse)
        HideMouse();
    }
    else
    {
      BPoint where;
      if (m->FindPoint("where", &where) == B_OK && where != mouse_point)
      {
        mouse_moved = true;
        mouse_point = where;
      }
    }
  }
}

void CrystApp::doKeyAction(BMessage* m)
{
  checkModifiers(m);

  char const* bytes;
  if (m->FindString("bytes", &bytes) == B_OK && strlen(bytes) == 1)
  {
    int const c = bytes[0];
    int key = 0;
    if (c == B_FUNCTION_KEY)
      key = classifyFunctionKey(m);
    else
      key = classifyAsciiKey(c);
    driver->QueueKeyEvent(key, m->what == B_KEY_DOWN);
  }
}

int CrystApp::classifyFunctionKey(BMessage* m) const
{
  int cs_key = 0;
  int32 be_key;
  if (m->FindInt32("key", &be_key) == B_OK)
  {
    switch (be_key)
    {
      case B_F1_KEY:	cs_key = CSKEY_F1;	break;
      case B_F2_KEY:	cs_key = CSKEY_F2;	break;
      case B_F3_KEY:	cs_key = CSKEY_F3;	break;
      case B_F4_KEY:	cs_key = CSKEY_F4;	break;
      case B_F5_KEY:	cs_key = CSKEY_F5;	break;
      case B_F6_KEY:	cs_key = CSKEY_F6;	break;
      case B_F7_KEY:	cs_key = CSKEY_F7;	break;
      case B_F8_KEY:	cs_key = CSKEY_F8;	break;
      case B_F9_KEY:	cs_key = CSKEY_F9;	break;
      case B_F10_KEY:	cs_key = CSKEY_F10;	break;
      case B_F11_KEY:	cs_key = CSKEY_F11;	break;
      case B_F12_KEY:	cs_key = CSKEY_F12;	break;
    }
  }
  return cs_key;
}

// *NOTE* CrystalSpace wants control-keys translated to their lower-case
// equivalents; that is: 'ctrl-c' --> 'c'.
int CrystApp::classifyAsciiKey(int c) const
{
  int cs_key = 0;
  switch (c)
  {
    case B_UP_ARROW:	cs_key = CSKEY_UP;			break;
    case B_DOWN_ARROW: 	cs_key = CSKEY_DOWN;			break;
    case B_LEFT_ARROW: 	cs_key = CSKEY_LEFT;			break;
    case B_RIGHT_ARROW:	cs_key = CSKEY_RIGHT;			break;
    case B_BACKSPACE:	cs_key = CSKEY_BACKSPACE;		break;
    case B_INSERT:	cs_key = CSKEY_INS;			break;
    case B_DELETE:	cs_key = CSKEY_DEL;			break;
    case B_PAGE_UP:	cs_key = CSKEY_PGUP;			break;
    case B_PAGE_DOWN:	cs_key = CSKEY_PGDN;			break;
    case B_HOME:	cs_key = CSKEY_HOME;			break;
    case B_END:		cs_key = CSKEY_END;			break;
    case B_ESCAPE:	cs_key = CSKEY_ESC;			break;
    case B_TAB:		cs_key = CSKEY_TAB;			break;
    case B_RETURN:	cs_key = CSKEY_ENTER;			break;
    default:		cs_key = (c < ' ' ? c + '`' : c);	break; // *NOTE*
  }
  return cs_key;
}

SysSystemDriver::SysSystemDriver () : csSystemDriver(), running(false)
{
  CONSTRUCT_IBASE (NULL);
  CHK (app = new CrystApp (this));
};

static int32 begin_loop(void* data)
{
  snooze(100000); // @@@FIXME: Necesary?
  int32 const rc = ((SysSystemDriver*)data)->LoopThread();
  be_app->PostMessage(B_QUIT_REQUESTED);
  return rc;
}

// Note that Loop() must be able to handle recursive calls properly.
// For instance, CSWS calls it recursively to implement modal loops.
// The first time Loop() is called, it is called from the "main" thread.
// All subsequent (recursive) calls come from "LoopThread", the thread
// which is actually driving the Crystal Space run-loop.
void SysSystemDriver::Loop()
{
  if (running)
    LoopThread();
  else
  {
    running = true;
    thread_id my_thread = spawn_thread(begin_loop, "LoopThread",
      B_DISPLAY_PRIORITY, (void*)this);
    resume_thread(my_thread);
    app->Run();
    app->ShowMouse();
  }
}

// This is the thread doing the loop itself.
long SysSystemDriver::LoopThread()
{
  bigtime_t prev_time = Time();	
  while (!Shutdown && !ExitLoop)
  {
    app->checkMouseMoved(); // @@@FIXME: Probably want to lock "moved" flag.
    bigtime_t curr_time = Time();
    NextFrame(curr_time - prev_time, curr_time);
    prev_time=curr_time;
  }	
  return 0;
}

void SysSystemDriver::ProcessUserEvent (BMessage* m)
{
  if (running)
  {
    switch (m->what)
    {
      case B_KEY_DOWN:
      case B_KEY_UP:
      case B_MOUSE_DOWN:
      case B_MOUSE_MOVED:
      case B_MOUSE_UP:
        app->PostMessage(m, 0);
        break;
    }
  }
}

bool SysSystemDriver::SetMouseCursor(csMouseCursorID shape, iTextureHandle*)
{
  return app->SetMouse(shape);
}
