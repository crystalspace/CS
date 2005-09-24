/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005 by Frank Richter

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

#ifndef __CS_CSGFX_SHADERVARFRAMEHOLDER_H__
#define __CS_CSGFX_SHADERVARFRAMEHOLDER_H__

#include "csextern.h"

#include "cstool/framedataholder.h"

#include "shadervarblockalloc.h"

/**\file
 * Frame data holder for shader variables.
 */

/**
 * Helper class to obtain a shader variable that was guaranteedly not used
 * in a frame yet.
 * \sa csFrameDataHolder
 */
class CS_CRYSTALSPACE_EXPORT csShaderVariableFrameHolder
{
  /// Actual holder
  csFrameDataHolder<csRef<csShaderVariable> > svCache;
  /// Allocator for variables
  csShaderVarBlockAlloc svAlloc;
  
  /// Allocate new
  csRef<csShaderVariable> AllocFrameSv (uint framenr);
public:
  ~csShaderVariableFrameHolder();

  /**
   * Allocate a shader variable that was not used this frame.
   * \param framenr Number of the current frame; used to track used and unused
   *   variable.
   * \param name Name of the variable
   * \remarks It's adviseable to really only use the returned variable for a
   *   single frame.
   */
  csRef<csShaderVariable> GetFrameUniqueSV (uint framenr,
    csStringID name);
};

#endif // __CS_CSGFX_SHADERVARFRAMEHOLDER_H__
