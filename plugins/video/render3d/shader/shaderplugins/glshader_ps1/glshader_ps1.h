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

#ifndef __GLSHADER_PS1_H__
#define __GLSHADER_PS1_H__

#include "iutil/comp.h"
#include "csplugincommon/shader/shaderplugin.h"
#include "ivideo/shader/shader.h"

struct csGLExtensionManager;
class csGLStateCache;

class csGLShader_PS1 : public iShaderProgramPlugin
{
private:
  bool enable;
  bool isOpen;

  void Report (int severity, const char* msg, ...);
public:
  csGLExtensionManager* ext;
  csGLStateCache* stateCache;
  iObjectRegistry* object_reg;
  bool useLists;
  bool doVerbose;

  SCF_DECLARE_IBASE;
  
  csGLShader_PS1 (iBase *parent);
  virtual ~csGLShader_PS1 ();

  
  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgramPlugin
  ////////////////////////////////////////////////////////////////////
  virtual csPtr<iShaderProgram> CreateProgram(const char* type) ;

  virtual bool SupportType(const char* type);

  void Open();


  ////////////////////////////////////////////////////////////////////
  //                          iComponent
  ////////////////////////////////////////////////////////////////////

  bool Initialize (iObjectRegistry* reg);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGLShader_PS1);
    virtual bool Initialize (iObjectRegistry* reg)
      { return scfParent->Initialize (reg); }
  } scfiComponent;
};

#endif //__GLSHADER_PS1_H__

