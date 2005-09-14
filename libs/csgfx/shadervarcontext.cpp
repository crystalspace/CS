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
#include "csgfx/shadervarcontext.h"

CS_LEAKGUARD_IMPLEMENT (csShaderVariableContext);

csShaderVariableContext::csShaderVariableContext () :
  scfImplementationType(this, 0)
{}

csShaderVariableContext::~csShaderVariableContext ()
{}

class SvVarArrayCmp : public csArrayCmp<csShaderVariable*, csStringID>
{
  static int SvKeyCompare (csShaderVariable* const& r, csStringID const& k)
  { 
    return r->GetName() - k;
  }
public:
  SvVarArrayCmp (csStringID key) : 
    csArrayCmp<csShaderVariable*, csStringID> (key, SvKeyCompare)
  {
  }
};

static int SvCompare (csShaderVariable* const& r, csShaderVariable* const& k)
{ 
  return r->GetName() - k->GetName();
}

void csShaderVariableContext::AddVariable 
  (csShaderVariable *variable) 
{
  csShaderVariable* var = GetVariable (variable->GetName());
  if (var == 0)
    variables.InsertSorted (variable, SvCompare);
  else
    *var = *variable;
}

csShaderVariable* csShaderVariableContext::GetVariable 
  (csStringID name) const 
{
  size_t index = variables.FindSortedKey (SvVarArrayCmp (name));
  if (index != csArrayItemNotFound)
    return variables[index];
  return 0;
}

void csShaderVariableContext::PushVariables 
  (csShaderVarStack &stacks) const
{
  for (size_t i=0; i<variables.GetSize (); ++i)
  {
    csStringID name = variables[i]->GetName ();
    if (stacks.GetSize () <= (size_t)name)
      stacks.SetSize (name+1, 0);
    stacks[name] = variables[i];
  }
}

void csShaderVariableContext::ReplaceVariable 
  (csShaderVariable *variable) 
{
  size_t index = variables.FindSortedKey (SvVarArrayCmp (
    variable->GetName()));
  if (index != csArrayItemNotFound)
    variables.Put (index, variable);
  else
    variables.InsertSorted (variable, SvCompare);
}
