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

#include "cssysdef.h"
#include "csgfx/shadervar.h"
#include "iengine/texture.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"

csShaderVariable::csShaderVariable (csStringID name)
: csRefCount (), Name (name) {}

bool csShaderVariable::GetValue(int& value) const
{ 
  value = Int; 
  return true; 
}

bool csShaderVariable::GetValue(float& value) const
{ 
  value = VectorValue.x; 
  return true; 
}

bool csShaderVariable::GetValue(iString*& value) const
{ 
/*  value = String; 
  switch (Type)
  {
    case STRING:
      break;
    case INT:
      String->Format ("%d", Int);
      break;
    case FLOAT:
      String->Format ("%g", VectorValue.x);
      break;
    case VECTOR3:
      String->Format ("%g %g %g", VectorValue.x, VectorValue.y, VectorValue.z);
      break;
    case VECTOR4:
      String->Format ("%g %g %g %g", VectorValue.x, VectorValue.y, 
	VectorValue.z, VectorValue.w);
      break;
  }
  return true; */
  if (Type != STRING) 
  {
    value = 0;
    return false;
  }
  value = String; 
  return true;
}

bool csShaderVariable::GetValue(csRGBpixel& value) const
{
  value.red = (char) VectorValue.x;
  value.green = (char) VectorValue.y;
  value.blue = (char) VectorValue.z;
  value.alpha = (char) VectorValue.w;
  return true;
}

bool csShaderVariable::GetValue(iTextureHandle*& value) const
{
  value = TextureHandValue;
  return true;
}

bool csShaderVariable::GetValue(iTextureWrapper*& value) const
{
  value = TextureWrapValue;
  return true;
}

bool csShaderVariable::GetValue(csVector2& value) const
{
  value.Set (VectorValue.x, VectorValue.y);
  return true;
}

bool csShaderVariable::GetValue(csVector3& value) const
{ 
  value.Set (VectorValue.x, VectorValue.y, VectorValue.z);
  return true; 
}

bool csShaderVariable::GetValue(csVector4& value) const
{ 
  value = VectorValue; 
  return true; 
}

bool csShaderVariable::SetValue(int value)
{ 
  Type = INT; 
  Int = value; 
  float f = value;
  VectorValue.Set (f, f, f, f);
  return true; 
}

bool csShaderVariable::SetValue(float value)
{ 
  Type = FLOAT; 
  Int = (int)value;
  VectorValue.Set (value, value, value, value);
  return true; 
}

bool csShaderVariable::SetValue(iString* value)
{ 
  Type = STRING; 
  String = value; 
  float floats[4];
  floats[0] = floats[1] = floats[2] = 0.0f;
  floats[3] = 1.0f;
  sscanf (value->GetData (), "%f %f %f %f", 
    &floats[0], &floats[1], &floats[2], &floats[3]);
  VectorValue.Set (floats[0], floats[1], floats[2], floats[3]);
  Int = (int)VectorValue.x;
  return true; 
}

bool csShaderVariable::SetValue(const csRGBpixel &value)
{
  Type = COLOR;
  VectorValue.x = (float) value.red;
  VectorValue.y = (float) value.green;
  VectorValue.z = (float) value.blue;
  VectorValue.w = (float) value.alpha;
  return true;
}

bool csShaderVariable::SetValue(iTextureHandle *value)
{
  Type = TEXTURE;
  TextureHandValue = value;
  return true;
}

bool csShaderVariable::SetValue(iTextureWrapper *value)
{
  Type = TEXTURE;
  TextureWrapValue = value;
  if (value)
    TextureHandValue = value->GetTextureHandle ();
  return true;
}

bool csShaderVariable::SetValue(const csVector2 &value)
{
  Type = VECTOR2;
  VectorValue.Set (value.x, value.y, 1.0f, 1.0f);
  Int = (int)value.x;
  return true;
}

bool csShaderVariable::SetValue(const csVector3 &value)
{ 
  Type = VECTOR3; 
  VectorValue.Set (value.x, value.y, value.z, 1.0f);
  Int = (int)value.x;
  return true; 
}

bool csShaderVariable::SetValue(const csVector4 &value)
{ 
  Type = VECTOR4; 
  VectorValue.Set (value.x, value.y, value.z, value.w);
  Int = (int)value.x;
  return true; 
}
