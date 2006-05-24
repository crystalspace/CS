/*
  Copyright (C) 2006 by Frank Richter
	    (C) 2006 by Jorrit Tyberghein

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

#include "cssysdef.h"

#include "mybitarray.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{
#define IMPLEMENT_STATIC_CLASSVAR_REF_T(Class,var,getterFunc,Type,\
  initParam) \
CS_SPECIALIZE_TEMPLATE Type *Class::var = 0;                   \
CS_SPECIALIZE_TEMPLATE                                         \
void Class::getterFunc ## _kill ()                             \
{                                                              \
  delete &getterFunc ();                                       \
}                                                              \
CS_SPECIALIZE_TEMPLATE                                         \
Type &Class::getterFunc ()                                     \
{                                                              \
  if (!var)                                                    \
  {                                                            \
    var = new Type initParam;                                  \
    csStaticVarCleanup (getterFunc ## _kill);                  \
  }                                                            \
  return *var;                                                 \
}

  IMPLEMENT_STATIC_CLASSVAR_REF_T(MyBitArrayAllocatorMalloc, bitsAlloc2,
    BitsAlloc2, MyBitArrayAllocatorMalloc::BitsAlloc2Type, (1024));
  IMPLEMENT_STATIC_CLASSVAR_REF_T(MyBitArrayAllocatorMalloc, bitsAlloc4,
    BitsAlloc4, MyBitArrayAllocatorMalloc::BitsAlloc4Type, (1024));

  IMPLEMENT_STATIC_CLASSVAR_REF_T(MyBitArrayAllocatorTemp, bitsAlloc2,
    BitsAlloc2, MyBitArrayAllocatorTemp::BitsAlloc2Type, (1024));
  IMPLEMENT_STATIC_CLASSVAR_REF_T(MyBitArrayAllocatorTemp, bitsAlloc4,
    BitsAlloc4, MyBitArrayAllocatorTemp::BitsAlloc4Type, (1024));
}
CS_PLUGIN_NAMESPACE_END(XMLShader)
