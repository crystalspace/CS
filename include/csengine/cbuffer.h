/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __CS_CBUFFER_H__
#define __CS_CBUFFER_H__

#include "csgeom/math2d.h"

struct iGraphics2D;
struct iGraphics3D;

/*=================*
 * Coverage Buffer *
 *=================*/

/**
 * A simple span in a CBuffer line.
 * Note that we work with a 'negative' c-buffer. This means
 * that every span indicates an empty region on screen.
 */
struct csCBufferSpan
{
  /// Inclusive range of span.
  int startx, endx;
  /// Next span.
  csCBufferSpan* next;
};

class csCBuffer;

/**
 * A line in the CBuffer.
 * Spans are allocated on an as-needed base. But they are
 * not freed. Instead, unused spans are put in the first_unused
 * list (in csCBuffer). The size of this list will always be equal
 * to the maximum number of spans once used on screen.
 */
class csCBufferLine
{
  friend class csCBuffer;

private:
  /// List of all empty spans on this line.
  csCBufferSpan* first_span;
  /// Pointer to last span in the list of empty spans.
  csCBufferSpan* last_span;
  /// Parent C-buffer.
  csCBuffer* parent;

private:
  /// Initialize a c-buffer line.
  csCBufferLine ();
  /**
   * Destruct a c-buffer line and free all used spans.
   * Note: the spans are really freed and not put in the unused list.
   */
  ~csCBufferLine ();

  /// Set parent.
  void SetParent (csCBuffer* par) { parent = par; }

  /**
   * Initialize this line to the given empty span.
   */
  void Initialize (int startx, int endx);

  /// Return true if this line if full (i.e. no empty spans left).
  bool IsFull () { return first_span == NULL; }

  /// Make this line full.
  void MakeFull ();

  /**
   * Take a full span and test if it would have changed this c-buffer
   * line on insertion. This means that the span is visible.
   */
  bool TestSpan (int startx, int endx);

  /**
   * Take a full span and insert it into the c-buffer line. Return
   * true if the span line was modified (i.e. the span is visible).
   */
  bool InsertSpan (int startx, int endx);

  /**
   * Test if a given one-element-scan is full or empty (i.e. return
   * true if visible).
   */
  bool TestSingle (int x) { return TestSpan (x, x); }

  /// Dump information about this scanline.
  void Dump ();
};

/**
 * The CBuffer.
 * Note that all ranges specified in this class are inclusive.
 */
class csCBuffer
{
  friend class csCBufferLine;

private:
  /// The lines of this c-buffer.
  csCBufferLine* lines;
  /// Number of vertical lines.
  int num_lines;
  /// Horizontal start and end (inclusive).
  int startx, endx;
  /// List of all unused spans on screen.
  csCBufferSpan* first_unused;
  /**
   * A vertical c-buffer line which is used to indicate
   * which horizontal lines of the c-buffer are full.
   * Again spans indicate not-full regions.
   */
  csCBufferLine vert_line;

  /// Allocate a span (possible from the unused list).
  csCBufferSpan* AllocSpan ()
  {
    csCBufferSpan* s;
    if (first_unused)
    {
      s = first_unused;
      first_unused = first_unused->next;
    }
    else
      s = new csCBufferSpan ();
    return s;
  }
  /// Free a span (put in the unused list).
  void FreeSpan (csCBufferSpan* span)
  {
    span->next = first_unused;
    first_unused = span;
  }

  /**
   * Take a full span and test if it would have changed the c-buffer
   * line on insertion. This means that the span is visible.
   */
  bool TestSpan (int s_spanx, int e_spanx, int y)
  {
    if (y < 0 || y >= num_lines) return false;
    return lines[y].TestSpan (s_spanx, e_spanx);
  }

  /**
   * Take a full span and insert it into the c-buffer line. Return
   * true if the span modified the buffer (i.e. span is visible).
   */
  bool InsertSpan (int s_spanx, int e_spanx, int y)
  {
    if (y < 0 || y >= num_lines) return false;
    return lines[y].InsertSpan (s_spanx, e_spanx);
  }

  /// Test if a line is full.
  bool IsFull (int y)
  {
    if (y < 0 || y >= num_lines) return false;
    return lines[y].IsFull ();
  }

public:
  /// Create a new c-buffer with the given dimensions.
  csCBuffer (int sx, int ex, int n_lines);
  /// Destroy the c-buffer.
  ~csCBuffer ();

  /// Initialize the c-buffer to empty.
  void Initialize ();

  /// Return true if the screen (c-buffer) is full.
  bool IsFull () { return vert_line.IsFull (); }

  /**
   * Take a polygon and test if it would have changed the c-buffer.
   * This means that the polygon is visible. Polygon vertices are
   * converted to integer before comparing.
   * Note that this function will work with both clockwise and anti-
   * clockwise oriented polygons and will assume both orientations
   * are visible. Backface culling needs to be done elsewhere.
   */
  bool TestPolygon (csVector2* verts, int num_verts);

  /**
   * Take a polygon and insert all spans in the c-buffer.
   * Returns true if the polygon is visible.
   * Note that this function will work with both clockwise and anti-
   * clockwise oriented polygons and will assume both orientations
   * are visible. Backface culling needs to be done elsewhere.
   * If 'negative' is true the polygon is inserted inverted.
   */
  bool InsertPolygon (csVector2* verts, int num_verts, bool negative = false);

  /**
   * Test if a given point is visible in the c-buffer.
   * Returns true if visible (i.e. c-buffer is empty at
   * that point).
   */
  bool TestPoint (const csVector2& point);


  /// Dump debug information for a scanline.
  void DumpLine (int y) { lines[y].Dump (); }

  /**
   * Do a graphical dump of the c-buffer contents on screen.
   */
  void GfxDump (iGraphics2D* ig2d, iGraphics3D* ig3d);
};

#endif // __CS_CBUFFER_H__
