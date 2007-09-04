/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#ifndef __COMMON_H__
#define __COMMON_H__

#include "crystalspace.h"

namespace lighter
{
  // Standard types
  typedef csArray<float> FloatArray;
  typedef csDirtyAccessArray<float> FloatDArray;

  typedef csArray<csVector2> Vector2Array;
  typedef csDirtyAccessArray<csVector2> Vector2DArray;

  typedef csArray<csVector3> Vector3Array;
  typedef csDirtyAccessArray<csVector3> Vector3DArray;

  typedef csArray<csColor> ColorArray;
  typedef csDirtyAccessArray<csColor> ColorDArray;

  typedef csArray<void*> VoidArray;
  typedef csDirtyAccessArray<void*> VoidDArray;

  typedef csArray<int> IntArray;
  typedef csDirtyAccessArray<int> IntDArray;

  typedef csArray<size_t> SizeTArray;
  typedef csDirtyAccessArray<size_t> SizeTDArray;

  typedef csArray<bool> BoolArray;
  typedef csDirtyAccessArray<bool> BoolDArray;

  // Constants
  static const float LITEPSILON = 1.0e-5f;

 
  class Configuration;

  //The global settings
  extern Configuration globalConfig;

}

// Debugging: uncomment to have normals dumped to the lightmaps
//#define DUMP_NORMALS

#endif // __COMMON_H__
