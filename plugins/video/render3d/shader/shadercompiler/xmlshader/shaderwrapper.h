/*
  Copyright (C) 2003-2006 by Marten Svanfeldt
		2005-2006 by Frank Richter

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

#ifndef __CS_SHADERWRAPPER_H__
#define __CS_SHADERWRAPPER_H__

#include "cssysdef.h"

#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/scfstr.h"
#include "csutil/stringreader.h"
#include "iutil/document.h"
#include "iutil/hiercache.h"
#include "iutil/string.h"
#include "ivideo/shader/shader.h"

#include "shader.h"
#include "xmlshader.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

class csXMLShaderWrapper :
  public scfImplementation1<csXMLShaderWrapper,
                            iShaderProgram>
{
private:
  // wrapped programs
  csRef<iShaderProgram> vp;
  csRef<iShaderProgram> fp;

public:
  CS_LEAKGUARD_DECLARE (csXMLShaderWrapper);

  csXMLShaderWrapper ();
  ~csXMLShaderWrapper ();

  virtual void Activate ()
  {
    if (vp) vp->Activate ();
    if (fp) fp->Activate ();
  }

  virtual bool Compile (iHierarchicalCache *cacheTo,
                        csRef<iString> *cacheTag = 0)
  {
    // you're not supposed to compile a shader wrapper
    return false;
  }

  virtual void Deactivate ()
  {
    if (fp) fp->Deactivate ();
    if (vp) vp->Deactivate ();
  }

  virtual void GetUsedShaderVars (csBitArray& bits) const
  {
      if (vp) vp->GetUsedShaderVars (bits);
      if (fp) vp->GetUsedShaderVars (bits);
  }

  virtual bool Load (iShaderDestinationResolver *resolve,
                     const char *program,
                     csArray<csShaderVarMapping> &mappings)
  {
    // you're not supposed to load a shader wrapper
    return false;
  }

  virtual bool Load (iShaderDestinationResolver *resolve,
                     iDocumentNode *node)
  {
    // you're not supposed to load a shader wrapper
    return false;
  }

  virtual iShaderProgram::CacheLoadResult LoadFromCache (
      iHierarchicalCache *cache, iBase *previous, iDocumentNode *programNode,
      csRef<iString> *failReason = 0, csRef<iString> *cacheTag = 0)
  {
    // you're not supposed to load a shader wrapper from cache
    return iShaderProgram::loadSuccessShaderValid; // hm.
  }

  virtual void ResetState ()
  {
      if (vp) vp->ResetState ();
      if (fp) fp->ResetState ();
  }

  virtual void SetupState (const CS::Graphics::RenderMesh *mesh,
                           CS::Graphics::RenderMeshModes &modes,
                           const csShaderVariableStack &stack)
  {
      if (vp) vp->SetupState (mesh, modes, stack);
      if (fp) fp->SetupState (mesh, modes, stack);
  }

  // wrapper specific functions: set vertex/fragment shaders
  void SetVP (csRef<iShaderProgram> vp);
  void SetFP (csRef<iShaderProgram> fp);
};

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif // __CS_SHADERWRAPPER_H__
