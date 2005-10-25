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

#include "cssysdef.h"
#include "csgeom/vector3.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"

#include "softshader_vp.h"
#include "softshader_fp.h"
#include "softshader.h"

#include "scanline.h"

CS_IMPLEMENT_PLUGIN

namespace cspluginSoftshader
{

SCF_IMPLEMENT_FACTORY (csSoftShader)

csSoftShader::csSoftShader(iBase* parent) : 
  scfImplementationType (this, parent), object_reg(0), scanlineRenderer(0), 
  enable(false)
{
}

csSoftShader::~csSoftShader()
{
  delete scanlineRenderer;
}

////////////////////////////////////////////////////////////////////
//                      iShaderProgramPlugin
////////////////////////////////////////////////////////////////////
bool csSoftShader::SupportType(const char* type)
{
  Open();
  if (!enable)
    return false;
  if( strcasecmp(type, "vp") == 0)
    return true;
  if( strcasecmp(type, "fp") == 0)
    return true;
  return false;
}

csPtr<iShaderProgram> csSoftShader::CreateProgram(const char* type)
{
  Open();
  if (!enable)
    return 0;
  if( strcasecmp(type, "vp") == 0)
    return csPtr<iShaderProgram>(new csSoftShader_VP(object_reg));
  else if( strcasecmp(type, "fp") == 0)
    return csPtr<iShaderProgram> (new csSoftShader_FP (this));
  else return 0;
}

void csSoftShader::Open()
{
  if(!object_reg)
    return;

  csRef<iGraphics3D> r = CS_QUERY_REGISTRY(object_reg,iGraphics3D);
  csRef<iFactory> f = SCF_QUERY_INTERFACE (r, iFactory);
  if (f != 0 && strcmp ("crystalspace.graphics3d.software", 
      f->QueryClassID ()) == 0)
    enable = true;
  
  if (enable)
  {
    softSRI = scfQueryInterface<iSoftShaderRenderInterface> (r);
    if (!softSRI.IsValid())
    {
      enable = false;
      return;
    }
    
    scanlineRenderer = new ScanlineRenderer;
  }
}

////////////////////////////////////////////////////////////////////
//                          iComponent
////////////////////////////////////////////////////////////////////
bool csSoftShader::Initialize(iObjectRegistry* reg)
{
  object_reg = reg;
  return true;
}

} // namespace cspluginSoftshader
