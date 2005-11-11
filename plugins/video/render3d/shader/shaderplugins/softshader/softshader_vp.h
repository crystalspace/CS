/*
Copyright (C) 2002 by Anders Stenberg
                      Marten Svanfeldt

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

#ifndef __SOFTSHADER_VP_H__
#define __SOFTSHADER_VP_H__

#include "csplugincommon/shader/shaderplugin.h"
#include "csplugincommon/shader/shaderprogram.h"
#include "csutil/strhash.h"
#include "ivideo/shader/shader.h"

struct iDataBuffer;

namespace cspluginSoftshader
{

class csSoftShader_VP : public csShaderProgram
{
public:
  csSoftShader_VP(iObjectRegistry* objreg) : csShaderProgram(objreg)
  {
  }
  virtual ~csSoftShader_VP ()
  {
  }

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate();

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate();

  virtual void SetupState (const csRenderMesh* /*mesh*/,
    csRenderMeshModes& /*modes*/,
    const csShaderVarStack& /*stacks*/) {}

  virtual void ResetState () {}

  /// Loads from a document-node
  virtual bool Load (iShaderDestinationResolver*, iDocumentNode* node);

  /// Loads from raw text
  virtual bool Load (iShaderDestinationResolver*, const char*, 
    csArray<csShaderVarMapping> &)
  { return false; }

  /// Compile a program
  virtual bool Compile();
};

} // namespace cspluginSoftshader

#endif //__SOFTSHADER_VP_H__

