/*
Copyright (C) 2002 by Mårten Svanfeldt
                      Anders Stenberg

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

#include "cssysdef.h"

#include "csutil/csvector.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csgeom/vector3.h"

#include "ivaria/reporter.h"
#include "ivideo/render3d.h"
#include "ivideo/shader/shader.h"

#include "../../opengl/glextmanager.h"

#include "glshader_avp.h"

SCF_IMPLEMENT_IBASE(csShaderGLAVP)
SCF_IMPLEMENTS_INTERFACE(iShaderProgram)
SCF_IMPLEMENT_IBASE_END

void csShaderGLAVP::Activate()
{
  glEnable(GL_VERTEX_PROGRAM_ARB);
  ext->glBindProgramARB(GL_VERTEX_PROGRAM_ARB, program_num);
}

void csShaderGLAVP::Deactivate()
{
  glDisable (GL_VERTEX_PROGRAM_ARB);
}

bool csShaderGLAVP::LoadProgram(const char* programstring)
{
  if(!ext)
    return false;

  if(!ext->CS_GL_ARB_vertex_program)
    return false;

  ext->glGenProgramsARB(1, &program_num);
  ext->glBindProgramARB(GL_VERTEX_PROGRAM_ARB, program_num);
  
  ext->glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, 
    strlen(programstring), (void*) programstring);

  const GLubyte * programErrorString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);

  int errorpos;
  glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorpos);
  if(errorpos != -1)
  {
    csReport ( object_reg, CS_REPORTER_SEVERITY_WARNING,"crystalspace.render3d.shader.glarb",
      "Couldn't load vertexprogram", NULL);
    csReport ( object_reg, CS_REPORTER_SEVERITY_WARNING,"crystalspace.render3d.shader.glarb",
      "Programerror at %d", errorpos);
    csReport ( object_reg, CS_REPORTER_SEVERITY_WARNING,"crystalspace.render3d.shader.glarb",
      "Errorstring %s", programErrorString);
    return false;
  }

  return true;
}