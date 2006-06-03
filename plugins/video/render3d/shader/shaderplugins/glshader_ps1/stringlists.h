/*
    Copyright (C) 2006 by Jorrit Tyberghein
	      (C) 2006 by Frank Richter

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

#ifndef __CS_STRINGLISTS_H__
#define __CS_STRINGLISTS_H__

#include "cstool/identstrings.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderPS1)
{

CS_IDENT_STRING_LIST(GLIdent)
  CS_IDENT_STRING(GL_NO_ERROR)
  CS_IDENT_STRING(GL_INVALID_ENUM)
  CS_IDENT_STRING(GL_INVALID_VALUE)
  CS_IDENT_STRING(GL_INVALID_OPERATION)
  CS_IDENT_STRING(GL_STACK_OVERFLOW)
  CS_IDENT_STRING(GL_STACK_UNDERFLOW)
  CS_IDENT_STRING(GL_OUT_OF_MEMORY)
CS_IDENT_STRING_LIST_END(GLIdent)

}
CS_PLUGIN_NAMESPACE_END(GLShaderPS1)

#endif // __CS_STRINGLISTS_H__
