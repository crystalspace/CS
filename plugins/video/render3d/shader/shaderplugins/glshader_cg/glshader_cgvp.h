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

#ifndef __GLSHADER_CGVP_H__
#define __GLSHADER_CGVP_H__

#include "../../common/shaderplugin.h"
#include "../common/shaderprogram.h"
#include "csgfx/shadervarcontext.h"
#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#include "glshader_cgcommon.h"

class csShaderGLCGVP : public csShaderGLCGCommon
{
  struct matrixtrackerentry
  {
    GLenum nvMatrix;
    GLenum nvTransform;
    GLuint nvAddress;
  };

  bool nvTrackMatrices;
  csArray<matrixtrackerentry> matrixtrackers;
  csRef<iShaderProgram> override;
public:
  csShaderGLCGVP (csGLShader_CG* shaderPlug) : 
    csShaderGLCGCommon (shaderPlug, "cgvp") { }

  /// Sets this program to be the one used when rendering
  virtual void Activate ();

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate();

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (csRenderMesh* mesh,
    const csShaderVarStack &stacks);

  /// Reset states to original
  virtual void ResetState ();

  /// Compile a program
  virtual bool Compile(csArray<iShaderVariableContext*> &staticContexts);
};


#endif //__GLSHADER_CGVP_H__
