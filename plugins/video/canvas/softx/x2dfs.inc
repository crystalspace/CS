/*
    Copyright (C) 1998 by Jorrit Tyberghein

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
    This file is included from different X11 canvas plugins.
    It is not a stand-alone sourcecode file. The X2D_CANVAS
    macro should be defined to the classname of the plugin.
*/

#ifdef XFREE86VM

static Bool GetModeInfo (Display *dpy, int scr, XF86VidModeModeInfo *info)
{
  XF86VidModeModeLine *l;

  l = (XF86VidModeModeLine *) ((char *)info + sizeof(info->dotclock));

  return XF86VidModeGetModeLine (dpy, scr, (int *)&info->dotclock, l);
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

static Bool wait_for_notify(Display */*display*/, XEvent *xEvent, XPointer arg)
{
  return (xEvent->type == MapNotify) && (xEvent->xmap.window == (Window)arg);
}

void X2D_CANVAS::FindBestMode ()
{
  XF86VidModeModeLine mode;
  XF86VidModeModeInfo **modes;
  int i, nModes;

  if (XF86VidModeGetModeLine(dpy, screen_num, &i, &mode)
   && XF86VidModeGetAllModeLines (dpy, screen_num, &nModes, &modes))
  {
    qsort (modes, nModes, sizeof (*modes), cmp_modes);

    // find best full screen mode
    for (i = nModes - 1; i >= 0; --i)
    {
      if (modes[i]->hdisplay >= Width && modes[i]->vdisplay >= Height)
      {
        fs_mode = *modes[i];
        break;
      }
    }

    XFree (modes);
  }
}
#endif

void X2D_CANVAS::EnterFullScreen ()
{
#ifdef XFREE86VM
  XEvent xEvent;
  int x;
  int y;
  int pointerX;
  int pointerY;
  Window rootReturn;
  Window childReturn;
  int childX;
  int childY;
  int rootX;
  int rootY;
  unsigned int maskReturn;

  // only switch if needed
  if (currently_full_screen)
    return;

  FindBestMode ();

  XResizeWindow (dpy, fs_window, fs_mode.hdisplay, fs_mode.vdisplay);

  XClearWindow (dpy, fs_window);

  // Wait until the window is mapped
  XSelectInput (dpy, fs_window, StructureNotifyMask);
  XMapRaised (dpy, fs_window);
  XIfEvent (dpy, &xEvent, wait_for_notify, (XPointer) fs_window);
  XSelectInput (dpy, fs_window, NoEventMask);

  // save current display information
  GetModeInfo (dpy, screen_num, &orig_mode);
  XF86VidModeGetViewPort (dpy, screen_num, &orig_x, &orig_y);

  // get pointer location inside original window so it appears not to move
  if (XQueryPointer (dpy, window, &rootReturn, &childReturn,
        &rootX, &rootY, &childX, &childY, &maskReturn))
  {
    pointerX = childX;
    pointerY = childY;
  }
  else
  {
    pointerX = fs_mode.hdisplay / 2;
    pointerY = fs_mode.vdisplay / 2;
  }

  // grab pointer and keyboard in fullscreen mode
  if (XGrabPointer (dpy, fs_window, True, 0, GrabModeAsync, GrabModeAsync,
        fs_window, None, CurrentTime) != GrabSuccess ||
      XGrabKeyboard (dpy, wm_window, True, GrabModeAsync,
        GrabModeAsync, CurrentTime) != GrabSuccess)
  {
    XUnmapWindow (dpy, fs_window);
    Report (CS_REPORTER_SEVERITY_ERROR, "Unable to grab focus");
    return;
  }

  currently_full_screen = true;

  if (fs_mode.hdisplay != orig_mode.hdisplay ||
      fs_mode.vdisplay != orig_mode.vdisplay)
  {
    // switch mode
    XF86VidModeLockModeSwitch (dpy, screen_num, True);

    if (!XF86VidModeSwitchToMode (dpy, screen_num, &fs_mode))
    {
      LeaveFullScreen ();
      Report (CS_REPORTER_SEVERITY_ERROR, "Unable to switch to mode %dx%d",
        fs_mode.hdisplay, fs_mode.vdisplay);
      return;
    }

    display_width  = fs_mode.hdisplay;
    display_height = fs_mode.vdisplay;

    XF86VidModeSetViewPort (dpy, screen_num, 0, 0);
  }

  int w = Width, h = Height;
  if (allow_canvas_resize)
  {
    w = fs_mode.hdisplay;
    h = fs_mode.vdisplay;
  }

  x = (fs_mode.hdisplay - w) / 2;
  y = (fs_mode.vdisplay - h) / 2;
  XReparentWindow (dpy, window, fs_window, x, y);
  XResizeWindow (dpy, window, w, h);

  XWarpPointer (dpy, None, window, 0, 0, 0, 0, pointerX, pointerY);

  XSync (dpy, False);
#endif // XFREE86VM
}

void X2D_CANVAS::LeaveFullScreen ()
{
#ifdef XFREE86VM
  XF86VidModeModeInfo mode;
  int pointerX;
  int pointerY;
  Window rootReturn;
  Window childReturn;
  int childX;
  int childY;
  int rootX;
  int rootY;
  unsigned int maskReturn;

  if (!currently_full_screen)
    return;

  // get pointer location inside original window so it appears not to move
  if (XQueryPointer (dpy, window, &rootReturn, &childReturn,
        &rootX, &rootY, &childX, &childY, &maskReturn))
  {
    pointerX = childX;
    pointerY = childY;
  }
  else
  {
    pointerX = orig_mode.hdisplay / 2;
    pointerY = orig_mode.vdisplay / 2;
  }

  GetModeInfo (dpy, screen_num, &mode);

  if (orig_mode.hdisplay != mode.hdisplay
   || orig_mode.vdisplay != mode.vdisplay)
  {
    XF86VidModeLockModeSwitch (dpy, screen_num, False);

    // switch to non-fullscreen mode
    if (!XF86VidModeSwitchToMode (dpy, screen_num, &orig_mode))
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "Unable to restore mode %dx%d",
        fs_mode.hdisplay, fs_mode.vdisplay);
      return;
    }

    display_width  = orig_mode.hdisplay;
    display_height = orig_mode.vdisplay;

    // restore viewport if offset inside virtual screen
    if (orig_x != 0 || orig_y != 0)
      XF86VidModeSetViewPort(dpy, screen_num, orig_x, orig_y);
  }

  XUngrabPointer (dpy, CurrentTime);
  XUngrabKeyboard (dpy, CurrentTime);

  XReparentWindow (dpy, window, wm_window, 0, 0);
  XResizeWindow (dpy, window, wm_width, wm_height);

  // restore pointer location
  XWarpPointer (dpy, None, window, 0, 0, 0, 0, pointerX, pointerY);

  currently_full_screen = false;

  XUnmapWindow (dpy, fs_window);
  XSync (dpy, False);
#endif // XFREE86VM
}
