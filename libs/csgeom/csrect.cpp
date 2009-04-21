/*
    Crystal Space Engine: rectangle class interface
    Copyright (C) 2001 by Jorrit Tyberghein
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
#include "cssysdef.h"
#include "csqint.h"
#include "csgeom/csrect.h"

csRect::csRect ()
{
  xmin = xmax = ymin = ymax = 0;
}

csRect::csRect (int ixmin, int iymin, int ixmax, int iymax)
{
  xmin = ixmin;
  xmax = ixmax;
  ymin = iymin;
  ymax = iymax;
}

csRect::csRect (const csRect &copy)
{
  xmin = copy.xmin;
  xmax = copy.xmax;
  ymin = copy.ymin;
  ymax = copy.ymax;
}

csRect::~csRect ()
{
}

void csRect::Intersect (int ixmin, int iymin, int ixmax, int iymax)
{
  if (IsEmpty () || ixmin >= ixmax || iymin >= iymax)
  {
    MakeEmpty ();
    return ;
  }

  if (xmin < ixmin) xmin = ixmin;
  if (ymin < iymin) ymin = iymin;
  if (xmax > ixmax) xmax = ixmax;
  if (ymax > iymax) ymax = iymax;
}

bool csRect::Intersects (const csRect &target) const
{
  if (
    IsEmpty () ||
    target.IsEmpty () ||
    (xmin >= target.xmax) ||
    (xmax <= target.xmin) ||
    (ymin >= target.ymax) ||
    (ymax <= target.ymin))
    return false;
  else
    return true;
}

void csRect::Union (int ixmin, int iymin, int ixmax, int iymax)
{
  if (ixmin >= ixmax || iymin >= iymax) return ;

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

void csRect::Exclude (int ixmin, int iymin, int ixmax, int iymax)
{
  if (IsEmpty ()) return ;

  if ((ymin >= iymin) && (ymax <= iymax))
  {
    if (xmin < ixmin)
    {
      if (xmax <= ixmin) return ; // no overlap
      else if (xmax <= ixmax)
        xmax = ixmin;
    }
    else
    {
      if (xmin >= ixmax) return ; // no overlap
      else if (xmax <= ixmax)
      {
        MakeEmpty ();
        return ;                  // null result
      }
      else
        xmin = ixmax;
    }
  }
  else if ((xmin >= ixmin) && (xmax <= ixmax))
  {
    if (ymin < iymin)
    {
      if (ymax <= iymin) return ; // no overlap
      else if (ymax <= iymax)
        ymax = iymin;
    }
    else
    {
      if (ymin >= iymax) return ; // no overlap
      else
        ymin = iymax;
    }
  }
  return ;
}

void csRect::Subtract (const csRect &rect)
{
  if (rect.IsEmpty () || IsEmpty ()) return ;

  if (
    (rect.xmax <= rect.xmin) ||
    (rect.ymax <= rect.ymin) ||
    (rect.xmin >= rect.xmax) ||
    (rect.ymin >= rect.ymax))
    return ;

  int area1 = ((rect.xmin - xmin) * Height ());
  int area2 = ((xmax - rect.xmax) * Height ());
  int area3 = (Width () * (rect.ymin - ymin));
  int area4 = (Width () * (ymax - rect.ymax));

  if (area1 >= area2)
    if (area1 >= area3)
      if (area1 >= area4) Set (xmin, ymin, rect.xmin, ymax);  //area1
  else
    Set (xmin, rect.ymax, xmax, ymax);                      //area4
  else if (area3 >= area4)
    Set (xmin, ymin, xmax, rect.ymin);                      //area3
  else
    Set (xmin, rect.ymax, xmax, ymax);                      //area4
  else if (area2 >= area3)
    if (area2 >= area4) Set (rect.xmax, ymin, xmax, ymax);  //area2
  else
    Set (xmin, rect.ymax, xmax, ymax);                      //area4
  else if (area3 >= area4)
    Set (xmin, ymin, xmax, rect.ymin);                      //area3
  else
    Set (xmin, rect.ymax, xmax, ymax);                      //area4
}

void csRect::AddAdjacent (const csRect &rect)
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
  if (tmp.Area () > Area ()) Set (tmp);
}

void csRect::Join (const csRect &rect)
{
  xmin = MIN (rect.xmin, xmin);
  ymin = MIN (rect.ymin, ymin);

  xmax = MAX (rect.xmax, xmax);
  ymax = MAX (rect.ymax, ymax);
}

void csRect::Outset (int n)
{
  xmin -= n;
  ymin -= n;
  xmax += n;
  ymax += n;
}

void csRect::Inset (int n)
{
  xmin += n;
  ymin += n;
  xmax -= n;
  ymax -= n;
}

bool csRect::ClipLineGeneral (int& x1, int& y1, int& x2, int& y2)
{
  int deltax = x2 - x1;
  int deltay = y2 - y1;

  int deltay_m_xmin = deltay * xmin;
  int deltay_m_xmax = deltay * xmax;
  int deltax_m_ymin = deltax * ymin;
  int deltax_m_ymax = deltax * ymax;
  int p = deltax * y1 - deltay*x1;
  int p11 = deltax_m_ymin - deltay_m_xmin;
  int p12 = deltax_m_ymax - deltay_m_xmin;
  int p21 = deltax_m_ymin - deltay_m_xmax;
  int p22 = deltax_m_ymax - deltay_m_xmax;
  bool inside = false;
  if ((p11<=p&&p<=p12) || (p12<=p&&p<=p11))
  {
    inside = true;
    if      (x1<xmin) { x1 = xmin; y1 = (p + deltay_m_xmin)/deltax;}
    else if (x2<xmin) { x2 = xmin; y2 = (p + deltay_m_xmin)/deltax;}
  }
  if ((p12<=p&&p<=p22) || (p22<=p&&p<=p12))
  {
    inside = true;
    if      (y1>ymax) { y1 = ymax; x1 = (deltax_m_ymax - p)/deltay;}
    else if (y2>ymax) { y2 = ymax; x2 = (deltax_m_ymax - p)/deltay;}
  }
  if ((p22<=p&&p<=p21) || (p21<=p&&p<=p22))
  {
    inside = true;
    if      (x1>xmax) { x1 = xmax; y1 = (p + deltay_m_xmax)/deltax;}
    else if (x2>xmax) { x2 = xmax; y2 = (p + deltay_m_xmax)/deltax;}
  }
  if ((p21<=p&&p<=p11) || (p11<=p&&p<=p21))
  {
    inside = true;
    if      (y1<ymin) { y1 = ymin; x1 = (deltax_m_ymin - p)/deltay;}
    else if (y2<ymin) { y2 = ymin; x2 = (deltax_m_ymin - p)/deltay;}
  }
  return inside;
}


bool csRect::ClipLine (int& x1, int& y1, int& x2, int& y2)
{
  // We first must be certain that the line (without boundries) does't cut
  // the rectangle.
  if ((x1<xmin&&x2<xmin) || (x1>xmax&&x2>xmax) ||
      (y1<ymin&&y2<ymin) || (y1>ymax&&y2>ymax))
  {
    return false;
  }

  if (x1 == x2)
  {
    // Trivial (vertical) case.
    if ((xmin<=x1 && x1<=xmax))
    {
      if      (y1>ymax) { y1 = ymax; }
      else if (y2>ymax) { y2 = ymax; }
      if      (y1<ymin) { y1 = ymin; }
      else if (y2<ymin) { y2 = ymin; }
      return true;
    }
    return false;
  }

  if (y1 == y2)
  {
    // Trivial (horizontal) case.
    if ((ymin<=y1 && y1<=ymax))
    {
      if      (x1>xmax) { x1 = xmax; }
      else if (x2>xmax) { x2 = xmax; }
      if      (x1<xmin) { x1 = xmin; }
      else if (x2<xmin) { x2 = xmin; }
      return true;
    }
    return false;
  }

  // If the line is fully in the rectangle then we can also return true.
  if (x1>=xmin && x1<=xmax && x2>=xmin && x2<=xmax &&
      y1>=ymin && y1<=ymax && y2>=ymin && y2<=ymax)
    return true;

  return ClipLineGeneral (x1, y1, x2, y2);
}

bool csRect::ClipLineSafe (int& x1, int& y1, int& x2, int& y2)
{
  // We first must be certain that the line (without boundries) does't cut
  // the rectangle.
  if ((x1<xmin&&x2<xmin) || (x1>xmax&&x2>xmax) ||
      (y1<ymin&&y2<ymin) || (y1>ymax&&y2>ymax))
  {
    return false;
  }

  if (x1 == x2)
  {
    // Trivial (vertical) case.
    if ((xmin<=x1 && x1<=xmax))
    {
      if      (y1>ymax) { y1 = ymax; }
      else if (y2>ymax) { y2 = ymax; }
      if      (y1<ymin) { y1 = ymin; }
      else if (y2<ymin) { y2 = ymin; }
      return true;
    }
    return false;
  }

  if (y1 == y2)
  {
    // Trivial (horizontal) case.
    if ((ymin<=y1 && y1<=ymax))
    {
      if      (x1>xmax) { x1 = xmax; }
      else if (x2>xmax) { x2 = xmax; }
      if      (x1<xmin) { x1 = xmin; }
      else if (x2<xmin) { x2 = xmin; }
      return true;
    }
    return false;
  }

  // If the line is fully in the rectangle then we can also return true.
  if (x1>=xmin && x1<=xmax && x2>=xmin && x2<=xmax &&
      y1>=ymin && y1<=ymax && y2>=ymin && y2<=ymax)
    return true;

  if (!((x1 >> 15) || (y1 >> 15) || (x2 >> 15) || (y2 >> 15)))
  {
    // Coordinates are safe.
    return ClipLineGeneral (x1, y1, x2, y2);
  }

  // Safer routine that takes care not to overflow multiplication with
  // integers.
  float deltax = x2 - x1;
  float deltay = y2 - y1;
  float deltay_m_xmin = deltay * float (xmin);
  float deltay_m_xmax = deltay * float (xmax);
  float deltax_m_ymin = deltax * float (ymin);
  float deltax_m_ymax = deltax * float (ymax);
  float p = deltax * float (y1) - deltay * float (x1);
  float p11 = deltax_m_ymin - deltay_m_xmin;
  float p12 = deltax_m_ymax - deltay_m_xmin;
  float p21 = deltax_m_ymin - deltay_m_xmax;
  float p22 = deltax_m_ymax - deltay_m_xmax;
  bool inside = false;
  if ((p11<=p&&p<=p12) || (p12<=p&&p<=p11))
  {
    inside = true;
    if      (x1<xmin) { x1 = xmin; y1 = csQint ((p + deltay_m_xmin)/deltax);}
    else if (x2<xmin) { x2 = xmin; y2 = csQint ((p + deltay_m_xmin)/deltax);}
  }
  if ((p12<=p&&p<=p22) || (p22<=p&&p<=p12))
  {
    inside = true;
    if      (y1>ymax) { y1 = ymax; x1 = csQint ((deltax_m_ymax - p)/deltay);}
    else if (y2>ymax) { y2 = ymax; x2 = csQint ((deltax_m_ymax - p)/deltay);}
  }
  if ((p22<=p&&p<=p21) || (p21<=p&&p<=p22))
  {
    inside = true;
    if      (x1>xmax) { x1 = xmax; y1 = csQint ((p + deltay_m_xmax)/deltax);}
    else if (x2>xmax) { x2 = xmax; y2 = csQint ((p + deltay_m_xmax)/deltax);}
  }
  if ((p21<=p&&p<=p11) || (p11<=p&&p<=p21))
  {
    inside = true;
    if      (y1<ymin) { y1 = ymin; x1 = csQint ((deltax_m_ymin - p)/deltay);}
    else if (y2<ymin) { y2 = ymin; x2 = csQint ((deltax_m_ymin - p)/deltay);}
  }
  return inside;
}

