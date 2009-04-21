/*
    Copyright (C) 2008 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_OPENGL_ASSUMEDSTATE_H__
#define __CS_CSPLUGINCOMMON_OPENGL_ASSUMEDSTATE_H__

/**\file
 * Set assumed GL state.
 */

#include "csextern_gl.h"
#include "glstates.h"

namespace CS
{
  namespace PluginCommon
  {
    namespace GL
    {
      /**
       * Set some states to values assumed throughout Crystal Space.
       * This is useful in embedding situations where the embedder may change
       * these states to other values; this function can set them to the
       * assumed values before control is passed back to CS.
       * 
       * States set by this function are:
       * - Depth clear value (glClearDepth)
       * - Depth range (glDepthRange)
       * - Pixel unpack alignment (GL_UNPACK_ALIGNMENT)
       * - Pixel byte swapping (GL_UNPACK_SWAP_BYTES)
       * - Vertex program point size (GL_VERTEX_PROGRAM_POINT_SIZE_ARB)
       * - GL_POINT_SPRITE_ARB texture environment setting
       * - Minimum, maximum point sizes
       * - Point fade threshold
       */
      extern void CS_CSPLUGINCOMMON_GL_EXPORT SetAssumedState (
	csGLStateCache* statecache,
        csGLExtensionManager* ext);
    } // namespace GL
  } // namespace PluginCommon
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_OPENGL_ASSUMEDSTATE_H__
