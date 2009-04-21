/*
  Copyright (C) 2002 by Marten Svanfeldt
                        Anders Stenberg

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CSPLUGINCOMMON_OPENGL_GLHELPER_H__
#define __CS_CSPLUGINCOMMON_OPENGL_GLHELPER_H__

/**\file
 * OpenGL utilities
 */

#include "csgeom/matrix3.h"
#include "csgeom/matrix4.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"

/**\addtogroup plugincommon
 * @{ */

/// Make an OpenGL matrix from a CS transform
static inline void makeGLMatrix (const csReversibleTransform& t, 
  float matrix[16], bool rowMajor = false)
{
  const csMatrix3 &orientation = t.GetO2T();
  const csVector3 &translation = t.GetO2TTranslation();

  int row, col;
  if (rowMajor)
  {
    col = 1; row = 4;
  }
  else
  {
    col = 4; row = 1;
  }

  matrix[col*0+row*0] = orientation.m11;
  matrix[col*0+row*1] = orientation.m12;
  matrix[col*0+row*2] = orientation.m13;
  matrix[col*0+row*3] = 0.0f;

  matrix[col*1+row*0] = orientation.m21;
  matrix[col*1+row*1] = orientation.m22;
  matrix[col*1+row*2] = orientation.m23;
  matrix[col*1+row*3] = 0.0f;

  matrix[col*2+row*0] = orientation.m31;
  matrix[col*2+row*1] = orientation.m32;
  matrix[col*2+row*2] = orientation.m33;
  matrix[col*2+row*3] = 0.0f;

  matrix[col*3+row*0] = translation.x;
  matrix[col*3+row*1] = translation.y;
  matrix[col*3+row*2] = translation.z;
  matrix[col*3+row*3] = 1.0f;
}

/// Make an OpenGL matrix from a CS matrix
static inline void makeGLMatrix (const csMatrix3& m, float matrix[16], 
				 bool rowMajor = false)
{
  int row, col;
  if (rowMajor)
  {
    col = 1; row = 4;
  }
  else
  {
    col = 4; row = 1;
  }

  matrix[col*0+row*0] = m.m11;
  matrix[col*0+row*1] = m.m12;
  matrix[col*0+row*2] = m.m13;
  matrix[col*0+row*3] = 0.0f;

  matrix[col*1+row*0] = m.m21;
  matrix[col*1+row*1] = m.m22;
  matrix[col*1+row*2] = m.m23;
  matrix[col*1+row*3] = 0.0f;

  matrix[col*2+row*0] = m.m31;
  matrix[col*2+row*1] = m.m32;
  matrix[col*2+row*2] = m.m33;
  matrix[col*2+row*3] = 0.0f;

  matrix[col*3+row*0] = 0.0f;
  matrix[col*3+row*1] = 0.0f;
  matrix[col*3+row*2] = 0.0f;
  matrix[col*3+row*3] = 1.0f;
}

namespace CS
{
  namespace PluginCommon
  {
    /// Make an OpenGL matrix from a CS matrix
    static inline void MakeGLMatrix3x3 (const csMatrix3& m, float matrix[9], 
				        bool rowMajor = false)
    {
      int row, col;
      if (rowMajor)
      {
	col = 1; row = 3;
      }
      else
      {
	col = 3; row = 1;
      }
    
      matrix[col*0+row*0] = m.m11;
      matrix[col*0+row*1] = m.m12;
      matrix[col*0+row*2] = m.m13;
    
      matrix[col*1+row*0] = m.m21;
      matrix[col*1+row*1] = m.m22;
      matrix[col*1+row*2] = m.m23;
    
      matrix[col*2+row*0] = m.m31;
      matrix[col*2+row*1] = m.m32;
      matrix[col*2+row*2] = m.m33;
    }

    /// Make an OpenGL matrix from a CS matrix
    static inline void MakeGLMatrix4x4 (const CS::Math::Matrix4& m, float matrix[16], 
				        bool rowMajor = false)
    {
      int row, col;
      if (rowMajor)
      {
	col = 1; row = 4;
      }
      else
      {
	col = 4; row = 1;
      }
    
      matrix[col*0+row*0] = m.m11;
      matrix[col*0+row*1] = m.m21;
      matrix[col*0+row*2] = m.m31;
      matrix[col*0+row*3] = m.m41;
    
      matrix[col*1+row*0] = m.m12;
      matrix[col*1+row*1] = m.m22;
      matrix[col*1+row*2] = m.m32;
      matrix[col*1+row*3] = m.m42;
    
      matrix[col*2+row*0] = m.m13;
      matrix[col*2+row*1] = m.m23;
      matrix[col*2+row*2] = m.m33;
      matrix[col*2+row*3] = m.m43;
    
      matrix[col*3+row*0] = m.m14;
      matrix[col*3+row*1] = m.m24;
      matrix[col*3+row*2] = m.m34;
      matrix[col*3+row*3] = m.m44;
    }
    
  } // namespace PluginCommon
} // namespace CS

/** @} */

#endif
