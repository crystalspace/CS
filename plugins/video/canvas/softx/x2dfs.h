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
    It is not a stand-alone include file.
*/

private:
  void EnterFullScreen ();
  void LeaveFullScreen ();

#ifdef XFREE86VM
  XF86VidModeModeInfo orig_mode;
  XF86VidModeModeInfo fs_mode;
  Window fs_window;
  int orig_x;
  int orig_y;
  void FindBestMode ();
#endif
