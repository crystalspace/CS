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

#ifndef __GLSHADER_CG_H__
#define __GLSHADER_CG_H__

#include "iutil/comp.h"
#include "csplugincommon/shader/shaderplugin.h"
#include "ivideo/shader/shader.h"
#include "csutil/leakguard.h"

#include <Cg/cg.h>
#include <Cg/cgGL.h>

class csGLExtensionManager;

class csGLShader_CG : public iShaderProgramPlugin
{
private:
  static void ErrorCallback ();

  bool enable;
  bool isOpen;
public:
  CS_LEAKGUARD_DECLARE (csGLShader_CG);

  static iObjectRegistry* object_reg;
  static CGcontext context;
  csRef<iShaderProgramPlugin> psplg;
  csRef<iShaderProgramPlugin> arbplg;
  csGLExtensionManager* ext;
  bool debugDump;
  char* dumpDir;
  bool doNVVPrealign;

  SCF_DECLARE_IBASE;
  
  csGLShader_CG (iBase *parent);
  virtual ~csGLShader_CG ();

  void Report (int severity, const char* msg, ...) CS_GNUC_PRINTF(3, 4);
  
  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgramPlugin
  ////////////////////////////////////////////////////////////////////
  virtual csPtr<iShaderProgram> CreateProgram(const char* type) ;

  virtual bool SupportType(const char* type);

  bool Open();


  ////////////////////////////////////////////////////////////////////
  //                          iComponent
  ////////////////////////////////////////////////////////////////////

  bool Initialize (iObjectRegistry* reg);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGLShader_CG);
    virtual bool Initialize (iObjectRegistry* reg)
      { return scfParent->Initialize (reg); }
  } scfiComponent;
};

#endif //__GLSHADER_CG_H__
