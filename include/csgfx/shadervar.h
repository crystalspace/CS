/*
    Copyright (C) 2003 by Mat Sutcliffe <oktal@gmx.co.uk>

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

#include "csutil/refcount.h"
#include "csutil/strhash.h"
#include "iutil/string.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csgeom/vector4.h"
#include "csgfx/rgbpixel.h"
#include "iengine/texture.h"
#include "ivideo/texture.h"
#include "csutil/refarr.h"

struct iTextureHandle;
struct iTextureWrapper;
struct csShaderVariableWrapper;

/**
 * Storage class for inheritable variables in the shader system.
 */
class csShaderVariable : public csRefCount
{
public:
  /// Data types able to be stored
  enum VariableType
  {
    INT = 1,
    FLOAT,
    STRING,
    COLOR,
    TEXTURE,
    VECTOR2,
    VECTOR3,
    VECTOR4
  };

private:


  VariableType Type;

  int Int;
  csRef<iString> String;
  csRef<iTextureHandle> TextureHandValue;
  csRef<iTextureWrapper> TextureWrapValue;
  csVector4 VectorValue;

public:
  csStringID Name;

  /// Constructor
  csShaderVariable (csStringID name);

  /// Get type of data stored
  VariableType GetType() const { return Type; }
  /// Set type (calling this after SetValue will cause undefined behaviour)
  void SetType (VariableType t) { Type = t; }

  /// Get the name of the variable
  csStringID GetName() const { return Name; }

  /// Retireve an int
  bool GetValue (int& value) const
  { 
    value = Int; 
    return true; 
  }

  /// Retrieve a float
  bool GetValue (float& value) const
  { 
    value = VectorValue.x; 
    return true; 
  }

  /// Retrieve a string
  bool GetValue (iString*& value) const
  { 
    if (Type != STRING) 
    {
      value = 0;
      return false;
    }
    value = String; 
    return true;
  }

  /// Retrieve a color
  bool GetValue (csRGBpixel& value) const
  {
    value.red = (char) VectorValue.x;
    value.green = (char) VectorValue.y;
    value.blue = (char) VectorValue.z;
    value.alpha = (char) VectorValue.w;
    return true;
  }

  /// Retrieve a texture handle
  bool GetValue (iTextureHandle*& value) const
  {
    value = TextureHandValue;
    return true;
  }

  /// Retrieve a texture wrapper
  bool GetValue (iTextureWrapper*& value) const
  {
    value = TextureWrapValue;
    return true;
  }

  /// Retireve a csVector2
  bool GetValue (csVector2& value) const
  {
    value.Set (VectorValue.x, VectorValue.y);
    return true;
  }

  /// Retrieve a csVector3
  bool GetValue (csVector3& value) const
  { 
    value.Set (VectorValue.x, VectorValue.y, VectorValue.z);
    return true; 
  }

  /// Retrieve a csVector4
  bool GetValue (csVector4& value) const
  { 
    value = VectorValue; 
    return true; 
  }


  /// Store an int
  bool SetValue (int value) 
  { 
    Type = INT; 
    Int = value; 
    float f = value;
    VectorValue.Set (f, f, f, f);
    return true; 
  }

  /// Store a float
  bool SetValue (float value)
  { 
    Type = FLOAT; 
    Int = (int)value;
    VectorValue.Set (value, value, value, value);
    return true; 
  }

  /// Store a string
  bool SetValue (iString* value)
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

  /// Store a color
  bool SetValue (const csRGBpixel &value)
  {
    Type = COLOR;
    VectorValue.x = (float) value.red;
    VectorValue.y = (float) value.green;
    VectorValue.z = (float) value.blue;
    VectorValue.w = (float) value.alpha;
    return true;
  }

  /// Store a texture handle
  bool SetValue (iTextureHandle* value)
  {
    Type = TEXTURE;
    TextureHandValue = value;
    return true;
  }

  /// Store a texture wrapper
  bool SetValue (iTextureWrapper* value)
  {
    Type = TEXTURE;
    TextureWrapValue = value;
    if (value)
      TextureHandValue = value->GetTextureHandle ();
    return true;
  }

  /// Store a csVector2
  bool SetValue (const csVector2 &value)
  {
    Type = VECTOR2;
    VectorValue.Set (value.x, value.y, 1.0f, 1.0f);
    Int = (int)value.x;
    return true;
  }

  /// Store a csVector3
  bool SetValue (const csVector3 &value)
  { 
    Type = VECTOR3; 
    VectorValue.Set (value.x, value.y, value.z, 1.0f);
    Int = (int)value.x;
    return true; 
  }

  /// Store a csVector4
  bool SetValue (const csVector4 &value)
  { 
    Type = VECTOR4; 
    VectorValue.Set (value.x, value.y, value.z, value.w);
    Int = (int)value.x;
    return true; 
  }

};

struct csShaderVariableProxy
{
public:

  csShaderVariableProxy () :
      Name (csInvalidStringID), userData (0), shaderVariable(0)
  {}

  csShaderVariableProxy (csStringID name, int ud) :
      Name (name), userData(ud), shaderVariable(0)
  {}
        
  csStringID Name;
  int userData;
  csShaderVariable *shaderVariable;
};


static int ShaderVariableCompare (csShaderVariable* const &item1,
                                  csShaderVariable* const &item2)
{
  if (item1->Name < item2->Name) return -1;
  else if (item1->Name > item2->Name) return 1;
  else return 0;
}

static int ShaderVariableKeyCompare (csShaderVariable* const& item1, void* item2)
{
  csStringID key = (csStringID)item2;
  if (item1->Name < key) return -1;
  else if (item1->Name > key) return 1;
  else return 0;
}


/**
 * Sorted list of shadervariables
 */
class csShaderVariableList : 
  public csArray<csShaderVariableProxy>
{
public:
  int InsertSorted (csShaderVariableProxy item);
  int Push (csShaderVariableProxy item);
};

class csShaderVariableContextHelper
{
public:
  csShaderVariableContextHelper () 
  {}

  ~csShaderVariableContextHelper()
  {
  }

  /// Add a variable to this context
  inline void AddVariable (csShaderVariable *variable) 
  {
    variables.InsertSorted (variable, ShaderVariableCompare);
  }

  /// Get a named variable from this context
  inline csShaderVariable* GetVariable (csStringID name) const 
  {
    int idx = variables.FindSortedKey ((void*)name, ShaderVariableKeyCompare);
    if (idx >= 0) return variables.Get (idx);
    else return 0;
  }

  /**
  * Fill a csShaderVariableList
  * It requires the passed list to be sorted.
  */
  inline void FillVariableList (csShaderVariableList *list) const
  {
    csRefArray<csShaderVariable>::Iterator varIter (variables.GetIterator ());
    csShaderVariableList::Iterator inputIter (list->GetIterator ());
    csShaderVariable* curVar=0;

    while (inputIter.HasNext())
    {
      csShaderVariableProxy *curInput = (csShaderVariableProxy*)&inputIter.Next();
      while (varIter.HasNext () && ((curVar=varIter.Next ())->Name < curInput->Name))
        ;
      if (curVar->Name == curInput->Name && curInput->shaderVariable == 0)
        curInput->shaderVariable = curVar;
      else if (varIter.HasNext ())
        continue; //still may have more
      else
        return;
    }
  }

private:
  /// List all variables. They are owned by this class
  csRefArray<csShaderVariable> variables;
};


#endif
