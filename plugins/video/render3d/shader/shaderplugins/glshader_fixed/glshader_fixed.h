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

#ifndef __GLSHADER_FIXED_H__
#define __GLSHADER_FIXED_H__

#include "csgfx/lightsvcache.h"
#include "csplugincommon/shader/shaderplugin.h"
#include "csutil/leakguard.h"
#include "iutil/comp.h"
#include "ivideo/shader/shader.h"

class csGLShader_FIXED : public iShaderProgramPlugin
{
public:
  csGLExtensionManager* ext;
  iObjectRegistry* object_reg;
  csConfigAccess config;

  bool isOpen;

  bool enable;
  bool enableCombine;
  GLint texUnits;

  csLightShaderVarCache lsvCache;

  void Report (int severity, const char* msg, ...);
public:
  CS_LEAKGUARD_DECLARE (csGLShader_FIXED);

  /// One-time initialization stuff
  void Open();

  SCF_DECLARE_IBASE;
  
  csGLShader_FIXED (iBase *parent);
  virtual ~csGLShader_FIXED ();


  
  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgramPlugin
  ////////////////////////////////////////////////////////////////////
  virtual csPtr<iShaderProgram> CreateProgram(const char* type) ;

  virtual bool SupportType(const char* type);

  //virtual void Open();


  ////////////////////////////////////////////////////////////////////
  //                          iComponent
  ////////////////////////////////////////////////////////////////////

  bool Initialize (iObjectRegistry* reg);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGLShader_FIXED);
    virtual bool Initialize (iObjectRegistry* reg)
      { return scfParent->Initialize (reg); }
  } scfiComponent;
};

#endif //__GLSHADER_FIXED_H__

