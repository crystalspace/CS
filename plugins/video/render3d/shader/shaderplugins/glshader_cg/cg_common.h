/*
  Copyright (C) 2007 by Frank Richter

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

#ifndef __CG_COMMON_H__
#define __CG_COMMON_H__

#include "csutil/dirtyaccessarray.h"
#include "csutil/stringarray.h"

#define WIN32_LEAN_AND_MEAN
#include <Cg/cg.h>
/* WIN32 is used in an "#if" inside <cgGL.h>, however, it is sometimes defined
* without value. */
#ifdef WIN32
#undef WIN32
#define WIN32 1
#endif 
#include <Cg/cgGL.h>

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{

typedef csDirtyAccessArray<const char*, csStringArrayElementHandler>
  ArgumentArray;

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)

#endif // __CG_COMMON_H__
