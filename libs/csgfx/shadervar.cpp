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

CS_IMPLEMENT_STATIC_CLASSVAR (csShaderVariable, matrixAlloc, MatrixAlloc,
    csBlockAllocator<csMatrix3>, (1024));
CS_IMPLEMENT_STATIC_CLASSVAR (csShaderVariable, matrix4Alloc, Matrix4Alloc,
    csBlockAllocator<CS::Math::Matrix4>, (1024));
CS_IMPLEMENT_STATIC_CLASSVAR (csShaderVariable, transformAlloc, TransformAlloc,
    csBlockAllocator<csReversibleTransform>, (1024));
CS_IMPLEMENT_STATIC_CLASSVAR (csShaderVariable, arrayAlloc,
    ShaderVarArrayAlloc, csBlockAllocator<csShaderVariable::SvArrayType>, (1024));


csShaderVariable::csShaderVariable () :
  csRefCount (), Name (CS::InvalidShaderVarStringID), Type (UNKNOWN),
  VectorValue (0), accessorData (0)
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

  case MATRIX3X3:
    MatrixValuePtr = MatrixAlloc()->Alloc (*other.MatrixValuePtr);
    break;

  case MATRIX4X4:
    Matrix4ValuePtr = Matrix4Alloc()->Alloc ();
    break;

  case TRANSFORM:
    TransformPtr = TransformAlloc()->Alloc (*other.TransformPtr);
    break;

  case ARRAY:
    ShaderVarArray = ShaderVarArrayAlloc()->Alloc ();
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

  case MATRIX3X3:
    MatrixAlloc()->Free (MatrixValuePtr);
    break;

  case MATRIX4X4:
    Matrix4Alloc()->Free (Matrix4ValuePtr);
    break;

  case TRANSFORM:
    TransformAlloc()->Free (TransformPtr);
    break;

  case ARRAY:
    ShaderVarArrayAlloc()->Free (ShaderVarArray);
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

  VariableType oldType = Type;
  NewType (copyFrom.Type);

  // Handle payload
  switch (Type)
  {
  case UNKNOWN:
    break;

  case INT:      
    Int = copyFrom.Int;
    break; 

  case TEXTURE:
    if (oldType == TEXTURE)
    {
      if (texture.HandValue)
	texture.HandValue->DecRef ();
      if (texture.WrapValue)
	texture.WrapValue->DecRef ();
    }
    texture = copyFrom.texture;
    if (texture.HandValue)
      texture.HandValue->IncRef ();
    if (texture.WrapValue)
      texture.WrapValue->IncRef ();
    break;

  case RENDERBUFFER:
    if (oldType == RENDERBUFFER)
    {
      if (RenderBuffer)
	RenderBuffer->DecRef ();
    }
    RenderBuffer = copyFrom.RenderBuffer;
    if (RenderBuffer)
      RenderBuffer->IncRef ();
    break;

  case FLOAT:
  case VECTOR2:
  case VECTOR3:
  case VECTOR4:
    break; //Nothing to copy more than whats done above      

  case MATRIX3X3:
    *MatrixValuePtr = *copyFrom.MatrixValuePtr;
    break;

  case MATRIX4X4:
    *Matrix4ValuePtr = *copyFrom.Matrix4ValuePtr;
    break;

  case TRANSFORM:
    *TransformPtr = *copyFrom.TransformPtr;
    break;

  case ARRAY:
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
  
  case MATRIX3X3:
    MatrixAlloc()->Free (MatrixValuePtr);
    break;
  
  case MATRIX4X4:
    Matrix4Alloc()->Free (Matrix4ValuePtr);
    break;
  
  case TRANSFORM:
    TransformAlloc()->Free (TransformPtr);
    break;

  case ARRAY:
    ShaderVarArrayAlloc()->Free (ShaderVarArray);
    break;

  default:
    ;
  }

  switch (nt)
  {
  case INT:      
  case TEXTURE:
  case RENDERBUFFER:
  case UNKNOWN:     
  case FLOAT:
  case VECTOR2:      
  case VECTOR3:      
  case VECTOR4:
    break; //Nothing to allocate      

  case MATRIX3X3:
    MatrixValuePtr = MatrixAlloc()->Alloc ();
    break;

  case MATRIX4X4:
    Matrix4ValuePtr = Matrix4Alloc()->Alloc ();
    break;

  case TRANSFORM:
    TransformPtr = TransformAlloc()->Alloc ();
    break;

  case ARRAY:
    ShaderVarArray = ShaderVarArrayAlloc()->Alloc ();
    break;

  default:
    ;
  }
  
  Type = nt;
}

