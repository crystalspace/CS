/*
    Copyright (C) 2002 by Keith Fulton

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

#ifndef __CS_IENGINE_IMPOSTER_H__
#define __CS_IENGINE_IMPOSTER_H__


/**\file
 */
/**
 * \addtogroup engine3d
 * @{ */

#include "csutil/scf.h"

struct iSharedVariable;
class csReversibleTransform;

/**
 * iImposter defines the interface a mesh (or other) class must
 * implement to be used as imposter mesh by the engine.
 */
struct iImposter : public virtual iBase
{
  SCF_INTERFACE(iImposter, 2, 0, 0);

  /// Self explanatory
  virtual void SetImposterActive (bool flag)=0;
  virtual bool GetImposterActive () const =0;

  /**
   * Minimum Imposter Distance is the distance from camera 
   * beyond which imposter is used. Imposter gets a 
   * ptr here because value is a shared variable 
   * which can be changed at runtime for many objects.
   */
  virtual void SetMinDistance (iSharedVariable* dist) = 0;

  /** 
   * Rotation Tolerance is the maximum allowable 
   * angle difference between when the imposter was 
   * created and the current position of the camera.
   * Angle greater than this triggers a re-render of
   * the imposter.
   */
  virtual void SetRotationTolerance (iSharedVariable* angle) = 0;

  /**
   * Tells the object to create its proctex and polygon
   * for use by main render process later.
   */
  virtual void CreateImposter (csReversibleTransform& pov) = 0;

  /// Draw imposter on screen.
  /// private: virtual void Draw(iRenderView* rview) = 0;

  /// Determine if imposter or true rendering will be used
  virtual bool WouldUseImposter (csReversibleTransform& pov) const = 0;
};

/** @} */

#endif // __CS_IENGINE_IMPOSTER_H__
