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

#ifndef __IBUGPLUG_H__
#define __IBUGPLUG_H__

#include "csutil/scf.h"

class csBox3;
class csVector3;
class csReversibleTransform;
struct iMeshObject;
struct iBugPlug;
struct iGraphics3D;

SCF_VERSION (iBugPlugRenderObject, 0, 0, 1);

/**
 * An application/module can implement this interface to render something.
 * BugPlug can call the render function when it decides it is time to
 * render the object.
 */
struct iBugPlugRenderObject : public iBase
{
  /// Render.
  virtual void Render (iGraphics3D* g3d, iBugPlug* bugplug) = 0;
};

SCF_VERSION (iBugPlug, 0, 0, 2);

/**
 * Using this interface you can communicate with the BugPlug plugin.
 * This can be useful for specialized debugging operations.
 */
struct iBugPlug : public iBase
{
  //=========================================================================

  /**
   * Setup the 'debug sector'. The debug sector is a sector which you
   * can fill with boxes and other objects. BugPlug can then switch
   * the view to that sector so that the objects are rendered. This can
   * be useful for debugging complicated systems in a graphical manner.
   * This function will clear any previously created debug sector.
   */
  virtual void SetupDebugSector () = 0;

  /**
   * Add a colored filled box to the debug sector. If name is not NULL it
   * will be shown in BugPlug when the mouse is over the object. If
   * iMeshObject* is not NULL it will be shown inside the box when the mouse
   * is over the object.
   */
  virtual void DebugSectorBox (const csBox3& box, float r, float g, float b,
  	const char* name = NULL, iMeshObject* mesh = NULL) = 0;

  /**
   * Add a transparent filled triangle to the debug sector.
   * The color will be max at s1 and completely black at s2 and s3.
   */
  virtual void DebugSectorTriangle (const csVector3& s1, const csVector3& s2,
  	const csVector3& s3, float r, float g, float b) = 0;

  /**
   * Switch BugPlug view to the debug sector. The given transform is
   * given to the camera.
   */
  virtual void SwitchDebugSector (const csReversibleTransform& trans) = 0;

  /**
   * Returns true if the debug sector is currently visible.
   */
  virtual bool CheckDebugSector () const = 0;

  //=========================================================================

  /**
   * Setup the 'debug 2dview'. To this view a plugin or application can
   * add various 2D objects (lines and points for example).
   * This function will clear any previously created debug view.
   */
  virtual void SetupDebugView () = 0;

  /**
   * Add a dragable point. Returns an index that can be used in a line.
   */
  virtual int DebugViewPoint (const csVector2& point) = 0;

  /**
   * Add a line. The two indices are as returned from DebugViewPoint().
   */
  virtual void DebugViewLine (int i1, int i2) = 0;

  /**
   * Return the current number of points.
   */
  virtual int DebugViewPointCount () const = 0;

  /**
   * Return a point.
   */
  virtual const csVector2& DebugViewGetPoint (int i) const = 0;

  /**
   * Return the current number of lines.
   */
  virtual int DebugViewLineCount () const = 0;

  /**
   * Return a line.
   */
  virtual void DebugViewGetLine (int i, int& i1, int& i2) const = 0;

  /**
   * Add some rendering code that will be rendered right before the
   * points and lines are rendered. If NULL the current object will be removed.
   */
  virtual void DebugViewRenderObject (iBugPlugRenderObject* obj) = 0;

  /**
   * Indicate if BugPlug should clear the screen before rendering the
   * debug view. True by default.
   */
  virtual void DebugViewClearScreen (bool cs) = 0;

  /**
   * Switch BugPlug view to the debug view.
   */
  virtual void SwitchDebugView () = 0;

  /**
   * Returns true if the debug view is currently visible.
   */
  virtual bool CheckDebugView () const = 0;

  //=========================================================================
};

#endif

