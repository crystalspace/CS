/*
    Copyright (C) 2002 by Mårten Svanfeldt
                          Anders Stenberg

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
#include "csgfx/shadervar.h"

static int ShaderVariableWrapperCompare (csShaderVariableProxy const &item1,
                                         csShaderVariableProxy const &item2)
{
  if (item1.Name < item2.Name) return -1;
  else if (item1.Name > item2.Name) return 1;
  else return 0;
}


csShaderVariable::csShaderVariable (csStringID name) : csRefCount (), 
  String(0), TextureHandValue(0), TextureWrapValue(0), VectorValue(0), 
  Name (name) 
{
}


int csShaderVariableList::InsertSorted (csShaderVariableProxy item)
{
  return csArray<csShaderVariableProxy>::InsertSorted (item, ShaderVariableWrapperCompare);
}

int csShaderVariableList::Push (csShaderVariableProxy item)
{
  return csArray<csShaderVariableProxy>::InsertSorted (item, ShaderVariableWrapperCompare);
}
