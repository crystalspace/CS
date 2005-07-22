/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __CS_LEAKGUARD_H__
#define __CS_LEAKGUARD_H__

#ifdef CS_USE_LEAK_GUARD
#define CS_LEAKGUARD_DECLARE(m) \
struct csLeakGuard \
{ \
  int counter; \
  csLeakGuard () : counter (0) { } \
  ~csLeakGuard () \
  { \
    if (counter != 0) \
    { \
      printf ("%d leaking instance(s) of %s detected!\n", counter, #m); \
      fflush (stdout); \
    } \
  } \
}; \
static csLeakGuard leakguard; \
struct csLeakGuardInstance \
{ \
  csLeakGuardInstance () \
  { \
    leakguard.counter++; \
  } \
  ~csLeakGuardInstance () \
  { \
    leakguard.counter--; \
  } \
}; \
csLeakGuardInstance leakguardinstance

#define CS_LEAKGUARD_IMPLEMENT(m) \
  m::csLeakGuard m::leakguard

#else
#define CS_LEAKGUARD_DECLARE(m) \
  struct csLeakGuard /* ignored; pacify -ansi -pedantic */
#define CS_LEAKGUARD_IMPLEMENT(m) \
  struct csLeakGuard /* ignored; pacify -ansi -pedantic */
#endif

#endif // __CS_LEAKGUARD_H__
