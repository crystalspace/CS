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

#ifndef __GLSHADER_CG_H__
#define __GLSHADER_CG_H__

#include "ivideo/shader/shader.h"

#include "CG/cg.h"
#include "CG/cgGL.h"

class csGLShader_CG : public iShaderProgramPlugin
{
private:
  static csRef<iObjectRegistry> object_reg;
  static CGcontext context;

  static void ErrorCallback ();
public:
  SCF_DECLARE_IBASE;
  
  csGLShader_CG (iBase *parent);
  virtual ~csGLShader_CG ();


  
  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgramPlugin
  ////////////////////////////////////////////////////////////////////
  virtual csPtr<iShaderProgram> CreateProgram(const char* type) ;

  virtual bool SupportType(const char* type);

  virtual void Open();

  virtual csPtr<iString> GetProgramID(const char* programstring);


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

