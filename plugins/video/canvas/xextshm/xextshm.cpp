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
#include "csutil/scf.h"
#include "ivaria/reporter.h"
#include "isys/system.h"
#include "csgeom/csrect.h"
#include "xextshm.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csXExtSHM)

SCF_EXPORT_CLASS_TABLE (xextshm)
  SCF_EXPORT_CLASS (csXExtSHM, "crystalspace.window.x.extshm",
    "X-Window Shared Memory Extension plugin for Crystal Space")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE(csXExtSHM)
  SCF_IMPLEMENTS_INTERFACE(iXExtSHM)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csXExtSHM::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csXExtSHM::csXExtSHM (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPlugin);
  dpy = NULL;
  screen_num = 0;
  Width = Height = 0;
}

csXExtSHM::~csXExtSHM ()
{

}

bool csXExtSHM::Initialize (iObjectRegistry *object_reg)
{
  this->object_reg = object_reg;


  return true;
}


void csXExtSHM::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
    rep->ReportV (severity, "crystalspace.window.x.extshm", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}


unsigned char *csXExtSHM::CreateMemory (int Width, int Height)
{
  int disp_depth = DefaultDepth(dpy,screen_num);
  int bitmap_pad = (disp_depth + 7) / 8;

  bitmap_pad = (bitmap_pad == 3) ? 32 : bitmap_pad*8;

  XImage *xim = XShmCreateImage(dpy, DefaultVisual(dpy,screen_num), 
				disp_depth,
				ZPixmap, 0, 
				&shmi, Width, Height);
  if (!xim)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "XShmCreateImage failed!");
    return NULL;
  }
  shm_image = *xim;
  shmi.shmid = shmget (IPC_PRIVATE, xim->bytes_per_line*xim->height,
		       IPC_CREAT | 0777);
  if (shmi.shmid == -1)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "shmget failed!");
    return NULL;
  }
  shmi.shmaddr = (char*)shmat (shmi.shmid, 0, 0);
  if (shmi.shmaddr == (char*) -1)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "shmat failed!");
    return NULL;
  }
  shmi.readOnly = FALSE;
  XShmAttach (dpy, &shmi);

  // Delete memory segment. The memory stays available until
  // the last client detaches from it.
  XSync (dpy, False);
  shmctl (shmi.shmid, IPC_RMID, 0);
  
  shm_image.data = shmi.shmaddr;
  shm_image.obdata = (char *)&shmi;

  this->Width = Width;
  this->Height = Height;

  return (unsigned char *)shmi.shmaddr;
}

void csXExtSHM::DestroyMemory ()
{
  XShmDetach (dpy, &shmi);
  shmdt (shmi.shmaddr);
}

void csXExtSHM::Print (Window window, GC gc, csRect *area)
{
  if (area)
    XShmPutImage (dpy, window, gc, &shm_image,
		  area->xmin, area->ymin, area->xmin, area->ymin,
		  area->Width (), area->Height (),
		  False);
  else
    XShmPutImage (dpy, 
		  window, gc, 
		  &shm_image, 
		  0, 0, 0, 0, Width, Height, 
		  False);
  XSync (dpy, False);
}
