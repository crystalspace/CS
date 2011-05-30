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

  virtual void Activate ();
  virtual void Deactivate ();

  virtual bool Compile (iHierarchicalCache *cacheTo,
                        csRef<iString> *cacheTag = 0);

  virtual void GetUsedShaderVars (csBitArray& bits) const;

  virtual bool Load (iShaderDestinationResolver *resolve,
                     const char *program,
                     csArray<csShaderVarMapping> &mappings);
  virtual bool Load (iShaderDestinationResolver *resolve,
                     iDocumentNode *node);

  virtual iShaderProgram::CacheLoadResult LoadFromCache (
      iHierarchicalCache *cache, iBase *previous, iDocumentNode *programNode,
      csRef<iString> *failReason = 0, csRef<iString> *cacheTag = 0);

  virtual void ResetState ();

  virtual void SetupState (const CS::Graphics::RenderMesh *mesh,
                           CS::Graphics::RenderMeshModes &modes,
                           const csShaderVariableStack &stack);

  // wrapper specific functions: set vertex/fragment shaders
  void SetVP (csRef<iShaderProgram> vp);
  void SetFP (csRef<iShaderProgram> fp);
};

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif // __CS_SHADERWRAPPER_H__
