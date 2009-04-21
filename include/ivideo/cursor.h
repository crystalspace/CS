/*
    Copyright (C) 1998-2000 by Andrew Zabolotny <bit@eltech.ru>

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

/*
                                *WARNING*
    This file must contain only plain-C code.  Do _not_ insert C++ code.
    This file is imported by non-C++ 2D driver implementations.

*/

#ifndef __CS_IVIDEO_CURSOR_H__
#define __CS_IVIDEO_CURSOR_H__

/**\file
 * Mouse cursor IDs
 */

/**
 * \addtogroup gfx2d
 * @{ */
 
/**
 * Standard mouse cursor IDs
 */
typedef enum _csMouseCursorID
{
  /// No cursor
  csmcNone = -1,
  /// Arrow cursor
  csmcArrow = 0,
  /// Lens cursor
  csmcLens,
  /// Cross-hatch cursor
  csmcCross,
  /// Pen cursor
  csmcPen,
  /// Window move cursor
  csmcMove,
  /// Diagonal (\) resizing cursor
  csmcSizeNWSE,
  /// Diagonal (/) resizing cursor
  csmcSizeNESW,
  /// Vertical sizing cursor
  csmcSizeNS,
  /// Horizontal sizing cursor
  csmcSizeEW,
  /// Invalid operation cursor
  csmcStop,
  /// Wait (longplay operation) cursor
  csmcWait
} csMouseCursorID;

/** @} */

#endif // __CS_IVIDEO_CURSOR_H__
