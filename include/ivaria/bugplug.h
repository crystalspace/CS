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

SCF_VERSION (iBugPlug, 0, 0, 1);

/**
 * Using this interface you can communicate with the BugPlug plugin.
 * This can be useful for specialized debugging operations.
 */
struct iBugPlug : public iBase
{
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
   * Add a wireframe box to the debug sector. If name is not NULL it
   * will be shown in BugPlug when the mouse is over the object.
   */
  virtual void DebugSectorWireBox (const csBox3& box, float r, float g, float b,
  	const char* name = NULL) = 0;

  /**
   * Add a 3D line to the debug sector.
   */
  virtual void DebugSectorLine (const csVector3& start, const csVector3& end,
  	float r, float g, float b) = 0;

  /**
   * Add a 3D marker to the debug sector. This will show up as a cross.
   */
  virtual void DebugSectorMarker (const csVector3& point,
  	float r, float g, float b) = 0;

  /**
   * Switch BugPlug view to the debug sector.
   */
  virtual void SwitchDebugSector (const csReversibleTransform& trans) = 0;

  /**
   * Returns true if the debug sector is currently visible.
   */
  virtual bool CheckDebugSector () const = 0;
};

#endif

