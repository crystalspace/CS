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
//#define CS_CM_4x4
//#define CS_CM_8x4
#define CS_CM_8x8

/**
 * Class representing a triage coverage mask
 * (a triage coverage masks holds three states:
 * in, out, partial).
 */
class csCovMaskTriage
{
private:
# if defined(CS_CM_4x4)
#   define CS_CM_HOR 4
#   define CS_CM_VER 4
    UShort out, in;
# elif defined(CS_CM_8x4)
#   define CS_CM_HOR 8
#   define CS_CM_VER 4
    ULong out, in;
# elif defined(CS_CM_8x8)
#   define CS_CM_HOR 8
#   define CS_CM_VER 8
    ULong out1, out2, in1, in2;
# endif
# define CS_CM_BITS (CS_CM_HOR*CS_CM_VER)

public:
  /// Clear a coverage mask.
  void Clear ()
  {
#   if defined(CS_CM_4x4) || defined(CS_CM_8x4)
      out = in = 0;
#   elif defined(CS_CM_8x8)
      out1 = out2 = in1 = in2 = 0;
#   endif
  }

  /**
   * Set the state of two coverage mask bits.
   * 'so' and 'si' are 0 or 1.
   * 'bit' is the index in the mask (between 0 and CS_CM_BITS-1).
   */
  void SetState (int bit, int so, int si)
  {
#   if defined(CS_CM_4x4) || defined(CS_CM_8x4)
      out = (out & ~(1<<bit)) | (so<<bit);
      in = (in & ~(1<<bit)) | (si<<bit);
#   elif defined(CS_CM_8x8)
      if (bit < 32)
      {
	out1 = (out1 & ~(1<<bit)) | (so<<bit);
	in1 = (in1 & ~(1<<bit)) | (si<<bit);
      }
      else
      {
	bit -= 32;
	out2 = (out2 & ~(1<<bit)) | (so<<bit);
	in2 = (in2 & ~(1<<bit)) | (si<<bit);
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
      return ((out & (1<<bit)) >> (bit-1)) |
      	     ((in & (1<<bit)) >> bit);
#   elif defined(CS_CM_8x8)
      if (bit < 32)
      {
        return ((out1 & (1<<bit)) >> (bit-1)) |
      	       ((in1 & (1<<bit)) >> bit);
      }
      else
      {
	bit -= 32;
        return ((out2 & (1<<bit)) >> (bit-1)) |
      	       ((in2 & (1<<bit)) >> bit);
      }
#   endif
  }

  /**
   * Dump state of this mask.
   */
  void Dump () const
  {
    static char st[5] = "*#.?";
    int i = 0;
#   if defined(CS_CM_4x4)
      printf ("%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)]);
#   elif defined(CS_CM_8x4)
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
#   elif defined(CS_CM_8x8)
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
#   endif
  }
};

/**
 * Class representing one end-point coverage mask.
 * An end-point coverage mask only holds one bit
 * per mask point instead of two. In other words, you
 * only have inside/outside information and not 'on
 * the edge' information.
 */
class csCovMask
{
protected:
# if defined(CS_CM_4x4)
  UShort in;
# elif defined(CS_CM_8x4)
  ULong in;
# elif defined(CS_CM_8x8)
  ULong in1, in2;
# endif

public:
  /// Clear a coverage mask.
  void Clear ()
  {
#   if defined(CS_CM_4x4)
      in = 0;
#   elif defined(CS_CM_8x4)
      in = 0;
#   elif defined(CS_CM_8x8)
      in1 = in2 = 0;
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
	in1 = (in1 & ~(1<<bit)) | (s<<bit);
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
        return (in1 & (1<<bit)) >> bit;
      }
      else
      {
	bit -= 32;
        return (in2 & (1<<bit)) >> bit;
      }
#   endif
  }

  /**
   * Dump state of this mask.
   */
  void Dump () const
  {
    static char st[3] = ".#";
    int i = 0;
#   if defined(CS_CM_4x4)
      printf ("%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)]);
#   elif defined(CS_CM_8x4)
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
#   elif defined(CS_CM_8x8)
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
      printf ("%c%c%c%c%c%c%c%c\n", st[GetState (i++)], st[GetState (i++)],
      	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)],
	st[GetState (i++)], st[GetState (i++)], st[GetState (i++)]);
#   endif
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
  // The dimensions of the box in integer units.
  int dimension;
  // Total number of possible edge intersections with box (i.e. 4*dimension).
  int num_edge_points;
  // Shift value to use instead of multiplying with num_edge_points.
  int dim_shift;
  // LUT.
  csCovMaskTriage* triage_masks;
  // LUT.
  csCovMask* masks;

  /// Build the lookup tables.
  void BuildTables ();

  /**
   * Take a line given as a start point and the two
   * gradients dx/dy and dy/dz. Also take a box at position
   * (0,0)-(box,box) (box must be a power of two).
   * Return the index in the masks tables for the
   * intersection of the line with the box.
   */
  int GetIndex (const csVector2& start, const csVector2& stop,
  	float dxdy, float dydx, int box) const;

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
   * (0,0)-(box,box) (box must be a power of two).
   * Return the triage mask for the intersection of the line with the box.
   */
  csCovMaskTriage& GetTriageMask (const csVector2& start,
  	const csVector2& stop,
  	float dxdy, float dydx, int box) const
  {
    return (triage_masks[GetIndex (start, stop, dxdy, dydx, box)]);
  }

  /**
   * Take a line given as a start point and the two
   * gradients dx/dy and dy/dz. Also take a box at position
   * (0,0)-(box,box) (box must be a power of two).
   * Return the mask for the intersection of the line with the box.
   */
  csCovMask& GetMask (const csVector2& start,
  	const csVector2& stop,
  	float dxdy, float dydx, int box) const
  {
    return (masks[GetIndex (start, stop, dxdy, dydx, box)]);
  }
};

#endif /*COVMASK_H*/

