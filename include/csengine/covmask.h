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

#ifndef COVMASK_H
#define COVMASK_H

// Define one of the three below to set the coverage mask size.
#define CS_CM_4x4
//#define CS_CM_8x4
//#define CS_CM_8x8

#if defined(CS_CM_4x4)
#   define CS_CM_HOR 4
#   define CS_CM_VER 4
#   define CS_CM_HORSHIFT 2
#   define CS_CM_VERSHIFT 2
#   define CS_CM_HORMASK 0x3
typedef UShort csMask;
#elif defined(CS_CM_8x4)
#   define CS_CM_HOR 8
#   define CS_CM_VER 4
#   define CS_CM_HORSHIFT 3
#   define CS_CM_VERSHIFT 2
#   define CS_CM_HORMASK 0x7
typedef ULong csMask;
#elif defined(CS_CM_8x8)
#   define CS_CM_HOR 8
#   define CS_CM_VER 8
#   define CS_CM_HORSHIFT 3
#   define CS_CM_VERSHIFT 3
#   define CS_CM_HORMASK 0x7
typedef ULong csMask;
#endif
#define CS_CM_BITS (CS_CM_HOR*CS_CM_VER)
#define CS_CM_MASKBITS (sizeof (csMask)*8)

struct iGraphics2D;

/**
 * Class representing one end-point coverage mask.
 * An end-point coverage mask only holds one bit
 * per mask point instead of two (for the csCovMaskTriage
 * class). In other words, you only have inside/outside
 * information and not 'on the edge' information.
 */
class csCovMask
{
public:
  csMask in;
# if defined(CS_CM_8x8)
    csMask in2;
# endif

public:
  /// Clear a coverage mask.
  void Clear ()
  {
    in = 0;
#   if defined(CS_CM_8x8)
      in2 = 0;
#   endif
  }

  /// Copy a mask.
  void Copy (const csCovMask& mask)
  {
    in = mask.in;
#   if defined(CS_CM_8x8)
      in2 = mask.in2;
#   endif
  }

  /// Invert a mask.
  void Invert ()
  {
    in = ~in;
#   if defined(CS_CM_8x8)
      in2 = ~in2;
#   endif
  }

  /**
   * Set the state of mask bits.
   * 's' is 0 or 1.
   * 'bit' is the index in the mask (between 0 and CS_CM_BITS-1).
   */
  void SetState (int bit, int s)
  {
#   if defined(CS_CM_4x4) || defined(CS_CM_8x4)
      in = (in & ~(1<<bit)) | (s<<bit);
#   elif defined(CS_CM_8x8)
      if (bit < 32)
      {
	in = (in & ~(1<<bit)) | (s<<bit);
      }
      else
      {
	bit -= 32;
	in2 = (in2 & ~(1<<bit)) | (s<<bit);
      }
#   endif
  }

  /**
   * Get the state of mask bits.
   * 'bit' is the index in the mask (between 0 and CS_CM_BITS-1).
   * Note that this is not an efficient function.
   * It is ment mostly for debugging.
   */
  int GetState (int bit) const
  {
#   if defined(CS_CM_4x4) || defined(CS_CM_8x4)
      return (in & (1<<bit)) >> bit;
#   elif defined(CS_CM_8x8)
      if (bit < 32)
      {
        return (in & (1<<bit)) >> bit;
      }
      else
      {
	bit -= 32;
        return (in2 & (1<<bit)) >> bit;
      }
#   endif
  }

  /// Dump state of this mask.
  void Dump () const;

  /// Graphics dump of this mask.
  void GfxDump (iGraphics2D* ig2d, int xoffs, int yoffs) const;

  /**
   * Return true if this mask is completely full (i.e. all in
   * bits are set).
   */
  bool IsFull () const
  {
#   if defined(CS_CM_4x4) || defined(CS_CM_8x4)
      return in == (csMask)~0;
#   elif defined(CS_CM_8x8)
      return (in == (csMask)~0) && (in2 == (csMask)~0);
#   endif
  }

  /**
   * Return true if this mask is completely empty (i.e. all in
   * bits are unset).
   */
  bool IsEmpty () const
  {
#   if defined(CS_CM_4x4) || defined(CS_CM_8x4)
      return in == 0;
#   elif defined(CS_CM_8x8)
      return (in == 0) && (in2 == 0);
#   endif
  }

  /**
   * Return true if this mask is partial.
   */
  bool IsPartial () const
  {
    return !(IsFull () || IsEmpty ());
  }

  /**
   * Return true if this mask contains invalid bits.
   * For csCovMask this always returns false as
   * a csCovMask cannot be invalid.
   */
  bool IsInvalid () const
  {
    return false;
  }

  /// Return the horizontal number of pixels for a mask.
  static int GetHorizontalSize ()
  {
    return CS_CM_HOR;
  }

  /// Return the vertical number of pixels for a mask.
  static int GetVerticalSize ()
  {
    return CS_CM_VER;
  }

  /**
   * Combine this mask with another mask so that you get
   * the intersection of 'in' space.
   */
  void Intersect (const csCovMask& mask)
  {
    in &= mask.in;
#   if defined(CS_CM_8x8)
      in2 &= mask.in2;
#   endif
  }

  /**
   * Make this mask a mask representing fully outside.
   */
  void Outside ()
  {
    Clear ();
  }

  /**
   * Make this mask a mask representing fully inside.
   */
  void Inside ()
  {
    in = (csMask)~0;
#   if defined(CS_CM_8x8)
      in2 = (csMask)~0;
#   endif
  }
};

/**
 * Class representing a triage coverage mask
 * (a triage coverage masks holds three states:
 * in, out, partial).
 */
class csCovMaskTriage : public csCovMask
{
public:
  csMask out;
# if defined(CS_CM_8x8)
    csMask out2;
# endif

public:
  /// Clear a coverage mask.
  void Clear ()
  {
    csCovMask::Clear ();
    out = 0;
#   if defined(CS_CM_8x8)
      out2 = 0;
#   endif
  }

  /// Copy a coverage mask.
  void Copy (const csCovMaskTriage& mask)
  {
    csCovMask::Copy (mask);
    out = mask.out;
#   if defined(CS_CM_8x8)
      out2 = mask.out2;
#   endif
  }

  /// Invert a coverage mask.
  void Invert ()
  {
    csMask s;
    s = in; in = out; out = s;
#   if defined(CS_CM_8x8)
      s = in2; in2 = out2; out2 = s;
#   endif
  }

  /**
   * Set the state of two coverage mask bits.
   * 'so' and 'si' are 0 or 1.
   * 'bit' is the index in the mask (between 0 and CS_CM_BITS-1).
   */
  void SetState (int bit, int so, int si)
  {
    csCovMask::SetState (bit, si);
#   if defined(CS_CM_4x4) || defined(CS_CM_8x4)
      out = (out & ~(1<<bit)) | (so<<bit);
#   elif defined(CS_CM_8x8)
      if (bit < 32)
	out = (out & ~(1<<bit)) | (so<<bit);
      else
      {
	bit -= 32;
	out2 = (out2 & ~(1<<bit)) | (so<<bit);
      }
#   endif
  }

  /**
   * Get the state of two coverage mask bits.
   * The bits are combined in one integer with 'out'
   * at bit position 1 and 'in' at bit position 0.
   * 'bit' is the index in the mask (between 0 and CS_CM_BITS-1).
   * Note that this is not an efficient function.
   * It is ment mostly for debugging.
   */
  int GetState (int bit) const
  {
#   if defined(CS_CM_4x4) || defined(CS_CM_8x4)
      return (((out & (1<<bit)) >> bit) << 1) |
      	     ((in & (1<<bit)) >> bit);
#   elif defined(CS_CM_8x8)
      if (bit < 32)
      {
        return (((out & (1<<bit)) >> bit) << 1) |
      	       ((in & (1<<bit)) >> bit);
      }
      else
      {
	bit -= 32;
        return (((out2 & (1<<bit)) >> bit) << 1) |
      	       ((in2 & (1<<bit)) >> bit);
      }
#   endif
  }

  /// Dump state of this mask.
  void Dump () const;

  /// Graphics dump of this mask.
  void GfxDump (iGraphics2D* ig2d, int xoffs, int yoffs) const;

  /**
   * Return true if this mask is completely full (i.e. all in
   * bits are set and none of the out bits.
   */
  bool IsFull () const
  {
#   if defined(CS_CM_4x4) || defined(CS_CM_8x4)
      return (in == (csMask)~0) && (out == 0);
#   elif defined(CS_CM_8x8)
      return (in == (csMask)~0) && (out == 0) &&
             (in2 == (csMask)~0) && (out2 == 0);
#   endif
  }

  /**
   * Return true if this mask is completely empty (i.e. all out
   * bits are set and none of the in bits.
   */
  bool IsEmpty () const
  {
#   if defined(CS_CM_4x4) || defined(CS_CM_8x4)
      return (out == (csMask)~0) && (in == 0);
#   elif defined(CS_CM_8x8)
      return (out == (csMask)~0) && (in == 0) &&
             (out2 == (csMask)~0) && (in2 == 0);
#   endif
  }

  /**
   * Return true if this mask is partial.
   * A mask is partial if it is not empty or full.
   */
  bool IsPartial () const
  {
    return !(IsFull () || IsEmpty ());
  }

  /**
   * Combine this mask with another triage mask so that you get
   * the intersection of 'in' space.
   */
  void Intersect (const csCovMaskTriage& mask)
  {
    in &= mask.in;
    out |= mask.out;
#   if defined(CS_CM_8x8)
      in2 &= mask.in2;
      out2 |= mask.out2;
#   endif
  }

  /**
   * Make this mask a mask representing fully outside.
   */
  void Outside ()
  {
    csCovMask::Outside ();
    out = (csMask)~0;
#   if defined(CS_CM_8x8)
      out2 = (csMask)~0;
#   endif
  }

  /**
   * Make this mask a mask representing fully inside.
   */
  void Inside ()
  {
    csCovMask::Inside ();
    out = 0;
#   if defined(CS_CM_8x8)
      out2 = 0;
#   endif
  }

  /**
   * Make this mask invalid.
   * This can be used for debugging purposes but serves no other
   * useful purpose.
   */
  void MakeInvalid ()
  {
    in = (csMask)~0;
    out = (csMask)~0;
#   if defined(CS_CM_8x8)
      in2 = (csMask)~0;
      out2 = (csMask)~0;
#   endif
  }

  /**
   * Return true if this mask contains invalid bits.
   * This function is not the complementary of MakeInvalid().
   * MakeInvalid() makes ALL bits invalid while this function
   * tests if there is at least one bit that is invalid.
   */
  bool IsInvalid () const
  {
    if (in & out) return true;
#   if defined(CS_CM_8x8)
      if (in2 & out2) return true;
#   endif
    return false;
  }

};

/**
 * This class represents a lookup table for our coverage
 * masks. It is used by the coverage mask tree. The input
 * for this LUT is a line crossing a box. The two intersections
 * of the line with the box are given with two edge indices.
 * An edge index is a number between 0 and n-1 with n beingu
 * equal to dimension*2+dimension*2.
 * The returned mask indicates the 'outside' of the line,
 * the 'inside' of the line, and everything on the line.
 */
class csCovMaskLUT
{
private:
  /// The dimensions of the box in integer units.
  int dimension;
  /// Total number of possible edge intersections with box (i.e. 4*dimension).
  int num_edge_points;
  /// Shift value to use instead of multiplying with num_edge_points.
  int dim_shift;
  /// LUT.
  csCovMaskTriage* triage_masks;
  /// LUT.
  csCovMask* masks;
  /// Index inside the masks tables for a mask fully outside.
  int index_outside;
  /// Index inside the masks tables for a mask fully inside.
  int index_inside;

  /// Build the lookup tables.
  void BuildTables ();

  /**
   * Take a line given as a start point and the two
   * gradients dx/dy and dy/dz. Also take a box at position
   * (hor_offs,ver_offs)-(hor_offs+box_hor,ver_offs+box_ver)
   * (box_??? must be a power of two).
   * Return the index in the masks tables for the
   * intersection of the line with the box.
   */
  int GetIndex (const csVector2& start, const csVector2& stop,
  	float dxdy, float dydx, int hor_offs, int ver_offs,
	int box_hor, int box_ver) const;

public:
  /**
   * Constructor. The 'dimension' argument defines
   * the number of divisions to make horizontally and
   * vertically. Note that only powers of two are allowed.
   */
  csCovMaskLUT (int dimension);

  /// Destructor.
  ~csCovMaskLUT ();

  /**
   * Return a triage coverage mask for the given line.
   * 'from' and 'to' are indices of the intersection between
   * the line and the box edges (between 0 and num_edge_points-1).
   */
  csCovMaskTriage& GetTriageMask (int from, int to) const
  {
    return triage_masks[(from<<dim_shift) + to];
  }

  /**
   * Return a coverage mask for the given line.
   * 'from' and 'to' are indices of the intersection between
   * the line and the box edges (between 0 and num_edge_points-1).
   */
  csCovMask& GetMask (int from, int to) const
  {
    return masks[(from<<dim_shift) + to];
  }

  /**
   * Take a line given as a start point and the two
   * gradients dx/dy and dy/dz. Also take a box at position
   * (hor_offs,ver_offs)-(hor_offs+box_hor,ver_offs+box_ver)
   * (box_??? must be a power of two).
   * Return the triage mask for the intersection of the line with the box.
   */
  csCovMaskTriage& GetTriageMask (const csVector2& start,
  	const csVector2& stop, float dxdy, float dydx,
	int hor_offs, int ver_offs, int box_hor, int box_ver) const
  {
    return (triage_masks[GetIndex (start, stop, dxdy, dydx,
    	hor_offs, ver_offs, box_hor, box_ver)]);
  }

  /**
   * Take a line given as a start point and the two
   * gradients dx/dy and dy/dz. Also take a box at position
   * (hor_offs,ver_offs)-(hor_offs+box_hor,ver_offs+box_ver)
   * (box_??? must be a power of two).
   * Return the mask for the intersection of the line with the box.
   */
  csCovMask& GetMask (const csVector2& start,
  	const csVector2& stop, float dxdy, float dydx,
	int hor_offs, int ver_offs, int box_hor, int box_ver) const
  {
    return (masks[GetIndex (start, stop, dxdy, dydx,
    	hor_offs, ver_offs, box_hor, box_ver)]);
  }

  /**
   * Take a polygon with vertices and gradients (dx/dy and dy/dx)
   * and return the triage mask for this polygon.
   */
  csCovMaskTriage GetTriageMask (csVector2* verts, int num_verts,
  	float* dxdy, float* dydx,
	int hor_offs, int ver_offs, int box_hor, int box_ver) const;

  /**
   * Take a polygon with vertices and gradients (dx/dy and dy/dx)
   * and return the mask for this polygon.
   */
  csCovMask GetMask (csVector2* verts, int num_verts,
  	float* dxdy, float* dydx,
	int hor_offs, int ver_offs, int box_hor, int box_ver) const;
};

#endif /*COVMASK_H*/

