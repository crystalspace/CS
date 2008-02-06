/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005 by Frank Richter

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

#include "csgfx/shadervarframeholder.h"

csRef<csShaderVariable> csShaderVariableFrameHolder::AllocFrameSv (uint framenr)
{
  bool created;
  csRef<csShaderVariable>& sv = svCache.GetUnusedData (created, framenr);
  if (created) sv = svAlloc.Alloc();
  return sv;
}

csShaderVariableFrameHolder::~csShaderVariableFrameHolder ()
{
  // Paranoia: make sure the SV cache classes are destructed in the right order
  svCache.Clear (true);
  // ... now no references to svAlloc-allocated SVs should be remain
}

csRef<csShaderVariable> csShaderVariableFrameHolder::GetFrameUniqueSV (
  uint framenr, csStringID name)
{
  csRef<csShaderVariable> sv = AllocFrameSv (framenr);
  sv->SetName (name);
  return sv;
}
