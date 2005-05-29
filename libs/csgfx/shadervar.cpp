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
  csRefCount (), VectorValue (0), Int(0), MatrixValuePtr(0), 
  TransformPtr (0), array(0), Name (name)
{
}

csShaderVariable& csShaderVariable::operator= (const csShaderVariable& copyFrom)
{
  Name = copyFrom.Name;
  Type = copyFrom.Type;
  accessor = copyFrom.accessor;
  switch (copyFrom.Type)
  {
    case MATRIX:
      SetValue (*copyFrom.MatrixValuePtr);
      break;
    case TRANSFORM:
      SetValue (*copyFrom.TransformPtr);
      break;
    case ARRAY:
      array = new csArray<csShaderVariable>;
      *array = *copyFrom.array;
      break;
    default:
      {
	// Just copy everything that doesn't need special handling
	TextureHandValue = copyFrom.TextureHandValue;
	TextureWrapValue = copyFrom.TextureWrapValue;
	RenderBuffer = copyFrom.RenderBuffer;
	VectorValue = copyFrom.VectorValue;
	Int = copyFrom.Int;
      }
      break;
  }
  return *this;
}
