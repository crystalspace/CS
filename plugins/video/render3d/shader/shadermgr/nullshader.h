/*
  Copyright (C) 2005 by Jorrit Tyberghein
	    (C) 2005 by Frank Richter

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef __CS_NULLSHADER_H__
#define __CS_NULLSHADER_H__

#include "csutil/csobject.h"
#include "ivideo/shader/shader.h"

class csNullShader : public iShader, public csObject
{
  csShaderMetadata allShaderMeta;
  csRefArray<csShaderVariable> dummySVs;
public:
  CS_LEAKGUARD_DECLARE (csNullShader);

  SCF_DECLARE_IBASE_EXT (csObject);

  csNullShader () { }
  virtual ~csNullShader () { }

  virtual iObject* QueryObject () 
  { return (iObject*)(csObject*)this; }

  const char* GetFileName () { return 0; }
  void SetFileName (const char* filename) {  }

  virtual size_t GetTicket (const csRenderMeshModes&, 
    const csShaderVarStack&) { return 0; }

  virtual size_t GetNumberOfPasses (size_t) { return 0; }
  virtual bool ActivatePass (size_t, size_t) { return false; }
  virtual bool SetupPass (size_t, const csRenderMesh*,
    csRenderMeshModes&, const csShaderVarStack &)
  { return false; }
  virtual bool TeardownPass (size_t)
  { return false; }
  virtual bool DeactivatePass (size_t) { return false; }
  virtual const csShaderMetadata& GetMetadata (size_t) const
  { return allShaderMeta; }

  //=================== iShaderVariableContext ================//
  void AddVariable (csShaderVariable *) { }
  csShaderVariable* GetVariable (csStringID) const { return 0; }
  const csRefArray<csShaderVariable>& GetShaderVariables () const
  { return dummySVs; }
  void PushVariables (csShaderVarStack &) const { }
  void PopVariables (csShaderVarStack &) const { }

  bool IsEmpty() const { return true; }
  void ReplaceVariable (csShaderVariable*) {}
  void Clear () { }
};

#endif // __CS_NULLSHADER_H__
