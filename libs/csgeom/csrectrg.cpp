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



#ifdef CS_DEBUG_RECT_REG
#include <time.h>
#include "csutil/randomgen.h"
#endif

const int MODE_EXCLUDE = 0;
const int MODE_INCLUDE = 1;

csRectRegion::csRectRegion () :
  region(0),
  region_count(0),
  region_max(0)

{
}

csRectRegion::~csRectRegion ()
{
  if (region != 0)
    free (region);
}

void csRectRegion::pushRect (csRect const &r)
{
  if (region_count >= region_max)
  {
    region_max += 64;

    int const nbytes = region_max * sizeof (region[0]);
    if (region == 0)
      region = (csRect *)malloc (nbytes);
    else
      region = (csRect *)realloc (region, nbytes);
  }

  region[region_count++] = r;
}

void csRectRegion::deleteRect (int i)
{
  if (region_count > 0 && i >= 0 && i < --region_count)
    memmove (
      region + i,
      region + i + 1,
      (region_count - i) * sizeof (region[0]));
}

void csRectRegion::makeEmpty ()
{
  region_count = 0;
}

//  This operation takes a rect r1 which completely contains rect r2
// and turns it into as many rects as it takes to exclude r2 from the
// area controlled by r1.
void csRectRegion::fragmentContainedRect (csRect &r1t, csRect &r2t)
{
  // Edge flags
  const unsigned int LX = 1, TY = 2, RX = 4, BY = 8;
  unsigned int edges = 0;

  csRect r1 (r1t), r2 (r2t);

  // First check for edging.
  edges |= (r1.xmin == r2.xmin ? LX : 0);
  edges |= (r1.ymin == r2.ymin ? TY : 0);
  edges |= (r1.xmax == r2.xmax ? RX : 0);
  edges |= (r1.ymax == r2.ymax ? BY : 0);

  //csPrintf("csrectrgn: fragmenting with rule %u\n", edges);
  //csPrintf("\t%d,%d,%d,%d\n", r1.xmin, r1.ymin, r1.xmax, r1.ymax);
  //csPrintf("\t%d,%d,%d,%d\n", r2.xmin, r2.ymin, r2.xmax, r2.ymax);
  switch (edges)
  {
    case 0:
      // This is the easy case. Split the r1 into four pieces that exclude r2.
      // The include function pre-checks for this case and exits, so it is
      // properly handled.
      pushRect (csRect (r1.xmin, r1.ymin, r2.xmin, r1.ymax)); //left
      pushRect (csRect (r2.xmax, r1.ymin, r1.xmax, r1.ymax)); //right
      pushRect (csRect (r2.xmin, r1.ymin, r2.xmax, r2.ymin)); //top
      pushRect (csRect (r2.xmin, r2.ymax, r2.xmax, r1.ymax)); //bottom
      return;

    case 1:
      // Three rects (top, right, bottom) [rect on left side, middle]
      pushRect (csRect (r1.xmin, r1.ymin, r1.xmax, r2.ymin)); //top
      pushRect (csRect (r2.xmax, r2.ymin, r1.xmax, r2.ymax)); //right
      pushRect (csRect (r1.xmin, r2.ymax, r1.xmax, r1.ymax)); //bot
      return;

    case 2:
      // Three rects (bot, left, right)   [rect on top side, middle]
      pushRect (csRect (r1.xmin, r2.ymax, r1.xmax, r1.ymax)); //bot
      pushRect (csRect (r1.xmin, r1.ymin, r2.xmin, r2.ymax)); //left
      pushRect (csRect (r2.xmax, r1.ymin, r1.xmax, r2.ymax)); //right
      return;

    case 3:
      // Two rects (right, bottom)        [rect on top left corner]
      pushRect (csRect (r2.xmax, r1.ymin, r1.xmax, r2.ymax)); //right
      pushRect (csRect (r1.xmin, r2.ymax, r1.xmax, r1.ymax)); //bot
      return;

    case 4:
      // Three rects (top, left, bottom)  [rect on right side, middle]
      pushRect (csRect (r1.xmin, r1.ymin, r1.xmax, r2.ymin)); //top
      pushRect (csRect (r1.xmin, r2.ymin, r2.xmin, r2.ymax)); //left
      pushRect (csRect (r1.xmin, r2.ymax, r1.xmax, r1.ymax)); //bot
      return;

    case 5:
      // Two rects (top, bottom)          [rect in middle, horizontally
      //                                  touches left and right sides]
      pushRect (csRect (r1.xmin, r1.ymin, r1.xmax, r2.ymin)); //top
      pushRect (csRect (r1.xmin, r2.ymax, r1.xmax, r1.ymax)); //bot
      return;

    case 6:
      // Two rects (left, bottom)         [rect on top, right corner]
      pushRect (csRect (r1.xmin, r1.ymin, r2.xmin, r1.ymax)); //left
      pushRect (csRect (r2.xmin, r2.ymax, r1.xmax, r1.ymax)); //bot
      return;

    case 7:
      // One rect (bottom)                [rect covers entire top]
      pushRect (csRect (r1.xmin, r2.ymax, r1.xmax, r1.ymax)); //bot
      return;

    case 8:
      // Three rects (top, left, right)   [rect on bottom side, middle]
      pushRect (csRect (r1.xmin, r1.ymin, r1.xmax, r2.ymin)); //top
      pushRect (csRect (r1.xmin, r2.ymin, r2.xmin, r1.ymax)); //left
      pushRect (csRect (r2.xmax, r2.ymin, r1.xmax, r1.ymax)); //right
      return;

    case 9:
      // Two rects (right, top)           [rect on bottom, left corner]
      pushRect (csRect (r2.xmax, r2.ymin, r1.xmax, r1.ymax)); //right
      pushRect (csRect (r1.xmin, r1.ymin, r1.xmax, r2.ymin)); //top
      return;

    case 10:
      // Two rects (left, right)          [rect middle, vert touches top/bot]
      pushRect (csRect (r1.xmin, r1.ymin, r2.xmin, r1.ymax)); //left
      pushRect (csRect (r2.xmax, r1.ymin, r1.xmax, r1.ymax)); //right
      return;

    case 11:
      // One rect (right)                 [rect left, vert touches top/bot]
      pushRect (csRect (r2.xmax, r1.ymin, r1.xmax, r1.ymax)); //right
      return;

    case 12:
      // Two rects (left, top)            [rect bottom, right corner]
      pushRect (csRect (r1.xmin, r1.ymin, r2.xmin, r1.ymax)); //left
      pushRect (csRect (r2.xmin, r1.ymin, r1.xmax, r2.ymin)); //top
      return;

    case 13:
      // One rect (top)                   [rect bottom, hor touches left/right]
      pushRect (csRect (r1.xmin, r1.ymin, r1.xmax, r2.ymin)); //top
      return;

    case 14:
      // One rect (left)                   [rect right, vert touches top/bot]
      pushRect (csRect (r1.xmin, r1.ymin, r2.xmin, r1.ymax)); //bottom
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
// Include() or Exclude(), since it simply fragments r1 (but always checks
// for swapping!)  For Exclusion, r2 is ALWAYS the excluding rect!
void csRectRegion::fragmentRect (csRect &r1, csRect &r2, int mode)
{
  // We may have to fragment r1 into three pieces if an entire edge of r2 is
  // inside r1.
  if (r1.Intersects (r2))
  {
    // Since fragment rect already test for all cases, the ideal method here
    // is to call fragment rect on the intersection of r1 and r2 with r1
    // as the fragmentee.  This creates a properly fragmented system.
    //
    // We know that rect1 is already good, so we simply fragment rect2 and
    // gather it's fragments into the fragment buffer for further consideration.
    //
    // In exclude mode, we don't want to remove parts of r2 from r1, whereas
    // in include mode we want to perform an optimal merge of the two, or
    // remove parts of r1 from r2.
    csRect ri (r1);
    ri.Intersect (r2);

    if (mode == MODE_INCLUDE)
    {
      if (r1.Area () < r2.Area ())
      {
        csRect temp (r1);
        r1.Set (r2);
        r2.Set (temp);
      }

      // Push r1 back into the regions list
      pushRect (r1);

      // Perform fragment and gather.
      markForGather ();
      fragmentContainedRect (r2, ri);
      gatherFragments ();
    }
    else
    {
      // Fragment inclusion rect around intersection (keep)
      fragmentContainedRect (r1, ri);
    }
    return;
  }
}

void csRectRegion::markForGather ()
{
  gather_mark = region_count;
}

void csRectRegion::gatherFragments ()
{
  int i, j = gather_mark;

  while (j < region_count)
  {
    for (i = 0; i < FRAGMENT_BUFFER_SIZE; ++i)
      if (fragment[i].IsEmpty ())
      {
        fragment[i].Set (region[j]);
        break;
      }

    j++;
  }

  region_count = gather_mark;
}

void csRectRegion::nkSplit (csRect &r1, csRect &r2)
{
  r2.Intersect (r1);

  if (r1.ymin < r2.ymin) // upper stripe
  {
    pushRect (csRect(r1.xmin,r1.ymin, r1.xmax, r2.ymin));
  }

  if (r1.xmin < r2.xmin) // left stripe
  {
    pushRect (csRect(r1.xmin,r2.ymin, r2.xmin, r2.ymax));
  }

  if (r1.xmax > r2.xmax) // right stripe
  {
    //pushRect (csRect(r2.xmin, r2.ymin, r1.xmax, r2.ymax));
    pushRect( csRect(r2.xmax, r2.ymin, r1.xmax, r2.ymax) );
  }

  if (r1.ymax > r2.ymax) // lower stripe
  {
    pushRect (csRect(r1.xmin, r2.ymax, r1.xmax, r1.ymax));
  }
}

void csRectRegion::Include (const csRect &nrect)
{
  // Ignore an empty rect
  if (nrect.IsEmpty ())
    return;

  // If there are no rects in the region, add this and leave.
  if (region_count == 0)
  {
    pushRect (nrect);
    return;
  }

  int i;
  bool no_fragments;
  csRect rect (nrect);

  /// Clear the fragment buffer
  for (i = 0; i < FRAGMENT_BUFFER_SIZE; ++i) fragment[i].MakeEmpty ();

  do
  {
    bool untouched = true;

    no_fragments = true;

    // Otherwise, we have to see if this rect creates a union with any other
    // rectangles.
    int last_to_consider = region_count;
    for (i = 0; i < last_to_consider; i++)
    {
      csRect &r1 = region[i];
      csRect r2 (rect);

      // Check to see if these even touch, if not, next.
      if (r2.Intersects (r1) == false) continue;

      // If r1 totally contains rect, then we leave.
      r2.Exclude (r1);
      if (r2.IsEmpty ())
      {
        // Mark it so we don't add it in
        untouched = false;
        break;
      }

      // If rect totally contains r1, then we kill r1 from the list.
      r2.Set (r1);
      r2.Exclude (rect);

      if (r2.IsEmpty ())
      {
        // Kill from list
        deleteRect (i);
        i--;
        last_to_consider--;
        // Iterate
        continue;
      }

      /*
      // Otherwise we have to do the most irritating part: A full split
      // operation that may create other rects that need to be tested against
      // the database recursively.  For this algorithm, we fragment the
      // one that is already in the database, that way we don't cause more
      // tests: we already know that that rect is good.
      r2.Set (rect);

      // Kill rect from list
      deleteRect (i);

      // Fragment it
      fragmentRect (r1, r2, MODE_INCLUDE);
      */

      r2.Set (rect);
      nkSplit (r1, r2);
      deleteRect (i);
      i--;
      last_to_consider--;
      // Mark it
      //      untouched = true;
    } // end for

    // In the end, we need to put the rect on the stack
    if (!rect.IsEmpty () && untouched)
      pushRect (rect);

    // Check and see if we have fragments to consider
    for (i = 0; i < FRAGMENT_BUFFER_SIZE; ++i)
    {
      if (!(fragment[i].IsEmpty ()))
      {
        rect.Set (fragment[i]);
        fragment[i].MakeEmpty ();
        no_fragments = false;
        break;
      }
    }
  } while (!no_fragments);
}

void csRectRegion::Exclude (const csRect &nrect)
{
  // Ignore an empty rect
  if (nrect.IsEmpty ())
    return;

  // If there are no rects in the region, just leave.
  if (region_count == 0)
    return;

  int i;
  csRect rect (nrect);

  /// Clear the fragment buffer
  for (i = 0; i < FRAGMENT_BUFFER_SIZE; ++i)
    fragment[i].MakeEmpty ();

  // Otherwise, we have to see if this rect overlaps or touches any other.
  for (i = 0; i < region_count; i++)
  {
    csRect r1 (region[i]);
    csRect r2 (rect);

    // Check to see if these even touch
    if (r2.Intersects (r1) == false) continue;

    // Check to see if the inclusion rect is totally dominated by the
    // exclusion rect.
    r1.Exclude (r2);
    if (r1.IsEmpty ())
    {
      deleteRect (i);
      i--;
      continue;
    }

    // Check to see if the exclusion rect is totally dominated by the
    // exclusion rect
    r1.Set (region[i]);
    r2.Exclude (r1);
    if (r2.IsEmpty ())
    {
      r2.Set (rect);
      deleteRect (i);
      fragmentContainedRect (r1, r2);
      i = 0;
      continue;
    }

    r2.Set (rect);

    // This part is similiar to Include, except that we are trying to remove
    // a portion.  Instead of calling chopEdgeIntersection, we actually have
    // to fragment rect1 and chop off an edge of the excluding rect.  This
    // code should be handled inside fragment rect.

    // Kill rect from list
    deleteRect (i);
    i--;

    // Fragment it
    fragmentRect (r1, r2, MODE_EXCLUDE);
  }
}

void csRectRegion::ClipTo (csRect &clip)
{
  for (int i = region_count - 1; i >= 0; i--)
  {
    region[i].Intersect (clip);
    if (region[i].IsEmpty ())
      deleteRect (i);
  }
}



/******************************* csRectRegionDebugger *********************************************/

#ifdef CS_DEBUG_RECT_REG

csRectRegionDebug::csRectRegionDebug() 
{
  rand_seed = time(0);
  num_tests_complete = 0;
  rand = new csRandomGen(rand_seed);
  MakeEmpty();
}

csRectRegionDebug::~csRectRegionDebug()
{
  delete rand;
}

void csRectRegionDebug::Include(const csRect &rect)
{
  CS_ASSERT(CheckBounds(rect));
  for(int i = rect.xmin; i < rect.xmax; i++)
  {
    for(int j = rect.ymin; j < rect.ymax; j++)
    {
      area[i][j] = true;
    }
  }
}

void csRectRegionDebug::Exclude(const csRect &rect)
{
  CS_ASSERT(CheckBounds(rect));
  for(int i = rect.xmin; i < rect.xmax; i++)
  {
    for(int j = rect.ymin; j < rect.ymax; j++)
    {
      area[i][j] = false;
    }
  }
}

void csRectRegionDebug::ClipTo(const csRect &clip)
{
  for(int i = 0; i < CS_RECT_REG_SIZE; i++)
  {
    for(int j = 0; j < CS_RECT_REG_SIZE; j++)
    {
      if(!clip.Contains(i,j))
        area[i][j] = false;
    }
  }
}

void csRectRegionDebug::AssertEqual(const csRectRegion &r)
{
  int i,j,k;
  for(i = 0; i < r.Count(); i++)
  {
    csRect rect = r.RectAt(i);
    CS_ASSERT(CheckBounds(rect));
    for(j = rect.xmin; j < rect.xmax; j++)
    {
      for(k = rect.ymin; k < rect.ymax; k++)
      {
        CS_ASSERT(area[j][k] == true);
        area[j][k] = false;
      }
    }
  }

  for(i = 0; i < CS_RECT_REG_SIZE; i++)
  {
    for(j = 0; j < CS_RECT_REG_SIZE; j++)
    {
      CS_ASSERT(area[i][j] == false);
    }
  }

  for(i = 0; i < r.Count(); i++)
  {
    csRect rect = r.RectAt(i);
    for(j = rect.xmin; j < rect.xmax; j++)
    {
      for(int k = rect.ymin; k < rect.ymax; k++)
      {
        area[j][k] = true;
      }
    }
  }

  num_tests_complete++;
}

void csRectRegionDebug::MakeEmpty()
{
  for(int i = 0; i < CS_RECT_REG_SIZE; i++)
  {
    for(int j = 0; j < CS_RECT_REG_SIZE; j++)
    {
      area[i][j] = false;
    }
  }
}

bool csRectRegionDebug::CheckBounds(const csRect &rect)
{
  return rect.xmin >= 0 && rect.ymin >= 0 &&
         rect.xmax <= CS_RECT_REG_SIZE && rect.ymax <= CS_RECT_REG_SIZE;
}


void csRectRegionDebug::UnitTest()
{

  csPrintf("Running tests");

  int i,j;
  csRectRegion rr;
  rr.makeEmpty();
  MakeEmpty();
  AssertEqual(rr);



  // include test
  for(i= 0; i < 500; i++)
  {
    csRect r = RandNonEmptyRect();
    rr.Include(r);
    Include(r);
    AssertEqual(rr);
  }

  rr.makeEmpty();
  MakeEmpty();
  AssertEqual(rr);

  for(i = 0; i < 500; i++)
  {
    csRect r = RandRect();
    rr.Include(r);
    Include(r);
    AssertEqual(rr);
  }

  rr.makeEmpty();
  MakeEmpty();
  AssertEqual(rr);
  


  //exclude test
  csRect r(0,0,CS_RECT_REG_SIZE,CS_RECT_REG_SIZE);
  Include(r);
  rr.Include(r);
  AssertEqual(rr);

  for(i = 0; i < 500; i++)
  {
    csRect r = RandRect();
    rr.Exclude(r);
    Exclude(r);
    AssertEqual(rr);
  }

  csRect r1(0,0,CS_RECT_REG_SIZE,CS_RECT_REG_SIZE);
  Include(r1);
  rr.Include(r1);
  AssertEqual(rr);

  for(i = 0; i < 500; i++)
  {
    csRect r = RandNonEmptyRect();
    rr.Exclude(r);
    Exclude(r);
    AssertEqual(rr);
  }



  // Clip test
  rr.makeEmpty();
  MakeEmpty();
  AssertEqual(rr);

  for(i = 0; i < 100; i++)
  {
    for(j = 0; j < 50; j++)
    {
      csRect r = RandNonEmptyRect();
      rr.Include(r);
      Include(r);
    }

    csRect r = RandNonEmptyRect();
    ClipTo(r);
    rr.ClipTo(r);
    AssertEqual(rr);
  }

  rr.makeEmpty();
  MakeEmpty();
  AssertEqual(rr);

  for(i = 0; i < 10; i++)
  {
    for(j = 0; j < 50; j++)
    {
      csRect r = RandRect();
      rr.Include(r);
      Include(r);
    }

    csRect r = RandRect();
    ClipTo(r);
    rr.ClipTo(r);
    AssertEqual(rr);
  }

  // intermixed op test
  rr.makeEmpty();
  MakeEmpty();
  AssertEqual(rr);

  for(i = 0; i < 50000; i++)
  {

    if(i % 1000 == 0)
      csPrintf(".");

    unsigned int op = rand->Get(11);
    csRect r = RandRect();
    if(op < 5)
    {
      Include(r);
      rr.Include(r);
    }
    else if(op < 10)
    {
      Exclude(r);
      rr.Exclude(r);
    }
    else
    {
      ClipTo(r);
      rr.ClipTo(r);
    }

    AssertEqual(rr);
  }

  csPrintf("Done\n");

}


csRect csRectRegionDebug::RandRect()
{
  csRect r;
  r.xmin = rand->Get(CS_RECT_REG_SIZE);
  r.ymin = rand->Get(CS_RECT_REG_SIZE);
  r.xmax = rand->Get(CS_RECT_REG_SIZE+1);
  r.ymax = rand->Get(CS_RECT_REG_SIZE+1);
  return r;
}

csRect csRectRegionDebug::RandNonEmptyRect()
{
  csRect r;
  r.xmin = rand->Get(CS_RECT_REG_SIZE);
  r.ymin = rand->Get(CS_RECT_REG_SIZE);
  r.xmax = r.xmin + rand->Get(CS_RECT_REG_SIZE-r.xmin) + 1;
  r.ymax = r.ymin + rand->Get(CS_RECT_REG_SIZE-r.ymin) + 1;
  return r;
}

#endif

