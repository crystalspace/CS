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

#ifndef __GLSHADER_AVP_H__
#define __GLSHADER_AVP_H__

#include "ivideo/shader/shader.h"

class csShaderGLAVP : public iShaderProgram
{
private:
  csGLExtensionManager* ext;
  csRef<iObjectRegistry> object_reg;

  unsigned int program_num;

public:
  SCF_DECLARE_IBASE;

  csShaderGLAVP(iObjectRegistry* objreg, csGLExtensionManager* ext)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    this->object_reg = objreg;
    this->ext = ext;
  }

  bool LoadProgram( const char* programstring );

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate();

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate();

  /* Propertybag - get property, return false if no such property found
   * Which properties there is is implementation specific
   */
  virtual bool GetProperty(const char* name, iString* string) {return false;};
  virtual bool GetProperty(const char* name, int* string) {return false;};
  virtual bool GetProperty(const char* name, csVector3* string) {return false;};
//  virtual bool GetProperty(const char* name, csVector4* string) {};

  /* Propertybag - set property.
   * Which properties there is is implementation specific
   */
  virtual bool SetProperty(const char* name, iString* string) {return false;};
  virtual bool SetProperty(const char* name, int* string) {return false;};
  virtual bool SetProperty(const char* name, csVector3* string) {return false;};
//  virtual bool SetProperty(const char* name, csVector4* string) {return false;};
};

#endif //__GLSHADER_AVP_H__