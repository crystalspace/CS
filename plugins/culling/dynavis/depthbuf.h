/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_DEPTHBUF_H__
#define __CS_DEPTHBUF_H__

#include "csgeom/vector2.h"
#include "csgeom/math2d.h"
#include "iutil/dbghelp.h"

struct iGraphics2D;
struct iGraphics3D;
class csString;
class csBox2;

/**
 * A 2D bounding box with integer coordinates.
 */
class csBox2Int
{
public:
  int minx, miny;
  int maxx, maxy;
};

/**
 * A 2D vector with integer coordinates.
 */
class csVector2Int
{
public:
  int x, y;
};


#define CS_DEPTH_CHILD_00 0
#define CS_DEPTH_CHILD_10 1
#define CS_DEPTH_CHILD_01 2
#define CS_DEPTH_CHILD_11 3

/**
 * A hierarchical depth buffer.
 */
class csDepthBuffer : public iBase
{
private:
  int width, height;
  int startx, starty;
  float min_depth;	// Min depth value in this node.
  float max_depth;	// Max depth value in this node.
  csDepthBuffer* children;

  bool InsertRectangle (const csBox2Int& bbox,
  	float rect_mindepth, float rect_maxdepth);

  bool InsertPolygon (csVector2Int* verts, int num_verts,
  	const csBox2Int& bbox,
  	float poly_mindepth, float poly_maxdepth);

public:
  /**
   * Create a new empty depth buffer.
   */
  csDepthBuffer ();
  /// Destroy the depth buffer.
  virtual ~csDepthBuffer ();

  /**
   * Setup the depth buffer with the given dimensions.
   * w and h should be equal for now and also a power of two.
   * This will also create four children unless w=h=8 at which point
   * it will stop.
   */
  void Setup (int sx, int sy, int w, int h);

  SCF_DECLARE_IBASE;

  /// Initialize the depth buffer to empty.
  void Initialize ();

  /**
   * Insert a rectangle in the depth buffer.
   * It will update the depth buffer if needed.
   * Function returns false if rectangle was not visible. This happens
   * when the polygon z is fully behind the current depth for all
   * nodes in the depth buffer. In that case the depth buffer will also
   * be modified if the rectangle actually covers this node.
   * The given bounding box is in screen space.
   * 'rect_mindepth' and 'rect_maxdepth' are the minimum and maximum depths of
   * the given rectangle (@@@ Better in the future?)
   */
  bool InsertRectangle (const csBox2& bbox,
  	float rect_mindepth, float rect_maxdepth);

  /**
   * Insert a polygon in the depth buffer.
   * It will update the depth buffer if needed.
   * Function returns false if polygon was not visible. This happens
   * when the polygon z is fully behind the current depth for all
   * nodes in the depth buffer. In that case the depth buffer will also
   * be modified if the polygon actually covers this node.
   * The given bounding box is in screen space.
   * 'poly_mindepth' and 'poly_maxdepth' are the minimum and maximum depths of
   * the given polygon (@@@ Better in the future?)
   */
  bool InsertPolygon (csVector2* verts, int num_verts, const csBox2& bbox,
  	float poly_mindepth, float poly_maxdepth);

  // Debugging functions.
  iString* Debug_UnitTest ();

  struct DebugHelper : public iDebugHelper
  {
    SCF_DECLARE_EMBEDDED_IBASE (csDepthBuffer);
    virtual int GetSupportedTests () const
    {
      return CS_DBGHELP_UNITTEST;
    }
    virtual iString* UnitTest ()
    {
      return scfParent->Debug_UnitTest ();
    }
    virtual iString* StateTest ()
    {
      return NULL;
    }
    virtual csTicks Benchmark (int /*num_iterations*/)
    {
      return 0;
    }
    virtual iString* Dump ()
    {
      return NULL;
    }
    virtual void Dump (iGraphics3D* /*g3d*/)
    {
    }
    virtual bool DebugCommand (const char*)
    {
      return false;
    }
  } scfiDebugHelper;
};

#endif // __CS_DEPTHBUF_H__

