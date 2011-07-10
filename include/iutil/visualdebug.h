/*
  Copyright (C) 2010 Christian Van Brussel, Institute of Information
      and Communication Technologies, Electronics and Applied Mathematics
      at Universite catholique de Louvain, Belgium
      http://www.uclouvain.be/en-icteam.html

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef __VISUALDEBUG_H__
#define __VISUALDEBUG_H__

#include "iutil/plugin.h"
#include "csutil/cscolor.h"

class csReversibleTransform;
class csVector3;
struct iObjectRegistry;
struct iView;

/**\file 
 * Visual debugging tools
 */

namespace CS {
namespace Debug {

/**
 * Visual debugging tools
 */
struct iVisualDebugger : public virtual iBase
{
  SCF_INTERFACE(iVisualDebugger, 1, 0, 0);

  /**
   * Add the given transform to the list of transforms to be displayed
   * on the next call to Display(). Each axis of the transform will be displayed,
   * with the X axis in red, the Y axis in green, and the Z axis in blue.
   * \param transform The transform to be displayed
   * \param persist Whether or not this transform has to be displayed in each
   * future frame or only for the next one.
   * \param size The size of the axis, in world units.
   */
  virtual void DebugTransform (const csReversibleTransform& transform,
			       bool persist = false,
			       float size = 0.1f) = 0;

  /**
   * Add the given position to the list of positions to be displayed
   * on the next call to Display(). A square dot will be displayed at that position.
   * \param position The position to be debugged
   * \param persist Whether or not this position has to be displayed in each
   * future frame or only for the next one.
   * \param color The color to be used when displaying the position
   * \param size The size of the dot that will be displayed, in pixels. Pay attention
   * that if you use an even number for this size, then the square will be shifted of
   * an half pixel.
   */
  virtual void DebugPosition (const csVector3& position,
			      bool persist = false,
			      csColor color = csColor (0.0f, 1.0f, 0.0f),
			      size_t size = 3) = 0;

  /**
   * Add the given vector to the list of vectors to be displayed
   * on the next call to Display().
   * \param transform The coordinate system of the vector
   * \param vector The vector to be debugged
   * \param persist Whether or not this vector has to be displayed in each
   * future frame or only for the next one.
   * \param color The color to be used when displaying the position
   */
  virtual void DebugVector (const csReversibleTransform& transform,
			    const csVector3& vector,
			    bool persist = false,
			    csColor color = csColor (0.0f, 1.0f, 0.0f)) = 0;

  /**
   * Display all transforms and positions defined by DebugTransform() and
   * DebugPosition(). You have to call this at each frame, after the 3D
   * display of the view. The list of transforms will be cleared.
   */
  virtual void Display (iView* view) = 0;
};

/**
 * Helper class for visual debugging tools. It lets you benefit of the
 * iVisualDebugger plugin without the need to find a reference to it.
 */
class VisualDebuggerHelper
{
 public:
  /**
   * Load the iVisualDebugger plugin and call iVisualDebugger::DebugTransform()
   */
  static void DebugTransform (iObjectRegistry* object_reg,
			      const csReversibleTransform& transform,
			      bool persist = false,
			      float size = 0.1f)
  {
    csRef<iVisualDebugger> debugger = csQueryRegistryOrLoad<iVisualDebugger>
      (object_reg, "crystalspace.utilities.visualdebugger");
    debugger->DebugTransform (transform, persist, size);
  }

  /**
   * Load the iVisualDebugger plugin and call iVisualDebugger::DebugPosition()
   */
  static void DebugPosition (iObjectRegistry* object_reg,
			     const csVector3& position,
			     bool persist = false,
			     csColor color = csColor (0.0f, 1.0f, 0.0f),
			     size_t size = 3)
  {
    csRef<iVisualDebugger> debugger = csQueryRegistryOrLoad<iVisualDebugger>
      (object_reg, "crystalspace.utilities.visualdebugger");
    debugger->DebugPosition (position, persist, color, size);
  }

  /**
   * Load the iVisualDebugger plugin and call iVisualDebugger::DebugVector()
   */
  static void DebugVector (iObjectRegistry* object_reg, 
			   const csReversibleTransform& transform,
			   const csVector3& vector,
			   bool persist = false,
			   csColor color = csColor (0.0f, 1.0f, 0.0f))
  {
    csRef<iVisualDebugger> debugger = csQueryRegistryOrLoad<iVisualDebugger>
      (object_reg, "crystalspace.utilities.visualdebugger");
    debugger->DebugVector (transform, vector, persist, color);
  }

  /**
   * Load the iVisualDebugger plugin and call iVisualDebugger::Display()
   */
  static void Display (iObjectRegistry* object_reg, iView* view)
  {
    csRef<iVisualDebugger> debugger = csQueryRegistryOrLoad<iVisualDebugger>
      (object_reg, "crystalspace.utilities.visualdebugger");
    debugger->Display (view);
  }
};

} // namespace CS
} // namespace Debug

#endif // __VISUALDEBUG_H__
