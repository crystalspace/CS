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

#include <stdarg.h>
#include "cssysdef.h"
#include "csutil/scf.h"
#include "glidedsp.h"
#include "../iogldisp.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

SCF_IMPLEMENT_FACTORY(csOpenGLGlideDisp)

SCF_EXPORT_CLASS_TABLE(oglglide)
    SCF_EXPORT_CLASS( csOpenGLGlideDisp, "crystalspace.graphics2d.glx.disp.glide", "Glide Displaydriver for Crystal Space GL/X")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE(csOpenGLGlideDisp)
  SCF_IMPLEMENTS_INTERFACE(iComponent)
  SCF_IMPLEMENTS_INTERFACE(iEventHandler)
  SCF_IMPLEMENTS_INTERFACE(iOpenGLDisp)
SCF_IMPLEMENT_IBASE_END

csOpenGLGlideDisp::csOpenGLGlideDisp (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
}

bool csOpenGLGlideDisp::Initialize(iObjectRegistry *)
{
   // This environment setting prevents fxmesa from setting atexit handlers
   // which when used in conjunction with scf (dlopen) causes at least with
   // the voodoo2 crashes on exit which trash the Xserver! This environment
   // setting is new with Mesa3.2.
   setenv ("MESA_FX_NO_SIGNALS", "1", 0);
   return true;
}

bool csOpenGLGlideDisp::open()
{
  // no special fx required ( e.g. in mesa this is done by setting MESA_GLX_FX=fullscreen in the enviroment )
  return true;
}

bool csOpenGLGlideDisp::close()
{
  grGlideShutdown();
  return true;
}

