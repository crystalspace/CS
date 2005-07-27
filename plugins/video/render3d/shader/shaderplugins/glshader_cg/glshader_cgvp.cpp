/*
Copyright (C) 2002 by Marten Svanfeldt
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

#include "csgeom/vector3.h"
#include "csplugincommon/opengl/glextmanager.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scanstr.h"
#include "csutil/scf.h"
#include "csutil/stringreader.h"
#include "iutil/document.h"
#include "iutil/string.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "iutil/databuff.h"

#include "glshader_cgvp.h"
#include "glshader_cg.h"

CS_LEAKGUARD_IMPLEMENT (csShaderGLCGVP);

bool csShaderGLCGVP::Compile ()
{
  csRef<iDataBuffer> programBuffer = GetProgramData();
  if (!programBuffer.IsValid())
    return false;
  csString programStr;
  programStr.Append ((char*)programBuffer->GetData(), programBuffer->GetSize());

  if (!DefaultLoadProgram (programStr, CG_GL_VERTEX, false, false))
    return false;

  csString compiledProgram = cgGetProgramString (program, 
    CG_COMPILED_PROGRAM);

  cgDestroyProgram (program);

  if (shaderPlug->doVerbose)
  {
    shaderPlug->Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Rewriting program...");
  }

  size_t insertionPoint = compiledProgram.Find ("PARAM");
  size_t constPoint = compiledProgram.Find ("#const");
  while (constPoint != (size_t)-1)
  {
    int constNum;
    float values[4] = {0, 0, 0, 0};
    csScanStr (compiledProgram.GetData() + constPoint,
      "#const c[%d] = %f %f %f %f", &constNum, 
      &values[0], &values[1], &values[2], &values[3]);
    csString paramString;
    compiledProgram.Insert (insertionPoint, 
      paramString.Format ("PARAM c_%d = {%f, %f, %f, %f};\n", 
        constNum, values[0], values[1], values[2], values[3]));
    csString find, replace;

    // Change all usages of c[N].
    find.Format ("c[%d]", constNum);
    replace.Format ("c_%d", constNum);
    compiledProgram.FindReplace (find, replace);

    // Change the #const comment back, to avoid CG errors.
    find.Format ("#const c_%d", constNum);
    replace.Format ("#const c[%d]", constNum);
    compiledProgram.FindReplace (find, replace);

    constPoint = compiledProgram.Find ("#const", constPoint+1);
  }

  if (!DefaultLoadProgram (compiledProgram, CG_GL_VERTEX, true))
    return false;

  return true;
}

