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

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

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

#ifdef WIN32
#include "csutil/win32/win32.h"
#include "csutil/win32/wintools.h"
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
    EventOutlet = q->CreateEventOutlet (this);

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

void* csGraphics2DWX::GetProcAddress (const char *funcname)
{
#ifdef WIN32
  return wglGetProcAddress (funcname);
#else
  return 0;
#endif
}

csGraphics2DWX::~csGraphics2DWX ()
{
  Close ();  
  // theCanvas is destroyed by wxWindows
}

bool csGraphics2DWX::Open()
{
  if (is_open) return true;

#ifdef WIN32
  csRef<iWin32Assistant> w32(CS_QUERY_REGISTRY(object_reg, iWin32Assistant));
  if(w32 != 0) w32->UseOwnMessageLoop(false);
#endif

  Report (CS_REPORTER_SEVERITY_NOTIFY, "Opening WX-GL canvas!\n");

  if(myParent == 0)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Parent frame in wxGLCanvas not initialized!!");
	return false;
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
      0
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

  AllowResize(true);

  int w, h;
  myParent->GetClientSize(&w, &h);

  theCanvas = new csGLCanvas(this, myParent, wxID_ANY,
                             wxPoint(0, 0), wxSize(w, h), 0, wxT(""), desired_attributes);

  if(theCanvas == 0) Report(CS_REPORTER_SEVERITY_ERROR, "Failed creating GL Canvas!");
  theCanvas->Show(true);

#ifdef WIN32
  {
    HDC hDC = (HDC)theCanvas->GetHDC();
    PIXELFORMATDESCRIPTOR pfd;
    if (DescribePixelFormat (hDC, ::GetPixelFormat (hDC), 
      sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0)
    {
      DWORD error = GetLastError();
      char* msg = cswinGetErrorMessage (error);
      Report (CS_REPORTER_SEVERITY_ERROR,
	"DescribePixelFormat failed: %s [%ul]",
	msg, error);
      delete[] msg;
    }
    else
    {
      currentFormat[glpfvColorBits] = pfd.cColorBits;
      currentFormat[glpfvAlphaBits] = pfd.cAlphaBits;
      currentFormat[glpfvDepthBits] = pfd.cDepthBits;
      currentFormat[glpfvStencilBits] = pfd.cStencilBits;
      currentFormat[glpfvAccumColorBits] = pfd.cAccumBits;
      currentFormat[glpfvAccumAlphaBits] = pfd.cAccumAlphaBits;

      Depth = pfd.cColorBits; 
    }
  }
#endif

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
	AllowResizing = iAllow;
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
  : wxGLCanvas(parent, id, pos, size, style | wxWANTS_CHARS, name, attr), g2d(g)
{
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
  wxPaintDC dc(this);
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
  // printf("got mouse event %i %i\n", event.GetX(), event.GetY());

  if(event.GetEventType() == wxEVT_MOTION)
  {
    g2d->EventOutlet->Mouse(0, false, event.GetX(), event.GetY());
  }
  else if(event.GetEventType() == wxEVT_LEFT_DOWN)
  {
    g2d->EventOutlet->Mouse(1, true, event.GetX(), event.GetY());
  }
  else if(event.GetEventType() == wxEVT_LEFT_UP)
  {
    g2d->EventOutlet->Mouse(1, false, event.GetX(), event.GetY());
  }
  else if(event.GetEventType() == wxEVT_MIDDLE_DOWN)
  {
    g2d->EventOutlet->Mouse(3, true, event.GetX(), event.GetY());
  }
  else if(event.GetEventType() == wxEVT_MIDDLE_UP)
  {
    g2d->EventOutlet->Mouse(3, false, event.GetX(), event.GetY());
  }
  else if(event.GetEventType() == wxEVT_RIGHT_DOWN)
  {
    g2d->EventOutlet->Mouse(2, true, event.GetX(), event.GetY());
  }
  else if(event.GetEventType() == wxEVT_RIGHT_UP)
  {
    g2d->EventOutlet->Mouse(2, false, event.GetX(), event.GetY());
  }
}

int wxCodeToCSCode(int wxkey)
{
  switch(wxkey) {
  case WXK_BACK: return CSKEY_BACKSPACE;
  case WXK_TAB: return CSKEY_TAB;
  case WXK_RETURN: return CSKEY_ENTER;
  case WXK_ESCAPE: return CSKEY_ESC;
  case WXK_SPACE: return CSKEY_SPACE;
  case WXK_DELETE: return CSKEY_DEL;
  case WXK_START: return 0;
  case WXK_LBUTTON: return 0;
  case WXK_RBUTTON: return 0;
  case WXK_CANCEL: return 0;
  case WXK_MBUTTON: return 0;
  case WXK_CLEAR: return CSKEY_DEL;
  case WXK_SHIFT: return CSKEY_SHIFT;
  case WXK_ALT: return CSKEY_ALT;
  case WXK_CONTROL: return CSKEY_CTRL;
  case WXK_MENU: return CSKEY_CONTEXT;
  case WXK_PAUSE: return CSKEY_PAUSE;
  case WXK_CAPITAL: return CSKEY_CAPSLOCK;
  case WXK_PRIOR: return CSKEY_PGUP;
  case WXK_NEXT: return CSKEY_PGDN;
  case WXK_END: return CSKEY_END;
  case WXK_HOME: return CSKEY_HOME;
  case WXK_LEFT: return CSKEY_LEFT;
  case WXK_UP: return CSKEY_UP;
  case WXK_RIGHT: return CSKEY_RIGHT;
  case WXK_DOWN: return CSKEY_DOWN;
  case WXK_SELECT: return 0;
  case WXK_PRINT: return CSKEY_PRINTSCREEN;
  case WXK_EXECUTE: return 0;
  case WXK_SNAPSHOT: return 0;
  case WXK_INSERT: return CSKEY_INS;
  case WXK_HELP: return 0;
  case WXK_NUMPAD0: return CSKEY_PAD0;
  case WXK_NUMPAD1: return CSKEY_PAD1;
  case WXK_NUMPAD2: return CSKEY_PAD2;
  case WXK_NUMPAD3: return CSKEY_PAD3;
  case WXK_NUMPAD4: return CSKEY_PAD4;
  case WXK_NUMPAD5: return CSKEY_PAD5;
  case WXK_NUMPAD6: return CSKEY_PAD6;
  case WXK_NUMPAD7: return CSKEY_PAD7;
  case WXK_NUMPAD8: return CSKEY_PAD8;
  case WXK_NUMPAD9: return CSKEY_PAD9;
  case WXK_MULTIPLY: return CSKEY_PADMULT;
  case WXK_ADD: return CSKEY_PADPLUS;
  case WXK_SEPARATOR: return 0;
  case WXK_SUBTRACT: return CSKEY_PADMINUS;
  case WXK_DECIMAL: return CSKEY_PADDECIMAL;
  case WXK_DIVIDE: return CSKEY_PADDIV;
  case WXK_F1: return CSKEY_F1;
  case WXK_F2: return CSKEY_F2;
  case WXK_F3: return CSKEY_F3;
  case WXK_F4: return CSKEY_F4;
  case WXK_F5: return CSKEY_F5;
  case WXK_F6: return CSKEY_F6;
  case WXK_F7: return CSKEY_F7;
  case WXK_F8: return CSKEY_F8;
  case WXK_F9: return CSKEY_F9;
  case WXK_F10: return CSKEY_F10;
  case WXK_F11: return CSKEY_F11;
  case WXK_F12: return CSKEY_F12;
  case WXK_F13: return 0;
  case WXK_F14: return 0;
  case WXK_F15: return 0;
  case WXK_F16: return 0;
  case WXK_F17: return 0;
  case WXK_F18: return 0;
  case WXK_F19: return 0;
  case WXK_F20: return 0;
  case WXK_F21: return 0;
  case WXK_F22: return 0;
  case WXK_F23: return 0;
  case WXK_F24: return 0;
  case WXK_NUMLOCK: return CSKEY_PADNUM;
  case WXK_SCROLL: return CSKEY_SCROLLLOCK;
  case WXK_PAGEUP: return CSKEY_PGUP;
  case WXK_PAGEDOWN: return CSKEY_PGDN;
  case WXK_NUMPAD_SPACE: return 0;
  case WXK_NUMPAD_TAB: return 0;
  case WXK_NUMPAD_ENTER: return 0;
  case WXK_NUMPAD_F1: return 0;
  case WXK_NUMPAD_F2: return 0;
  case WXK_NUMPAD_F3: return 0;
  case WXK_NUMPAD_F4: return 0;
  case WXK_NUMPAD_HOME: return 0;
  case WXK_NUMPAD_LEFT: return 0;
  case WXK_NUMPAD_UP: return 0;
  case WXK_NUMPAD_RIGHT: return 0;
  case WXK_NUMPAD_DOWN: return 0;
  case WXK_NUMPAD_PRIOR: return 0;
  case WXK_NUMPAD_PAGEUP: return 0;
  case WXK_NUMPAD_NEXT: return 0;
  case WXK_NUMPAD_PAGEDOWN: return 0;
  case WXK_NUMPAD_END: return 0;
  case WXK_NUMPAD_BEGIN: return 0;
  case WXK_NUMPAD_INSERT: return 0;
  case WXK_NUMPAD_DELETE: return 0;
  case WXK_NUMPAD_EQUAL: return 0;
  case WXK_NUMPAD_MULTIPLY: return 0;
  case WXK_NUMPAD_ADD: return 0;
  case WXK_NUMPAD_SEPARATOR: return 0;
  case WXK_NUMPAD_SUBTRACT: return 0;
  case WXK_NUMPAD_DECIMAL: return 0;
  case WXK_NUMPAD_DIVIDE: return 0;
  default: return 0;
  }
}

void csGLCanvas::EmitKeyEvent(wxKeyEvent& event, bool down)
{
  long wxkey = event.GetKeyCode();
  long cskey_raw = 0, cskey_cooked = 0;

  // printf("got key %s event %i\n", (down ? "down" : "up"), wxkey);

  if((wxkey >= '!' && wxkey <= '/')
	  || (wxkey >= '0' && wxkey <= '9')
	  || (wxkey >= ':' && wxkey <= '@')
	  || (wxkey >= '[' && wxkey <= '`')
	  || (wxkey >= 'a' && wxkey <= 'z')
	  || (wxkey >= '{' && wxkey <= '~'))
  {
	  cskey_raw = cskey_cooked = wxkey;
  }
  else if(wxkey >= 'A' && wxkey <= 'Z')
  {
	  cskey_raw = wxkey + 32;
	  cskey_cooked = wxkey;
  }
  else 
  {
	  cskey_raw = cskey_cooked = wxCodeToCSCode(wxkey);
  }

  if(cskey_raw != 0) g2d->EventOutlet->Key(cskey_raw, cskey_cooked, down);
}

void csGLCanvas::OnKeyDown( wxKeyEvent& event )
{
  EmitKeyEvent(event, true);
}

void csGLCanvas::OnKeyUp( wxKeyEvent& event )
{
  EmitKeyEvent(event, false);
}
