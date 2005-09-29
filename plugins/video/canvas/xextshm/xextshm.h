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

#ifndef __CS_XEXTSHM_H__
#define __CS_XESTSHM_H__

#include <stdarg.h>
#include "csutil/scf.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "plugins/video/canvas/xwindowcommon/xextshm.h"
#include "ivideo/graph2d.h"

#define XK_MISCELLANY 1
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

extern "C" {
#  include <X11/extensions/XShm.h>
#  include <sys/ipc.h>
#  include <sys/shm.h>
}

class csXExtSHM : public iXExtSHM
{
  /// The Object Registry
  iObjectRegistry *object_reg;
  /// The X-display
  Display* dpy;
  /// The Screen Number (not necessarilly the default)
  int screen_num;

  XShmSegmentInfo shmi;
  XImage *shm_image;

  int Width, Height;

public:
  SCF_DECLARE_IBASE;

  csXExtSHM (iBase*);
  virtual ~csXExtSHM ();

  virtual bool Initialize (iObjectRegistry*);
  void Report (int severity, const char* msg, ...);
  virtual void SetDisplayScreen (Display *dpy, int screen_num)
  { this->dpy = dpy; this->screen_num = screen_num; }
  virtual unsigned char *CreateMemory (int Width, int Height);
  virtual void DestroyMemory ();
  virtual void Print (Window window, GC gc, csRect const* area);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csXExtSHM);
    virtual bool Initialize (iObjectRegistry *o)
    { return scfParent->Initialize(o); }
  } scfiComponent;

};

#endif // __CS_XEXTSHM_H__
