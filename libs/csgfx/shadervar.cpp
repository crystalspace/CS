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
    CS::Memory::BlockAllocatorSafe<csMatrix3>, (1024));
CS_IMPLEMENT_STATIC_CLASSVAR (csShaderVariable, matrix4Alloc, Matrix4Alloc,
    CS::Memory::BlockAllocatorSafe<CS::Math::Matrix4>, (1024));
CS_IMPLEMENT_STATIC_CLASSVAR (csShaderVariable, transformAlloc, TransformAlloc,
    CS::Memory::BlockAllocatorSafe<csReversibleTransform>, (1024));
CS_IMPLEMENT_STATIC_CLASSVAR (csShaderVariable, arrayAlloc,
    ShaderVarArrayAlloc, CS::Memory::BlockAllocatorSafe<csShaderVariable::SvArrayType>, (1024));
CS_IMPLEMENT_STATIC_CLASSVAR (csShaderVariable, accessorAlloc,
    AccessorValuesAlloc, CS::Memory::BlockAllocatorSafe<csShaderVariable::AccessorValues>, (1024));


csShaderVariable::csShaderVariable () :
  csRefCount (), 
  nameAndType (nameMask | (UNKNOWN << typeShift)),
  accessor (0)
{
  // Zero out the data as good as we can
  Int = 0;
  memset (&Vector, 0, sizeof (Vector));
  texture.HandValue = 0;
  texture.WrapValue = 0;
}

csShaderVariable::csShaderVariable (CS::ShaderVarStringID name) :
  csRefCount (), nameAndType ((name & nameMask) | (UNKNOWN << typeShift)),
  accessor (0)
{
  CS_ASSERT((name == CS::InvalidShaderVarStringID)
    || (uint(name) < nameMask));
  // Zero out the data as good as we can
  Int = 0;
  memset (&Vector, 0, sizeof (Vector));
  texture.HandValue = 0;
  texture.WrapValue = 0;
}

csShaderVariable::csShaderVariable (const csShaderVariable& other)
  : csRefCount (), nameAndType (other.nameAndType)
{
  if (other.accessor)
    AllocAccessor (*other.accessor);
  else
    accessor = 0;

  // Handle payload
  switch (GetTypeI())
  {
  case UNKNOWN:
    break;
  case INT:
    Int = other.Int;
    break;
  case FLOAT:
  case VECTOR2:
  case VECTOR3:
  case VECTOR4:
    memcpy (&Vector, &other.Vector, sizeof (Vector));
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
  switch (GetTypeI())
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

  if (accessor != 0)
  {
    AccessorValuesAlloc()->Free (accessor);
  }
}

csShaderVariable& csShaderVariable::operator= (const csShaderVariable& copyFrom)
{
  SetName (copyFrom.GetName());
  //Type = copyFrom.Type;
  if (copyFrom.accessor != 0)
  {
    AllocAccessor (*copyFrom.accessor);
  }
  else
  {
    FreeAccessor ();
  }

  VariableType oldType = GetTypeI();
  NewType (copyFrom.GetTypeI());

  // Handle payload
  switch (GetTypeI())
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
    memcpy (&Vector, &copyFrom.Vector, sizeof (Vector));
    break;

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
  if (GetTypeI() == nt)
    return;

  switch (GetTypeI())
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
  
  nameAndType &= nameMask;
  nameAndType |= nt << typeShift;
}

void csShaderVariable::AllocAccessor (const AccessorValues& other)
{
  accessor = AccessorValuesAlloc()->Alloc (other);
}

void csShaderVariable::FreeAccessor ()
{
  AccessorValuesAlloc()->Free (accessor);
  accessor = 0;
}

