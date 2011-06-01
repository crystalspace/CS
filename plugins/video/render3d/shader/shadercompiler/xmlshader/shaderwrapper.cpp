/*
  Copyright (C) 2003-2006 by Marten Svanfeldt
		2004-2006 by Frank Richter

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

#include "shaderwrapper.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

CS_LEAKGUARD_IMPLEMENT (csXMLShaderWrapper);

csXMLShaderWrapper::csXMLShaderWrapper () :
    scfImplementationType (this), vp(), fp()
{
}

csXMLShaderWrapper::~csXMLShaderWrapper ()
{
}

void csXMLShaderWrapper::Activate ()
{
  if (vp) vp->Activate ();
  if (fp) fp->Activate ();
}

void csXMLShaderWrapper::Deactivate ()
{
  if (fp) fp->Deactivate ();
  if (vp) vp->Deactivate ();
}

bool csXMLShaderWrapper::Compile (iHierarchicalCache *cacheTo,
                                  csRef<iString> *cacheTag)
{
  // you're not supposed to compile a shader wrapper
  return false;
}

void csXMLShaderWrapper::GetUsedShaderVars (csBitArray& bits) const
{
  if (vp) vp->GetUsedShaderVars (bits);
  if (fp) fp->GetUsedShaderVars (bits);
}

bool csXMLShaderWrapper::Load (iShaderDestinationResolver *resolve,
                               const char *program,
                               csArray<csShaderVarMapping> &mappings)
{
  // you're not supposed to load a shader wrapper
  return false;
}

bool csXMLShaderWrapper::Load (iShaderDestinationResolver *resolve,
                               iDocumentNode *node)
{
  // you're not supposed to load a shader wrapper
  return false;
}

iShaderProgram::CacheLoadResult csXMLShaderWrapper::LoadFromCache (
  iHierarchicalCache *cache, iBase *previous, iDocumentNode *programNode,
  csRef<iString> *failReason, csRef<iString> *cacheTag)
{
  // you're not supposed to load a shader wrapper from cache
  return iShaderProgram::loadSuccessShaderValid; // hm.
}

void csXMLShaderWrapper::ResetState ()
{
  if (vp) vp->ResetState ();
  if (fp) fp->ResetState ();
}

void csXMLShaderWrapper::SetupState (const CS::Graphics::RenderMesh *mesh,
                                     CS::Graphics::RenderMeshModes &modes,
                                     const csShaderVariableStack &stack)
{
  if (vp) vp->SetupState (mesh, modes, stack);
  if (fp) fp->SetupState (mesh, modes, stack);
}

void csXMLShaderWrapper::SetVP (csRef<iShaderProgram> vp)
{
  this->vp = vp;
}

void csXMLShaderWrapper::SetFP (csRef<iShaderProgram> fp)
{
  this->fp = fp;
}

}
CS_PLUGIN_NAMESPACE_END(XMLShader)
