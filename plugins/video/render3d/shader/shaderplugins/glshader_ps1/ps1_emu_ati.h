/*
  Copyright (C) 2002 by John Harger

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

#ifndef __GLSHADER_PS1_ATI_H__
#define __GLSHADER_PS1_ATI_H__

#include "csplugincommon/shader/shaderplugin.h"
#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"
#include "ps1_emu_common.h"
#include "ps1_parser.h"

class csShaderGLPS1_ATI : public csShaderGLPS1_Common
{
private:
  GLuint program_num;

  bool GetATIShaderCommand (const csPSProgramInstruction &instruction);
public:
  csShaderGLPS1_ATI (csGLShader_PS1* shaderPlug)
    : csShaderGLPS1_Common(shaderPlug)
  {
  }
  virtual ~csShaderGLPS1_ATI ()
  {
  }

  bool LoadProgramStringToGL ();

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate ();

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate();

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (const csRenderMesh* mesh,
    csRenderMeshModes& modes,
    const csShaderVarStack &stacks);

  /// Reset states to original
  virtual void ResetState ();
};


#endif //__GLSHADER_PS1_ATI_H__

