/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CS_UTIL_DEBUG_H__
#define __CS_UTIL_DEBUG_H__

/**\file
 */

#include "csextern.h"

struct iBase;
struct iObjectRegistry;

namespace CS
{
  namespace Macros
  {
    CS_DEPRECATED_METHOD_MSG ("Discontinued cruft") inline void DG_ADD () {}
    CS_DEPRECATED_METHOD_MSG ("Discontinued cruft") inline void DG_ADDI () {}
    CS_DEPRECATED_METHOD_MSG ("Discontinued cruft") inline void DG_TYPE () {}
    CS_DEPRECATED_METHOD_MSG ("Discontinued cruft") inline void DG_DESCRIBE0 () {}
    CS_DEPRECATED_METHOD_MSG ("Discontinued cruft") inline void DG_DESCRIBE1 () {}
    CS_DEPRECATED_METHOD_MSG ("Discontinued cruft") inline void DG_DESCRIBE2 () {}
    CS_DEPRECATED_METHOD_MSG ("Discontinued cruft") inline void DG_REM () {}
    CS_DEPRECATED_METHOD_MSG ("Discontinued cruft") inline void DG_ADDCHILD () {}
    CS_DEPRECATED_METHOD_MSG ("Discontinued cruft") inline void DG_ADDPARENT () {}
    CS_DEPRECATED_METHOD_MSG ("Discontinued cruft") inline void DG_REMCHILD () {}
    CS_DEPRECATED_METHOD_MSG ("Discontinued cruft") inline void DG_REMPARENT () {}
    CS_DEPRECATED_METHOD_MSG ("Discontinued cruft") inline void DG_LINK () {}
    CS_DEPRECATED_METHOD_MSG ("Discontinued cruft") inline void DG_UNLINK () {}
  } // namespace Macros
} // namespace CS

/**\def DG_ADD
 * \deprecated Discontinued cruft
 */
#define DG_ADD(a,b) CS::Macros::DG_ADD ()
/**\def DG_ADDI
 * \deprecated Discontinued cruft
 */
#define DG_ADDI(a,b) CS::Macros::DG_ADDI ()
/**\def DG_TYPE
 * \deprecated Discontinued cruft
 */
#define DG_TYPE(a,b) CS::Macros::DG_TYPE ()
/**\def DG_DESCRIBE0
 * \deprecated Discontinued cruft
 */
#define DG_DESCRIBE0(a,b) CS::Macros::DG_DESCRIBE0 ()
/**\def DG_DESCRIBE1
 * \deprecated Discontinued cruft
 */
#define DG_DESCRIBE1(a,b,c) CS::Macros::DG_DESCRIBE1 ()
/**\def DG_DESCRIBE2
 * \deprecated Discontinued cruft
 */
#define DG_DESCRIBE2(a,b,c,d) CS::Macros::DG_DESCRIBE2 ()
/**\def DG_REM
 * \deprecated Discontinued cruft
 */
#define DG_REM(a) CS::Macros::DG_REM ()
/**\def DG_ADDCHILD
 * \deprecated Discontinued cruft
 */
#define DG_ADDCHILD(a,b) CS::Macros::DG_ADDCHILD ()
/**\def DG_ADDPARENT
 * \deprecated Discontinued cruft
 */
#define DG_ADDPARENT(a,b) CS::Macros::DG_ADDPARENT ()
/**\def DG_REMCHILD
 * \deprecated Discontinued cruft
 */
#define DG_REMCHILD(a,b) CS::Macros::DG_REMCHILD ()
/**\def DG_REMPARENT
 * \deprecated Discontinued cruft
 */
#define DG_REMPARENT(a,b) CS::Macros::DG_REMPARENT ()
/**\def DG_LINK
 * \deprecated Discontinued cruft
 */
#define DG_LINK(a,b) CS::Macros::DG_LINK ()
/**\def DG_UNLINK
 * \deprecated Discontinued cruft
 */
#define DG_UNLINK(a,b) CS::Macros::DG_UNLINK ()

/**
 * \deprecated Discontinued cruft
 */
class CS_DEPRECATED_TYPE_MSG("Discontinued cruft") csDebuggingGraph
{
public:
  static void SetupGraph (iObjectRegistry*) {}
  static void AddObject (iObjectRegistry*, void*, bool, char*, int, 
        char*, ...) {}
  static void AttachDescription (iObjectRegistry*, void*t, char*, ...) 
        CS_GNUC_PRINTF (3, 4) {}
  static void AttachType (iObjectRegistry*, void*, char*) {}
  static void RemoveObject (iObjectRegistry*, void*, char*, int) {}
  static void AddChild (iObjectRegistry*, void*, void*) {}
  static void AddParent (iObjectRegistry*, void*, void*) {}
  static void RemoveChild (iObjectRegistry*, void*, void*) {}
  static void RemoveParent (iObjectRegistry*, void*, void*) {}
  static void Clear (iObjectRegistry*) {}
  static void Dump (iObjectRegistry*) {}
  static void Dump (iObjectRegistry*, void*, bool reset_mark = true) {}
};

#endif //__CS_UTIL_DEBUG_H__

