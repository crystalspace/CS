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
#include "cssysdef.h"
#include "csgeom/subrec.h"
#include "csutil/sysfunc.h"

csSubRectangles::csSubRectangles (const csRect &region)
{
  csSubRectangles::region = region;
  first = 0;
  Clear ();
}

csSubRectangles::~csSubRectangles ()
{
  Clear ();
  delete first; // Clean up the first region.
}

void csSubRectangles::Clear ()
{
  while (first)
  {
    csSubRect *n = first->next;
    delete first;
    first = n;
  }

  first = new csSubRect (region);
  first->prev = 0;
  first->next = 0;
}

bool csSubRectangles::Alloc (int w, int h, csRect &rect)
{
  // @@@ This is not a good algo. Needs to be improved!
  csSubRect *near_fit = 0;
  csSubRect *fit = 0;
  csSubRect *s = first;
  while (s)
  {
    int rw = s->Width ();
    int rh = s->Height ();
    if (w == rw && h == rh)
    {
      // We have an exact fit. Return now.
      if (s->prev)
        s->prev->next = s->next;
      else
        first = s->next;
      if (s->next) s->next->prev = s->prev;
      rect = *s;
      delete s;
      return true;
    }
    else if (w <= rw && h <= rh)
    {
      if (w == rw || h == rh)
        near_fit = s;
      else
        fit = s;
    }

    s = s->next;
  }

  if (near_fit)
  {
    // We have a near fit (i.e. one dimensions fits exactly).
    rect.Set (
        near_fit->xmin,
        near_fit->ymin,
        near_fit->xmin + w,
        near_fit->ymin + h);

    // Shrink the free block.
    int rw = near_fit->Width ();
    if (w == rw)
      near_fit->Set (
          near_fit->xmin,
          near_fit->ymin + h,
          near_fit->xmax,
          near_fit->ymax);
    else
      near_fit->Set (
          near_fit->xmin + w,
          near_fit->ymin,
          near_fit->xmax,
          near_fit->ymax);
    return true;
  }
  else if (fit)
  {
    // We have to create an additional block.
    rect.Set (fit->xmin, fit->ymin, fit->xmin + w, fit->ymin + h);

    csSubRect *s2 = new csSubRect (
        fit->xmin + w,
        fit->ymin,
        fit->xmax,
        fit->ymin + h);
    s2->next = first;
    s2->prev = 0;
    if (first) first->prev = s2;
    first = s2;
    fit->Set (fit->xmin, fit->ymin + h, fit->xmax, fit->ymax);
    return true;
  }

  // No room.
  return false;
}

void csSubRectangles::Dump ()
{
  csSubRect *s = first;
  while (s)
  {
    csPrintf ("  free: %d,%d,%d,%d\n", s->xmin, s->ymin, s->xmax, s->ymax);
    s = s->next;
  }
}
