/*
    Copyright (C) 1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
  
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
#include <unistd.h>
#include <Application.h>
#include <Beep.h>
#include <UTF8.h>
#include <View.h>

#include "cssysdef.h"
#include "cssys/system.h"
#include "cssys/be/csbe.h"
#include "csutil/util.h"
#include "iutil/event.h"
#include "iutil/eventq.h"

SCF_IMPLEMENT_IBASE_EXT (SysSystemDriver)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (SysSystemDriver::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

//-----------------------------------------------------------------------------
// Locally defined BMessage types passed from BeOS thread to Crystal Space
// thread via the system driver's thread-safe message queue.
//-----------------------------------------------------------------------------
enum
{
  CS_BE_QUIT = 'CSEX',
  CS_BE_CONTEXT_CLOSE = 'CSCL'
};


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
SysSystemDriver::SysSystemDriver(iObjectRegistry* object_reg) :
  csSystemDriver(object_reg), running(false), event_outlet(0),
  shift_down(false), alt_down(false), ctrl_down(false),
  real_mouse(true), mouse_moved(false), mouse_point(0,0)
{
  int i;
  for (i = CSBE_MOUSE_BUTTON_COUNT; i-- > 0; )
    button_state[i] = false;
  BeHelper* behelper = new BeHelper (this);
  if (!object_reg->Register (behelper, "iBeHelper"))
  {
    printf ("Could not register the BeHelper!\n");
    exit (0);
  }
}


//-----------------------------------------------------------------------------
// Destructor
//
// *DELETE*
//	Unfortunately, we can not delete 'be_app' here even though we probably
//	created it.  The problem is that the architecture of the system is
//	such that plugin modules get cleaned up by ~csSystemDriver() only
//	_after_ this destructor is invoked.  When the BeOS 2D drivers clean up
//	their windows and bitmaps, be_app must exist.  (This is a requirement
//	of the InterfaceKit and ApplicationKit.)
//-----------------------------------------------------------------------------
SysSystemDriver::~SysSystemDriver()
{
  if (be_app != 0)
  {
    if (be_app->Lock())
    {
      ShowMouse();
      if (running)
      {
        be_app->Quit();
        status_t err_code, exit_value;
        err_code = wait_for_thread(find_thread("BeThread"), &exit_value);
#if defined(CS_DEBUG)
	printf("BeThread termination code: %lx, exit value: %lx \n",
	  err_code, exit_value);
#endif
      }
      be_app->SetPreferredHandler(0);
      be_app->RemoveHandler(this);
      be_app->Unlock();
    }
    // delete app;	// *DELETE*
  }
  if (event_outlet != 0)
    event_outlet->DecRef();

  iEventQueue* q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
  CS_ASSERT (q != NULL);
  q->RemoveListener (&scfiEventHandler);
  q->DecRef ();
}


//-----------------------------------------------------------------------------
// Initialize the system driver.
//
// *1*
//	Determine the path of the executable and set it as the "current
//	working directory".  This allows Crystal Space applications to easily
//	locate the common resource files, such as scf.cfg and vfs.cfg which,
//	it expects to find in the current directory, even when launched via
//	double-clicking the application from the Tracker.
// *2*
//	Only instantiate a BApplication object if one has not already been
//	instantiated.  This allows a BeOS-specific Crystal Space client to
//	easily employ a custom BApplication subclass by instantiating it prior
//	to calling system::Initialize().
// *3*
//	Sets the system driver as the application's preferred message handler
//	so that it can intercept common BeOS messages and forward appropriate
//	ones to the Crystal Space event-loop which runs in the main thread.
//-----------------------------------------------------------------------------
bool SysSystemDriver::Initialize ()
{
  char path[MAXPATHLEN];		// *1*
  char name[MAXPATHLEN];
  csSplitPath(argv[0], path, sizeof(path), name, sizeof(name));
  if (strlen(path) > 0)
    chdir(path);

  iEventQueue* q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
  CS_ASSERT (q != NULL);
  event_outlet = q->CreateEventOutlet(this);
  q->RegisterListener (&scfiEventHandler, CSMASK_Nothing);
  q->DecRef ();

  if (be_app == 0)			// *2*
    (void)new BApplication("application/x-vnd.xsware-crystal");
  be_app->AddHandler(this);		// *3*
  be_app->SetPreferredHandler(this);

  return superclass::Initialize ();
}


//-----------------------------------------------------------------------------
// Entry-point function for the subthread in which the BApplication runs.
//-----------------------------------------------------------------------------
int32 SysSystemDriver::ThreadEntry(void* p)
{
  SysSystemDriver* sys = (SysSystemDriver*)p;
  be_app->Lock();		 // Thread invoking Run() must hold lock.
  be_app->Run();
  sys->QueueMessage (CS_BE_QUIT);// BApplication terminated, so ask CS to quit.
  return 0;
}


//-----------------------------------------------------------------------------
// Whenever a 2D driver (canvas) is about to place a window on-screen, it asks
// the system driver -- via PerformExtension("BeginUI") -- to ensure that the
// BeOS event loop is active so that it can respond to events in window.  This
// method runs the BApplication in a subthread the first time "BeginUI" is
// requested.
//-----------------------------------------------------------------------------
bool SysSystemDriver::RunBeApp()
{
  if (!running)
  {
    running = true;
    set_thread_priority(find_thread(0), B_DISPLAY_PRIORITY);
    be_app->Unlock(); // Allow sub-thread to lock BAapplication before Run().
    thread_id const ident = spawn_thread (ThreadEntry, "BeThread",
      B_NORMAL_PRIORITY, this);
    resume_thread(ident);
  }
  return true;
}


bool SysSystemDriver::HandleEvent (iEvent& e)
{
  if (e.Type == csevBroadcast && e.Command.Code == cscmdPreProcess)
  {
    if (message_queue.Lock())
    {
      BMessage* m;
      while ((m = message_queue.NextMessage()) != 0)
      {
        DispatchMessage(m);
        delete m;
      }
      message_queue.Unlock();
    }
    CheckMouseMoved();
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
// iEventPlug Implementation.
//-----------------------------------------------------------------------------
unsigned int SysSystemDriver::GetPotentiallyConflictingEvents()
  { return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }

unsigned int SysSystemDriver::QueryEventPriority(unsigned int)
  { return 150; }


//-----------------------------------------------------------------------------
// Hide the mouse.
//-----------------------------------------------------------------------------
void SysSystemDriver::HideMouse()
{
  CS_ASSERT(be_app != 0);
  if (!be_app->IsCursorHidden())
    be_app->HideCursor();
}


//-----------------------------------------------------------------------------
// Show the mouse.
//-----------------------------------------------------------------------------
void SysSystemDriver::ShowMouse()
{
  CS_ASSERT(be_app != 0);
  if (be_app->IsCursorHidden())
    be_app->ShowCursor();
}


//-----------------------------------------------------------------------------
// Set the mouse pointer to one of the pre-defined Crystal Space shapes.
// Returns 'true' if the pointer shape is directly supported; else 'false' if
// Crystal Space must simulate the mouse pointer for this shape.
//-----------------------------------------------------------------------------
bool SysSystemDriver::SetMouse(csMouseCursorID shape)
{
  real_mouse = false;
  if (shape == csmcArrow)
  {
    CS_ASSERT(be_app != 0);
    be_app->SetCursor(B_HAND_CURSOR);
    real_mouse = true;
  }

  if (real_mouse)
    ShowMouse();
  else
    HideMouse();

  return real_mouse;
}


//-----------------------------------------------------------------------------
// User actions, such as keyboard and mouse events, are received from the
// subthread running the BApplication.  Queue them up for processing by
// csSystemDriver::Loop() which is running in the main thread.
//-----------------------------------------------------------------------------
bool SysSystemDriver::QueueMessage(BMessage* m)
{
  bool const ok = message_queue.Lock();
  if (ok)
  {
    message_queue.AddMessage(new BMessage(*m));
    message_queue.Unlock();
  }
  return ok;
}


//-----------------------------------------------------------------------------
// Queue up a custom, port-specific message for processing by the main thread.
//-----------------------------------------------------------------------------
bool SysSystemDriver::QueueMessage(uint32 code, void const* data)
{
  BMessage m(code);
  if (data != 0)
    m.AddPointer("data", data);
  return QueueMessage(&m);
}


//-----------------------------------------------------------------------------
// This is an override of BHandler::MessageRecieved().  Since this object is
// the BApplication's preferred message handler, it is called by the
// BApplication object running in the subthread.
//-----------------------------------------------------------------------------
void SysSystemDriver::MessageReceived(BMessage* m)
{	
  switch(m->what)
  {
    case B_KEY_DOWN:
    case B_KEY_UP:
    case B_MOUSE_DOWN:
    case B_MOUSE_UP:
    case B_MOUSE_MOVED:
      QueueMessage(m);
      break;
    default:
      BHandler::MessageReceived(m);
      break;
  }
}


//-----------------------------------------------------------------------------
// Dispatch a message which was received from the subthread and queued for
// processing in the main thread.
//-----------------------------------------------------------------------------
void SysSystemDriver::DispatchMessage(BMessage* m)
{	
  switch(m->what)
  {
    case B_KEY_DOWN:
    case B_KEY_UP:
      DoKeyAction(m);
      break;
    case B_MOUSE_DOWN:
    case B_MOUSE_UP:
      DoMouseAction(m);
      break;
    case B_MOUSE_MOVED:
      DoMouseMotion(m);
      break;
    case CS_BE_QUIT:
      event_outlet->Broadcast(cscmdQuit);
      break;
    case CS_BE_CONTEXT_CLOSE:
      DoContextClose(m);
      break;
    default:
      printf(
        "DispatchUserAction(): Unrecognized message (%lu)\n", m->what);
      m->PrintToStream();
      break;
  }
}


//-----------------------------------------------------------------------------
// Broadcast a context-close message via the Crystal Space event loop.
//-----------------------------------------------------------------------------
void SysSystemDriver::DoContextClose(BMessage* m)
{
  void* p = 0;
  if (m->FindPointer("data", &p) == B_OK)
  {
    CS_ASSERT(p != 0);
    event_outlet->Broadcast(cscmdContextClose, (iGraphics2D*)p);
  }
}


//-----------------------------------------------------------------------------
// Queue a mouse-related event to the Crystal Space event loop.
//-----------------------------------------------------------------------------
void SysSystemDriver::QueueMouseEvent(int button, bool down, BPoint where)
{
  if (where.x >= 0 && where.y >= 0)
    event_outlet->Mouse(button, down, where.x, where.y);
}


//-----------------------------------------------------------------------------
// If the mouse moved, queue a mouse-moved event to the Crystal Space event
// loop.
//-----------------------------------------------------------------------------
void SysSystemDriver::CheckMouseMoved()
{
  if (mouse_moved)
  {
    QueueMouseEvent(0, false, mouse_point);
    mouse_moved = false;
  }
}


//-----------------------------------------------------------------------------
// If a mouse button's state changed, queue a mouse event to the Crystal Space
// event-loop.
//-----------------------------------------------------------------------------
void SysSystemDriver::CheckButton(
  int button, int32 buttons, int32 mask, BPoint where)
{
  if (button < CSBE_MOUSE_BUTTON_COUNT)
  {
    bool const state = (buttons & mask) != 0;
    if (state != button_state[button])
    {
      button_state[button] = state;
      CheckMouseMoved();
      QueueMouseEvent(button + 1, state, where);
    }
  }
}


//-----------------------------------------------------------------------------
// Check for mouse button state changes.
//-----------------------------------------------------------------------------
void SysSystemDriver::CheckButtons(BMessage* m)
{
  BPoint where;
  int32 buttons;
  if (m->FindPoint("where",   &where  ) == B_OK &&
      m->FindInt32("buttons", &buttons) == B_OK)
  {
    CheckButton(0, buttons, B_PRIMARY_MOUSE_BUTTON,   where);
    CheckButton(1, buttons, B_SECONDARY_MOUSE_BUTTON, where);
    CheckButton(2, buttons, B_TERTIARY_MOUSE_BUTTON,  where);
  }
}


//-----------------------------------------------------------------------------
// If a modifier key's state changed, queue a keyboard event to the Crystal
// Space event-loop.
//-----------------------------------------------------------------------------
void SysSystemDriver::CheckModifier(
  long flags, long mask, int tag, bool& state) const
{
  bool const new_state = (flags & mask) != 0;
  if (state != new_state)
  {
    state = new_state;
    event_outlet->Key(tag, -1, state);
  }
}


//-----------------------------------------------------------------------------
// Check keyboard modifiers (shift, alternate, control) for state changes.
//-----------------------------------------------------------------------------
void SysSystemDriver::CheckModifiers(BMessage* m)
{
  int32 flags;
  if (m->FindInt32("modifiers", &flags) == B_OK)
  {
    CheckModifier(flags, B_SHIFT_KEY,   CSKEY_SHIFT, shift_down);
    CheckModifier(flags, B_OPTION_KEY,  CSKEY_ALT,   alt_down  );
    CheckModifier(flags, B_CONTROL_KEY, CSKEY_CTRL,  ctrl_down );
  }
}


//-----------------------------------------------------------------------------
// Classify mouse related events.
//-----------------------------------------------------------------------------
void SysSystemDriver::DoMouseAction(BMessage* m)
{
  CheckModifiers(m);
  CheckButtons(m);
}


//-----------------------------------------------------------------------------
// Handle mouse movement events, taking into special account whether or not
// the mouse has left or entered the canvas' window.
//-----------------------------------------------------------------------------
void SysSystemDriver::DoMouseMotion(BMessage* m)
{
  CheckModifiers(m);
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


//-----------------------------------------------------------------------------
// Classify keyboard-related events.
//-----------------------------------------------------------------------------
void SysSystemDriver::DoKeyAction(BMessage* m)
{
  CheckModifiers(m);

  long c;
  if (m->FindInt32("raw_char", &c) == B_OK)
  {
    int cs_key = 0;
    int cs_char = -1;
    if (c == B_FUNCTION_KEY)
      ClassifyFunctionKey(m, cs_key, cs_char);
    else
      ClassifyNormalKey(c, m, cs_key, cs_char);
    event_outlet->Key(cs_key, cs_char, m->what == B_KEY_DOWN);
  }
}


//-----------------------------------------------------------------------------
// BeOS to Crystal Space function key translation.
//-----------------------------------------------------------------------------
void SysSystemDriver::ClassifyFunctionKey(
  BMessage* m, int& cs_key, int& cs_char) const
{
  cs_key = 0;
  cs_char = -1;
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
}


//-----------------------------------------------------------------------------
// BeOS to Crystal Space non-function key translation.
//-----------------------------------------------------------------------------
void SysSystemDriver::ClassifyNormalKey(
  int c, BMessage* m, int& cs_key, int& cs_char) const
{
  cs_char = -1;
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
    default:		cs_key = c;
			cs_char = ClassifyUnicodeKey(m);	break;
  }
}


//-----------------------------------------------------------------------------
// Extract a Unicode value from a keyboard event.  The Crystal Space event
// interface does not actually specify how to handle non-ASCII input
// characters so, for lack of anything better, we return a Unicode value.
//
// *ASCII*
//	First test for the simple ASCII case before doing the hard work of
//	converting a multi-byte UTF-8 value into a Unicode 2.0 value.
// *UNICODE*
//	Unicode values are 16-bits in size.  Since this key event represents
//	a single keystroke, it should translate to a single Unicode value,
//	thus the buffer need be only 16-bits in width (even though the
//	incoming UTF-8 value may be represented by 1, 2, 3, or 4 bytes).
// *CONVERT*
//	Convert the 2-byte Unicode byte string into an endian-correct Unicode
//	16-bit numeric value.
//-----------------------------------------------------------------------------
int SysSystemDriver::ClassifyUnicodeKey(BMessage* m) const
{
  int cs_char = -1;
  void const* data;
  ssize_t bytes;
  if (m->FindData("bytes",B_STRING_TYPE,&data,&bytes) == B_OK && bytes != 0)
  {
    unsigned char const* p = (unsigned char const*)data;
    if ((p[0] & 0x80) == 0)		// *ASCII*
      cs_char = p[0];
    else
    {
      char unicode[2];			// *UNICODE*
      int32 unilen = sizeof(unicode);
      if (convert_from_utf8(B_UNICODE_CONVERSION, (char const*)data, &bytes,
        unicode, &unilen, 0) == B_OK)
      {
        p = (unsigned char const*)unicode;
        cs_char = (p[1] << 8) | p[0];	// *CONVERT*
      }
    }
  }
  return cs_char;
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (BeHelper)
  SCF_IMPLEMENTS_INTERFACE (iBeHelper)
SCF_IMPLEMENT_IBASE_END

BeHelper::BeHelper (SysSystemDriver* sys)
{
  SCF_CONSTRUCT_IBASE (NULL);
  BeHelper::sys = sys;
}

BeHelper::~BeHelper ()
{
}

bool BeHelper::UserAction (BMessage* msg)
{
  return sys->QueueMessage (msg);
}

bool BeHelper::SetCursor (csMouseCursorID id)
{
  return sys->SetMouse (id);
}

bool BeHelper::BeginUI ()
{
  return sys->RunBeApp ();
}

bool BeHelper::Quit ()
{
  return sys->QueueMessage (CS_BE_QUIT);
}

bool BeHelper::ContextClose (iGraphics2D* g2d)
{
  return sys->QueueMessage (CS_BE_CONTEXT_CLOSE, g2d);
}

