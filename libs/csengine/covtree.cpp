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
bool csCovTreeNode<Child>::TestPolygonNotEmpty (csVector2* poly, int num_poly,
  	float* dxdy, float* dydx,
	int hor_offs, int ver_offs) const
{
  csCovMaskTriage pol_mask = lut->GetTriageMask (poly, num_poly, dxdy, dydx,
  	hor_offs, ver_offs, GetPixelSize (), GetPixelShift ());

  // Trivial case. If polygon mask is empty then nothing remains to be done.
  if (pol_mask.IsEmpty ()) return false;

  // Trivial case. If polygon mask is full we are visible.
  if (pol_mask.IsFull ()) return true;

  // Test all bits.
  csMask traverse;
  int bits, idx, col, row, new_hor_offs, new_ver_offs;

  // First we compute the case where this function
  // will return true (ignoring the children that need to be traversed).
  // This case is where there is a bit in the 'in' mask that is true.
  if (pol_mask.in) return true;
# if defined(CS_CM_8x8)
  if (pol_mask.in2) return true;
# endif

  // For every bit where the polygon mask is partial we have to
  // traverse to the corresponding child to see if we have visibility
  // or not. We put the bit-mask for this case in 'traverse'.
  // Since we already know that 'in' has no bits set to 1 (previous test)
  // we need only look at 'out'.
  traverse = ~pol_mask.out;

  idx = 0;		// Index for computing column/row in mask.
# if defined(CS_CM_8x8)
  bool looped = false;
again:
# endif
  if (traverse)
  {
    bits = CS_CM_MASKBITS;
    while (bits > 0)
    {
      if (traverse & 1)
      {
        col = idx & CS_CM_DIMMASK;
        row = idx >> CS_CM_DIMSHIFT;
        new_hor_offs = hor_offs + (col << Child::GetPixelShift ());
        new_ver_offs = ver_offs + (row << Child::GetPixelShift ());
        if (children[idx].TestPolygonNotEmpty (poly, num_poly, dxdy, dydx,
		new_hor_offs, new_ver_offs))
	  return true;
      }

      // Go to next bit.
      traverse >>= 1;
      bits--;
      idx++;
    }
  }

# if defined(CS_CM_8x8)
  if (!looped)
  {
    traverse = ~pol_mask.out2;
    idx = CS_CM_MASKBITS;
    looped = true;
    goto again;
  }
# endif

  return false;
}

template <class Child>
bool csCovTreeNode<Child>::TestPolygon (csVector2* poly, int num_poly,
  	float* dxdy, float* dydx,
	int hor_offs, int ver_offs) const
{
  // Trivial case. If this mask is full then nothing remains to be done.
  if (IsFull ()) return false;

  // If this mask is empty then we cannot depend on the state of the
  // children being ok. So we call TestPolygonNotEmpty() which will test
  // if the polygon is actually there in this mask part (while ignoring
  // the tree which is empty anyway).
  if (IsEmpty ()) return TestPolygonNotEmpty (poly, num_poly, dxdy, dydx,
  	hor_offs, ver_offs);

  csCovMaskTriage pol_mask = lut->GetTriageMask (poly, num_poly, dxdy, dydx,
  	hor_offs, ver_offs, GetPixelSize (), GetPixelShift ());

  // Trivial case. If polygon mask is empty then nothing remains to be done.
  if (pol_mask.IsEmpty ()) return false;

  // Trivial case. If polygon mask is full we are visible.
  if (pol_mask.IsFull ()) return true;

  // Test all bits.
  csMask traverse, traverse_empty;
  int bits, idx, col, row, new_hor_offs, new_ver_offs;

  // First we compute the case where this function
  // will return true (ignoring the children that need to be traversed).
  // This case is where there exists a bit in this mask that is empty
  // or partial and the corresponding bit in the polygon mask is full.
  if (~((~pol_mask.in) | pol_mask.out | in)) return true;
# if defined(CS_CM_8x8)
  if (~((~pol_mask.in2) | pol_mask.out2 | in2)) return true;
# endif

  // For every bit where this mask is empty and the
  // corresponding bit in the polygon mask is partial we have to
  // continue testing the polygon for the empty sub-tree of
  // the corresponding child.
  traverse_empty = ~(pol_mask.in | pol_mask.out | in | ~out);

  // For every bit where this mask is partial and the
  // corresponding bit in the polygon mask is also partial we have to
  // traverse to the corresponding child to see if we have visibility
  // or not. We put the bit-mask for this case in 'traverse'.
  traverse = ~(pol_mask.in | pol_mask.out | in | out);

  idx = 0;		// Index for computing column/row in mask.
# if defined(CS_CM_8x8)
  bool looped = false;
again:
# endif
  if (traverse || traverse_empty)
  {
    bits = CS_CM_MASKBITS;
    while (bits > 0)
    {
      if (traverse_empty & 1)
      {
        col = idx & CS_CM_DIMMASK;
        row = idx >> CS_CM_DIMSHIFT;
        new_hor_offs = hor_offs + (col << Child::GetPixelShift ());
        new_ver_offs = ver_offs + (row << Child::GetPixelShift ());
        if (children[idx].TestPolygonNotEmpty (poly, num_poly, dxdy, dydx,
		new_hor_offs, new_ver_offs))
	  return true;
      }
      if (traverse & 1)
      {
        col = idx & CS_CM_DIMMASK;
        row = idx >> CS_CM_DIMSHIFT;
        new_hor_offs = hor_offs + (col << Child::GetPixelShift ());
        new_ver_offs = ver_offs + (row << Child::GetPixelShift ());
        if (children[idx].TestPolygon (poly, num_poly, dxdy, dydx,
		new_hor_offs, new_ver_offs))
	  return true;
      }

      // Go to next bit.
      traverse >>= 1;
      traverse_empty >>= 1;
      bits--;
      idx++;
    }
  }

# if defined(CS_CM_8x8)
  if (!looped)
  {
    traverse_empty = ~(pol_mask.in2 | pol_mask.out2 | in2 | ~out2);
    traverse = ~(pol_mask.in2 | pol_mask.out2 | in2 | out2);
    idx = CS_CM_MASKBITS;
    looped = true;
    goto again;
  }
# endif

  return false;
}

template <class Child>
bool csCovTreeNode<Child>::UpdatePolygon (csVector2* poly, int num_poly,
  	float* dxdy, float* dydx,
	int hor_offs, int ver_offs)
{
  // Copy polygon mask to this one.
  Copy (lut->GetTriageMask (poly, num_poly, dxdy, dydx,
  	hor_offs, ver_offs, GetPixelSize (), GetPixelShift ()));

  // Trivial case. If mask is empty then nothing remains to be done.
  if (IsEmpty ()) return false;

  // Trivial case. If mask is full then we can simply return true
  // and nothing remains to be done.
  if (IsFull ()) return true;

  // Compute a combined mask of in and out so that we can
  // quickly discover all partial childs.
  csMask partial = ~(in | out);

  // Mask for setting this mask to inside for all children that
  // are full after traversing.
  csMask setinside = 0;

  // Mask for setting this mask to outside for all children that
  // are empty after traversing.
  csMask setoutside = 0;

  int bits;
  int idx = 0;		// Index for computing column/row in mask.
  int col, row, new_hor_offs, new_ver_offs;

  // If some of the bits are inside then modified will certainly be true.
  bool modified = !!in;

# if defined(CS_CM_8x8)
  bool looped = false;
again:
#endif
  if (partial)
  {
    bits = CS_CM_MASKBITS;
    while (bits > 0)
    {
      // If partial then we traverse to the child to update that.
      if (partial & 1)
      {
        // Partial. Update child.
        col = idx & CS_CM_DIMMASK;
        row = idx >> CS_CM_DIMSHIFT;
        new_hor_offs = hor_offs + (col << Child::GetPixelShift ());
        new_ver_offs = ver_offs + (row << Child::GetPixelShift ());
	if (children[idx].UpdatePolygon (poly, num_poly, dxdy, dydx,
		new_hor_offs, new_ver_offs))
	{
	  if (children[idx].IsFull ())
	    setinside |= 1 << (CS_CM_MASKBITS-bits);
	  modified = true;
	}
	else
	{
	  // If no modification then it is possible that child
	  // is empty.
	  if (children[idx].IsEmpty ())
	    setoutside |= 1 << (CS_CM_MASKBITS-bits);
	}
      }

      // Go to next bit.
      partial >>= 1;
      bits--;
      idx++;
    }
  }

# if defined(CS_CM_8x8)
  if (!looped)
  {
    if (setinside != 0)
    {
      // Set 'inside' mode for all bits in 'setinside'.
      in |= setinside;
      out &= ~setinside;
    }
    if (setoutside != 0)
    {
      // Set 'outside' mode for all bits in 'setoutside'.
      in &= ~setoutside;
      out |= setoutside;
    }
    partial = ~(in2 | out2);
    setinside = 0;
    setoutside = 0;
    idx = CS_CM_MASKBITS;
    looped = true;
    goto again;
  }
  else
  {
    if (setinside != 0)
    {
      // Set 'inside' mode for all bits in 'setinside'.
      in2 |= setinside;
      out2 &= ~setinside;
    }
    if (setoutside != 0)
    {
      // Set 'outside' mode for all bits in 'setoutside'.
      in2 &= ~setoutside;
      out2 |= setoutside;
    }
  }
# else
  if (setinside != 0)
  {
    // Set 'inside' mode for all bits in 'setinside'.
    in |= setinside;
    out &= ~setinside;
  }
  if (setoutside != 0)
  {
    // Set 'outside' mode for all bits in 'setoutside'.
    in &= ~setoutside;
    out |= setoutside;
  }
# endif

  return modified;
}

template <class Child>
bool csCovTreeNode<Child>::UpdatePolygonInverted (csVector2* poly, int num_poly,
  	float* dxdy, float* dydx,
	int hor_offs, int ver_offs)
{
  // Copy polygon mask to this one.
  Copy (lut->GetTriageMask (poly, num_poly, dxdy, dydx,
  	hor_offs, ver_offs, GetPixelSize (), GetPixelShift ()));
  
  // Invert this mask.
  Invert ();

  // Trivial case. If mask is empty then nothing remains to be done.
  if (IsEmpty ()) return false;

  // Trivial case. If mask is full then we can simply return true
  // and nothing remains to be done.
  if (IsFull ()) return true;

  // Compute a combined mask of in and out so that we can
  // quickly discover all partial childs.
  csMask partial = ~(in | out);

  // Mask for setting this mask to inside for all children that
  // are full after traversing.
  csMask setinside = 0;

  // Mask for setting this mask to outside for all children that
  // are empty after traversing.
  csMask setoutside = 0;

  int bits;
  int idx = 0;		// Index for computing column/row in mask.
  int col, row, new_hor_offs, new_ver_offs;

  // If some of the bits are inside then modified will certainly be true.
  bool modified = !!in;

# if defined(CS_CM_8x8)
  bool looped = false;
again:
#endif
  if (partial)
  {
    bits = CS_CM_MASKBITS;
    while (bits > 0)
    {
      // If partial then we traverse to the child to update that.
      if (partial & 1)
      {
        // Partial. Update child.
        col = idx & CS_CM_DIMMASK;
        row = idx >> CS_CM_DIMSHIFT;
        new_hor_offs = hor_offs + (col << Child::GetPixelShift ());
        new_ver_offs = ver_offs + (row << Child::GetPixelShift ());
	if (children[idx].UpdatePolygonInverted (poly, num_poly, dxdy, dydx,
		new_hor_offs, new_ver_offs))
	{
	  if (children[idx].IsFull ())
	    setinside |= 1 << (CS_CM_MASKBITS-bits);
	  modified = true;
	}
	else
	{
	  // If no modification then it is possible that child
	  // is empty.
	  if (children[idx].IsEmpty ())
	    setoutside |= 1 << (CS_CM_MASKBITS-bits);
	}
      }

      // Go to next bit.
      partial >>= 1;
      bits--;
      idx++;
    }
  }

# if defined(CS_CM_8x8)
  if (!looped)
  {
    if (setinside != 0)
    {
      // Set 'inside' mode for all bits in 'setinside'.
      in |= setinside;
      out &= ~setinside;
    }
    if (setoutside != 0)
    {
      // Set 'outside' mode for all bits in 'setoutside'.
      in &= ~setoutside;
      out |= setoutside;
    }
    partial = ~(in2 | out2);
    setinside = 0;
    setoutside = 0;
    idx = CS_CM_MASKBITS;
    looped = true;
    goto again;
  }
  else
  {
    if (setinside != 0)
    {
      // Set 'inside' mode for all bits in 'setinside'.
      in2 |= setinside;
      out2 &= ~setinside;
    }
    if (setoutside != 0)
    {
      // Set 'outside' mode for all bits in 'setoutside'.
      in2 &= ~setoutside;
      out2 |= setoutside;
    }
  }
# else
  if (setinside != 0)
  {
    // Set 'inside' mode for all bits in 'setinside'.
    in |= setinside;
    out &= ~setinside;
  }
  if (setoutside != 0)
  {
    // Set 'outside' mode for all bits in 'setoutside'.
    in &= ~setoutside;
    out |= setoutside;
  }
# endif

  return modified;
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
  	hor_offs, ver_offs, GetPixelSize (), GetPixelShift ());

  // Trivial case. If polygon mask is empty then nothing remains to be done.
  if (pol_mask.IsEmpty ()) return false;

  // Trivial case. If polygon mask is full then we can simply set
  // this mask to full and return modified.
  if (pol_mask.IsFull ())
  {
    Inside ();
    return true;
  }

  // Test all bits.
  csMask setinside, setpartial, insert, update;
  int bits, idx, col, row, new_hor_offs, new_ver_offs;

  // Modified status. Initialize at false.
  bool modified = false;

  // First we compute a bitmap in 'update' which indicates all
  // children that we need to call UpdatePolygon() on. These are
  // all children corresponding to those bit positions where this
  // mask is empty and the polygon mask is partial.
  update = ~(pol_mask.in | pol_mask.out | in | ~out);

  // Then we compute the bitmap in 'insert' which will contain a one
  // for all bit positions where we need to call InsertPolygon() on
  // the child. This corresponds to those cases where both this and
  // the polygon mask are partial.
  insert = ~(pol_mask.in | pol_mask.out | in | out);

  // Then we compute the bitmap in 'setinside' which will contain
  // a one for all bit positions where we need to set this mask to one.
  // This initially corresponds to all bit positions where this mask is
  // empty or partial and the polygon mask is full. Later 'setinside'
  // will be extended.
  setinside = ~((~pol_mask.in) | pol_mask.out | in);

  // This bitmap will indicate all bit positions where we need to
  // set partial mode.
  setpartial = 0;

  idx = 0;		// Index for computing column/row in mask.

# if defined(CS_CM_8x8)
  bool looped = false;
again:
# endif
  if (update || insert)
  {
    bits = CS_CM_MASKBITS;
    while (bits > 0)
    {
      if (update & 1)
      {
      	col = idx & CS_CM_DIMMASK;
      	row = idx >> CS_CM_DIMSHIFT;
        new_hor_offs = hor_offs + (col << Child::GetPixelShift ());
        new_ver_offs = ver_offs + (row << Child::GetPixelShift ());
      	if (children[idx].UpdatePolygon (poly, num_poly, dxdy, dydx,
		new_hor_offs, new_ver_offs))
	{
	  modified = true;
	  // UpdatePolygon() actually did something so we need
	  // to set partial or inside mode here.
	  if (children[idx].IsFull ())
	    setinside |= 1 << (CS_CM_MASKBITS-bits);
	  else
	    setpartial |= 1 << (CS_CM_MASKBITS-bits);
	}
      }
      else if (insert & 1)
      {
      	col = idx & CS_CM_DIMMASK;
      	row = idx >> CS_CM_DIMSHIFT;
        new_hor_offs = hor_offs + (col << Child::GetPixelShift ());
        new_ver_offs = ver_offs + (row << Child::GetPixelShift ());
	if (children[idx].InsertPolygon (poly, num_poly, dxdy, dydx,
		new_hor_offs, new_ver_offs))
	{
	  modified = true;
	  // If child is now completely full/inside then we can set
	  // the state of this bit to full as well.
	  if (children[idx].IsFull ())
	    // Update the already initialized 'setinside' mask for
	    // one extra full bit.
	    setinside |= 1 << (CS_CM_MASKBITS-bits);
	}
      }

      // Go to next bit.
      insert >>= 1;
      update >>= 1;
      bits--;
      idx++;
    }
  }

# if defined(CS_CM_8x8)
  if (looped)
  {
    if (setinside != 0)
    {
      // Set 'inside' mode for all bits in 'setinside'.
      in2 |= setinside;
      out2 &= ~setinside;
      modified = true;
    }
    if (setpartial != 0)
    {
      // Set 'partial' mode for all bits in 'setpartial'.
      in2 &= ~setpartial;
      out2 &= ~setpartial;
    }

    return modified;
  }
  else
  {
    if (setinside != 0)
    {
      // Set 'inside' mode for all bits in 'setinside'.
      in |= setinside;
      out &= ~setinside;
      modified = true;
    }
    if (setpartial != 0)
    {
      // Set 'partial' mode for all bits in 'setpartial'.
      in &= ~setpartial;
      out &= ~setpartial;
    }

    // Prepare to do the second part of the mask.
    update = ~(pol_mask.in2 | pol_mask.out2 | in2 | ~out2);
    insert = ~(pol_mask.in2 | pol_mask.out2 | in2 | out2);
    setinside = ~((~pol_mask.in2) | pol_mask.out2 | in2);
    setpartial = 0;

    idx = CS_CM_MASKBITS;
    looped = true;
    goto again;
  }
# else
  if (setinside != 0)
  {
    // Set 'inside' mode for all bits in 'setinside'.
    in |= setinside;
    out &= ~setinside;
    modified = true;
  }
  if (setpartial != 0)
  {
    // Set 'partial' mode for all bits in 'setpartial'.
    in &= ~setpartial;
    out &= ~setpartial;
  }
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

# if defined(CS_CM_8x8)
  bool looped = false;
again:
# endif

  while (bits > 0)
  {
    col = idx & CS_CM_DIMMASK;
    row = idx >> CS_CM_DIMSHIFT;
    new_hor_offs = hor_offs + (col << Child::GetPixelShift ());
    new_ver_offs = ver_offs + (row << Child::GetPixelShift ());
    if (INSIDE (msk_in))
    {
      for (c = 0 ; c < Child::GetPixelSize () ; c++)
        for (r = 0 ; r < Child::GetPixelSize () ; r++)
      	  ig2d->DrawPixel (new_hor_offs+c, /*ig2d->GetHeight () -*/ (new_ver_offs+r), ~0);
    }
    else if (OUTSIDE (msk_out))
    {
      // Nothing to dump here.
    }
    else
    {
      // Partial.
      col = idx & CS_CM_DIMMASK;
      row = idx >> CS_CM_DIMSHIFT;
      new_hor_offs = hor_offs + (col << Child::GetPixelShift ());
      new_ver_offs = ver_offs + (row << Child::GetPixelShift ());
      if (level > 1)
      {
        // Traverse to child.
	children[idx].GfxDump (ig2d, level-1,
		new_hor_offs, new_ver_offs);
      }
      else
      {
        for (c = 0 ; c < Child::GetPixelSize () ; c++)
          for (r = 0 ; r < Child::GetPixelSize () ; r++)
      	    ig2d->DrawPixel (new_hor_offs+c, /*ig2d->GetHeight () -*/ (new_ver_offs+r), 0x5555);
      }
    }
    if (level <= 1)
    {
      for (c = 0 ; c < Child::GetPixelSize () ; c++)
      	ig2d->DrawPixel (new_hor_offs+c,
		/*ig2d->GetHeight () -*/ (new_ver_offs+Child::GetPixelSize ()-1), 0xaaaa);
      for (r = 0 ; r < Child::GetPixelSize () ; r++)
        ig2d->DrawPixel (new_hor_offs+Child::GetPixelSize ()-1,
		/*ig2d->GetHeight () -*/ (new_ver_offs+r), 0xaaaa);
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

template <class Child>
void csCovTreeNode<Child>::MakeInvalid ()
{
  int i;

  for (i = 0 ; i < CS_CM_BITS ; i++)
    children[i].MakeInvalid ();
  csCovMaskTriage::MakeInvalid ();
}

template <class Child>
bool csCovTreeNode<Child>::TestConsistency (int hor_offs, int ver_offs) const
{
  if (IsInvalid ())
  {
    printf ("Node at %dx%d (size %dx%d) has invalid bits!\n",
    	hor_offs, ver_offs, GetPixelSize (), GetPixelSize ());
    return false;
  }

  // Test all bits.
  int bits, idx, col, row, new_hor_offs, new_ver_offs;
  csMask msk_in, msk_out;

  idx = 0;		// Index for computing column/row in mask.
  msk_in = in;
  msk_out = out;

# if defined(CS_CM_8x8)
  bool looped = false;
again:
# endif
  bits = CS_CM_MASKBITS;
  while (bits > 0)
  {
    if (PARTIAL (msk_in, msk_out) && !children[idx].IsPartial ())
    {
      printf ("Child %d for node %dx%d (size %dx%d) is %s while mask\n\
bit says it should be partial!\n", idx, hor_offs, ver_offs,
	GetPixelSize (), GetPixelSize (),
		children[idx].IsFull () ? "full" : children[idx].IsEmpty () ? "empty" :
		children[idx].IsInvalid () ? "invalid" : "unknown");
      return false;
    }
    if (PARTIAL (msk_in, msk_out))
    {
      col = idx & CS_CM_DIMMASK;
      row = idx >> CS_CM_DIMSHIFT;
      new_hor_offs = hor_offs + (col << Child::GetPixelShift ());
      new_ver_offs = ver_offs + (row << Child::GetPixelShift ());
      if (!children[idx].TestConsistency (new_hor_offs, new_ver_offs))
        return false;
    }
    msk_in >>= 1;
    msk_out >>= 1;
    bits--;
    idx++;
  }

# if defined(CS_CM_8x8)
  if (!looped)
  {
    idx = CS_CM_MASKBITS;
    bits = CS_CM_MASKBITS;
    msk_in = in2;
    msk_out = out2;
    looped = true;
    goto again;
  }
# endif
  return true;
}


//-------------------------------------------------------------------

bool csCovTreeNode0::UpdatePolygon (csVector2* poly, int num_poly,
  	float* dxdy, float* dydx,
	int hor_offs, int ver_offs)
{
  Copy (lut->GetMask (poly, num_poly, dxdy, dydx,
  	hor_offs, ver_offs, GetPixelSize (), GetPixelShift ()));

  // Trivial case. If mask is empty then nothing remains to be done.
  if (IsEmpty ()) return false;

  return true;
}

bool csCovTreeNode0::UpdatePolygonInverted (csVector2* poly, int num_poly,
  	float* dxdy, float* dydx,
	int hor_offs, int ver_offs)
{
  Copy (lut->GetMask (poly, num_poly, dxdy, dydx,
  	hor_offs, ver_offs, GetPixelSize (), GetPixelShift ()));
  Invert ();

  // Trivial case. If mask is empty then nothing remains to be done.
  if (IsEmpty ()) return false;

  return true;
}

bool csCovTreeNode0::TestPolygonNotEmpty (csVector2* poly, int num_poly,
  	float* dxdy, float* dydx,
	int hor_offs, int ver_offs) const
{
  csCovMask pol_mask = lut->GetMask (poly, num_poly, dxdy, dydx,
  	hor_offs, ver_offs, GetPixelSize (), GetPixelShift ());

  // Trivial case. If polygon mask is empty then nothing remains to be done.
  if (pol_mask.IsEmpty ()) return false;

  // Otherwise we have visibility.
  return true;
}

bool csCovTreeNode0::TestPolygon (csVector2* poly, int num_poly,
  	float* dxdy, float* dydx,
	int hor_offs, int ver_offs) const
{
  // Trivial case. If this mask is full then nothing remains to be done.
  if (IsFull ()) return false;

  csCovMask pol_mask = lut->GetMask (poly, num_poly, dxdy, dydx,
  	hor_offs, ver_offs, GetPixelSize (), GetPixelShift ());

  // Trivial case. If polygon mask is empty then nothing remains to be done.
  if (pol_mask.IsEmpty ()) return false;

  // Trivial case. If this mask is empty then we have visibility.
  if (IsEmpty ()) return true;

  // If one of the bits in the polygon mask is set while the equivalent
  // bit in this mask is unset then we have visibility.
  if (pol_mask.in & ~in) return true;
# if defined(CS_CM_8x8)
  if (pol_mask.in2 & ~in2) return true;
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
  	hor_offs, ver_offs, GetPixelSize (), GetPixelShift ());

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

  // Modified status. Initialize at false.
  bool modified = false;

  // If one of the bits in the polygon mask is set while the equivalent
  // bit in this mask is unset then we have visibility.
  csMask visible = pol_mask.in & ~in;
  if (visible)
  {
    modified = true;
    in |= visible;
  }

# if defined(CS_CM_8x8)
  visible = pol_mask.in2 & ~in2;
  if (visible)
  {
    modified = true;
    in2 |= visible;
  }
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

# if defined(CS_CM_8x8)
  bool looped = false;
again:
# endif

  while (bits > 0)
  {
    if (INSIDE (msk_in))
    {
      col = idx & CS_CM_DIMMASK;
      row = idx >> CS_CM_DIMSHIFT;
      ig2d->DrawPixel (hor_offs+col, /*ig2d->GetHeight () -*/ (ver_offs+row), ~0);
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
printf ("1: %lu (%dx%d)\n", (unsigned long)sizeof(csCovTreeNode1), csCovTreeNode1::GetPixelSize (), csCovTreeNode1::GetPixelSize ());
printf ("2: %lu (%dx%d)\n", (unsigned long)sizeof(csCovTreeNode2), csCovTreeNode2::GetPixelSize (), csCovTreeNode2::GetPixelSize ());
printf ("3: %lu (%dx%d)\n", (unsigned long)sizeof(csCovTreeNode3), csCovTreeNode3::GetPixelSize (), csCovTreeNode3::GetPixelSize ());
printf ("4: %lu (%dx%d)\n", (unsigned long)sizeof(csCovTreeNode4), csCovTreeNode4::GetPixelSize (), csCovTreeNode4::GetPixelSize ());
printf ("5: %lu (%dx%d)\n", (unsigned long)sizeof(csCovTreeNode5), csCovTreeNode5::GetPixelSize (), csCovTreeNode5::GetPixelSize ());
}

//---------------------------------------------------------------------------

csCoverageMaskTree::csCoverageMaskTree (csCovMaskLUT* lut, const csBox& box)
{
  calc_size (); //@@@ Debug
  this->lut = lut;
  bbox = box;
  // @@@ Configurable!
# if defined(CS_CM_8x8)
  CHK (tree = (void*)new csCovTreeNode3 ());
# else
  CHK (tree = (void*)new csCovTreeNode4 ());
# endif
}

csCoverageMaskTree::~csCoverageMaskTree ()
{
  // @@@ Configurable!
# if defined(CS_CM_8x8)
  CHK (delete (csCovTreeNode3*)tree);
# else
  CHK (delete (csCovTreeNode4*)tree);
# endif
}

void csCoverageMaskTree::UpdatePolygonInverted (csVector2* verts, int num_verts)
{
  // @@@ Configurable!
# if defined(CS_CM_8x8)
  csCovTreeNode3* tree3 = (csCovTreeNode3*)tree;
# else
  csCovTreeNode4* tree3 = (csCovTreeNode4*)tree;
# endif

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
  tree3->UpdatePolygonInverted (verts, num_verts, dxdy, dydx, 0, 0);
}

bool csCoverageMaskTree::InsertPolygon (csVector2* verts, int num_verts,
	const csBox& pol_bbox)
{
  // @@@ Configurable!
# if defined(CS_CM_8x8)
  csCovTreeNode3* tree3 = (csCovTreeNode3*)tree;
# else
  csCovTreeNode4* tree3 = (csCovTreeNode4*)tree;
# endif

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
# if defined(CS_CM_8x8)
  csCovTreeNode3* tree3 = (csCovTreeNode3*)tree;
# else
  csCovTreeNode4* tree3 = (csCovTreeNode4*)tree;
# endif

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
# if defined(CS_CM_8x8)
  csCovTreeNode3* tree3 = (csCovTreeNode3*)tree;
# else
  csCovTreeNode4* tree3 = (csCovTreeNode4*)tree;
# endif
  tree3->Outside ();
}

void csCoverageMaskTree::GfxDump (iGraphics2D* ig2d, int level)
{
  // @@@ Configurable!
# if defined(CS_CM_8x8)
  csCovTreeNode3* tree3 = (csCovTreeNode3*)tree;
# else
  csCovTreeNode4* tree3 = (csCovTreeNode4*)tree;
# endif
  tree3->GfxDump (ig2d, level, 0, 0);
}

void csCoverageMaskTree::MakeInvalid ()
{
  // @@@ Configurable!
# if defined(CS_CM_8x8)
  csCovTreeNode3* tree3 = (csCovTreeNode3*)tree;
# else
  csCovTreeNode4* tree3 = (csCovTreeNode4*)tree;
# endif
  tree3->MakeInvalid ();
}

void csCoverageMaskTree::TestConsistency ()
{
  // @@@ Configurable!
# if defined(CS_CM_8x8)
  csCovTreeNode3* tree3 = (csCovTreeNode3*)tree;
# else
  csCovTreeNode4* tree3 = (csCovTreeNode4*)tree;
# endif
  tree3->TestConsistency (0, 0);
}


//---------------------------------------------------------------------------

