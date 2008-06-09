/*
    Copyright (C) 2002-2008 by Marten Svanfeldt
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

csShaderVariable::csShaderVariable () :
  csRefCount (), Name (csInvalidStringID), Type (UNKNOWN), VectorValue (0),
  accessorData (0)
{
  // Zero out the data as good as we can
  Int = 0;
  texture.HandValue = 0;
  texture.WrapValue = 0;
}

csShaderVariable::csShaderVariable (CS::ShaderVarStringID name) :
  csRefCount (), Name (name), Type (UNKNOWN), VectorValue (0), accessorData (0)
{
  // Zero out the data as good as we can
  Int = 0;
  texture.HandValue = 0;
  texture.WrapValue = 0;
}

csShaderVariable::csShaderVariable (const csShaderVariable& other)
  : csRefCount (), Name (other.Name), Type (other.Type), VectorValue (other.VectorValue),
  accessor (other.accessor), accessorData (other.accessorData)
{
  // Handle payload
  switch (Type)
  {
  case UNKNOWN:
    break;
  case INT:      
  case FLOAT:
    Int = other.Int;
    break;

  case TEXTURE:
    texture = other.texture;
    if (texture.HandValue)
      texture.HandValue->IncRef ();
    if (texture.WrapValue)
      texture.WrapValue->IncRef ();
    break;

  case RENDERBUFFER:
    RenderBuffer = other.RenderBuffer;
    if (RenderBuffer)
      RenderBuffer->IncRef ();
    break;

  case VECTOR2:      
  case VECTOR3:      
  case VECTOR4:
    Int = other.Int;
    break;

  case MATRIX:    
    MatrixValuePtr = new csMatrix3 (*other.MatrixValuePtr);
    break;

  case TRANSFORM:
    TransformPtr = new csReversibleTransform (*other.TransformPtr);
    break;

  case ARRAY:
    ShaderVarArray = new csRefArray<csShaderVariable>;
    *ShaderVarArray = *other.ShaderVarArray;
    break;

  default:
    ;
  }
}

csShaderVariable::~csShaderVariable ()
{
  switch (Type)
  {
  case UNKNOWN:     
  case INT:      
  case FLOAT:
    break; //Nothing to deallocate

  case TEXTURE:
    if (texture.HandValue)
      texture.HandValue->DecRef ();
    if (texture.WrapValue)
      texture.WrapValue->DecRef ();
    break;

  case RENDERBUFFER:
    if (RenderBuffer)
      RenderBuffer->DecRef ();
    break;

  case VECTOR2:      
  case VECTOR3:      
  case VECTOR4:
    break; //Nothing to deallocate      

  case MATRIX:
    delete MatrixValuePtr;
    break;

  case TRANSFORM:
    delete TransformPtr;
    break;

  case ARRAY:
    delete ShaderVarArray;
    break;

  default:
    ;
  }
}

csShaderVariable& csShaderVariable::operator= (const csShaderVariable& copyFrom)
{
  Name = copyFrom.Name;
  //Type = copyFrom.Type;
  VectorValue = copyFrom.VectorValue;
  accessor = copyFrom.accessor;  
  accessorData = copyFrom.accessorData;

  NewType (copyFrom.Type);

  // Handle payload
  switch (Type)
  {
  case UNKNOWN:
    break;

  case INT:      
  case FLOAT:
    Int = copyFrom.Int;
    break; 

  case TEXTURE:
    texture = copyFrom.texture;
    if (texture.HandValue)
      texture.HandValue->IncRef ();
    if (texture.WrapValue)
      texture.WrapValue->IncRef ();
    break;

  case RENDERBUFFER:
    RenderBuffer = copyFrom.RenderBuffer;
    if (RenderBuffer)
      RenderBuffer->IncRef ();
    break;

  case VECTOR2:      
  case VECTOR3:      
  case VECTOR4:
    break; //Nothing to copy more than whats done above      

  case MATRIX:
    MatrixValuePtr = new csMatrix3 (*copyFrom.MatrixValuePtr);
    break;

  case TRANSFORM:
    TransformPtr = new csReversibleTransform (*copyFrom.TransformPtr);
    break;

  case ARRAY:
    ShaderVarArray = new csRefArray<csShaderVariable>;
    *ShaderVarArray = *copyFrom.ShaderVarArray;
    break;

  default:
    ;
  }

  return *this;
}

void csShaderVariable::NewType (VariableType nt)
{
  if (Type == nt)
    return;

  switch (Type)
  {
  case UNKNOWN:     
  case INT:      
  case FLOAT:
    break; //Nothing to deallocate

  case TEXTURE:
    if (texture.HandValue)
      texture.HandValue->DecRef ();
    if (texture.WrapValue)
      texture.WrapValue->DecRef ();
    break;
  
  case RENDERBUFFER:
    if (RenderBuffer)
      RenderBuffer->DecRef ();
    break;

  case VECTOR2:      
  case VECTOR3:      
  case VECTOR4:
    break; //Nothing to deallocate      
  
  case MATRIX:
    delete MatrixValuePtr;
    break;
  
  case TRANSFORM:
    delete TransformPtr;
    break;

  case ARRAY:
    delete ShaderVarArray;
    break;

  default:
    ;
  }

  switch (nt)
  {
  case UNKNOWN:     
  case INT:      
  case FLOAT:
  case TEXTURE:    
  case RENDERBUFFER:
  case VECTOR2:      
  case VECTOR3:      
  case VECTOR4:
    break; //Nothing to allocate      

  case MATRIX:
    MatrixValuePtr = new csMatrix3;
    break;

  case TRANSFORM:
    TransformPtr = new csReversibleTransform;
    break;

  case ARRAY:
    ShaderVarArray = new csRefArray<csShaderVariable>;
    break;

  default:
    ;
  }
  
  Type = nt;
}

