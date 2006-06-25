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

void* operator new (size_t s, const CS::AllocPlatform&)
{ return ::operator new (s); }
void* operator new[] (size_t s, const CS::AllocPlatform&)
{ return ::operator new (s); }
void operator delete (void* p, const CS::AllocPlatform&)
{ ::operator delete (p); }
void operator delete[] (void* p, const CS::AllocPlatform&)
{ ::operator delete (p); }
