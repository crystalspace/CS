/*
    Null 2d canvas for Crystal Space (source)
    Copyright (C) 2002 by Matze Braun <MatzeBraun@gmx.de>

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
#include "null2d.h"
#include "csgeom/csrect.h"
#include "csutil/csinput.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics2DNull)


csGraphics2DNull::csGraphics2DNull(iBase* iParent)
    : csGraphics2D(iParent)
{
}

csGraphics2DNull::~csGraphics2DNull()
{
}

bool csGraphics2DNull::Initialize (iObjectRegistry* object_reg)
{
    if (!csGraphics2D::Initialize(object_reg))
	return false;

    pfmt.RedMask = 0xf800;
    pfmt.GreenMask = 0x07e0;
    pfmt.BlueMask = 0x001f;

    pfmt.complete ();
    pfmt.PalEntries = 0;
    pfmt.PixelBytes = 2;
      
    return true;		
}

bool csGraphics2DNull::Open()
{
    return csGraphics2D::Open();
}

void csGraphics2DNull::Close()
{
    csGraphics2D::Close();
}

bool csGraphics2DNull::BeginDraw()
{
    return csGraphics2D::BeginDraw();
}

void csGraphics2DNull::FinishDraw()
{
    csGraphics2D::FinishDraw();
}

void csGraphics2DNull::Print(csRect const*)
{
}

void csGraphics2DNull::SetRGB(int,int,int,int)
{
}

bool csGraphics2DNull::SetMousePosition (int,int)
{
    return true;
}

bool csGraphics2DNull::SetMouseCursor (csMouseCursorID)
{
    return true;
}

