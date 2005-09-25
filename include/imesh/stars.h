/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CS_IMESH_STARS_H__
#define __CS_IMESH_STARS_H__

/**\file
 * Stars particle mesh object
 */ 

#include "csutil/scf.h"

class csBox3;
class csColor;
class csVector3;

/**\addtogroup meshplugins
 * @{ */

SCF_VERSION (iStarsState, 0, 0, 2);

/**
 * This interface describes the API for the stars mesh object.
 */
struct iStarsState : public iBase
{
  /// Set box where the stars live.
  virtual void SetBox (const csBox3& b) = 0;
  /// Get the box.
  virtual void GetBox (csBox3& b) const = 0;

  /// Set the global color to use.
  virtual void SetColor (const csColor& col) = 0;
  /// Get the color.
  virtual csColor GetColor () const = 0;
  /**
   * Set the color used in the distance.
   * If this is used then stars at max distance will have
   * this color (fading is used).
   */
  virtual void SetMaxColor (const csColor& col) = 0;
  /// Get the max color.
  virtual csColor GetMaxColor () const = 0;
  /// Return true if max color is used.
  virtual bool IsMaxColorUsed () const = 0;

  /// Set density.
  virtual void SetDensity (float d) = 0;
  /// Get density.
  virtual float GetDensity () const = 0;
  /// Set max distance at which stars are visible.
  virtual void SetMaxDistance (float maxdist) = 0;
  /// Get max distance at which stars are visible.
  virtual float GetMaxDistance () const = 0;
};

/** @} */

#endif // __CS_IMESH_STARS_H__

