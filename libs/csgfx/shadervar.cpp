/*
    Copyright (C) 2002-2003 by Marten Svanfeldt
                  2002      by Anders Stenberg

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

//CS_LEAKGUARD_IMPLEMENT (csShaderVariable);

csShaderVariable::csShaderVariable (csStringID name) :
  csRefCount (), TextureHandValue (0), TextureWrapValue (0), VectorValue (0),
  MatrixValuePtr(0), TransformPtr (0), accessor (0), Name (name)
{
}

csShaderVariable& csShaderVariable::operator= (csShaderVariable& copyFrom)
{
  switch (copyFrom.Type)
  {
    case INT:
      {
        int val;
        copyFrom.GetValue (val); 
        SetValue (val);
      }
      break;
    case TEXTURE:
      {
	if (copyFrom.TextureWrapValue != 0)
        {
          iTextureWrapper *val;
          copyFrom.GetValue (val);  SetValue (val);
        }
	else
        {
          iTextureHandle* val;
	  copyFrom.GetValue (val);  SetValue (val);
        }
      }
      break;
    case RENDERBUFFER:
      {
        iRenderBuffer* val;
	copyFrom.GetValue (val);  SetValue (val);
      }
      break;
    case FLOAT: // Stored in VectorValue.
    case COLOR: // Ditto.
    case VECTOR2:
    case VECTOR3:
    case VECTOR4:
      {
	csVector4 v; 
	copyFrom.GetValue (v); SetValue (v);
	Type = copyFrom.Type;
      }
      break;
    case MATRIX:
      {
	csMatrix3 v;
	copyFrom.GetValue(v); SetValue(v);
      }
      break;
    case TRANSFORM:
      {
	csReversibleTransform v;
	copyFrom.GetValue(v); SetValue(v);
      }
      break;
  }
  return *this;
}
