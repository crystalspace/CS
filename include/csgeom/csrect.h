/*
    Crystal Space Windowing System: rectangle class interface
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CSRECT_H__
#define __CSRECT_H__

#include "types.h"
#include "csutil/csbase.h"

/**
 * Rectangle class: simple class for manipulating 2D rectangles.
 * This class is somewhat similar to Box, but uses integer coordinates
 * and is mostly used for CrystalSpace Windowing System.<p>
 * Example of a rectangle (xmin = 0, ymin = 0, xmax = 3, ymax = 2):
 * <pre>
 *     0  1  2  3  4 ...
 *     |  |  |  |  |  |
 * 0 --@@@@@@@@@@--+--+--
 *     @//|//|//@  |  |
 * 1 --@--+--+--@--+--+--
 *     @//|//|//@  |  |
 * 2 --@@@@@@@@@@--+--+--
 *     |  |  |  |  |  |
 * 3 --+--+--+--+--+--+--
 *     |  |  |  |  |  |
 *...--+--+--+--+--+--+--
 * </pre>
 * Vertical line 'X=3' and horizontal line 'Y=2' does NOT belong to
 * the rectangle.
 */
class csRect : public csBase
{
public:
  /// Rectangle bounds.
  int xmin, ymin, xmax, ymax;

  /// Create a empty rectangle.
  csRect ();

  /// Create a new rectangle.
  csRect (int ixmin, int iymin, int ixmax, int iymax);

  /// Copy constructor.
  csRect (csRect &copy);

  virtual ~csRect ();

  /**
   * Intersect with another rectangle; return false if
   * resulting rectangle is empty.
   */
  void Intersect (int ixmin, int iymin, int ixmax, int iymax);

  /**
   * Intersect with another rectangle; return false if
   * resulting rectangle is empty.
   */
  void Intersect (csRect &target);

  /// Return true if rectangle intersects with target.
  bool Intersects (csRect &target);

  /**
   * Add a rectangle: find minimal rectangle
   * that embeds both given rectangles.
   */
  void Union (int ixmin, int iymin, int ixmax, int iymax);

  /**
   * Add a rectangle: find minimal rectangle
   * that embeds both given rectangles.
   */
  void Union (csRect &target);

  /**
   * Subtract rectangle: find the minimal rectangle which embeds all
   * parts of this rectangle which are not covered by given rectangle.
   * If rectangle is fully covered by argument, it becomes empty.
   */
  void Exclude (int ixmin, int iymin, int ixmax, int iymax);

  /**
   * Alternative subtraction: find maximal area of this rectangle that
   * is not covered by argument.
   */
  void Subtract (csRect &rect);

  /// Return true if rectangle is empty.
  inline bool IsEmpty ()
  { return (xmin >= xmax) || (ymin >= ymax); }

  /// Make rectangle empty.
  inline void MakeEmpty ()
  { xmin = xmax = 0; }

  /// Set rectangle to given ixmin,iymin,ixmax,iymax position.
  inline void Set (int ixmin, int iymin, int ixmax, int iymax)
  {
    xmin = ixmin; xmax = ixmax;
    ymin = iymin; ymax = iymax;
  }

  /// Copy rectangle.
  inline void Set (csRect &target)
  {
    xmin = target.xmin; xmax = target.xmax;
    ymin = target.ymin; ymax = target.ymax;
  }

  /// Set rectangle xmin,ymin position.
  inline void SetPos (int x, int y)
  { xmin = x; ymin = y; }

  /// Set rectangle size.
  inline void SetSize (int w, int h)
  { xmax = xmin + w; ymax = ymin + h; }

  /// Move rectangle by deltaX, deltaY.
  inline void Move (int dX, int dY)
  { xmin += dX; xmax += dX; ymin += dY; ymax += dY; }

  /// Return the width of rectangle.
  inline int Width () { return xmax - xmin; }

  /// Return the height of rectangle.
  inline int Height () { return ymax - ymin; }

  /// Return true if a point lies within rectangle bounds.
  inline bool Contains (int x, int y)
  { return (x >= xmin) && (x < xmax) && (y >= ymin) && (y < ymax); }

  /// Return true if a relative point lies within rectangle bounds.
  inline bool ContainsRel (int x, int y)
  { return (x >= 0) && (x < Width ()) && (y >= 0) && (y < Height ()); }

  /// Return true if rectangle is the same.
  inline bool Equal (int ixmin, int iymin, int ixmax, int iymax)
  { return (xmin == ixmin) && (ymin == iymin) && (xmax == ixmax) && (ymax == iymax); }

  /// Normalize a rectangle such that xmin <= xmax and ymin <= ymax.
  inline void Normalize ()
  {
    if (xmin > xmax) { int tmp = xmin; xmin = xmax; xmax = tmp; }
    if (ymin > ymax) { int tmp = ymin; ymin = ymax; ymax = tmp; }
  }

  /// Return area of this rectangle.
  inline int Area ()
  {
    if (IsEmpty ())
      return 0;
    else
      return Width () * Height ();
  }

  /// Add an adjanced rectangle if resulting rectangle will have larger area.
  void AddAdjanced (csRect &rect);

  /// Test inequality of two rectangles.
  inline bool operator != (csRect &rect)
  {
    return (xmin != rect.xmin) || (ymin != rect.ymin)
        || (xmax != rect.xmax) || (ymax != rect.ymax);
  }

  /// Extend rectangle so that it will include given point
  inline void Extend (int x, int y)
  {
    if (xmin > x) xmin = x; if (xmax < x) xmax = x;
    if (ymin > y) ymin = y; if (ymax < y) ymax = y;
  }
};

#endif // __CSRECT_H__
