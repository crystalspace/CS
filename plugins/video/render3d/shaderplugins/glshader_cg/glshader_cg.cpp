/*
Copyright (C) 2002 by Anders Stenberg
                      Mårten Svanfeldt

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
#include "csutil/hashmap.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/csmd5.h"
#include "csutil/scfstr.h"
#include "csgeom/vector3.h"

#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivideo/render3d.h"
#include "ivideo/shader/shader.h"

#include "glshader_cgvp.h"
#include "glshader_cgfp.h"

#include "glshader_cg.h"

csRef<iObjectRegistry> csGLShader_CG::object_reg;
CGcontext csGLShader_CG::context;

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGLShader_CG)

SCF_EXPORT_CLASS_TABLE (glshader_cg)
SCF_EXPORT_CLASS_DEP (csGLShader_CG, "crystalspace.render3d.shader.glcg",
                      "OpenGL specific shaderprogram provider for Render3D",
                      "")
SCF_EXPORT_CLASS_TABLE_END


SCF_IMPLEMENT_IBASE(csGLShader_CG)
SCF_IMPLEMENTS_INTERFACE(iShaderProgramPlugin)
SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGLShader_CG::eiComponent)
SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


csGLShader_CG::csGLShader_CG(iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

  context = cgCreateContext ();
  cgSetErrorCallback (ErrorCallback);
}

csGLShader_CG::~csGLShader_CG()
{
  cgDestroyContext (context);
}

void csGLShader_CG::ErrorCallback ()
{
  CGerror error = cgGetError();
  csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,"crystalspace.render3d.shader.glcg",
    cgGetErrorString (error), NULL);
  if (error == CG_COMPILER_ERROR)
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,"crystalspace.render3d.shader.glcg",
      cgGetLastListing (context), NULL);
}

////////////////////////////////////////////////////////////////////
//                      iShaderProgramPlugin
////////////////////////////////////////////////////////////////////
bool csGLShader_CG::SupportType(const char* type)
{
  if( strcasecmp(type, "gl_cg_vp") == 0)
    return true;
  if( strcasecmp(type, "gl_cg_fp") == 0)
    return true;
  return false;
}

csPtr<iShaderProgram> csGLShader_CG::CreateProgram(const char* type)
{
  if( strcasecmp(type, "gl_cg_vp") == 0)
    return csPtr<iShaderProgram>(new csShaderGLCGVP(object_reg, context));
  else if( strcasecmp(type, "gl_cg_fp") == 0)
    return csPtr<iShaderProgram>(new csShaderGLCGFP(object_reg, context));
  else return NULL;
}

void csGLShader_CG::Open()
{
  if(!object_reg)
    return;

  csRef<iRender3D> r = CS_QUERY_REGISTRY(object_reg,iRender3D);
  csRef<iShaderRenderInterface> sri = SCF_QUERY_INTERFACE(r, iShaderRenderInterface);
}

csPtr<iString> csGLShader_CG::GetProgramID(const char* programstring)
{
  csMD5::Digest d = csMD5::Encode(programstring);
  scfString* str = new scfString();
  str->Append((const char*)d.data[0], 16);
  return csPtr<iString>(str);
}

////////////////////////////////////////////////////////////////////
//                          iComponent
////////////////////////////////////////////////////////////////////
bool csGLShader_CG::Initialize(iObjectRegistry* reg)
{
  object_reg = reg;
  return true;
}

