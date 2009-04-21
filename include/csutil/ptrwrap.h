/*
    Copyright (C) 2006 by Frank Richter

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

#ifndef __CS_CSUTIL_PTRWRAP_H__
#define __CS_CSUTIL_PTRWRAP_H__

/**\file
 * Pointer wrapper.
 */
 
/**
 * Simple pointer wrapper class.
 * The difference to a "normal" pointer is that this one will be initialized
 * with 0.
 */
template<typename T>
class csPtrWrap
{
  /// The wrapped pointer
  T* ptr;
public:
  //@{
  /// Constructor
  csPtrWrap () : ptr (0) {}
  csPtrWrap (const csPtrWrap& other) : ptr (other.ptr) {}
  csPtrWrap (T* ptr) : ptr (ptr) {}
  //@}
    
  //@{
  /// Obtain the wrapped pointer
  operator const T* () const { return ptr; }
  operator T*& () { return ptr; }
  //@}
  
  //@{
  /// Assign another pointer
  csPtrWrap& operator = (T* ptr) { this->ptr = ptr; return *this; }
  csPtrWrap& operator = (const T* ptr) { this->ptr = ptr; return *this; }
  //@}
};

#endif // __CS_CSUTIL_PTRWRAP_H__
