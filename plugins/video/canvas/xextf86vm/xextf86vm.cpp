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

#include <stdarg.h>
#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "csutil/scf.h"
#include "ivaria/reporter.h"
#include "csgeom/csrect.h"
#include "csutil/cfgacc.h"
#include "xextf86vm.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csXExtF86VM)


SCF_IMPLEMENT_IBASE(csXExtF86VM)
  SCF_IMPLEMENTS_INTERFACE(iXExtF86VM)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csXExtF86VM::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csXExtF86VM::csXExtF86VM (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  dpy = 0;
  screen_num = 0;
  width = height = 0;
  fs_win = wm_win = ctx_win = 0;
}

csXExtF86VM::~csXExtF86VM ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csXExtF86VM::Initialize (iObjectRegistry *object_reg)
{
  this->object_reg = object_reg;
  csConfigAccess Config(object_reg, "/config/video.cfg");
  full_screen = Config->GetBool ("Video.FullScreen", false);

  return true;
}


void csXExtF86VM::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.window.x.extf86vm", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

bool csXExtF86VM::Open (Display *dpy, int screen_num,
			XVisualInfo *xvis, Colormap cmap)
{
  if (!ctx_win || !wm_win)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No Windows Set\n");
    return false;
  }

  this->dpy = dpy;
  this->screen_num = screen_num;
  unsigned long cw_fs_mask  = (CWOverrideRedirect |
			       CWBorderPixel |
			       (cmap ? CWColormap : 0));
  XSetWindowAttributes swa;
  memset (&swa, 0, sizeof(swa));
  swa.colormap = cmap;
  swa.override_redirect = True;
  swa.background_pixel = 0;
  swa.border_pixel = 0;
  swa.event_mask = 0;

  fs_win = XCreateWindow (dpy,
			  RootWindow (dpy, screen_num),
			  0, 0, 1, 1,
			  0,
			  xvis->depth,
			  InputOutput,
			  xvis->visual,
			  cw_fs_mask,
			  &swa);

  XStoreName (dpy, fs_win, "Full Screen");
  XSetWindowBackground (dpy, fs_win, BlackPixel (dpy, screen_num));
  XSelectInput (dpy, fs_win, 0);

  if (full_screen)
  {
    full_screen = false;
    EnterFullScreen ();
    return full_screen;
  }
  return true;
}

void csXExtF86VM::Close ()
{
  ctx_win = wm_win = 0;
  if (full_screen)
    LeaveFullScreen ();
  XDestroyWindow (dpy, fs_win);
  fs_win = 0;
}

bool csXExtF86VM::SetFullScreen (bool yesno)
{
  if (!ctx_win)
  {
    // In initialization phase and configuring
    full_screen = yesno;
    return false;
  }
  if (full_screen != yesno)
  {
    if (yesno)
      EnterFullScreen ();
    else
      LeaveFullScreen ();
    return (full_screen == yesno);
  }
  return false;
}

static Bool GetModeInfo (Display *dpy, int scr, XF86VidModeModeInfo *info)
{
  XF86VidModeModeLine *l;

  l = (XF86VidModeModeLine *) ((char *)info + sizeof(info->dotclock));

  return XF86VidModeGetModeLine (dpy, scr, (int *)&info->dotclock, l);
}

void csXExtF86VM::ChangeVideoMode (int zoom)
{
  XF86VidModeLockModeSwitch (dpy, screen_num, false);
  if (XF86VidModeSwitchMode (dpy, screen_num, zoom))
  {
    if (!GetModeInfo (dpy, screen_num, &fs_mode))
      Report (CS_REPORTER_SEVERITY_ERROR, "Unable to retrieve mode info ");
    width = fs_mode.hdisplay;
    height = fs_mode.vdisplay;
    XResizeWindow (dpy, fs_win, fs_mode.hdisplay, fs_mode.vdisplay);
    XF86VidModeSetViewPort(dpy, screen_num, 0, 0);
    Report (CS_REPORTER_SEVERITY_NOTIFY, "%s VIDEOMODE: %d, %d\n",
	    zoom == -1 ? "UP" : "DOWN",
	    width, height);
  }
  XF86VidModeLockModeSwitch (dpy, screen_num, true);
}

static int cmp_modes (const void *va, const void *vb)
{
  XF86VidModeModeInfo *a = *(XF86VidModeModeInfo **) va;
  XF86VidModeModeInfo *b = *(XF86VidModeModeInfo **) vb;

  if (a->hdisplay > b->hdisplay)
    return -1;
  else
    return b->vdisplay - a->vdisplay;
}

void csXExtF86VM::FindBestMode (int ctx_width, int ctx_height)
{
  XF86VidModeModeLine mode;
  XF86VidModeModeInfo **modes;
  int i, nModes, best_mode = 0;
  bool valid = false;
  unsigned int diff;
  unsigned int best_diff = (unsigned int) -1;
  if (XF86VidModeGetModeLine(dpy, screen_num, &i, &mode)
   && XF86VidModeGetAllModeLines (dpy, screen_num, &nModes, &modes))
  {
    qsort (modes, nModes, sizeof (*modes), cmp_modes);

    // find best full screen mode
    for (i = nModes - 1; i >= 0; --i)
    {
      if (modes[i]->hdisplay >= ctx_width && modes[i]->vdisplay >= ctx_height)
      {
        fs_mode = *modes[i];
	valid = true;
        break;
      }
      diff = ctx_width - modes[i]->hdisplay;
      if (diff < best_diff)
	best_mode = i;
    }
    if (!valid)
      fs_mode = *modes[best_mode];

    XFree (modes);
  }
}

bool csXExtF86VM::SwitchMode (XF86VidModeModeInfo *to_mode,
			      XF86VidModeModeInfo *from_mode,
			      bool lock, int vp_x, int vp_y)
{
  XF86VidModeLockModeSwitch (dpy, screen_num, lock);
  if (to_mode->hdisplay != from_mode->hdisplay ||
      to_mode->vdisplay != from_mode->vdisplay)
  {
    if (!XF86VidModeSwitchToMode (dpy, screen_num, to_mode))
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "Unable to restore mode %hux%hu",
	      to_mode->hdisplay, to_mode->vdisplay);
      return false;
    }
  }
  XF86VidModeSetViewPort(dpy, screen_num, vp_x, vp_y);
  return true;
}

void csXExtF86VM::EnterFullScreen ()
{
  XWindowAttributes wa;
  // only switch if needed
  if (full_screen)
    return;
  if (!XGetWindowAttributes (dpy, ctx_win, &wa))
    return;

  FindBestMode (wa.width, wa.height);

  #ifdef CS_DEBUG
    printf ("Entering fullscreen: win %d, %d to fs_mode %hu, %hu\n\n",
      wa.width, wa.height, fs_mode.hdisplay, fs_mode.vdisplay);
  #endif
        
  XResizeWindow (dpy, fs_win, fs_mode.hdisplay, fs_mode.vdisplay);
  XClearWindow (dpy, fs_win);
  XMapRaised (dpy, fs_win);

  // save current display information
  GetModeInfo (dpy, screen_num, &wm_mode);
  XF86VidModeGetViewPort (dpy, screen_num, &viewport_x, &viewport_y);

  // grab pointer and keyboard in fullscreen mode
  if ((XGrabPointer (dpy, fs_win, True,
		    0, GrabModeAsync, GrabModeAsync,
		    fs_win, None, CurrentTime) == GrabSuccess) &&
      (XGrabKeyboard (dpy, wm_win, True, GrabModeAsync,
		      GrabModeAsync, CurrentTime) == GrabSuccess) &&
      SwitchMode (&fs_mode, &wm_mode, true, 0, 0))
  {
    full_screen = true;

    XReparentWindow (dpy, ctx_win, fs_win, 0, 0);
    XWarpPointer (dpy, None, ctx_win,
		  0, 0, 0, 0,
		  fs_mode.hdisplay >> 1,
		  fs_mode.vdisplay >> 1);

    width = fs_mode.hdisplay;
    height = fs_mode.vdisplay;
    Report (CS_REPORTER_SEVERITY_NOTIFY, "FULL SCREEN: %d, %d", width, height);
    XSync (dpy, False);
  }
  else
  {
    XUnmapWindow (dpy, fs_win);
    Report (CS_REPORTER_SEVERITY_ERROR, "Unable to switch mode");
  }
}

void csXExtF86VM::LeaveFullScreen ()
{
  XWindowAttributes wa;
  XF86VidModeModeInfo mode;
  if (!full_screen)
    return;

  GetModeInfo (dpy, screen_num, &mode);

  bool ret = SwitchMode (&wm_mode, &fs_mode, false, viewport_x, viewport_y);
  XUngrabPointer (dpy, CurrentTime);
  XUngrabKeyboard (dpy, CurrentTime);

  if (!ret)
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Unable to return to windowed mode....aborting\n");
    exit (-1);
  }

  if (wm_win != 0)
  {
    if (!XGetWindowAttributes (dpy, wm_win, &wa))
      return;
    XReparentWindow (dpy, ctx_win, wm_win, 0, 0);
    width = wa.width;
    height = wa.height;
    XWarpPointer (dpy, None, ctx_win,
		  0, 0, 0, 0,
		  wa.width >> 1,
		  wa.height >> 1);
  }
  full_screen = false;
  XUnmapWindow (dpy, fs_win);
  XSync (dpy, False);
}
