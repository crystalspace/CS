/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
    Copyright (C) 2004 by Daniel Fryer and Peter Amstutz

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
#include "csutil/sysfunc.h"
#include "csutil/scf.h"
#include "csutil/csinput.h"
#include "csgeom/csrect.h"
#include "csutil/cfgacc.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "iutil/cfgfile.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "GLWXDriver2D.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE_EXT (csGraphics2DWX)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iOpenGLInterface)
  SCF_IMPLEMENTS_INTERFACE (iWxWindow)
  SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGraphics2DWX::eiOpenGLInterface)
  SCF_IMPLEMENTS_INTERFACE (iOpenGLInterface)
  SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csGraphics2DWX)

// csGraphics2DWX function
  csGraphics2DWX::csGraphics2DWX (iBase *iParent) :
    csGraphics2DGLCommon (iParent), myParent(0)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiOpenGLInterface);
}

void csGraphics2DWX::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.canvas.glwx2d", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}



bool csGraphics2DWX::Initialize (iObjectRegistry *object_reg)
{
  if (!csGraphics2DGLCommon::Initialize (object_reg))
    return false;

  csConfigAccess config(object_reg, "/config/opengl.cfg");

  // The texture manager only needs to know this:
  pfmt.PalEntries = 0;


  // Create the event outlet
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
    eventOutlet = q->CreateEventOutlet (this);

  return true;
}

void csGraphics2DWX::SetParent(wxWindow* wx)
{
  myParent = wx;
}

wxWindow* csGraphics2DWX::GetWindow()
{
  return theCanvas;
}

csGraphics2DWX::~csGraphics2DWX ()
{
  // Destroy your graphic interface
  Close ();
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiOpenGLInterface);
  delete theCanvas;
}

bool csGraphics2DWX::Open()
{
  if (is_open) return true;

  Report (CS_REPORTER_SEVERITY_NOTIFY, "Opening WX-GL canvas!\n");

  if(myParent == 0)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Parent frame in wxGLCanvas not initialized!!");
  }

  /*
    GLPixelFormat format;

    const int colorBits = format[glpfvColorBits];
    const int colorComponentSize =
    ((colorBits % 32) == 0) ? colorBits / 4 : colorBits / 3;
    const int accumBits = format[glpfvAccumColorBits];
    const int accumComponentSize =
    ((accumBits % 32) == 0) ? accumBits / 4 : accumBits / 3;
  */

  int desired_attributes[] =
    {
      WX_GL_RGBA,
      WX_GL_DOUBLEBUFFER,
      None
    };

  /*WX_GL_DEPTH_SIZE, format[glpfvDepthBits],
    WX_GL_MIN_RED, colorComponentSize,
    WX_GL_MIN_BLUE, colorComponentSize,
    WX_GL_MIN_GREEN, colorComponentSize,*/
  /*WX_GL_MIN_ALPHA, format[glpfvAlphaBits],
    WX_GL_STENCIL_SIZE, format[glpfvStencilBits],
    WX_GL_MIN_ACCUM_RED, accumComponentSize,
    WX_GL_MIN_ACCUM_BLUE, accumComponentSize,
    WX_GL_MIN_ACCUM_GREEN, accumComponentSize,
    WX_GL_MIN_ACCUM_ALPHA, format[glpfvAccumAlphaBits],*/

  int w, h;
  myParent->GetClientSize(&w, &h);

  //replace these with real size parameters
  theCanvas = new csGLCanvas(this, myParent, wxID_ANY,
                             wxPoint(0, 0), wxSize(w, h), 0, "", desired_attributes);

  if(theCanvas == 0) Report(CS_REPORTER_SEVERITY_ERROR, "Failed creating GL Canvas!");
  theCanvas->Show(true);

  // Open your graphic interface
  if (!csGraphics2DGLCommon::Open ())
  {
    Report(CS_REPORTER_SEVERITY_ERROR, "Graphics2DGLCommon failed to open");
    return false;
  }
  return true;
}

void csGraphics2DWX::Close(void)
{
  if (!is_open) return;

  // Close your graphic interface
  csGraphics2DGLCommon::Close ();

}

bool csGraphics2DWX::BeginDraw(void)
{
  theCanvas->SetCurrent();
  //glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT |                                                 GL_ACCUM_BUFFER_BIT);
  if( csGraphics2DGLCommon::BeginDraw())
  {
    return true;
  }
  else
  {
    Report(CS_REPORTER_SEVERITY_ERROR,"Common BeginDraw fails");
    return false;
  }

}

void csGraphics2DWX::FinishDraw(void)
{
  csGraphics2DGLCommon::FinishDraw();
}

bool csGraphics2DWX::PerformExtensionV (char const* command, va_list args)
{
  if (!strcasecmp (command, "hardware_accelerated"))
  {
    return true;
  }
  if (!strcasecmp (command, "fullscreen"))
  {
    return false;
  }
  if (!strcasecmp (command, "setglcontext"))
  {
    theCanvas->SetCurrent();
    return true;
  }
  return csGraphics2DGLCommon::PerformExtensionV (command, args);
}

void csGraphics2DWX::Print (csRect const* /*area*/)
{
  //Make some sort of swapbuffers call
  glFlush();
  theCanvas->SwapBuffers();
}


void csGraphics2DWX::SetFullScreen (bool yesno)
{
  //Do nothing here, not valid for this type of canvas
}

void csGraphics2DWX::AllowResize (bool iAllow)
{
}

BEGIN_EVENT_TABLE(csGLCanvas, wxGLCanvas)
  EVT_SIZE(csGLCanvas::OnSize)
  EVT_PAINT(csGLCanvas::OnPaint)
  EVT_ERASE_BACKGROUND(csGLCanvas::OnEraseBackground)
  EVT_KEY_DOWN( csGLCanvas::OnKeyDown )
  EVT_KEY_UP( csGLCanvas::OnKeyUp )
  EVT_ENTER_WINDOW( csGLCanvas::OnEnterWindow )
  EVT_MOUSE_EVENTS( csGLCanvas::OnMouseEvent )
END_EVENT_TABLE()

csGLCanvas::csGLCanvas(csGraphics2DWX* g, wxWindow *parent,
                       wxWindowID id,
                       const wxPoint& pos,
                       const wxSize& size, long style,
                       const wxString& name, int* attr)
  : wxGLCanvas(parent, id, pos, size, style, name, attr), g2d(g)
{

  m_init = false;
  m_gllist = 0;

  int w, h;
  GetClientSize(&w, &h);

  SetCurrent();
  g2d->Resize(w, h);
}


csGLCanvas::~csGLCanvas()
{
}

void csGLCanvas::OnEnterWindow( wxMouseEvent& WXUNUSED(event) )
{
  SetFocus();
}

void csGLCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
}

void csGLCanvas::OnSize(wxSizeEvent& event)
{
  // this is also necessary to update the context on some platforms
  wxGLCanvas::OnSize(event);

  int w, h;
  GetClientSize(&w, &h);

  SetCurrent();
  g2d->Resize(w, h);
}

void csGLCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
  // Do nothing, to avoid flashing.
}

void csGLCanvas::OnMouseEvent( wxMouseEvent& event )
{
  if(event.GetEventType() == wxEVT_MOTION)
  {
    g2d->eventOutlet->Mouse(0, false, event.GetX(), event.GetY());
  }
  else if(event.GetEventType() == wxEVT_LEFT_DOWN)
  {
    g2d->eventOutlet->Mouse(1, true, event.GetX(), event.GetY());
  }
  else if(event.GetEventType() == wxEVT_LEFT_UP)
  {
    g2d->eventOutlet->Mouse(1, false, event.GetX(), event.GetY());
  }
  else if(event.GetEventType() == wxEVT_MIDDLE_DOWN)
  {
    g2d->eventOutlet->Mouse(3, true, event.GetX(), event.GetY());
  }
  else if(event.GetEventType() == wxEVT_MIDDLE_UP)
  {
    g2d->eventOutlet->Mouse(3, false, event.GetX(), event.GetY());
  }
  else if(event.GetEventType() == wxEVT_RIGHT_DOWN)
  {
    g2d->eventOutlet->Mouse(2, true, event.GetX(), event.GetY());
  }
  else if(event.GetEventType() == wxEVT_RIGHT_UP)
  {
    g2d->eventOutlet->Mouse(2, false, event.GetX(), event.GetY());
  }
}

int keyTable[WXK_NUMPAD_DIVIDE - WXK_START + 1] = {
  0, // WXK_START
  0, //WXK_LBUTTON
  0, //WXK_RBUTTON
  0, //WXK_CANCEL
  0, //WXK_MBUTTON
  CSKEY_DEL, //WXK_CLEAR
  CSKEY_SHIFT, //WXK_SHIFT
  CSKEY_ALT, //WXK_ALT
  CSKEY_CTRL, //WXK_CONTROL
  CSKEY_CONTEXT, //WXK_MENU
  CSKEY_PAUSE, //WXK_PAUSE
  CSKEY_CAPSLOCK, //WXK_CAPITAL
  CSKEY_PGUP, //WXK_PRIOR
  CSKEY_PGDN, //WXK_NEXT
  CSKEY_END, //WXK_END
  CSKEY_HOME, //WXK_HOME
  CSKEY_LEFT, //WXK_LEFT
  CSKEY_UP, //WXK_UP
  CSKEY_RIGHT, //WXK_RIGHT
  CSKEY_DOWN, //WXK_DOWN
  0,//WXK_SELECT
  CSKEY_PRINTSCREEN, //WXK_PRINT
  0, //WXK_EXECUTE
  0, //WXK_SNAPSHOT
  CSKEY_INS, //WXK_INSERT
  0, //WXK_HELP
  CSKEY_PAD0, //WXK_NUMPAD0
  CSKEY_PAD1, //WXK_NUMPAD1
  CSKEY_PAD2, //WXK_NUMPAD2
  CSKEY_PAD3, //WXK_NUMPAD3
  CSKEY_PAD4, //WXK_NUMPAD4
  CSKEY_PAD5, //WXK_NUMPAD5
  CSKEY_PAD6, //WXK_NUMPAD6
  CSKEY_PAD7, //WXK_NUMPAD7
  CSKEY_PAD8, //WXK_NUMPAD8
  CSKEY_PAD9, //WXK_NUMPAD9
  CSKEY_PADMULT, //WXK_MULTIPLY
  CSKEY_PADPLUS, //WXK_ADD
  0, //WXK_SEPARATOR
  CSKEY_PADMINUS, //WXK_SUBTRACT
  CSKEY_PADDECIMAL, //WXK_DECIMAL
  CSKEY_PADDIV, //WXK_DIVIDE
  CSKEY_F1, //WXK_F1
  CSKEY_F2, //WXK_F2
  CSKEY_F3, //WXK_F3
  CSKEY_F4, //WXK_F4
  CSKEY_F5, //WXK_F5
  CSKEY_F6, //WXK_F6
  CSKEY_F7, //WXK_F7
  CSKEY_F8, //WXK_F8
  CSKEY_F9, //WXK_F9
  CSKEY_F10, //WXK_F10
  CSKEY_F11, //WXK_F11
  CSKEY_F12, //WXK_F12
  0, //WXK_F13
  0, //WXK_F14
  0, //WXK_F15
  0, //WXK_F16
  0, //WXK_F17
  0, //WXK_F18
  0, //WXK_F19
  0, //WXK_F20
  0, //WXK_F21
  0, //WXK_F22
  0, //WXK_F23
  0, //WXK_F24
  CSKEY_PADNUM, //WXK_NUMLOCK
  CSKEY_SCROLLLOCK, //WXK_SCROLL
  CSKEY_PGUP, // WXK_PAGEDOWN,
  CSKEY_PGDN, // WXK_PAGEDOWN,
  0, //WXK_NUMPAD_SPACE,
  0, //WXK_NUMPAD_TAB,
  0, //WXK_NUMPAD_ENTER,
  0, //WXK_NUMPAD_F1,
  0, //WXK_NUMPAD_F2,
  0, //WXK_NUMPAD_F3,
  0, //WXK_NUMPAD_F4,
  0, //WXK_NUMPAD_HOME,
  0, //WXK_NUMPAD_LEFT,
  0, //WXK_NUMPAD_UP,
  0, //WXK_NUMPAD_RIGHT,
  0, //WXK_NUMPAD_DOWN,
  0, //WXK_NUMPAD_PRIOR,
  0, //WXK_NUMPAD_PAGEUP,
  0, //WXK_NUMPAD_NEXT,
  0, //WXK_NUMPAD_PAGEDOWN,
  0, //WXK_NUMPAD_END,
  0, //WXK_NUMPAD_BEGIN,
  0, //WXK_NUMPAD_INSERT,
  0, //WXK_NUMPAD_DELETE,
  0, //WXK_NUMPAD_EQUAL,
  0, //WXK_NUMPAD_MULTIPLY,
  0, //WXK_NUMPAD_ADD,
  0, //WXK_NUMPAD_SEPARATOR,
  0, //WXK_NUMPAD_SUBTRACT,
  0, //WXK_NUMPAD_DECIMAL,
  0, //WXK_NUMPAD_DIVIDE
};

void csGLCanvas::EmitKeyEvent(wxKeyEvent& event, bool down)
{
  long wxkey = event.GetKeyCode();
  long cskey_raw = 0, cskey_cooked = 0;



  if(wxkey == WXK_BACK || wxkey == WXK_TAB
     || wxkey == WXK_RETURN || wxkey == WXK_ESCAPE
     || wxkey == WXK_SPACE)
  {
    cskey_raw = cskey_cooked = wxkey;
  }
  else if(wxkey == WXK_DELETE)
  {
    cskey_raw = cskey_cooked = CSKEY_DEL;
  }
  else if(wxkey >= '!' && wxkey <= '/')
  {
    cskey_raw = cskey_cooked = wxkey;
  }
  else if(wxkey >= '0' && wxkey <= '9')
  {
    cskey_raw = cskey_cooked = wxkey;
  }
  else if(wxkey >= ':' && wxkey <= '@')
  {
    cskey_raw = cskey_cooked = wxkey;
  }
  else if(wxkey >= 'A' && wxkey <= 'Z')
  {
    cskey_raw = wxkey + 32;
    cskey_cooked = wxkey;
  }
  else if(wxkey >= '[' && wxkey <= '`')
  {
    cskey_raw = cskey_cooked = wxkey;
  }
  else if(wxkey >= 'a' && wxkey <= 'z')
  {
    cskey_raw = cskey_cooked = wxkey;
  }
  else if(wxkey >= '{' && wxkey <= '~')
  {
    cskey_raw = cskey_cooked = wxkey;
  }
  else if(wxkey >= WXK_START && wxkey <= WXK_NUMPAD_DIVIDE)
  {
    cskey_raw = cskey_cooked = keyTable[wxkey - WXK_START];
  }

  if(cskey_raw != 0) g2d->eventOutlet->Key(cskey_raw, cskey_cooked, down);
}

void csGLCanvas::OnKeyDown( wxKeyEvent& event )
{
  EmitKeyEvent(event, true);
}

void csGLCanvas::OnKeyUp( wxKeyEvent& event )
{
  EmitKeyEvent(event, false);
}
