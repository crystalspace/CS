
/*
    Copyright (C) 2002 by Mårten Svanfeldt
                          Anders Stenberg

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef __SHADERVAR_H__
#define __SHADERVAR_H__

#include "csutil/csstring.h"

struct iShaderVariable;

class csShaderVariable : public iShaderVariable
{
private:
  csString name;

  VariableType type;

  int intval;
  float floatval;
  csRef<iString> istring;
  csVector3 v3;

public:
  SCF_DECLARE_IBASE;

  csShaderVariable()
  {
    type = INT;
    intval = 0;
  }
  virtual ~csShaderVariable() { }

  virtual VariableType GetType() { return type; }
  virtual void SetType(VariableType newtype) { type = newtype; }

  virtual void SetName(const char* name) { csShaderVariable::name = csString(name); }
  virtual const char* GetName() { return name; }
  virtual bool GetValue(int& value) { if(type==INT) {value = intval; return true;} return false; }
  virtual bool GetValue(float& value) { if(type==VECTOR1) {value = floatval; return true;} return false; }
  virtual bool GetValue(iString* value) { if(type==STRING) {value = istring; return true;} return false; }
  virtual bool GetValue(csVector3& value) { if(type==VECTOR3) {value = v3; return true;} return false; }
//  virtual bool GetValue(csVector4* value);
  virtual bool SetValue(int value) { type=INT; intval = value; return true; }
  virtual bool SetValue(float value) { type=VECTOR1; floatval = value; return true; }
  virtual bool SetValue(iString* value) { type=STRING; istring = value; return true; }
  virtual bool SetValue(csVector3 value) { type=VECTOR3; v3 = value; return true; }
//  virtual bool SetValue(csVector4* value);
};

SCF_IMPLEMENT_IBASE( csShaderVariable )
  SCF_IMPLEMENTS_INTERFACE( iShaderVariable )
SCF_IMPLEMENT_IBASE_END

#endif

