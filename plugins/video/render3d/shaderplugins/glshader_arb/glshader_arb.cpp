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
#include "csutil/hashmap.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csgeom/vector3.h"

#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "ivideo/render3d.h"
#include "ivideo/shader/shader.h"

#include "../../opengl/glextmanager.h"

#include "glshader_avp.h"

#include "glshader_arb.h"


CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGLShader_ARB)

SCF_EXPORT_CLASS_TABLE (glshader_arb)
SCF_EXPORT_CLASS_DEP (csGLShader_ARB, "crystalspace.render3d.shader.glarb",
                      "OpenGL specific shaderprogram provider for Render3D",
                      "")
SCF_EXPORT_CLASS_TABLE_END


SCF_IMPLEMENT_IBASE(csGLShader_ARB)
SCF_IMPLEMENTS_INTERFACE(iShaderProgramPlugin)
SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGLShader_ARB::eiComponent)
SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


csGLShader_ARB::csGLShader_ARB(iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csGLShader_ARB::~csGLShader_ARB()
{

}

////////////////////////////////////////////////////////////////////
//                      iShaderProgramPlugin
////////////////////////////////////////////////////////////////////
bool csGLShader_ARB::SupportType(const char* type)
{
  if( strcasecmp(type, "gl_arb_vp") == 0)
    return true;
  return false;
}

csPtr<iShaderProgram> csGLShader_ARB::CreateShaderProgram(const char* programstring, void* parameters, const char* type)
{
  if( strcasecmp(type, "gl_arb_vp") == 0)
  {
    csShaderGLAVP* myshader = new csShaderGLAVP(object_reg, ext);
    if(myshader->LoadProgram(programstring))
      return (iShaderProgram*) myshader;
  }
  else if (false)
  {
  }
  return NULL;
}

void csGLShader_ARB::Open()
{
  if(!object_reg)
    return;

  csRef<iRender3D> r = CS_QUERY_REGISTRY(object_reg,iRender3D);
  csRef<iShaderRenderInterface> sri = SCF_QUERY_INTERFACE(r, iShaderRenderInterface);

  ext = (csGLExtensionManager*) sri->GetObject("ext");
}

////////////////////////////////////////////////////////////////////
//                          iComponent
////////////////////////////////////////////////////////////////////
bool csGLShader_ARB::Initialize(iObjectRegistry* reg)
{
  object_reg = reg;
  return true;
}

