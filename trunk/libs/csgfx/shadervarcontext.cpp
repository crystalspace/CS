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

namespace CS
{
namespace Graphics
{

ShaderVariableContextImpl::~ShaderVariableContextImpl ()
{}

namespace
{
class SvVarArrayCmp : public csArrayCmp<csShaderVariable*, 
  CS::ShaderVarStringID>
{
  static int SvKeyCompare (csShaderVariable* const& r,
		                         CS::ShaderVarStringID const& k)
  { 
    return r->GetName() - k;
  }
public:
  SvVarArrayCmp (CS::ShaderVarStringID key) : 
    csArrayCmp<csShaderVariable*, CS::ShaderVarStringID> (key, SvKeyCompare)
  {
  }
};

static int SvCompare (csShaderVariable* const& r, csShaderVariable* const& k)
{ 
  return r->GetName() - k->GetName();
}
}

void ShaderVariableContextImpl::AddVariable (csShaderVariable *variable) 
{
  csShaderVariable* var = GetVariable (variable->GetName());
  if (var == 0)
    variables.InsertSorted (variable, SvCompare);
  else
    *var = *variable;
}

csShaderVariable* ShaderVariableContextImpl::GetVariable (
  ShaderVarStringID name) const 
{
  size_t index = variables.FindSortedKey (SvVarArrayCmp (name));
  if (index != csArrayItemNotFound)
    return variables[index];
  return 0;
}

void ShaderVariableContextImpl::PushVariables (
  csShaderVariableStack& stack) const
{
  for (size_t i = 0; i < variables.GetSize (); ++i)
  {
    ShaderVarStringID name = variables[i]->GetName ();
    if (name >= stack.GetSize ())
      /* Not really an error: can happen if new shader vars are created
       * after the stack was set up */
      return;
    stack[name] = variables[i];
  }
}

void ShaderVariableContextImpl::ReplaceVariable (csShaderVariable *variable) 
{
  size_t index = variables.FindSortedKey (SvVarArrayCmp (
    variable->GetName()));
  if (index != csArrayItemNotFound)
    variables.Put (index, variable);
  else
    variables.InsertSorted (variable, SvCompare);
}

bool ShaderVariableContextImpl::RemoveVariable (csShaderVariable* variable)
{
  return variables.Delete (variable);
}

bool ShaderVariableContextImpl::RemoveVariable (ShaderVarStringID name)
{
  size_t index = variables.FindSortedKey (SvVarArrayCmp (name));
  if (index != csArrayItemNotFound)
  {
      return variables.DeleteIndex (index);
  }
  return false;
}

} // namespace Graphics
} // namespace CS

//---------------------------------------------------------------------------

CS_LEAKGUARD_IMPLEMENT (csShaderVariableContext);

csShaderVariableContext::csShaderVariableContext () :
  scfImplementationType (this, 0)
{}

csShaderVariableContext::csShaderVariableContext (
  const csShaderVariableContext& other) :
  iBase (), iShaderVariableContext(),
  scfImplementationType(this), CS::ShaderVariableContextImpl ()
{
  variables = other.variables;
}

csShaderVariableContext::~csShaderVariableContext ()
{}
