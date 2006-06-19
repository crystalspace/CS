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

#ifndef __SOFTSHADER_FP_H__
#define __SOFTSHADER_FP_H__

#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"
#include "csgfx/shadervarcontext.h"
#include "csplugincommon/shader/shaderplugin.h"
#include "csplugincommon/shader/shaderprogram.h"

#include "softshader.h"

CS_PLUGIN_NAMESPACE_BEGIN(SoftShader)
{

class csSoftShader_FP : public csShaderProgram
{
private:
  csSoftShader* shaderPlug;

  csStringHash tokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/video/render3d/shader/shaderplugins/softshader/softshader_fp.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE

  ProgramParam flatColor;
  ProgramParam cfactor;
  ProgramParam afactor;
  bool colorSum;
public:
  csSoftShader_FP (csSoftShader* shaderPlug) : 
    csShaderProgram(shaderPlug->object_reg), shaderPlug(shaderPlug), 
    colorSum (false)
  {
    InitTokenTable (tokens);
  }
  virtual ~csSoftShader_FP ()
  {
  }

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate();

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate();

  virtual void SetupState (const csRenderMesh* mesh,
    csRenderMeshModes& modes,
    const iShaderVarStack* stacks);

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

}
CS_PLUGIN_NAMESPACE_END(SoftShader)

#endif //__SOFTSHADER_FP_H__
