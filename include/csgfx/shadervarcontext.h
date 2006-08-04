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

#ifndef __CS_CSGFX_SHADERVARCONTEXT_H__
#define __CS_CSGFX_SHADERVARCONTEXT_H__

#include "csextern.h"

#include "csutil/scf_implementation.h"

#include "ivideo/shader/shader.h"
#include "shadervar.h"

/**\file
 * Simple implementation for iShaderVariableContext.
 */

namespace CS
{
  /**
   * Simple implementation for iShaderVariableContext.
   * Can be inherited from for use in SCF classes. For an example,
   * see csShaderVariableContext.
   */
  class CS_CRYSTALSPACE_EXPORT ShaderVariableContextImpl :
    public iShaderVariableContext
  {
  protected:
    csRefArray<csShaderVariable> variables;

  public:
    virtual ~ShaderVariableContextImpl();

    const csRefArray<csShaderVariable>& GetShaderVariables () const
    { return variables; }
    virtual void AddVariable (csShaderVariable *variable);
    virtual csShaderVariable* GetVariable (csStringID name) const;
    virtual void PushVariables (iShaderVarStack* stacks) const;
    virtual bool IsEmpty() const { return variables.Length() == 0; }  
    virtual void ReplaceVariable (csShaderVariable *variable);
    virtual void Clear () { variables.Empty(); }
  };
}

/**
 * Complete SCF class implementing iShaderVariableContext.
 */
class CS_CRYSTALSPACE_EXPORT csShaderVariableContext :
  public scfImplementation1<csShaderVariableContext, 
			     scfFakeInterface<iShaderVariableContext> >,
  public CS::ShaderVariableContextImpl
{
public:
  CS_LEAKGUARD_DECLARE (csShaderVariableContext);

  csShaderVariableContext ();
  csShaderVariableContext (const csShaderVariableContext& other);
  virtual ~csShaderVariableContext ();
};

#endif // __CS_CSGFX_SHADERVARCONTEXT_H__
