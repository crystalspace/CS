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

#include "cstool/shaderbranch.h"

csBaseShaderBranch::~csBaseShaderBranch ()
{
}

void csBaseShaderBranch::AddChild (iShaderBranch *child)
{
  symtab.AddChild (child->GetSymbolTable ());
}

void csBaseShaderBranch::AddVariable (csShaderVariable* variable)
{
  symtab.SetSymbol (variable->GetName (), variable);
}

csShaderVariable* csBaseShaderBranch::GetVariable (csStringID name)
{
  return (csShaderVariable*)symtab.GetSymbol (name);
}

csSymbolTable* csBaseShaderBranch::GetSymbolTable ()
{
  return &symtab;
}

csSymbolTable* csBaseShaderBranch::GetSymbolTable (int index)
{
  return &symtab;
}

void csBaseShaderBranch::SelectSymbolTable (int index)
{
}

//--------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csShaderBranch)
  SCF_IMPLEMENTS_INTERFACE(iShaderBranch)
SCF_IMPLEMENT_IBASE_END

csShaderBranch::csShaderBranch()
{
  SCF_CONSTRUCT_IBASE(0);
}

