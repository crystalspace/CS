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


//  This operation takes a rect r1 which completely contains rect r2
// and turns it into as many rects as it takes to exclude r2 from the
// area controlled by r1.
void
csRectRegion::fragmentContainedRect(csRect &r1, csRect &r2)
{
  // Edge flags
  const unsigned int LX=1, TY=2, RX=4, BY=8;
  unsigned int edges=0;

  // First check for edging.
  edges |= (r1.xmin == r2.xmin ? LX : 0); 
  edges |= (r1.ymin == r2.ymin ? TY : 0); 
  edges |= (r1.xmax == r2.xmax ? RX : 0); 
  edges |= (r1.ymax == r2.ymax ? BY : 0); 

  switch(edges)
  {
  case 0: 
    // This is the easy case. Split the r1 into four pieces that exclude r2.
    // The include function pre-checks for this case and exits, so it is 
    // properly handled.

    pushRect(csRect(r1.xmin,   r1.ymin,   r2.xmin-1, r1.ymax));   //left
    pushRect(csRect(r2.xmax+1, r1.ymin,   r1.xmax,   r1.ymax));   //right
    pushRect(csRect(r2.xmin,   r1.ymin,   r2.xmax,   r1.ymin-1)); //top
    pushRect(csRect(r2.xmin,   r2.ymax+1, r2.xmax,   r1.ymax));   //bottom
           
    return;

  case 1:
    // Three rects (top, right, bottom) [rect on left side, middle]
    
    pushRect(csRect(r1.xmin,   r1.ymin,   r1.xmax,   r2.ymin-1)); //top
    pushRect(csRect(r2.xmax+1, r2.ymin,   r1.xmax,   r2.ymax));   //right
    pushRect(csRect(r1.xmin,   r2.ymax+1, r1.xmax,   r1.ymax));   //bot
  
    return;

  case 2:
    // Three rects (bot, left, right)   [rect on top side, middle]
    
    pushRect(csRect(r1.xmin,   r2.ymax+1, r1.xmax,   r1.ymax));   //bot
    pushRect(csRect(r1.xmin,   r1.ymin,   r2.xmin-1, r2.ymax));   //left
    pushRect(csRect(r2.xmax+1, r1.ymin,   r1.xmax,   r2.ymax));   //right
        
    return;
  
  case 3:
    // Two rects (right, bottom)        [rect on top left corner]
    
    pushRect(csRect(r2.xmax+1, r1.ymin, r1.xmax,   r2.ymax)); //right
    pushRect(csRect(r1.xmin, r2.ymax+1, r1.xmax,   r1.ymax)); //bot
  
    return;
  
  case 4:
    // Three rects (top, left, bottom)  [rect on right side, middle]
    
    pushRect(csRect(r1.xmin, r1.ymin,   r1.xmax,   r2.ymin-1)); //top
    pushRect(csRect(r1.xmin, r2.ymin,   r2.xmax-1, r2.ymax));   //left
    pushRect(csRect(r1.xmin, r2.ymax+1, r1.xmax,   r1.ymax));   //bot
  
    return;

  case 5:
    // Two rects (top, bottom)          [rect in middle, horizontally touches left and right sides]
    
    pushRect(csRect(r1.xmin, r1.ymin,   r1.xmax,   r2.ymin-1)); //top
    pushRect(csRect(r1.xmin, r2.ymax+1, r1.xmax,   r1.ymax));   //bot
  
    return;

  case 6:
    // Two rects (left, bottom)         [rect on top, right corner]
    
    pushRect(csRect(r1.xmin, r1.ymin,   r2.xmin-1, r1.ymax));   //left
    pushRect(csRect(r2.xmin, r2.ymax+1, r1.xmax,   r1.ymax));   //bot
       
    return;

  case 7:
    // One rect (bottom)                [rect covers entire top]
    
    pushRect(csRect(r1.xmin, r2.ymax+1, r1.xmax,   r1.ymax));   //bot
       
    return;


  case 8:
    // Three rects (top, left, right)   [rect on bottom side, middle]
    
    pushRect(csRect(r1.xmin,   r1.ymin, r1.xmax,   r2.ymin-1)); //top
    pushRect(csRect(r1.xmin,   r2.ymin, r2.xmin-1, r1.ymax));   //left
    pushRect(csRect(r2.xmax+1, r2.ymin, r1.xmax,   r2.ymax));   //right
        
    return;

  case 9:
    // Two rects (right, top)           [rect on bottom, left corner]
    
    pushRect(csRect(r2.xmax+1, r2.ymin, r1.xmax, r1.ymax));   //right
    pushRect(csRect(r1.xmin,   r1.ymin, r1.xmax, r2.ymin-1)); //top
       
    return;

  case 10:
    // Two rects (left, right)          [rect in middle, vertically touches top and bottom sides]
    
    pushRect(csRect(r1.xmin,   r1.ymin, r2.xmin-1, r1.ymax)); //left
    pushRect(csRect(r2.xmax+1, r2.ymin, r1.xmax,   r1.ymax)); //right
  
    return;

  case 11:
    // One rect (right)                 [rect on left, vertically touches top and bottom sides]
    
    pushRect(csRect(r2.xmax+1, r1.ymin, r1.xmax,   r1.ymax)); //right
  
    return;

  case 12:
    // Two rects (left, top)            [rect on bottom, right corner]
    
    pushRect(csRect(r1.xmin, r1.ymin, r2.xmin-1, r1.ymax));   //left
    pushRect(csRect(r2.xmin, r1.ymin, r1.xmax,   r2.ymin-1)); //top
  
    return;

  case 13:
    // One rect (top)                   [rect on bottom, horizontally touches left and right sides]
    
    pushRect(csRect(r1.xmin, r1.ymin, r2.xmax,   r2.ymin-1)); //top
  
    return;

  case 14:
    // One rect (bot)                   [rect on top, horizontally touches left and right sides]
    
    pushRect(csRect(r1.xmin, r2.ymax+1, r1.xmax,   r1.ymax)); //bottom
  
    return;

  case 15:
    // No rects 
    // In this case, the rects cancel themselves out.

    return;
  
  }

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
    csRect tr1(r1), tr2(r2);

    tr2.Exclude(tr1);

    if (tr2.IsEmpty()) 
    {
      // It now becomes irritating.  Not only do we have to possibly split it into two
      // rectangles, we may have to split it into as few as one.  Call a separate
      // function to handle this business.

      fragmentContainedRect(r1, r2);

      return;
    }
      
  }

  // We may have to fragment r1 into three pieces if an entire edge of r2 is
  // inside r1.
  if (!testedEdge)
  {

    if (r1.Intersects(r2)) 
    {

    // Since fragment rect already test for all cases, the ideal method here is to call
    // fragment rect on the intersection of r1 and r2 with r1 as the fragmentee.  This
    // creates a properly fragmented system.

     csRect ri(r1);
     ri.Intersect(r2);

     fragmentContainedRect(r1, ri);
     
     return;
    }
  }
  
  // If we've gotten here then we should only have to break the rect into 2
  // pieces.
  if (r1.Contains(r2.xmin, r2.ymin))
  {
    // Break r1 into left, top since top left corner of r2 is inside.
    pushRect(csRect(r1.xmin, r2.ymin, r2.xmin-1, r1.ymax));   //left
    pushRect(csRect(r1.xmin, r1.ymin, r1.xmax,   r2.ymin-1)); //top 
  }
  else if (r1.Contains(r2.xmin, r2.ymax))
  {
    // Break r1 into left, bot since bot left corner of r2 is inside.
    pushRect(csRect(r1.xmin, r1.ymin,   r2.xmin-1, r2.ymax));  //left
    pushRect(csRect(r1.xmin, r2.ymax+1, r1.xmax,   r1.ymax));  //bot
  }
  else if (r1.Contains(r2.xmax, r2.ymin))
  {
    // Break r1 into right, top since top right corner of r2 is inside.
    pushRect(csRect(r2.xmax+1, r1.ymin, r1.xmax, r1.ymax));    //right
    pushRect(csRect(r1.xmin,   r1.ymin, r2.xmax, r2.ymin-1));  //top
  } 
  else
  {
    // Break r1 into right, bot since bot right corner of r2 is inside.
    pushRect(csRect(r2.xmax+1, r1.ymin,   r1.xmax, r1.ymax)); //right
    pushRect(csRect(r1.xmin,   r2.ymax+1, r2.xmax, r1.ymax)); //bot    
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
  int i;
  for(i = 0; i < region_count; i++)
  {
    csRect &r1 = region[i];
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

    r2.Set(rect);

    // Kill rect from list
    deleteRect(i);
  
    // Fragment it
    fragmentRect(r1, r2, true, true);
  } // end for
}

void 
csRectRegion::Exclude(csRect &rect)
{
  // If there are no rects in the region, just leave.  
  if (region_count == 0)
    return;

  // Otherwise, we have to see if this rect overlaps or touches any other.
  int i;
  for(i = 0; i < region_count; i++)
  {
    csRect &r1 = region[i];
    csRect r2(rect);

    // Check to see if these even touch
    if (r2.Intersects(r1)==false)
      continue;
    
    // If r1 totally contains rect, then we call the fragment contained rect function.
    r2.Exclude(r1);
    if (r2.IsEmpty())
    {
      csRect rc1(region[i]);
      deleteRect(i);

      fragmentContainedRect(rc1, rect);
      continue;
    }

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
    
    // This part is similiar to Include, except that we are trying to remove a portion.  Instead
    //  of calling chopEdgeIntersection, we actually have to fragment rect1 and chop off an edge
    //  of the excluding rect.  This code should be handled inside fragment rect.
    
    r2.Set(rect);

    // Kill rect from list
    deleteRect(i);
  
    // Fragment it
    fragmentRect(r1, r2, true, false);
  } // end for


  


}



