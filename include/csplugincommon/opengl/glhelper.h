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

static void makeGLMatrix (const csReversibleTransform& t, float matrix[16])
{
  const csMatrix3 &orientation = t.GetO2T();
  const csVector3 &translation = t.GetO2TTranslation();

  matrix[0] = orientation.m11;
  matrix[1] = orientation.m12;
  matrix[2] = orientation.m13;
  matrix[3] = 0.0f;

  matrix[4] = orientation.m21;
  matrix[5] = orientation.m22;
  matrix[6] = orientation.m23;
  matrix[7] = 0.0f;

  matrix[8] = orientation.m31;
  matrix[9] = orientation.m32;
  matrix[10] = orientation.m33;
  matrix[11] = 0.0f;

  matrix[12] = translation.x;
  matrix[13] = translation.y;
  matrix[14] = translation.z;
  matrix[15] = 1.0f;
}

static void makeGLMatrix (const csMatrix3& m, float matrix[16])
{
  matrix[0] = m.m11;
  matrix[1] = m.m12;
  matrix[2] = m.m13;
  matrix[3] = 0.0f;

  matrix[4] = m.m21;
  matrix[5] = m.m22;
  matrix[6] = m.m23;
  matrix[7] = 0.0f;

  matrix[8] = m.m31;
  matrix[9] = m.m32;
  matrix[10] = m.m33;
  matrix[11] = 0.0f;

  matrix[12] = 0.0f;
  matrix[13] = 0.0f;
  matrix[14] = 0.0f;
  matrix[15] = 1.0f;
}

#endif
