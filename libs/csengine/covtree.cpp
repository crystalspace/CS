/*
    Copyright (C) 1999 by Jorrit Tyberghein
  
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
#include "csgeom/math2d.h"
#include "csengine/covtree.h"
#include "csengine/covtreep.h"
#include "igraph2d.h"

//---------------------------------------------------------------------------

csCovMaskLUT* lut;

// These defines test bit 0 of a mask.
#define INSIDE(imsk) ((imsk & 1) == 1)
#define OUTSIDE(omsk) ((omsk & 1) == 1)
#define NOT_INSIDE(imsk) ((imsk & 1) == 0)
#define NOT_OUTSIDE(omsk) ((omsk & 1) == 0)
#define PARTIAL(imsk,omsk) ((imsk & 1) == 0 && (omsk & 1) == 0)

template <class Child>
bool csCovTreeNode<Child>::UpdatePolygon (csVector2* poly, int num_poly,
  	float* dxdy, float* dydx,
	int hor_offs, int ver_offs)
{
  csCovMaskTriage pol_mask = lut->GetTriageMask (poly, num_poly, dxdy, dydx,
  	hor_offs, ver_offs, GetHorizontalSize (), GetVerticalSize ());

  // Copy polygon mask to this one.
  Copy (pol_mask);

  // Trivial case. If mask is empty then nothing remains to be done.
  if (IsEmpty ()) return false;

  // Trivial case. If mask is full then we can simply return true
  // and nothing remains to be done.
  if (IsFull ()) return true;

  // Scan all bits of current mask and traverse to child whenever
  // a bit is 1.
  csMask msk_in = in;
  csMask msk_out = out;
  int bits = CS_CM_MASKBITS;
  int idx = 0;		// Index for computing column/row in mask.
  int col, row, new_hor_offs, new_ver_offs;

  bool looped = false;
again:

  while (bits > 0)
  {
    // If partial then we proceed.
    if (PARTIAL (msk_in, msk_out))
    {
      // Partial. Update child.
      col = idx & CS_CM_HORMASK;
      row = idx >> CS_CM_HORSHIFT;
      new_hor_offs = hor_offs + col * Child::GetHorizontalSize ();
      new_ver_offs = ver_offs + row * Child::GetVerticalSize ();
      children[idx].UpdatePolygon (poly, num_poly, dxdy, dydx,
		new_hor_offs, new_ver_offs);
    }

    // Go to next bit.
    msk_in >>= 1;
    msk_out >>= 1;
    bits--;
    idx++;
  }

# if defined(CS_CM_8x8)
  if (looped) return true;
  msk_in = in2;
  msk_out = out2;
  bits = CS_CM_MASKBITS;
  looped = true;
  goto again;
# endif

  return true;
}

template <class Child>
bool csCovTreeNode<Child>::TestPolygon (csVector2* poly, int num_poly,
  	float* dxdy, float* dydx,
	int hor_offs, int ver_offs) const
{
  // Trivial case. If this mask is full then nothing remains to be done.
  if (IsFull ()) return false;

  csCovMaskTriage pol_mask = lut->GetTriageMask (poly, num_poly, dxdy, dydx,
  	hor_offs, ver_offs, GetHorizontalSize (), GetVerticalSize ());

//@@@@@ THERE IS A TRIVIAL CASE WITH LOGICAL OPS ON ENTIRE TWO MASKS!

  // Trivial case. If polygon mask is empty then nothing remains to be done.
  if (pol_mask.IsEmpty ()) return false;

  // Trivial case. If this mask is empty then we have visibility.
  if (IsEmpty ()) return true;

  // Scan all bits of the two masks and combine them.
  // @@@ Consider an optimization where we first test on ranges
  // of 4 or 8 bits before continuing further tests.
  csMask pol_in = pol_mask.in;
  csMask msk_in = in;
  csMask pol_out = pol_mask.out;
  csMask msk_out = out;
  int bits = CS_CM_MASKBITS;
  int idx = 0;		// Index for computing column/row in mask.
  int col, row;
  int new_hor_offs, new_ver_offs;

  bool looped = false;
again:

  while (bits > 0)
  {
    // If outside polygon then nothing happens.
    if (NOT_OUTSIDE (pol_out))
    {
      // Otherwise we test if we are fully inside the polygon.
      if (INSIDE (pol_in))
      {
        // Fully inside.
	// If this mask is outside or partial then we have a hit.
	if (NOT_INSIDE (msk_in)) return true;
      }
      else
      {
        // Partially inside.
	// If this mask not fully inside then we need to test further.
	if (NOT_INSIDE (msk_in))
	{
	  // If this mask is outside then we have visibility.
	  // Otherwise we traverse this child and let the child control
	  // visibility.
	  if (OUTSIDE (msk_out))
	  {
	    // Mask outside.
	    return true;
	  }
	  else
	  {
      	    col = idx & CS_CM_HORMASK;
      	    row = idx >> CS_CM_HORSHIFT;
      	    new_hor_offs = hor_offs + col * Child::GetHorizontalSize ();
      	    new_ver_offs = ver_offs + row * Child::GetVerticalSize ();
	    // Mask partial.
	    if (children[idx].TestPolygon (poly, num_poly, dxdy, dydx,
		new_hor_offs, new_ver_offs))
	    {
	      return true;
	    }
	  }
	}
      }
    }

    // Go to next bit.
    pol_in >>= 1;
    msk_in >>= 1;
    pol_out >>= 1;
    msk_out >>= 1;
    bits--;
    idx++;
  }

# if defined(CS_CM_8x8)
  if (looped) return false;

  pol_in = pol_mask.in2;
  msk_in = in2;
  pol_out = pol_mask.out2;
  msk_out = out2;
  bits = CS_CM_MASKBITS;

  looped = true;
  goto again;
# endif

  return false;
}

template <class Child>
bool csCovTreeNode<Child>::InsertPolygon (csVector2* poly, int num_poly,
  	float* dxdy, float* dydx,
	int hor_offs, int ver_offs)
{
  // Trivial case. If this mask is full then nothing remains to be done.
  if (IsFull ()) return false;

  // Trivial case. If this mask is empty then we simply copy
  // the polygon onto the mask and the children of this node.
  if (IsEmpty ())
  {
    return UpdatePolygon (poly, num_poly, dxdy, dydx, hor_offs, ver_offs);
  }

  csCovMaskTriage pol_mask = lut->GetTriageMask (poly, num_poly, dxdy, dydx,
  	hor_offs, ver_offs, GetHorizontalSize (), GetVerticalSize ());

  // Trivial case. If polygon mask is empty then nothing remains to be done.
  if (pol_mask.IsEmpty ()) return false;

  // Trivial case. If polygon mask is full then we can simply set
  // this mask to full and return modified.
  if (pol_mask.IsFull ())
  {
    Inside ();
    return true;
  }

//@@@@ THERE IS ANOTHER TRIVIAL CASE BASED ON LOGICAL OPS OF THE MASKS TO SEE
//IF ANYTHING NEEDS TO BE DONE!


  // Modified status. Initialize at false.
  bool modified = false;

  // Scan all bits of the two masks and combine them.
  // @@@ Consider an optimization where we first test on ranges
  // of 4 or 8 bits before continuing further tests.
  csMask pol_in = pol_mask.in;
  csMask msk_in = in;
  csMask new_msk_in = msk_in;
  csMask pol_out = pol_mask.out;
  csMask msk_out = out;
  csMask new_msk_out = msk_out;
  int bits = CS_CM_MASKBITS;
  int idx = 0;		// Index for computing column/row in mask.
  int col, row;
  int new_hor_offs, new_ver_offs;

  bool looped = false;
again:

  while (bits > 0)
  {
    // If outside polygon then nothing happens.
    if (NOT_OUTSIDE (pol_out))
    {
      // Otherwise we test if we are fully inside the polygon.
      if (INSIDE (pol_in))
      {
        // Fully inside.
	// If this mask is outside or partial then we have a hit.
	if (NOT_INSIDE (msk_in))
	{
      	  modified = true;
	  new_msk_in |= 1 << (CS_CM_MASKBITS-bits);
	  new_msk_out &= ~(1 << (CS_CM_MASKBITS-bits));
	}
      }
      else
      {
        // Partially inside.
	// If this mask not fully inside then we need to test further.
	if (NOT_INSIDE (msk_in))
	{
	  // If this mask is outside then we set to partial and
	  // update this child. Otherwise we also traverse this
	  // child and let the childs control modification.
      	  col = idx & CS_CM_HORMASK;
      	  row = idx >> CS_CM_HORSHIFT;
      	  new_hor_offs = hor_offs + col * Child::GetHorizontalSize ();
      	  new_ver_offs = ver_offs + row * Child::GetVerticalSize ();

	  if (OUTSIDE (msk_out))
	  {
	    // Mask outside.
      	    children[idx].UpdatePolygon (poly, num_poly, dxdy, dydx,
		new_hor_offs, new_ver_offs);
	    modified = true;
	    // Set to partial.
	    new_msk_in &= ~(1 << (CS_CM_MASKBITS-bits));
	    new_msk_out &= ~(1 << (CS_CM_MASKBITS-bits));
	  }
	  else
	  {
	    // Mask partial.
	    if (children[idx].InsertPolygon (poly, num_poly, dxdy, dydx,
		new_hor_offs, new_ver_offs))
	    {
	      modified = true;
	      // If child is now completely full/inside then we can set
	      // the state of this bit to full as well.
	      if (children[idx].IsFull ())
	      {
	        new_msk_in |= 1 << (CS_CM_MASKBITS-bits);
	        new_msk_out &= ~(1 << (CS_CM_MASKBITS-bits));
	      }
	    }
	  }
	}
      }
    }

    // Go to next bit.
    pol_in >>= 1;
    msk_in >>= 1;
    pol_out >>= 1;
    msk_out >>= 1;
    bits--;
    idx++;
  }

# if defined(CS_CM_8x8)
  if (looped)
  {
    in2 = new_msk_in;
    out2 = new_msk_out;
    return modified;
  }
  in = new_msk_in;
  out = new_msk_out;

  pol_in = pol_mask.in2;
  msk_in = in2;
  new_msk_in = msk_in;
  pol_out = pol_mask.out2;
  msk_out = out2;
  new_msk_out = msk_out;
  bits = CS_CM_MASKBITS;

  looped = true;
  goto again;
# else
  in = new_msk_in;
  out = new_msk_out;
# endif

  return modified;
}

template <class Child>
void csCovTreeNode<Child>::GfxDump (iGraphics2D* ig2d, int level,
	int hor_offs, int ver_offs)
{
  // Scan all bits of the mask and dump them.
  csMask msk_in = in;
  csMask msk_out = out;
  int bits = CS_CM_MASKBITS;
  int idx = 0;
  int col, row;
  int new_hor_offs, new_ver_offs;
  int c, r;

  bool looped = false;
again:

  while (bits > 0)
  {
    col = idx & CS_CM_HORMASK;
    row = idx >> CS_CM_HORSHIFT;
    new_hor_offs = hor_offs + col * Child::GetHorizontalSize ();
    new_ver_offs = ver_offs + row * Child::GetVerticalSize ();
    if (INSIDE (msk_in))
    {
      for (c = 0 ; c < Child::GetHorizontalSize () ; c++)
        for (r = 0 ; r < Child::GetVerticalSize () ; r++)
      	  ig2d->DrawPixel (new_hor_offs+c, ig2d->GetHeight () - (new_ver_offs+r), ~0);
    }
    else if (OUTSIDE (msk_out))
    {
      // Nothing to dump here.
    }
    else
    {
      // Partial.
      col = idx & CS_CM_HORMASK;
      row = idx >> CS_CM_HORSHIFT;
      new_hor_offs = hor_offs + col * Child::GetHorizontalSize ();
      new_ver_offs = ver_offs + row * Child::GetVerticalSize ();
      if (level > 1)
      {
        // Traverse to child.
	children[idx].GfxDump (ig2d, level-1,
		new_hor_offs, new_ver_offs);
      }
      else
      {
        for (c = 0 ; c < Child::GetHorizontalSize () ; c++)
          for (r = 0 ; r < Child::GetVerticalSize () ; r++)
      	    ig2d->DrawPixel (new_hor_offs+c, ig2d->GetHeight () - (new_ver_offs+r), 0x5555);
      }
    }
    if (level <= 1)
    {
      for (c = 0 ; c < Child::GetHorizontalSize () ; c++)
      	ig2d->DrawPixel (new_hor_offs+c,
		ig2d->GetHeight () - (new_ver_offs+Child::GetVerticalSize ()-1), 0xaaaa);
      for (r = 0 ; r < Child::GetVerticalSize () ; r++)
        ig2d->DrawPixel (new_hor_offs+Child::GetHorizontalSize ()-1,
		ig2d->GetHeight () - (new_ver_offs+r), 0xaaaa);
    }

    // Go to next bit.
    msk_in >>= 1;
    msk_out >>= 1;
    bits--;
    idx++;
  }

# if defined(CS_CM_8x8)
  if (!looped)
  {
    msk_in = in2;
    msk_out = out2;
    bits = CS_CM_MASKBITS;

    looped = true;
    goto again;
  }
# endif
}

//-------------------------------------------------------------------

bool csCovTreeNode0::UpdatePolygon (csVector2* poly, int num_poly,
  	float* dxdy, float* dydx,
	int hor_offs, int ver_offs)
{
  csCovMask pol_mask = lut->GetMask (poly, num_poly, dxdy, dydx,
  	hor_offs, ver_offs, GetHorizontalSize (), GetVerticalSize ());

  // Copy polygon mask to this one.
  Copy (pol_mask);

  // Trivial case. If mask is empty then nothing remains to be done.
  if (IsEmpty ()) return false;

  return true;
}

bool csCovTreeNode0::TestPolygon (csVector2* poly, int num_poly,
  	float* dxdy, float* dydx,
	int hor_offs, int ver_offs) const
{
  // Trivial case. If this mask is full then nothing remains to be done.
  if (IsFull ()) return false;

  csCovMask pol_mask = lut->GetMask (poly, num_poly, dxdy, dydx,
  	hor_offs, ver_offs, GetHorizontalSize (), GetVerticalSize ());

//@@@@@ THERE IS A TRIVIAL CASE WITH LOGICAL OPS ON ENTIRE TWO MASKS!

  // Trivial case. If polygon mask is empty then nothing remains to be done.
  if (pol_mask.IsEmpty ()) return false;

  // Trivial case. If this mask is empty then we have visibility.
  if (IsEmpty ()) return true;

  // Scan all bits of the two masks and combine them.
  // @@@ Consider an optimization where we first test on ranges
  // of 4 or 8 bits before continuing further tests.
  csMask pol_in = pol_mask.in;
  csMask msk_in = in;
  int bits = CS_CM_MASKBITS;

  bool looped = false;
again:

  while (bits > 0)
  {
    // If bit in polygon mask is zero (i.e. outside polygon)
    // then nothing happens.
    if (INSIDE (pol_in) && NOT_INSIDE (msk_in)) return true;

    // Go to next bit.
    pol_in >>= 1;
    msk_in >>= 1;
    bits--;
  }

# if defined(CS_CM_8x8)
  if (looped) return false;

  pol_in = pol_mask.in2;
  msk_in = in2;
  bits = CS_CM_MASKBITS;

  looped = true;
  goto again;
# endif

  return false;
}

bool csCovTreeNode0::InsertPolygon (csVector2* poly, int num_poly,
  	float* dxdy, float* dydx,
	int hor_offs, int ver_offs)
{
  // Trivial case. If this mask is full then nothing remains to be done.
  if (IsFull ()) return false;

  csCovMask pol_mask = lut->GetMask (poly, num_poly, dxdy, dydx,
  	hor_offs, ver_offs, GetHorizontalSize (), GetVerticalSize ());

  // Trivial case. If polygon mask is empty then nothing remains to be done.
  if (pol_mask.IsEmpty ()) return false;

  // Trivial case. If polygon mask is full then we can simply set
  // this mask to full and return modified.
  if (pol_mask.IsFull ())
  {
    Inside ();
    return true;
  }

  // Trivial case. If this mask is empty then we need to copy
  // the polygon mask to this mask.
  if (IsEmpty ())
  {
    Copy (pol_mask);
    return true;
  }

//@@@@ THERE IS ANOTHER TRIVIAL CASE BASED ON LOGICAL OPS OF THE MASKS TO SEE
//IF ANYTHING NEEDS TO BE DONE!

  // Modified status. Initialize at false.
  bool modified = false;

  // Scan all bits of the two masks and combine them.
  // @@@ Consider an optimization where we first test on ranges
  // of 4 or 8 bits before continuing further tests.
  csMask pol_in = pol_mask.in;
  csMask msk_in = in;
  csMask new_msk_in = msk_in;
  int bits = CS_CM_MASKBITS;

  bool looped = false;
again:

  while (bits > 0)
  {
    // If bit in polygon mask is zero (i.e. outside polygon)
    // then nothing happens.
    if (INSIDE (pol_in))
    {
      // Otherwise we test this mask. If outside then we have a hit.
      if (NOT_INSIDE (msk_in))
      {
      	modified = true;
	new_msk_in |= 1 << (CS_CM_MASKBITS-bits);
      }
    }

    // Go to next bit.
    pol_in >>= 1;
    msk_in >>= 1;
    bits--;
  }

# if defined(CS_CM_8x8)
  if (looped)
  {
    in2 = new_msk_in;
    return modified;
  }
  in = new_msk_in;

  pol_in = pol_mask.in2;
  msk_in = in2;
  new_msk_in = msk_in;
  bits = CS_CM_MASKBITS;

  looped = true;
  goto again;
# else
  in = new_msk_in;
# endif

  return modified;
}

void csCovTreeNode0::GfxDump (iGraphics2D* ig2d, int /*level*/,
	int hor_offs, int ver_offs)
{
  // Scan all bits of the mask and dump them.
  csMask msk_in = in;
  int bits = CS_CM_MASKBITS;
  int idx = 0;
  int col, row;

  bool looped = false;
again:

  while (bits > 0)
  {
    if (INSIDE (msk_in))
    {
      col = idx & CS_CM_HORMASK;
      row = idx >> CS_CM_HORSHIFT;
      ig2d->DrawPixel (hor_offs+col, ig2d->GetHeight () - (ver_offs+row), ~0);
    }

    // Go to next bit.
    msk_in >>= 1;
    bits--;
    idx++;
  }

# if defined(CS_CM_8x8)
  if (looped) return;

  msk_in = in2;
  bits = CS_CM_MASKBITS;

  looped = true;
  goto again;
# endif
}

void calc_size ()
{
printf ("1: %d\n", sizeof(csCovTreeNode1));
printf ("2: %d\n", sizeof(csCovTreeNode2));
printf ("3: %d\n", sizeof(csCovTreeNode3));
printf ("4: %d\n", sizeof(csCovTreeNode4));
printf ("5: %d\n", sizeof(csCovTreeNode5));
printf ("%dx%d\n",
  csCovTreeNode3::GetHorizontalSize (),
  csCovTreeNode3::GetVerticalSize ());
}

//---------------------------------------------------------------------------

csCoverageMaskTree::csCoverageMaskTree (csCovMaskLUT* lut, const csBox& box)
{
  this->lut = lut;
  bbox = box;
  // @@@ Configurable!
  CHK (tree = (void*)new csCovTreeNode3 ());
}

csCoverageMaskTree::~csCoverageMaskTree ()
{
  // @@@ Configurable!
  CHK (delete (csCovTreeNode3*)tree);
}

bool csCoverageMaskTree::InsertPolygon (csVector2* verts, int num_verts,
	const csBox& pol_bbox)
{
  // @@@ Configurable!
  csCovTreeNode3* tree3 = (csCovTreeNode3*)tree;

  // If root is already full then there is nothing that can happen further.
  if (tree3->IsFull ()) return false;

  // If the bounding box of the tree does not overlap with the bounding box of
  // the polygon then we can return false here.
  if (!bbox.Overlap (pol_bbox)) return false;

  // If bounding box of tree is completely inside bounding box of polygon then
  // it is possible that tree is completely in polygon. We test that condition
  // further.
  //@@@@@@@ IMPLEMENT!
  //if (bbox < pol_bbox)
  //{
    //if (BoxEntirelyInPolygon (verts, num_verts, bbox))
    //{
      //// Polygon completely covers tree. In that case set state
      //// of tree to full and return true.
      //root->SetState (CS_QUAD_FULL);
      //return true;
    //}
  //}

  float dxdy[100], dydx[100];	// @@@ HARDCODED BAD!
  int i, i1;
  i1 = num_verts-1;
  // @@@ Can we somehow combine the next divides with the
  // perspective correction divides that are done earlier?
  for (i = 0 ; i < num_verts ; i++)
  {
    float dx = verts[i].x - verts[i1].x;
    float dy = verts[i].y - verts[i1].y;
    if (dy >= 0 && dy < SMALL_EPSILON) dy = SMALL_EPSILON;
    else if (dy <= 0 && dy > -SMALL_EPSILON) dy = -SMALL_EPSILON;
    dxdy[i1] = dx / dy;
    dy = verts[i].y - verts[i1].y;
    if (dx >= 0 && dx < SMALL_EPSILON) dx = SMALL_EPSILON;
    else if (dx <= 0 && dx > -SMALL_EPSILON) dx = -SMALL_EPSILON;
    dydx[i1] = dy / dx;
    i1 = i;
  }

  ::lut = lut;
  return tree3->InsertPolygon (verts, num_verts, dxdy, dydx, 0, 0);
}

bool csCoverageMaskTree::TestPolygon (csVector2* verts, int num_verts,
	const csBox& pol_bbox) const
{
  // @@@ Configurable!
  csCovTreeNode3* tree3 = (csCovTreeNode3*)tree;

  // If root is already full then there is nothing that can happen further.
  if (tree3->IsFull ()) return false;

  // If the bounding box of the tree does not overlap with the bounding box of
  // the polygon then we can return false here.
  if (!bbox.Overlap (pol_bbox)) return false;

  // If bounding box of tree is completely inside bounding box of polygon then
  // it is possible that tree is completely in polygon. We test that condition
  // further.
  //@@@@@@@ IMPLEMENT!
  //if (bbox < pol_bbox)
  //{
    //if (BoxEntirelyInPolygon (verts, num_verts, bbox))
    //{
      //// Polygon completely covers tree. In that case return
      //// true because polygon will be visible (node is not full).
      //return true;
    //}
  //}

  float dxdy[100], dydx[100];	// @@@ HARDCODED BAD!
  int i, i1;
  i1 = num_verts-1;
  // @@@ Can we somehow combine the next divides with the
  // perspective correction divides that are done earlier?
  for (i = 0 ; i < num_verts ; i++)
  {
    float dx = verts[i].x - verts[i1].x;
    float dy = verts[i].y - verts[i1].y;
    if (dy >= 0 && dy < SMALL_EPSILON) dy = SMALL_EPSILON;
    else if (dy <= 0 && dy > -SMALL_EPSILON) dy = -SMALL_EPSILON;
    dxdy[i1] = dx / dy;
    dy = verts[i].y - verts[i1].y;
    if (dx >= 0 && dx < SMALL_EPSILON) dx = SMALL_EPSILON;
    else if (dx <= 0 && dx > -SMALL_EPSILON) dx = -SMALL_EPSILON;
    dydx[i1] = dy / dx;
    i1 = i;
  }

  ::lut = lut;
  return tree3->TestPolygon (verts, num_verts, dxdy, dydx, 0, 0);
}

void csCoverageMaskTree::MakeEmpty ()
{
  // @@@ Configurable!
  csCovTreeNode3* tree3 = (csCovTreeNode3*)tree;
  tree3->Outside ();
}

void csCoverageMaskTree::GfxDump (iGraphics2D* ig2d, int level)
{
  // @@@ Configurable!
  csCovTreeNode3* tree3 = (csCovTreeNode3*)tree;
  tree3->GfxDump (ig2d, level, 0, 0);
}

//---------------------------------------------------------------------------

