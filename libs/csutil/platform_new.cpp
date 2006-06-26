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

/* This file is intended to wrap the platform's default operator new,
 * we definitely don't want  the ptmalloc-overridden version.
 */
#define CS_NO_NEW_OVERRIDE
#include "cssysdef.h"

#include <new>

namespace CS
{
  const AllocPlatform allocPlatform = AllocPlatform();
}

void* operator new (size_t s, const CS::AllocPlatform&) throw ()
{ 
  /* @@@ Technically, this is a guess: while likely most platforms do
   * resort to malloc for the default new implementation, some may
   * theoretically do something completely different.
   *
   * It would be preferable to use the real original platform's
   * operator new, however, this is tricky: it has the exact same
   * signature as our own override operator new (after all, that's how
   * the whole override thing works), so using the original operator new
   * is non-trivial here. 
   * For now, solve the issue pragmatically and employ platform_malloc().
   */
  return platform_malloc (s);
}
void* operator new[] (size_t s, const CS::AllocPlatform&) throw ()
{ 
  return platform_malloc (s);
}
void operator delete (void* p, const CS::AllocPlatform&) throw ()
{ 
  platform_free (p);
}
void operator delete[] (void* p, const CS::AllocPlatform&) throw ()
{ 
  platform_free (p);
}
