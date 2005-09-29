/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Copyright (C) 2001 by Samuel Humphreys

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

#ifndef __CS_XWINDOW_H__
#define __CS_XWINDOW_H__

#include <stdarg.h>
#include "csutil/scf.h"
#include "csutil/hash.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/event.h"
#include "plugins/video/canvas/xwindowcommon/xwindow.h"
#include "plugins/video/canvas/xwindowcommon/xextf86vm.h"
#include "ivideo/graph2d.h"

#define XK_MISCELLANY 1
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>

class csXWindow : public iXWindow, public iEventPlug
{
  /// The Object Registry
  iObjectRegistry *object_reg;
  /// The Canvas
  iGraphics2D *Canvas;
  /// The event outlet
  csRef<iEventOutlet> EventOutlet;
  /// The XFree86-VidModeExtension
  csRef<iXExtF86VM> xf86vm;
  /// The Window Title
  char *win_title;
  /// The X-display
  Display* dpy;
  /// The Screen Number (not necessarilly the default)
  int screen_num;
  // Window colormap
  Colormap cmap;
  /// The Graphic Context
  GC gc;
  /// The Visual Information Structure
  XVisualInfo *xvis;
  /// The Context Window
  Window ctx_win;
  /// The Window Managers Window
  Window wm_win;
  /// Dimensions
  int wm_width, wm_height;

  // "WM_DELETE_WINDOW" atom
  Atom wm_delete_window;

  bool allow_resize;
  /// Determines grab status of keyboard (only in release build)
  int keyboard_grabbed;

  //-------------------------------------------------------------
  // Hardware mouse cursor or software emulation?
  bool do_hwmouse;
  /// Mouse cursors (if hardware mouse cursors are used)
  Cursor MouseCursor [int(csmcWait) + 1];
  /// Empty mouse cursor (consist of EmptyPixmap)
  Cursor EmptyMouseCursor;
  /// A empty pixmap
  Pixmap EmptyPixmap;
  /// List of image-based cursors
  csHash<Cursor, csStrKey> cachedCursors;
  //------------------------------------------------------------

  void Report (int severity, const char* msg, ...);
  void SetVideoMode (bool full, bool up, bool down);

public:
  SCF_DECLARE_IBASE;

  csXWindow (iBase*);
  virtual ~csXWindow ();

  virtual bool Initialize (iObjectRegistry*);
  virtual bool HandleEvent (iEvent& Event);

  virtual bool Open ();
  virtual void Close ();

  virtual bool GetFullScreen ()
  { return xf86vm ? xf86vm->IsFullScreen () : false; }

  virtual void SetFullScreen (bool yesno)
  { SetVideoMode (yesno, false, false); }

  virtual void AllowResize (bool iAllow);

  virtual void SetTitle (const char* title);
  virtual void SetCanvas (iGraphics2D *canvas);

  virtual Display *GetDisplay ()
  { return dpy; }
  virtual int GetScreen ()
  { return screen_num; }
  virtual Window GetWindow ()
  { return ctx_win; }
  virtual GC GetGC ()
  { return gc; }

  virtual void SetVisualInfo (XVisualInfo *vis)
  { xvis = vis; }
  virtual void SetColormap (Colormap cmap)
  { this->cmap = cmap; }

  // Should be in the window manager
  virtual bool SetMousePosition (int x, int y);
  virtual bool SetMouseCursor (csMouseCursorID iShape);

  virtual bool SetMouseCursor (iImage *image, const csRGBcolor* keycolor, 
                               int hotspot_x, int hotspot_y,
                               csRGBcolor fg, csRGBcolor bg);

  virtual bool AlertV (int type, const char* title, const char* okMsg,
  	const char* msg, va_list arg) CS_GNUC_PRINTF (5, 0);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csXWindow);
    virtual bool Initialize (iObjectRegistry *o)
    { return scfParent->Initialize(o); }
  } scfiComponent;
  struct EventHandler : public iEventHandler
  {
  private:
    csXWindow* parent;
  public:
    SCF_DECLARE_IBASE;
    EventHandler (csXWindow* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
      SCF_DESTRUCT_IBASE();
    }
    virtual bool HandleEvent (iEvent& e) { return parent->HandleEvent(e); }
  } * scfiEventHandler;

  //------------------------ iEventPlug interface ---------------------------//

  virtual unsigned GetPotentiallyConflictingEvents ()
  { return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }
  virtual unsigned QueryEventPriority (unsigned /*iType*/)
  { return 150; }

};

#endif // __CS_XWINDOW_H__
