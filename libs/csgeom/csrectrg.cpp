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

#include <stdio.h>

const int MODE_EXCLUDE=0;
const int MODE_INCLUDE=1;

csRectRegion::csRectRegion() : region(0), region_count(0), region_max(0) {}
csRectRegion::~csRectRegion()
{
  if (region != 0)
    free(region);
}

void csRectRegion::pushRect(csRect const &r)
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

void 
csRectRegion::makeEmpty()
{
  region_count=0;
}


//  This operation takes a rect r1 which completely contains rect r2
// and turns it into as many rects as it takes to exclude r2 from the
// area controlled by r1.
void
csRectRegion::fragmentContainedRect(csRect &r1t, csRect &r2t)
{
  // Edge flags
  const unsigned int LX=1, TY=2, RX=4, BY=8;
  unsigned int edges=0;

  csRect r1(r1t), r2(r2t);

  // First check for edging.
  edges |= (r1.xmin == r2.xmin ? LX : 0); 
  edges |= (r1.ymin == r2.ymin ? TY : 0); 
  edges |= (r1.xmax == r2.xmax ? RX : 0); 
  edges |= (r1.ymax == r2.ymax ? BY : 0); 

  
  //printf("csrectrgn: fragmenting with rule %d\n", edges);
  //printf("\t%d,%d,%d,%d\n", r1.xmin, r1.ymin, r1.xmax, r1.ymax);
  //printf("\t%d,%d,%d,%d\n", r2.xmin, r2.ymin, r2.xmax, r2.ymax);

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
    // One rect (left)                   [rect on right, vertically touches top and bottom ]
    
    pushRect(csRect(r1.xmin, r1.ymin, r2.xmin-1, r1.ymax)); //bottom
  
    return;

  case 15:
    // No rects 
    // In this case, the rects cancel themselves out.

    // Include needs to special case this, otherwise it will not
    //  be handled correctly.

    return;
  
  }

}


// The purpose of this function is to take r1 and fragment it around r2,
// removing the area that overlaps.  This function can be used by either
// Include() or Exclude(), since it simply fragments r1.
void csRectRegion::fragmentRect(csRect &r1, csRect &r2, int mode)
{
  // We may have to fragment r1 into four pieces if r2 is totally inside r1
  if (mode==MODE_EXCLUDE)
  {
    csRect tr1(r1), tr2(r2);

    tr2.Exclude(tr1);

    if (tr2.IsEmpty()) 
    {
      // It now becomes irritating.  Not only do we have to possibly split it into two
      // rectangles, we may have to split it into as few as one.  Call a separate
      // function to handle this business.

      fragmentContainedRect(r2, r1);

      return;
    }
      
  }

  // We may have to fragment r1 into three pieces if an entire edge of r2 is
  // inside r1.
  if (r1.Intersects(r2)) 
  {
     // Since fragment rect already test for all cases, the ideal method here is to call
     // fragment rect on the intersection of r1 and r2 with r1 as the fragmentee.  This
     // creates a properly fragmented system.
     //
     // We know that rect1 is already good, so we simply fragment rect2 and gather it's
     // fragments into the fragment buffer for further consideration.
     //
     // In exclude mode, we don't want to remove parts of r2 from r1, whereas in include
     // mode we want to perform an optimal merge of the two, or remove parts of r1 from r2.

     csRect ri(r1);
     ri.Intersect(r2);

     if (mode==MODE_INCLUDE)
     {        

       if (r1.Area() < r2.Area()) 
       {
         csRect temp(r1);
         
         r1.Set(r2);
         r2.Set(temp);
       }

       // Push r1 back into the regions list
       pushRect(r1);

       // Perform fragment and gather.
       markForGather();
       fragmentContainedRect(r2, ri);
       gatherFragments();
     }
     else
     {
       // Perform fragment on r1
       fragmentContainedRect(r1, ri);

       // Now fragment r2 and gather it for further consideration.
       markForGather();
       fragmentContainedRect(r2, ri);
       gatherFragments();
     }

     return;
  }
}

void 
csRectRegion::markForGather()
{
  gather_mark=region_count;
}

void 
csRectRegion::gatherFragments()
{
  int i,j=gather_mark;

  while(j<region_count)
  {
    for(i=0; i<32; ++i)
      if (fragment[i].IsEmpty())
      {
        fragment[i].Set(region[j]);
        j++;
        break;
      }
  }

  region_count=gather_mark;
}

void csRectRegion::Include(csRect &nrect)
{
  // Ignore an empty rect
  if (nrect.IsEmpty())
    return;

  // If there are no rects in the region, add this and leave.
  if (region_count == 0)
  {
    pushRect(nrect);
    return;
  }

  int i;
  bool no_fragments;
  csRect rect(nrect);

  /// Clear the fragment buffer
  for(i=0; i<32; ++i)
    fragment[i].MakeEmpty();

  do
  {
    
    bool untouched=true;

    no_fragments=true;

    // Otherwise, we have to see if this rect creates a union with any other
    // rectangles.
    for(i = 0; i < region_count; i++)
    {
      csRect &r1 = region[i];
      csRect r2(rect);
    
      // Check to see if these even touch, if not, next.
      if (r2.Intersects(r1)==false)
        continue;
    
      // If r1 totally contains rect, then we leave.
      r2.Exclude(r1);
      if (r2.IsEmpty())
      {
        // Mark it so we don't add it in
        untouched=false;
        break;
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
         
      // Otherwise we have to do the most irritating part: A full split operation
      // that may create other rects that need to be tested against the database
      // recursively.  For this algorithm, we fragment the one that is already in
      // the database, that way we don't cause more tests: we already know that
      // that rect is good.

      r2.Set(rect);

      // Kill rect from list
      deleteRect(i);
  
      // Fragment it
      fragmentRect(r1, r2, MODE_INCLUDE);
      
      // Mark it
      untouched=false;
    
    } // end for

    // In the end, we need to put the rect on the stack
    if (!rect.IsEmpty() && untouched) pushRect(rect);

    // Check and see if we have fragments to consider
    for(i=0; i<32; ++i)
    {
      if (!(fragment[i].IsEmpty()))
      {
        rect.Set(fragment[i]);
        fragment[i].MakeEmpty();
        no_fragments=false;
        break;
      }
    }

  } while(!no_fragments);
}

void 
csRectRegion::Exclude(csRect &rect)
{
  // Ignore an empty rect
  if (rect.IsEmpty())
    return;

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
    fragmentRect(r1, r2, MODE_EXCLUDE);
  } // end for

}



