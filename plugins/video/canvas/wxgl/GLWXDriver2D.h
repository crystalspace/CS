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

#ifndef __CS_GLWX2D_H__
#define __CS_GLWX2D_H__

#include "csutil/scf.h"
#include "csplugincommon/opengl/glcommon2d.h"
#include "csplugincommon/iopengl/openglinterface.h"
#include "ivideo/wxwin.h"
#include "wx/glcanvas.h"

class csGLCanvas;

class csGraphics2DWX : public scfImplementationExt2<csGraphics2DWX, 
						      csGraphics2DGLCommon, 
						      iWxWindow,
						      iOpenGLInterface>
{
  wxWindow* myParent;
  csGLCanvas* theCanvas;

public:
  csGraphics2DWX (iBase *iParent);
  virtual ~csGraphics2DWX ();

  virtual bool Initialize (iObjectRegistry *object_reg);
  virtual bool Open ();
  virtual void Close ();

  /**
   * This routine should be called before any draw operations.
   * It should return true if graphics context is ready.
   */
  virtual bool BeginDraw ();
  /// This routine should be called when you finished drawing
  virtual void FinishDraw ();

  void Report (int severity, const char* msg, ...);

  virtual void Print (csRect const* area = 0);

  virtual bool PerformExtensionV (char const* command, va_list);

  virtual void AllowResize (bool iAllow);

  virtual void SetTitle (const char* title)
    { ; }

  virtual void SetFullScreen (bool yesno);

  virtual bool GetFullScreen ()
    { return false; }
  /// Set mouse position.
  // should be the window manager
  virtual bool SetMousePosition (int x, int y)
    { return false; }

  /// Set mouse cursor shape
  // should be the window manager
  virtual bool SetMouseCursor (csMouseCursorID iShape)
    { return false;}

  virtual void SetParent(wxWindow* wx);
  virtual wxWindow* GetWindow();

  void *GetProcAddress (const char *funcname);
};

class csGLCanvas: public wxGLCanvas
{
private:
  csGraphics2DWX* g2d;

public:
  csGLCanvas(csGraphics2DWX* g2d, wxWindow *parent, wxWindowID id = wxID_ANY,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = 0, const wxString& name = _T("TestGLCanvas"),
             int* attr = 0);

  ~csGLCanvas();

  void EmitKeyEvent(wxKeyEvent& event, bool down);

  void OnPaint(wxPaintEvent& event);
  void OnSize(wxSizeEvent& event);
  void OnEraseBackground(wxEraseEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnKeyUp(wxKeyEvent& event);
  void OnMouseEvent(wxMouseEvent& event);
  void OnEnterWindow(wxMouseEvent& event);

  DECLARE_EVENT_TABLE();
};
#endif // __CS_GLWX2D_H__
