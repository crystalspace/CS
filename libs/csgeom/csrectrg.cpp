/*
    Copyright (C) 2001 by Christopher Nelson
    
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
#include "csgeom/csrectrg.h"

csRectRegion::csRectRegion() : region(0), region_count(0), region_max(0) {}
csRectRegion::~csRectRegion()
{
  if (region != 0)
    free(region);
}

void csRectRegion::pushRect(csRect const& r)
{
  if (region_count >= region_max)
  {
    region_max += 64;
    int const nbytes = region_max * sizeof(region[0]);
    if (region == 0)
      region = (csRect*)malloc(nbytes);
    else
      region = (csRect*)realloc(region, nbytes);
  }
  region[region_count++] = r;
}

void csRectRegion::deleteRect(int i)
{
  if (region_count > 0 && i >= 0 && i < --region_count)
    memmove(region + i, region + i + 1, region_count - i);
}

// The purpose of this function is to take r1 and fragment it around r2,
// removing the area that overlaps.  This function can be used by either
// Include() or Exclude(), since it simply fragments r1.
void csRectRegion::fragmentRect(
  csRect &r1, csRect &r2, bool testedContains, bool testedEdge)
{
  // We may have to fragment r1 into four pieces if r2 is totally inside r1
  if (!testedContains)
  {
  }

  // We may have to fragment r1 into three pieces if an entire edge of r2 is
  // inside r1.
  if (!testedEdge)
  {
  }
  
  // If we've gotten here then we should only have to break the rect into 2
  // pieces.
  if (r1.Contains(r2.xmin, r2.ymin))
  {
    // Break r1 into left, top since top left corner of r2 is inside.
    csRect left, top;
    left.Set(r1.xmin, r2.ymin, r2.xmin-1, r1.ymax);
     top.Set(r1.xmin, r1.ymin, r1.xmax,   r2.ymin-1);  
    pushRect(left);
    pushRect(top);
  }
  else if (r1.Contains(r2.xmin, r2.ymax))
  {
    // Break r1 into left, bot since bot left corner of r2 is inside.
    csRect left, bot;
    left.Set(r1.xmin, r1.ymin,   r2.xmin-1, r2.ymax);
     bot.Set(r1.xmin, r2.ymax+1, r1.xmax,   r1.ymax);  
    pushRect(left);
    pushRect(bot);
  }
  else if (r1.Contains(r2.xmax, r2.ymin))
  {
    // Break r1 into right, top since top right corner of r2 is inside.
    csRect right, top;
    right.Set(r2.xmax+1, r1.ymin, r1.xmax, r1.ymax);
      top.Set(r1.xmin,   r1.ymin, r2.xmax, r2.ymin-1);  
    pushRect(right);
    pushRect(top);
  } 
  else
  {
    // Break r1 into right, bot since bot right corner of r2 is inside.
    csRect right, bot;
    right.Set(r2.xmax+1, r1.ymin,   r1.xmax, r1.ymax);
      bot.Set(r1.xmin,   r2.ymax+1, r2.xmax, r1.ymax);  
    pushRect(right);
    pushRect(bot);
  }
}

// Chop the intersection of rect1 and rect2 off of rect1 (the smaller rect)
// Tests to make sure the intersection happens on an edge and not a corner.
// Returns false if it is unable to process the chop because of bad
// intersections
bool csRectRegion::chopEdgeIntersection(csRect &rect1, csRect &rect2)
{
  if ((rect1.xmax <= rect2.xmax && rect1.xmin >= rect2.xmin) ||
      (rect1.ymax <= rect2.ymax && rect1.ymin >= rect2.ymin))
  {
    csRect i(rect1);
    i.Intersect(rect2);
    rect1.Subtract(i);
    return true;
  }
  return false;
}

void csRectRegion::Include(csRect &rect)
{
  // If there are no rects in the region, add this and leave.
  if (region_count == 0)
  {
    pushRect(rect);
    return;
  }

  // Otherwise, we have to see if this rect creates a union with any other
  // rectangles.
  for(int i = 0; i < region_count; i++)
  {
    csRect const& r1 = region[i];
    csRect r2(rect);

    // Check to see if these even touch
    if (r2.Intersects(r1)==false)
      continue;
    
    // If r1 totally contains rect, then we leave.
    r2.Exclude(r1);
    if (r2.IsEmpty())
      return;

    // If rect totally contains r1, then we kill r1 from the list.
    r2.Set(r1);
    r2.Exclude(rect);

    if (r2.IsEmpty())
    {
      // Kill from list
      deleteRect(i);
      // Iterate
      continue;
    }
     
    // We know that the two rects intersect, but neither of them completely
    // contains each other.  Therefore, we must now see what to chop off of
    // what.  It may be more efficient to chop part off of one or the other.
    // Usually, it's easier to chop up the smaller one.

    bool edgeChopWorked;

    if (!(edgeChopWorked = chopEdgeIntersection(r1, r2)))
      edgeChopWorked = chopEdgeIntersection(r2, r1);
  
    // If we were able to chop the edge off of one of them, cool.
    if (edgeChopWorked)
      continue;

    // Otherwise we have to do the most irritating part: A full split operation
    // that may create other rects that need to be tested against the database
    // recursively.  For this algorithm, we fragment the one that is already in
    // the database, that way we don't cause more tests: we already know that
    // that rect is good.

    // Kill rect from list
    deleteRect(i);
  
    // Fragment it
    fragmentRect(r1, r2, true, true);
  } // end for
}
