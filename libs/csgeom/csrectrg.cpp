#include "cssysdef.h"
#include "csgeom/csrectrg.h"


void
csRectRegion::fragmentRect(csRect &r1, csRect &r2, bool testedContains, bool testedEdge)
{
  //  The purpose of this function is to take r1 and fragment it around r2, removing
  // the area that overlaps.  This function can be used by either Include or Exclude,
  // since it simply fragments r1.

  //  We may have to fragment r1 into four pieces if r2 is totally inside r1
  if (!testedContains)
  {

  }

  //  We may have to fragment r1 into three pieces if an entire edge of r2 is inside r1.
  if (!testedEdge)
  {

  }
  
  //  If we've gotten here then we should only have to break the rect into 2 pieces.
  if (r1.Contains(r2.xmin, r2.ymin))
  {
    // Break r1 into left, top since top left corner of r2 is inside.
    csRect *left=new csRect, *top=new csRect;

    left->Set(r1.xmin, r2.ymin, r2.xmin-1, r1.ymax);
     top->Set(r1.xmin, r1.ymin, r1.xmax,   r2.ymin-1);  

     region.Push(left);
     region.Push(top);
  }
  else if (r1.Contains(r2.xmin, r2.ymax))
  {
    // Break r1 into left, bot since bot left corner of r2 is inside.
    csRect *left=new csRect, *bot=new csRect;

    left->Set(r1.xmin, r1.ymin,   r2.xmin-1, r2.ymax);
     bot->Set(r1.xmin, r2.ymax+1, r1.xmax,   r1.ymax);  

     region.Push(left);
     region.Push(bot);
  }
  else if (r1.Contains(r2.xmax, r2.ymin))
  {
    // Break r1 into right, top since top right corner of r2 is inside.
    csRect *right=new csRect, *top=new csRect;

    right->Set(r2.xmax+1, r1.ymin, r1.xmax, r1.ymax);
      top->Set(r1.xmin,   r1.ymin, r2.xmax, r2.ymin-1);  

     region.Push(right);
     region.Push(top);
  } 
  else
  {
    // Break r1 into right, bot since bot right corner of r2 is inside.
    csRect *right=new csRect, *bot=new csRect;

    right->Set(r2.xmax+1, r1.ymin,   r1.xmax, r1.ymax);
      bot->Set(r1.xmin,   r2.ymax+1, r2.xmax, r1.ymax);  

     region.Push(right);
     region.Push(bot);
  }
}

bool 
csRectRegion::chopEdgeIntersection(csRect &rect1, csRect &rect2)
{
  //  Chop the intersection of rect1 and rect2 off of rect1 (the smaller rect)
  // Tests to make sure the intersection happens on an edge and not a corner.
  // Returns false if it is unable to process the chop because of bad intersections
 
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

void
csRectRegion::Include(csRect &rect)
{
  int i;

  // If there are no rects in the region, add this and leave.
  if (region.Length()==0)
  {
    csRect *nr = new csRect(rect);
    region.Push(nr);

    return;
  }

  // Otherwise, we have to see if this rect creates a union with any other rectangles.
  for(i=0; i<region.Length(); ++i)
  {
    csRect *r1 = (csRect *)region[i];
    csRect  r2(rect);


    // Check to see if these even touch
    if (r2.Intersects(*r1)==false)
      continue;
    
    // If r1 totally contains rect, then we leave.
    r2.Exclude(*r1);

    if (r2.IsEmpty())
      return;

    // If rect totally contains r1, then we kill r1 from the list.
    r2.Set(*r1);
    r2.Exclude(rect);

    if (r2.IsEmpty())
    {
      // Release space
      delete r1;

      // Kill from list
      region.Delete(i);

      // Iterate
      continue;
    }

     
    // We know that the two rects intersect, but neither of them completely contains
    //  each other.  Therefore, we must now see what to chop off of what. It may be
    //  more efficient to chop part off of one or the other.  Usually, it's easier 
    //  to chop up the smaller one.

 
    bool edgeChopWorked;

    if ((edgeChopWorked=chopEdgeIntersection(*r1, r2))==false)
      edgeChopWorked=chopEdgeIntersection(r2, *r1);
  
    // If we were able to chop the edge off of one of them, cool.
    if (edgeChopWorked)
      continue;

    // Otherwise we have to do the most irritating part: A full split operation that
    //  may create other rects that need to be tested against the database recursively.
    //  For this algorithm, we fragment the one that is already in the database, that
    //  way we don't cause more tests: we already know that that rect is good.

    // Kill rect from list
    region.Delete(i);
  
    // Fragment it
    fragmentRect(*r1, r2, true, true);

    // Delete old rect
    delete r1;
      
  } // end for
}