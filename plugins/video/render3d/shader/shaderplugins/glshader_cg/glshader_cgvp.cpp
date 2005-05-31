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
#include "csutil/scf.h"
#include "csutil/stringreader.h"
#include "iutil/document.h"
#include "iutil/string.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "glshader_cgvp.h"
#include "glshader_cg.h"

CS_LEAKGUARD_IMPLEMENT (csShaderGLCGVP);

void csShaderGLCGVP::Activate()
{
  csShaderGLCGCommon::Activate();

  if (cgTrackMatrices)
  {
    for (size_t i = 0; i < cgMatrixTrackers.Length(); ++i)
    {
      const CGMatrixTrackerEntry& mt = cgMatrixTrackers[i];

      cgGLSetStateMatrixParameter(mt.cgParameter, mt.cgMatrix, 
	mt.cgTransform);
    }
  }
  if (nvTrackMatrices)
  {
    for (size_t i = 0; i < nvMatrixTrackers.Length(); ++i)
    {
      const NVMatrixTrackerEntry& mt = nvMatrixTrackers[i];

      shaderPlug->ext->glTrackMatrixNV (GL_VERTEX_PROGRAM_NV,
	mt.nvParameter, mt.nvMatrix, mt.nvTransform);
    }
  }
}

bool csShaderGLCGVP::Compile ()
{
  csRef<iDataBuffer> programBuffer = GetProgramData();
  if (!programBuffer.IsValid())
    return false;
  csString programStr;
  programStr.Append ((char*)programBuffer->GetData(), programBuffer->GetSize());

  CGprofile progProf = CG_PROFILE_UNKNOWN;

  // @@@ Hack: Make sure ARB_v_p is used
  if (cg_profile != 0)
    progProf = cgGetProfile (cg_profile);

  if(progProf == CG_PROFILE_UNKNOWN)
    progProf = cgGLGetLatestProfile (CG_GL_VERTEX);
  if (progProf < CG_PROFILE_ARBVP1)
  {
    delete[] cg_profile;
    cg_profile = csStrNew ("arbvp1");
  }

  if (!DefaultLoadProgram (programStr, CG_GL_VERTEX))
    return false;

  return true;
}

