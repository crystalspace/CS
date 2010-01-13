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

#ifndef __CS_CSPLUGINCOMMON_OPENGL_GLENUM_IDENTSTRS_H__
#define __CS_CSPLUGINCOMMON_OPENGL_GLENUM_IDENTSTRS_H__

/**\file
 * Convert OpenGL constants to strings.
 * csOpenGLEnums is an implementation of csIdentStrings that obtains strings
 * for OpenGL-related constants (GL_xxx) to a string.
 * \sa csIdentStrings
 */

#include "cstool/identstrings.h"

CS_IDENT_STRING_LIST(csOpenGLEnums)
  CS_IDENT_STRING(GL_ADD)
  CS_IDENT_STRING(GL_SRC_COLOR)
  CS_IDENT_STRING(GL_ONE_MINUS_SRC_COLOR)
  CS_IDENT_STRING(GL_SRC_ALPHA)
  CS_IDENT_STRING(GL_ONE_MINUS_SRC_ALPHA)
  CS_IDENT_STRING(GL_TEXTURE)
  CS_IDENT_STRING(GL_STENCIL_INDEX)
  CS_IDENT_STRING(GL_DEPTH_COMPONENT)
  CS_IDENT_STRING(GL_REPLACE)
  CS_IDENT_STRING(GL_MODULATE)
  CS_IDENT_STRING(GL_SUBTRACT_ARB)
  CS_IDENT_STRING(GL_DEPTH_STENCIL_EXT)
  CS_IDENT_STRING(GL_ADD_SIGNED_ARB)
  CS_IDENT_STRING(GL_INTERPOLATE_ARB)
  CS_IDENT_STRING(GL_CONSTANT_ARB)
  CS_IDENT_STRING(GL_PRIMARY_COLOR_ARB)
  CS_IDENT_STRING(GL_PREVIOUS_ARB)
  CS_IDENT_STRING(GL_DOT3_RGB_ARB)
  CS_IDENT_STRING(GL_DOT3_RGBA_ARB)
CS_IDENT_STRING_LIST_END(csOpenGLEnums)

namespace CS
{
  namespace PluginCommon
  {
    CS_IDENT_STRING_LIST(OpenGLErrors)
      CS_IDENT_STRING(GL_NO_ERROR)
      CS_IDENT_STRING(GL_INVALID_ENUM)
      CS_IDENT_STRING(GL_INVALID_VALUE)
      CS_IDENT_STRING(GL_INVALID_OPERATION)
      CS_IDENT_STRING(GL_STACK_OVERFLOW)
      CS_IDENT_STRING(GL_STACK_UNDERFLOW)
      CS_IDENT_STRING(GL_OUT_OF_MEMORY)
    CS_IDENT_STRING_LIST_END(OpenGLErrors)
  } // namespace PluginCommon
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_OPENGL_GLENUM_IDENTSTRS_H__
