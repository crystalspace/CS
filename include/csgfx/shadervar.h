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

struct iTextureHandle;
struct iTextureWrapper;

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
  csStringID Name;

  VariableType Type;

  int Int;
  csRef<iString> String;
  csRef<iTextureHandle> TextureHandValue;
  csRef<iTextureWrapper> TextureWrapValue;
  csVector4 VectorValue;

public:
  /// Constructor
  csShaderVariable (csStringID name);

  /// Get type of data stored
  VariableType GetType() const { return Type; }
  /// Set type (calling this after SetValue will cause undefined behaviour)
  void SetType (VariableType t) { Type = t; }

  /// Get the name of the variable
  csStringID GetName() const { return Name; }

  /// Retireve an int
  bool GetValue (int& value) const;
  /// Retrieve a float
  bool GetValue (float& value) const;
  /// Retrieve a string
  bool GetValue (iString*& value) const;
  /// Retrieve a color
  bool GetValue (csRGBpixel& value) const;
  /// Retrieve a texture handle
  bool GetValue (iTextureHandle*& value) const;
  /// Retrieve a texture wrapper
  bool GetValue (iTextureWrapper*& value) const;
  /// Retireve a csVector2
  bool GetValue (csVector2& value) const;
  /// Retrieve a csVector3
  bool GetValue (csVector3& value) const;
  /// Retrieve a csVector4
  bool GetValue (csVector4& value) const;

  /// Store an int
  bool SetValue (int value);
  /// Store a float
  bool SetValue (float value);
  /// Store a string
  bool SetValue (iString* value);
  /// Store a color
  bool SetValue (const csRGBpixel &value);
  /// Store a texture handle
  bool SetValue (iTextureHandle* value);
  /// Store a texture wrapper
  bool SetValue (iTextureWrapper* value);
  /// Store a csVector2
  bool SetValue (const csVector2 &value);
  /// Store a csVector3
  bool SetValue (const csVector3 &value);
  /// Store a csVector4
  bool SetValue (const csVector4 &value);
};

#endif
