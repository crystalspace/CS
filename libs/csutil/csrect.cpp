/*
    Crystal Space Windowing System: rectangle class
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

#include "sysdef.h"
#include "csutil/csrect.h"

csRect::csRect ()
{
  xmin = xmax = ymin = ymax = 0;
}

csRect::csRect (int ixmin, int iymin, int ixmax, int iymax)
{
  xmin = ixmin; xmax = ixmax;
  ymin = iymin; ymax = iymax;
}

csRect::csRect (csRect &copy)
{
  xmin = copy.xmin; xmax = copy.xmax;
  ymin = copy.ymin; ymax = copy.ymax;
}

csRect::~csRect ()
{
  // empty
}

void csRect::Intersect (int ixmin, int iymin, int ixmax, int iymax)
{
  if (IsEmpty ())
    return;

  if (ixmin > ixmax) { int _tmp_ = ixmin; ixmin = ixmax; ixmax = _tmp_; }
  if (iymin > iymax) { int _tmp_ = iymin; iymin = iymax; iymax = _tmp_; }
  if (xmin < ixmin) xmin = ixmin;
  if (ymin < iymin) ymin = iymin;
  if (xmax > ixmax) xmax = ixmax;
  if (ymax > iymax) ymax = iymax;
}

void csRect::Intersect (csRect &target)
{
  if (IsEmpty () || target.IsEmpty ())
  {
    MakeEmpty ();
    return;
  }

  if (xmin < target.xmin) xmin = target.xmin;
  if (ymin < target.ymin) ymin = target.ymin;
  if (xmax > target.xmax) xmax = target.xmax;
  if (ymax > target.ymax) ymax = target.ymax;
}

bool csRect::Intersects (csRect &target)
{
  if (IsEmpty () || target.IsEmpty ()
   || (xmin >= target.xmax)
   || (xmax <= target.xmin)
   || (ymin >= target.ymax)
   || (ymax <= target.ymin))
    return false;
  else
    return true;
}

void csRect::Union (int ixmin, int iymin, int ixmax, int iymax)
{
  if (ixmin > ixmax) { int _tmp_ = ixmin; ixmin = ixmax; ixmax = _tmp_; }
  if (iymin > iymax) { int _tmp_ = iymin; iymin = iymax; iymax = _tmp_; }

  if (IsEmpty ())
    Set (ixmin, iymin, ixmax, iymax);
  else
  {
    if (xmin > ixmin) xmin = ixmin;
    if (ymin > iymin) ymin = iymin;
    if (xmax < ixmax) xmax = ixmax;
    if (ymax < iymax) ymax = iymax;
  }
}

void csRect::Union (csRect &target)
{
  if (target.IsEmpty ())
    return;

  if (IsEmpty ())
    Set (target);
  else
  {
    if (xmin > target.xmin) xmin = target.xmin;
    if (ymin > target.ymin) ymin = target.ymin;
    if (xmax < target.xmax) xmax = target.xmax;
    if (ymax < target.ymax) ymax = target.ymax;
  }
}

void csRect::Exclude (int ixmin, int iymin, int ixmax, int iymax)
{
  if (IsEmpty ())
    return;

  if (ixmin > ixmax) { int _tmp_ = ixmin; ixmin = ixmax; ixmax = _tmp_; }
  if (iymin > iymax) { int _tmp_ = iymin; iymin = iymax; iymax = _tmp_; }

  if ((ymin >= iymin) && (ymax <= iymax))
  {
    if (xmin < ixmin)
    {
      if (xmax <= ixmin)
        return;		// no overlap
      else if (xmax <= ixmax)
        xmax = ixmin;
    } else
    {
      if (xmin >= ixmax)
        return;		// no overlap
      else if (xmax <= ixmax)
      {
        MakeEmpty ();
        return;		// null result
      }
      else
        xmin = ixmax;
    } /* endif */
  } else
  if ((xmin >= ixmin) && (xmax <= ixmax))
  {
    if (ymin <= iymin)
    {
      if (ymax <= iymin)
        return;		// no overlap
      else if (ymax <= iymax)
        ymax = iymin;
    } else
    {
      if (ymin >= iymax)
        return;		// no overlap
      else
        ymin = iymax;
    } /* endif */
  } /* endif */
  return;
}

void csRect::Subtract (csRect &rect)
{
  if (rect.IsEmpty () || IsEmpty ())
    return;

  if ((rect.xmax <= rect.xmin)
   || (rect.ymax <= rect.ymin)
   || (rect.xmin >= rect.xmax)
   || (rect.ymin >= rect.ymax))
    return;

  int area1 = ((rect.xmin - xmin) * Height ());
  int area2 = ((xmax - rect.xmax) * Height ());
  int area3 = (Width () * (rect.ymin - ymin));
  int area4 = (Width () * (ymax - rect.ymax));

  if (area1 >= area2)
    if (area1 >= area3)
      if (area1 >= area4)
        Set (xmin, ymin, rect.xmin, ymax);//area1
      else
        Set (xmin, rect.ymax, xmax, ymax);//area4
    else if (area3 >= area4)
        Set (xmin, ymin, xmax, rect.ymin);//area3
      else
        Set (xmin, rect.ymax, xmax, ymax);//area4
  else
    if (area2 >= area3)
      if (area2 >= area4)
        Set (rect.xmax, ymin, xmax, ymax);//area2
      else
        Set (xmin, rect.ymax, xmax, ymax);//area4
    else if (area3 >= area4)
        Set (xmin, ymin, xmax, rect.ymin);//area3
      else
        Set (xmin, rect.ymax, xmax, ymax);//area4
}

void csRect::AddAdjanced (csRect &rect)
{
  csRect tmp;
  if (xmin == rect.xmax)
    tmp.Set (rect.xmin, MAX (ymin, rect.ymin), xmax, MIN (ymax, rect.ymax));
  else if (xmax == rect.xmin)
    tmp.Set (xmin, MAX (ymin, rect.ymin), rect.xmax, MIN (ymax, rect.ymax));
  else if (ymin == rect.ymax)
    tmp.Set (MAX (xmin, rect.xmin), rect.ymin, MIN (xmax, rect.xmax), ymax);
  else if (ymax == rect.ymin)
    tmp.Set (MAX (xmin, rect.xmin), ymin, MIN (xmax, rect.xmax), rect.ymax);
  if (tmp.Area () > Area ())
    Set (tmp);
}
