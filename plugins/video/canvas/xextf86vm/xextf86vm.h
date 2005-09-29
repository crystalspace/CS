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

#ifndef __CS_XEXTF86VM_H__
#define __CS_XESTF86VM_H__

#include <stdarg.h>
#include "csutil/scf.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "plugins/video/canvas/xwindowcommon/xextf86vm.h"
#include "ivideo/graph2d.h"

#define XK_MISCELLANY 1
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <X11/extensions/xf86vmode.h>

class csXExtF86VM : public iXExtF86VM
{
  /// The Object Registry
  iObjectRegistry *object_reg;
  /// The X-display
  Display* dpy;
  /// The Screen Number (not necessarilly the default)
  int screen_num;
  /// The Full Screen Window
  Window fs_win;
  /// The context window and its parent window while in windowed mode
  Window ctx_win, wm_win;
  /// Is Full Screen ?
  bool full_screen;
  /// The dimensions
  int width, height;
  /// The windowed mode viewport (if X is running a virtual screen)
  int viewport_x, viewport_y;

  /// Windowed mode
  XF86VidModeModeInfo wm_mode;
  /// Current FullScreen mode;
  XF86VidModeModeInfo fs_mode;

  /// Internal helper routines
  void FindBestMode (int ctx_width, int ctx_height);
  bool SwitchMode (XF86VidModeModeInfo *to_mode,
		   XF86VidModeModeInfo *from_mode,
		   bool lock, int vp_x, int vp_y);

  void EnterFullScreen ();
  void LeaveFullScreen ();
  void ChangeVideoMode (int zoom);
public:
  SCF_DECLARE_IBASE;

  csXExtF86VM (iBase*);
  virtual ~csXExtF86VM ();

  virtual bool Initialize (iObjectRegistry*);
  void Report (int severity, const char* msg, ...);


  virtual bool Open (Display *dpy, int screen_num,
		     XVisualInfo *xvis, Colormap cmap);
  virtual void Close ();
  virtual void SetWindows (Window ctx_win, Window wm_win)
  { this->ctx_win = ctx_win; this->wm_win = wm_win; }
  virtual bool SetFullScreen (bool yesno);
  virtual bool IsFullScreen ()
  { return full_screen; }

  virtual void IncVideoMode ()
  { ChangeVideoMode (-1); }

  virtual void DecVideoMode ()
  { ChangeVideoMode (1); }

  virtual void GetDimensions (int &w, int &h)
  { w = width; h = height; }

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csXExtF86VM);
    virtual bool Initialize (iObjectRegistry *o)
    { return scfParent->Initialize(o); }
  } scfiComponent;

};

#endif // __CS_XEXTF86VM_H__
