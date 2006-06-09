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
#include "csutil/csendian.h"

// ptmalloc functions
namespace CS
{
  namespace ptmalloc_
  {
    extern "C"
    {
      extern void* ptmalloc (size_t n);
      extern void ptfree (void* p);
      extern void* ptrealloc (void* p, size_t n);
      extern void* ptmemalign (size_t a, size_t n);
      extern void* ptcalloc (size_t n, size_t s);
    }
  }
}

using namespace CS;

#ifdef CS_DEBUG
/* In debug mode, a small "cookie" is placed in front and after the memory
 * returned by the allocators in order to detect corruption, and, since
 * the cookie value is different per module, freeing memory across 
 * modules. */
typedef time_t CookieType;
static CookieType cookie;
inline static CookieType CookieSwap (CookieType x)
{
  if (sizeof (CookieType) >= 8)
    return csSwapBytes::UInt64 (x);
  else
    return csSwapBytes::UInt32 (x);
}
static CookieType GetCookie (void* p)
{
  while (cookie == 0)
  {
    // Semi-random
    time (&cookie);
    // Make somewhat unique
    cookie = cookie ^ (CookieType)&cookie;
  }
  return CookieSwap (cookie) ^ CookieType (p);
}
#endif

void* ptmalloc (size_t n)
{ 
#ifdef CS_DEBUG
  uint8* p = (uint8*)ptmalloc_::ptmalloc (
    n + sizeof (size_t) + 2*sizeof (CookieType));
  const CookieType startCookie = GetCookie (p);
  const CookieType endCookie = CookieSwap (startCookie);
  // Write allocated size(needed for checks in free()) and cookies.
  *((size_t*)p) = n;
  p += sizeof (size_t);
  *((CookieType*)p) = startCookie;
  p += sizeof (cookie);
  *((CookieType*)(p + n)) = endCookie;
  // Pepper.
  memset (p, 0xca, n);
  return p;
#else
  return ptmalloc_::ptmalloc (n); 
#endif
}
void ptfree (void* P)
{ 
#ifdef CS_DEBUG
  if (P == 0) return;
  // Compute original allocated address
  uint8* p = (uint8*)P;
  p -= sizeof(CookieType);
  const CookieType startCookie = GetCookie (p - sizeof(size_t));
  const CookieType endCookie = CookieSwap (startCookie);
  // Verify cookies
  CS_ASSERT_MSG("Memory block has wrong cookie "
	"(was probably allocated in another module)",
	*(CookieType*)p == startCookie);
  p -= sizeof(size_t);
  size_t n = *((size_t*)p);
  CS_ASSERT_MSG("Memory block has wrong cookie "
	"(probably corrupted by an overflow)",
	*(CookieType*)((uint8*)P + n) == endCookie);
  // Salt.
  memset (p, 0xcf, n + sizeof (size_t) + 2*sizeof (CookieType));
  ptmalloc_::ptfree (p); 
#else
  ptmalloc_::ptfree (P); 
#endif
}
void* ptrealloc (void* P, size_t n)
{ 
#ifdef CS_DEBUG
  if (P == 0) return ptmalloc (n);
  // Compute original allocated address
  uint8* p = (uint8*)P;
  p -= sizeof(CookieType);
  // Verify cookies
  const CookieType startCookie = GetCookie (p - sizeof(size_t));
  const CookieType endCookie = CookieSwap (startCookie);
  CS_ASSERT_MSG("Memory block has wrong cookie "
	"(was probably allocated in another module)",
	*(CookieType*)p == startCookie);
  p -= sizeof(size_t);
  size_t nOld = *((size_t*)p);
  CS_ASSERT_MSG("Memory block has wrong cookie "
	"(probably corrupted by an overflow)",
	*(CookieType*)((uint8*)P + nOld) == endCookie);

  uint8* np = (uint8*)ptmalloc_::ptrealloc (p, 
    n + sizeof (size_t) + 2*sizeof (CookieType)); 
  // Cookie may have changed since the memory address may have changed,
  // update
  const CookieType newStartCookie = GetCookie (np);
  const CookieType newEndCookie = CookieSwap (newStartCookie);
  *((size_t*)np) = n;
  np += sizeof (size_t);
  *((CookieType*)np) = newStartCookie;
  np += sizeof (cookie);
  *((CookieType*)(np + n)) = newEndCookie;
  // Spice the enlarged area
  if (n > nOld)
    memset (np + nOld, 0xca, n-nOld);
  return np;
#else
  return ptmalloc_::ptrealloc (P, n); 
#endif
}
void* ptmemalign (size_t a, size_t n)
{ 
  return ptmalloc_::ptmemalign (a, n); 
}
void* ptcalloc (size_t n, size_t s)
{ 
#ifdef CS_DEBUG
  return ptmalloc_::ptmalloc (n * s); 
#else
  return ptmalloc_::ptcalloc (n, s); 
#endif
}
