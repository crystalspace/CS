/*
    Crystal Space: Named Object Vector class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_NOBJVEC_H__
#define __CS_NOBJVEC_H__

//-----------------------------------------------------------------------------
// Note *1*: The explicit "this->" is needed by modern compilers (such as gcc
// 3.4.x) which distinguish between dependent and non-dependent names in
// templates.  See: http://gcc.gnu.org/onlinedocs/gcc/Name-lookup.html
//-----------------------------------------------------------------------------

#include "csextern.h"
#include "refarr.h"
#include "iutil/object.h"

/**\file 
 * Named Object Vector class
 */

/**
 * This class implements a typed array that correctly keeps track
 * of reference count and also is able to find by name. Assumes
 * the types used for this implement QueryObject() to get the iObject
 * that has GetName().
 */
template <class T>
class csRefArrayObject : public csRefArray<T>
{
public:
  csRefArrayObject (int ilimit = 0, int ithreshold = 0)
  	: csRefArray<T> (ilimit, ithreshold)
  {
  }

  size_t GetIndexByName (const char* name) const
  {
    size_t i;
    for (i = 0 ; i < this->Length () ; i++) // see *1*
    {
      T* o = (*this)[i];
      const char* n = o->QueryObject ()->GetName ();
      if (n && !strcmp (n, name))
        return i;
    }
    return csArrayItemNotFound;
  }

  T* FindByName (const char* name) const
  {
    size_t i = GetIndexByName (name);
    if (i != csArrayItemNotFound)
      return (*this)[i];
    else
      return 0;
  }
};


#endif // __CS_NOBJVEC_H__
