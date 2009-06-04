/*
    Copyright (C) 2003 by Mat Sutcliffe <oktal@gmx.co.uk>
                  2003-2008 by Marten Svanfeldt

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

#ifndef __CS_GFX_SHADERVAR_H__
#define __CS_GFX_SHADERVAR_H__

/**\file
 * Shader variable.
 */
 
#include "csextern.h"

#include "csgeom/math.h"
#include "csgeom/quaternion.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csgeom/vector4.h"
#include "csgfx/rgbpixel.h"
#include "csutil/blockallocator.h"
#include "csutil/cscolor.h"
#include "csutil/leakguard.h"
#include "csutil/refarr.h"
#include "csutil/refcount.h"
#include "csutil/strset.h"

#include "iengine/texture.h"
#include "ivideo/texture.h"
#include "ivideo/rndbuf.h"

struct iTextureHandle;
struct iTextureWrapper;
struct csShaderVariableWrapper;

class csShaderVariable;

namespace CS
{
  namespace StringSetTag
  {
    struct ShaderVar;
  } // namespace StringSetTag
  
  /// String ID for shader variable name
  typedef StringID<StringSetTag::ShaderVar> ShaderVarStringID;
  /// Invalid shader variable name
  ShaderVarStringID const InvalidShaderVarStringID =
    InvalidStringID<StringSetTag::ShaderVar> ();
} // namespace CS

/// String set for shader variable names
struct iShaderVarStringSet :
  public iStringSetBase<CS::StringSetTag::ShaderVar>
{
  CS_ISTRINGSSET_SCF_VERSION(iShaderVarStringSet);
};

/**\addtogroup gfx3d
 * @{ */

/**
 * Interface to an accessorcallback for shadervariables.
 * This is used when we know the object providing the value of a single
 * variable, but the exact value cannot be predetermined.
 */
struct iShaderVariableAccessor : public virtual iBase
{
  SCF_INTERFACE (iShaderVariableAccessor, 2, 0, 0);

  /// Called before the value of the attached SV is returned
  virtual void PreGetValue (csShaderVariable *variable) = 0;
};

/**
 * Storage class for "shader vars", inheritable variables in the shader 
 * system. Shader vars are a primary system to transport information from
 * the engine/meshes/etc. to the renderer.
 */
class CS_CRYSTALSPACE_EXPORT csShaderVariable : public csRefCount
{
public:
  /**
   * Data types that can be stored.
   * Data storage and retrieval is not strict - data stored as INT, FLOAT 
   * or any VECTORx data can also be retrieved as any other of those.
   */
  enum VariableType
  {
    /// No value was yet set, hence the type is unknown.
    UNKNOWN = 0,
    /// Integer
    INT = 1,
    /// Float
    FLOAT,
    /// Texture
    TEXTURE,
    /// Renderbuffer
    RENDERBUFFER,
    /// Vector with 2 components
    VECTOR2,
    /// Vector with 3 components
    VECTOR3,
    /// Vector with 4 components
    VECTOR4,
    /// 3x3 Matrix
    MATRIX3X3,
    MATRIX = MATRIX3X3,
    /// Transform
    TRANSFORM,
    /// Array
    ARRAY,
    /// 4x4 Matrix
    MATRIX4X4,
    
    /**
     * Color
     * \deprecated Same as VECTOR4.
     */
    COLOR = VECTOR4
  };


  /**
   * Construct without a name. SetName() must be called before the variable
   * can be used.
   */
  csShaderVariable ();
  /// Construct with name.
  csShaderVariable (CS::ShaderVarStringID name);

  csShaderVariable (const csShaderVariable& other);

  virtual ~csShaderVariable ();  

  csShaderVariable& operator= (const csShaderVariable& copyFrom);

  /// Get type of data stored
  VariableType GetType() 
  { 
    /* The accessor should be called at least once so the var has a proper
     * type set */
    if ((Type == UNKNOWN) && accessor) 
      accessor->PreGetValue (this);
    return Type; 
  }
  /// Set type (calling this after SetValue will cause undefined behaviour)
  void SetType (VariableType t) 
  {
    NewType (t);
  }

  /// Set an accessor to use when getting the value
  void SetAccessor (iShaderVariableAccessor* a, intptr_t extraData = 0) 
  { 
    accessor = a;
    accessorData = extraData;
  }

  /**
   * Set the name of the variable
   * \warning Changing the name of a variable while it's in use can cause 
   *    unexpected behaviour.
   */
  void SetName (CS::ShaderVarStringID newName)
  { Name = newName; }
  
  /// Get the name of the variable
  CS::ShaderVarStringID GetName () const
  { return Name; }

  /// Get the accessor
  iShaderVariableAccessor* GetAccessor () const
  {
    return accessor;
  }

  /// Get the extra accessor data
  intptr_t GetAccessorData () const
  {
    return accessorData;
  }

  /// Retrieve an int
  bool GetValue (int& value)
  { 
    if (accessor) 
      accessor->PreGetValue (this);

    value = Int; 
    return true; 
  }

  /// Retrieve a float
  bool GetValue (float& value)
  { 
    if (accessor) 
      accessor->PreGetValue (this);

    value = VectorValue.x; 
    return true; 
  }

  /// Retrieve a color
  bool GetValue (csRGBpixel& value)
  {
    if (accessor) 
      accessor->PreGetValue (this);

    value.red = 
      (unsigned char) csClamp (int (VectorValue.x * 255.0f), 255, 0);
    value.green = 
      (unsigned char) csClamp (int (VectorValue.y * 255.0f), 255, 0);
    value.blue = 
      (unsigned char) csClamp (int (VectorValue.z * 255.0f), 255, 0);
    value.alpha = 
      (unsigned char) csClamp (int (VectorValue.w * 255.0f), 255, 0);;
    return true;
  }

  /// Retrieve a texture handle
  bool GetValue (iTextureHandle*& value)
  {
    if (accessor) 
      accessor->PreGetValue (this);

    if (Type != TEXTURE)
    {
      value = 0;
      return false;
    }

    value = texture.HandValue;
    if (!value && texture.WrapValue)
    {
      value = texture.HandValue = texture.WrapValue->GetTextureHandle ();
      if(value)
        value->IncRef();
    }
    return true;
  }

  /// Retrieve a texture wrapper
  bool GetValue (iTextureWrapper*& value)
  {
    if (accessor) 
      accessor->PreGetValue (this);

    if (Type != TEXTURE)
    {
      value = 0;
      return false;
    }

    value = texture.WrapValue;
    return true;
  }

  /// Retrieve a iRenderBuffer
  bool GetValue (iRenderBuffer*& value)
  {
    if (accessor) 
      accessor->PreGetValue (this);

    value = RenderBuffer;
    return true;
  }

  /// Retrieve a csVector2
  bool GetValue (csVector2& value)
  {
    if (accessor) 
      accessor->PreGetValue (this);

    value.Set (VectorValue.x, VectorValue.y);
    return true;
  }

  /// Retrieve a csVector3
  bool GetValue (csVector3& value)
  { 
    if (accessor) 
      accessor->PreGetValue (this);

    value.Set (VectorValue.x, VectorValue.y, VectorValue.z);
    return true; 
  }

  /// Retrieve a csColor
  bool GetValue (csColor& value)
  { 
    if (accessor) 
      accessor->PreGetValue (this);

    value.Set (VectorValue.x, VectorValue.y, VectorValue.z);
    return true; 
  }

  /// Retrieve a csVector4
  bool GetValue (csVector4& value)
  { 
    if (accessor) 
      accessor->PreGetValue (this);

    value = VectorValue; 
    return true; 
  }

  /// Retrieve a csQuaternion
  bool GetValue (csQuaternion& value)
  { 
    if (accessor) 
      accessor->PreGetValue (this);

    value.Set (VectorValue.x, VectorValue.y, VectorValue.z, VectorValue.w);
    return true; 
  }

  /// Retrieve a csMatrix3
  bool GetValue (csMatrix3& value)
  {
    if (accessor) 
      accessor->PreGetValue (this);

    if (Type == MATRIX)
    {
      value = *MatrixValuePtr;
      return true;
    }
    
    value = csMatrix3();    
    return false;
  }

  /// Retrieve a csReversibleTransform
  bool GetValue (csReversibleTransform& value)
  {
    if (accessor)
      accessor->PreGetValue (this);

    if (Type == TRANSFORM)
    {
      value = *TransformPtr;
      return true;
    }
    
    value = csReversibleTransform();    
    return false;
  }

  /// Retrieve a CS::Math::Matrix4
  bool GetValue (CS::Math::Matrix4& value)
  {
    if (accessor) 
      accessor->PreGetValue (this);

    if (Type == MATRIX4X4)
    {
      value = *Matrix4ValuePtr;
      return true;
    }
    else if (Type == MATRIX3X3)
    {
      value = *MatrixValuePtr;
      return true;
    }
    else if (Type == TRANSFORM)
    {
      value = *TransformPtr;
      return true;
    }
    
    value = CS::Math::Matrix4();    
    return false;
  }


  /// Store an int
  bool SetValue (int value) 
  { 
    if (Type != INT)
      NewType (INT);

    Int = value; 
    float f = (float)value;
    VectorValue.Set (f, f, f, f);
    return true; 
  }

  /// Store a float
  bool SetValue (float value)
  { 
    if (Type != FLOAT)
      NewType (FLOAT);

    Int = (int)value;
    VectorValue.Set (value, value, value, value);
    return true; 
  }

  /// Store a color
  bool SetValue (const csRGBpixel &value)
  {    
    if (Type != COLOR)
      NewType (COLOR);

    VectorValue.x = (float)value.red / 255.0f;
    VectorValue.y = (float)value.green / 255.0f;
    VectorValue.z = (float)value.blue / 255.0f;
    VectorValue.w = (float)value.alpha / 255.0f;
    return true;
  }

  /// Store a texture handle
  bool SetValue (iTextureHandle* value)
  {    
    if (Type != TEXTURE)
    {
      NewType (TEXTURE);
      texture.WrapValue = 0;
    }
    else
    {
      if (texture.HandValue)
	texture.HandValue->DecRef();
    }
    texture.HandValue = value;
    
    if (value)
      value->IncRef ();
    return true;
  }

  /// Store a texture wrapper
  bool SetValue (iTextureWrapper* value)
  {    
    if (Type != TEXTURE)
    {
      NewType (TEXTURE);
      texture.HandValue = 0;
    }
    else
    {
      if (texture.WrapValue)
	texture.WrapValue->DecRef();
    }
    
    texture.WrapValue = value;
    
    if (value)
      value->IncRef ();
    return true;
  }

  /// Store a render buffer
  bool SetValue (iRenderBuffer* value)
  {    
    if (Type != RENDERBUFFER)
      NewType (RENDERBUFFER);
    else
    {
      if (RenderBuffer)
	RenderBuffer ->DecRef();
    }
    RenderBuffer = value;
    
    if (value)
      value->IncRef ();
    return true;
  }

  /// Store a csVector2
  bool SetValue (const csVector2 &value)
  {
    if (Type != VECTOR2)
      NewType (VECTOR2);
    
    VectorValue.Set (value.x, value.y, 0.0f, 1.0f);
    Int = (int)value.x;
    return true;
  }

  /// Store a csVector3
  bool SetValue (const csVector3 &value)
  { 
    if (Type != VECTOR3)
      NewType (VECTOR3);

    VectorValue.Set (value.x, value.y, value.z, 1.0f);
    Int = (int)value.x;
    return true; 
  }

  /// Store a csColor
  bool SetValue (const csColor& value)
  { 
    if (Type != VECTOR3)
      NewType (VECTOR3);

    VectorValue.Set (value.red, value.green, value.blue, 1.0f);
    Int = (int)value.red;
    return true; 
  }

  /// Store a csColor4
  bool SetValue (const csColor4& value)
  { 
    if (Type != VECTOR4)
      NewType (VECTOR4);

    VectorValue.Set (value.red, value.green, value.blue, value.alpha);
    Int = (int)value.red;
    return true; 
  }

  /// Store a csVector4
  bool SetValue (const csVector4 &value)
  { 
    if (Type != VECTOR4)
      NewType (VECTOR4);

    VectorValue.Set (value.x, value.y, value.z, value.w);
    Int = (int)value.x;
    return true; 
  }

  bool SetValue (const csQuaternion& value)
  {
    if (Type != VECTOR4)
      NewType (VECTOR4);

    VectorValue.Set (value.v.x, value.v.y, value.v.z, value.w);
    return true;
  }

  /// Store a csMatrix3
  bool SetValue (const csMatrix3 &value)
  {
    if (Type != MATRIX)
      NewType (MATRIX);

    *MatrixValuePtr = value;
        
    return true;
  }

  /// Store a csReversibleTransform
  bool SetValue (const csReversibleTransform &value)
  {
    if (Type != TRANSFORM)
      NewType (TRANSFORM);

    *TransformPtr = value;
   
    return true;
  }

  /// Store a CS::Math::Matrix4
  bool SetValue (const CS::Math::Matrix4& value)
  {
    if (Type != MATRIX4X4)
      NewType (MATRIX4X4);

    *Matrix4ValuePtr = value;
        
    return true;
  }
  
  void AddVariableToArray (csShaderVariable *variable)
  {
    if (Type == ARRAY) 
      ShaderVarArray->Push (variable);
  }

  void RemoveFromArray (size_t element)
  {
    if (Type == ARRAY) 
      ShaderVarArray->DeleteIndex (element);
  }

  /// Set the number of elements in an array variable
  void SetArraySize (size_t size)
  {
    if (Type != ARRAY)
      NewType (ARRAY);

    ShaderVarArray->SetSize (size);
  }

  /// Get the number of elements in an array variable
  size_t GetArraySize ()
  {
    return (Type == ARRAY) ? ShaderVarArray->GetSize () : 0;
  }

  /**
   * Get a specific element in an array variable
   * Do not hold on to this for long, since it might change if
   * the array size changes.
   */
  csShaderVariable *GetArrayElement (size_t element)
  {
    if (Type == ARRAY && element < ShaderVarArray->GetSize ())
    {
      return ShaderVarArray->Get (element);
    }
    return 0;
  }

  /**
   * Set a specific element in an array variable
   */
  void SetArrayElement (size_t element, csShaderVariable *variable)
  {
    if (Type != ARRAY) NewType (ARRAY);
    ShaderVarArray->Put (element, variable);
  }

  /** 
   * Find a specific element in an array variable. 
   * \return The index of the found element, or csArrayItemNotFound if not found. 
   */ 
  size_t FindArrayElement (const csRef<csShaderVariable>& sv) 
  { 
    if (ShaderVarArray == 0) 
      return csArrayItemNotFound; 
    else 
      return ShaderVarArray->Find (sv); 
  } 

private:
  CS::ShaderVarStringID Name;
  VariableType Type;

  // Storage for types that can be combined..
  typedef csRefArray<csShaderVariable,
    CS::Memory::LocalBufferAllocatorUnchecked<csShaderVariable*, 8,
      CS::Memory::AllocatorMalloc, true>,
    csArrayCapacityFixedGrow<8> > SvArrayType;
  union
  {
    // Refcounted
    struct
    {
      iTextureHandle* HandValue;
      iTextureWrapper* WrapValue;
    } texture;    
    iRenderBuffer* RenderBuffer;

    int Int;
    csMatrix3* MatrixValuePtr;
    CS::Math::Matrix4* Matrix4ValuePtr;
    csReversibleTransform* TransformPtr;
    SvArrayType* ShaderVarArray;
  };

  csVector4 VectorValue;  
  csRef<iShaderVariableAccessor> accessor;
  intptr_t accessorData;
  
  CS_DECLARE_STATIC_CLASSVAR (matrixAlloc, MatrixAlloc,
    csBlockAllocator<csMatrix3>)
  CS_DECLARE_STATIC_CLASSVAR (matrix4Alloc, Matrix4Alloc,
    csBlockAllocator<CS::Math::Matrix4>)
  CS_DECLARE_STATIC_CLASSVAR (transformAlloc, TransformAlloc,
    csBlockAllocator<csReversibleTransform>)
  CS_DECLARE_STATIC_CLASSVAR (arrayAlloc, ShaderVarArrayAlloc,
    csBlockAllocator<SvArrayType>)

  virtual void NewType (VariableType nt);
};

namespace CS
{
  /// Helper class to obtain an ID for a shader variable.
  struct ShaderVarName
  {
    ShaderVarStringID name;
    
    ShaderVarName() : name (InvalidShaderVarStringID) {}
    ShaderVarName (iStringSetBase<StringSetTag::ShaderVar>* strings,
      const char* name) : name (strings->Request (name)) { }
    
    operator ShaderVarStringID () const { return name; }
  };
  
} // namespace CS

/** @} */

#endif
