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

#ifndef __GLSHADER_CGCOMMON_H__
#define __GLSHADER_CGCOMMON_H__

#include "../../common/shaderplugin.h"
#include "../common/shaderprogram.h"
#include "csgfx/shadervarcontext.h"
#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"

#include <Cg/cg.h>
#include <Cg/cgGL.h>

class csGLShader_CG;

class csShaderGLCGCommon : public csShaderProgram
{
protected:
  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "video/render3d/shader/shaderplugins/glshader_cg/glshader_cg.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE

  struct matrixtrackerentry
  {
    CGGLenum matrix;
    CGGLenum modifier;
    CGparameter parameter;
  };

  csArray<matrixtrackerentry> matrixtrackers;

  csGLShader_CG* shaderPlug;

  CGprogram program;
  const char* cg_profile;
  const char* entrypoint;

  bool validProgram;

  bool DefaultLoadProgram (const char* programStr, CGGLenum type,
    csArray<iShaderVariableContext*> &staticContexts);
public:
  csShaderGLCGCommon (csGLShader_CG* shaderPlug);
  virtual ~csShaderGLCGCommon ();

  void SetValid(bool val) { validProgram = val; }

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate ();

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate();

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (csRenderMesh* mesh,
    const csShaderVarStack &stacks);

  /// Reset states to original
  virtual void ResetState ();

  /// Check if valid
  virtual bool IsValid () { return validProgram;} 

  /// Loads from a document-node
  virtual bool Load (iDocumentNode* node);

  /// Loads from raw text
  virtual bool Load (const char* program, csArray<csShaderVarMapping> &mappings)
  { return false; }

  /**
   * Prepares the shaderprogram for usage. Must be called before the shader
   * is assigned to a material.
   */
  //virtual bool Prepare(iShaderPass *pass);
};


#endif //__GLSHADER_CGCOMMON_H__
