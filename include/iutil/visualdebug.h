/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

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

class csReversibleTransform;
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
   * Add the given transform at the list of transforms to be displayed
   * on the next call to Display().
   */
  virtual void DebugTransform (csReversibleTransform& transform) = 0;

  /**
   * Display all transforms defined by DebugTransform(). You have to call
   * this at each frame, after the 3D display of the view. The list of
   * transforms will be cleared.
   */
  virtual void Display (iView* view, float axisSize = 1.0f) = 0;
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
			      csReversibleTransform& transform)
  {
    csRef<iVisualDebugger> debugger = csQueryRegistryOrLoad<iVisualDebugger>
      (object_reg, "crystalspace.utilities.visualdebugger");
    debugger->DebugTransform (transform);
  }

  /**
   * Load the iVisualDebugger plugin and call iVisualDebugger::Display()
   */
  static void Display (iObjectRegistry* object_reg,
		       iView* view, float axisSize = 1.0f)
  {
    csRef<iVisualDebugger> debugger = csQueryRegistryOrLoad<iVisualDebugger>
      (object_reg, "crystalspace.utilities.visualdebugger");
    debugger->Display (view, axisSize);
  }
};

} // namespace CS
} // namespace Debug

#endif // __VISUALDEBUG_H__
