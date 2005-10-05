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

#ifndef __SOFTSHADER_H__
#define __SOFTSHADER_H__

#include "iutil/comp.h"
#include "ivideo/shader/shader.h"

#include "csplugincommon/softshader/renderinterface.h"
#include "csplugincommon/shader/shaderplugin.h"

#include "scanline.h"

namespace cspluginSoftshader
{

using namespace CrystalSpace::SoftShader;

class csSoftShader : public iShaderProgramPlugin
{
public:
  iObjectRegistry* object_reg;
  ScanlineRendererBase* scanlineRenderer;
  csRef<iSoftShaderRenderInterface> softSRI;

private:
  bool enable;
public:
  SCF_DECLARE_IBASE;
  
  csSoftShader (iBase *parent);
  virtual ~csSoftShader ();

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgramPlugin
  ////////////////////////////////////////////////////////////////////
  virtual csPtr<iShaderProgram> CreateProgram(const char* type) ;

  virtual bool SupportType(const char* type);

  virtual void Open();


  ////////////////////////////////////////////////////////////////////
  //                          iComponent
  ////////////////////////////////////////////////////////////////////

  bool Initialize (iObjectRegistry* reg);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoftShader);
    virtual bool Initialize (iObjectRegistry* reg)
      { return scfParent->Initialize (reg); }
  } scfiComponent;
};

} // namespace cspluginSoftshader

#endif //__SOFTSHADER_H__

