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
#include "sysdef.h"
#include "csutil/scf.h"
#include "emptydsp.h"
#include "cs2d/openglx/iogldisp.h"
#include "iplugin.h"
#include "isystem.h"

IMPLEMENT_FACTORY(csOpenGLEmptyDisp)

EXPORT_CLASS_TABLE(oglempty)
    EXPORT_CLASS( csOpenGLEmptyDisp, "crystalspace.graphics2d.glx.disp.empty", "Empty Displaydriver for Crystal Space GL/X")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE(csOpenGLEmptyDisp)
  IMPLEMENTS_INTERFACE(iPlugIn)
  IMPLEMENTS_INTERFACE(iOpenGLDisp)
IMPLEMENT_IBASE_END

csOpenGLEmptyDisp::csOpenGLEmptyDisp (iBase *iParent)
{
  CONSTRUCT_IBASE (iParent);
}

bool csOpenGLEmptyDisp::open()
{
  return true;
}

bool csOpenGLEmptyDisp::close()
{
  return true;
}

