/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#include "cssysdef.h"

#include "csplugincommon/renderstep/basesteptype.h"

SCF_IMPLEMENT_IBASE (csBaseRenderStepType)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
  SCF_IMPLEMENTS_INTERFACE (iRenderStepType)
SCF_IMPLEMENT_IBASE_END;

csBaseRenderStepType:: csBaseRenderStepType (iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
}

csBaseRenderStepType::~csBaseRenderStepType ()
{
  SCF_DESTRUCT_IBASE();
}

bool csBaseRenderStepType::Initialize(iObjectRegistry *object_reg)
{
  csBaseRenderStepType::object_reg = object_reg;

  return true;
}
