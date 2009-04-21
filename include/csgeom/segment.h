/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __CS_SEGMENT_H__
#define __CS_SEGMENT_H__

/**\file 
 * 2D line segment.
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "csgeom/vector2.h"
#include "csgeom/vector3.h"

/**
 * A 2D line segment.
 */
class csSegment2
{
private:
  /// Start.
  csVector2 start;
  /// End.
  csVector2 end;

public:
  /// Make a new segment and initialize with the given values.
  csSegment2 (const csVector2& s, const csVector2& e) { start = s; end = e; }
  /// Make a new uninitialized segment.
  csSegment2 () { }
  /// Destructor.
  ~csSegment2 () { }

  /// Set segment to given values.
  inline void Set (const csVector2& s, const csVector2& e)
  { start = s; end = e; }

  /// Set the start of the segment.
  inline void SetStart (const csVector2& s) { start = s; }

  /// Set the end of the segment.
  inline void SetEnd (const csVector2& e) { end = e; }

  /// Get the start of the segment.
  inline const csVector2& Start () const { return start; }

  /// Get the end of the segment.
  inline const csVector2& End () const { return end; }

  /// Get the start of the segment.
  inline csVector2& Start () { return start; }

  /// Get the end of the segment.
  inline csVector2& End () { return end; }
};

/**
 * A 3D line segment.
 */
class csSegment3
{
private:
  /// Start.
  csVector3 start;
  /// End.
  csVector3 end;

public:
  /// Make a new segment and initialize with the given values.
  csSegment3 (const csVector3& s, const csVector3& e) { start = s; end = e; }
  /// Make a new uninitialized segment.
  csSegment3 () { }

  /// Set segment to given values.
  inline void Set (const csVector3& s, const csVector3& e)
  { start = s; end = e; }

  /// Set the start of the segment.
  inline void SetStart (const csVector3& s) { start = s; }

  /// Set the end of the segment.
  inline void SetEnd (const csVector3& e) { end = e; }

  /// Get the start of the segment.
  inline const csVector3& Start () const { return start; }

  /// Get the end of the segment.
  inline const csVector3& End () const { return end; }

  /// Get the start of the segment.
  inline csVector3& Start () { return start; }

  /// Get the end of the segment.
  inline csVector3& End () { return end; }
};

/** @} */

#endif // __CS_SEGMENT_H__
