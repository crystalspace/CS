/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __SIMPLE1_H__
#define __SIMPLE1_H__

#include <stdarg.h>
#include "csutil/ref.h"
#include "ivideo/wxwin.h"

#include "csutil/custom_new_disable.h"
#include <wx/wx.h>
#include "csutil/custom_new_enable.h"

struct iEngine;
struct iLoader;
struct iGraphics3D;
struct iKeyboardDriver;
struct iVirtualClock;
struct iObjectRegistry;
struct iEvent;
struct iSector;
struct iView;
struct iWxWindow;
class FramePrinter;

class Simple : public wxFrame,
               public CS::Utility::WeakReferenced
{
private:
  iObjectRegistry* object_reg;
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  csRef<iView> view;
  csRef<iWxWindow> wxwindow;
  csRef<iSector> room;
  csRef<FramePrinter> printer;

  float rotY;
  float rotX;

  static bool SimpleEventHandler (iEvent& ev);
  bool HandleEvent (iEvent& ev);
  void SetupFrame ();

  CS_DECLARE_EVENT_SHORTCUTS;
  csEventID MouseDown;
  csEventID MouseUp;
  csEventID MouseMove;
  csEventID KeyboardDown;

public:
  Simple (iObjectRegistry* object_reg);
  ~Simple ();

  bool Initialize ();
  void PushFrame ();
  void OnClose (wxCloseEvent& event);
  void OnIconize (wxIconizeEvent& event);
  void OnShow (wxShowEvent& event);
  void OnSize (wxSizeEvent& ev);

  DECLARE_EVENT_TABLE ();

  class Panel : public wxPanel
  {
  public:
    Panel(wxWindow* parent, Simple* s)
      : wxPanel (parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), s (s)
    {}
    
    virtual void OnSize (wxSizeEvent& ev)
    { s->OnSize (ev); }

    private:
      Simple* s;

      DECLARE_EVENT_TABLE()
  };
};

#endif // __SIMPLE1_H__
