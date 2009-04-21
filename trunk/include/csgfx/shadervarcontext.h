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
  namespace Graphics
  {
    /**
     * Simple implementation for iShaderVariableContext.
     * Can be inherited from for use in SCF classes. For an example,
     * see csShaderVariableContext.
     */
    class CS_CRYSTALSPACE_EXPORT ShaderVariableContextImpl :
      public virtual iShaderVariableContext
    {
    protected:
      csRefArray<csShaderVariable> variables;
  
    public:
      virtual ~ShaderVariableContextImpl();
  
      const csRefArray<csShaderVariable>& GetShaderVariables () const
      { return variables; }
      virtual void AddVariable (csShaderVariable *variable);
      virtual csShaderVariable* GetVariable (ShaderVarStringID name) const;
      virtual void PushVariables (csShaderVariableStack& stacks) const;
      virtual bool IsEmpty() const { return variables.GetSize () == 0; }  
      virtual void ReplaceVariable (csShaderVariable *variable);
      virtual void Clear () { variables.Empty(); }
      virtual bool RemoveVariable (csShaderVariable* variable);
      virtual bool RemoveVariable (ShaderVarStringID name);
    };
    
    /**
     * iShaderVariableContext implementation that overlays (or merges) it's 
     * variables over the variables of a given parent context.
     */
    class OverlayShaderVariableContextImpl : public ShaderVariableContextImpl
    {
      csRef<iShaderVariableContext> parentSVC;
    public:
      OverlayShaderVariableContextImpl (iShaderVariableContext* parent = 0) :
	parentSVC (parent) {}
	  
      iShaderVariableContext* GetParentContext() const { return parentSVC; }
      void SetParentContext (iShaderVariableContext* parent)
      { parentSVC = parent; }
  
      void AddVariable (csShaderVariable *variable)
      { ShaderVariableContextImpl::AddVariable (variable); }
      csShaderVariable* GetVariable (ShaderVarStringID name) const
      { 
	csShaderVariable* sv = ShaderVariableContextImpl::GetVariable (name); 
	if ((sv == 0) && (parentSVC.IsValid()))
	  sv = parentSVC->GetVariable (name);
	return sv;
      }
      const csRefArray<csShaderVariable>& GetShaderVariables () const
      { 
	// @@@ Will not return parent SVs
	return ShaderVariableContextImpl::GetShaderVariables (); 
      }
      void PushVariables (csShaderVariableStack& stacks) const
      { 
	if (parentSVC.IsValid()) parentSVC->PushVariables (stacks);
	ShaderVariableContextImpl::PushVariables (stacks); 
      }
      bool IsEmpty () const 
      {
	return ShaderVariableContextImpl::IsEmpty() 
	  && (!parentSVC.IsValid() || parentSVC->IsEmpty());
      }
      void ReplaceVariable (csShaderVariable *variable)
      { ShaderVariableContextImpl::ReplaceVariable (variable); }
      void Clear () { ShaderVariableContextImpl::Clear(); }
      bool RemoveVariable (csShaderVariable* variable)
      { 
	// @@@ Also remove from parent?
	return ShaderVariableContextImpl::RemoveVariable (variable); 
      }
      bool RemoveVariable (ShaderVarStringID name)
      { 
	// @@@ Also remove from parent?
	return ShaderVariableContextImpl::RemoveVariable (name); 
      }
    };
  } // namespace Graphics
  
  // Deprecated in 1.3
  typedef CS_DEPRECATED_TYPE_MSG("Use Graphics::ShaderVariableContextImpl")
    Graphics::ShaderVariableContextImpl ShaderVariableContextImpl;
}

/**
 * Complete SCF class implementing iShaderVariableContext.
 */
class CS_CRYSTALSPACE_EXPORT csShaderVariableContext :
  public scfImplementation1<csShaderVariableContext, 
			     scfFakeInterface<iShaderVariableContext> >,
  public CS::Graphics::ShaderVariableContextImpl
{
public:
  CS_LEAKGUARD_DECLARE (csShaderVariableContext);

  csShaderVariableContext ();
  csShaderVariableContext (const csShaderVariableContext& other);
  virtual ~csShaderVariableContext ();
};

#endif // __CS_CSGFX_SHADERVARCONTEXT_H__
