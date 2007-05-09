/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#ifndef __CSUTIL_COMPILEASSERT_H__
#define __CSUTIL_COMPILEASSERT_H__

/**\file 
 * Compile time assert macro.
 */
namespace CS
{
namespace Utility
{
namespace Implementation
{
  
  template <bool x> 
  struct CompileAssertFailure;

  template <> 
  struct CompileAssertFailure<true>
  {
    enum { value = 1};
  };

  template <int x>
  struct CompileAssertTest
  {
  };

} // namespace Implementation
} // namespace Utility
} // namespace CS


#define CS_JOIN( X, Y ) CS_DO_JOIN( X, Y )
#define CS_DO_JOIN( X, Y ) CS_DO_JOIN2(X,Y)
#define CS_DO_JOIN2( X, Y ) X##Y

#define CS_COMPILE_ASSERT(B) \
  typedef CS::Utility::Implementation::CompileAssertTest< \
    sizeof(CS::Utility::Implementation::CompileAssertFailure<(bool)(B)>)> \
    CS_JOIN(CrystalSpaceCompileAssertTypedef, __LINE__)


#endif // __CSUTIL_COMPILEASSERT_H__
