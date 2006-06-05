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

#include "cssysdef.h"

// ptmalloc functions
extern "C"
{
  extern void* ptmalloc (size_t n);
  extern void ptfree (void* p);
  extern void* ptrealloc (void* p, size_t n);
  extern void* ptmemalign (size_t a, size_t n);
  extern void* ptcalloc (size_t n, size_t s);
};

namespace CS
{
  namespace Memory
  {
    void* ptmalloc (size_t n)
    { return ::ptmalloc (n); }
    void ptfree (void* p)
    { ::ptfree (p); }
    void* ptrealloc (void* p, size_t n)
    { return ::ptrealloc (p, n); }
    void* ptmemalign (size_t a, size_t n)
    { return ::ptmemalign (a, n); }
    void* ptcalloc (size_t n, size_t s)
    { return ::ptcalloc (n, s); }
  } // namespace Memory
} // namespace CS

