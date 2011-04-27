/*
    Copyright (C) 2001 by Jorrit Tyberghein and Richard D. Shank

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

#ifndef __CS_IENGINE_LOD_H__
#define __CS_IENGINE_LOD_H__

/**\file
 * Level Of Detail (LOD) interfaces
 */
/**
 * \addtogroup engine3d_meshes
 * @{ */
 
#include "csutil/scf.h"

struct iSharedVariable;

/**
 * The iLODControl interface represents an object that has controllable
 * Level Of Detail (LOD) features. The LOD manager can work with this.
 */
struct iLODControl : public virtual iBase
{
  SCF_INTERFACE(iLODControl, 2,0,1);

  /**
   * Set the parameters of the function used to compute the LOD of the mesh
   * depending on the distance to the camera. The function is as follows:
   * <pre>
   *    float lod = m * distance + a;
   * </pre>
   * The result of this function will be capped to the [0,1] range, with
   * 0 meaning worst quality possible (highest speed) and 1 highest
   * quality.
   */
  virtual void SetLOD (float m, float a) = 0;

  /**
   * Get the current parameters of the LOD function.
   */
  virtual void GetLOD (float& m, float& a) const = 0;

  /**
   * Set the parameters of the LOD function using shared variables.
   */
  virtual void SetLOD (iSharedVariable* varm, iSharedVariable* vara) = 0;

  /**
   * Return the parameters of the LOD function as shared variables. If the
   * parameters were not set using shared variables then varm and vara will be
   * set to 0.
   */
  virtual void GetLOD (iSharedVariable*& varm, iSharedVariable*& vara)
  	const = 0;

  /**
   * Get a rough estimate of the number of polygons for a given LOD value
   * (between 0 and 1, similar to the value used by SetLOD()).
   * Note that a mesh object that doesn't support LOD should always return
   * the same number of polygons.
   */
  virtual int GetLODPolygonCount (float lod) const = 0;

  // @@@ Poke res for docs
  /**
   * Set the fading factor to be used while transitioning from one LOD to another.
   */
  virtual void SetLODFade (float f) = 0;

  /**
   * Get the fading factor to be used while transitioning from one LOD to another.
   */
  virtual void GetLODFade (float& f) const = 0;

  /**
   * Set the fading factor as a shared variable.
   */
  virtual void SetLODFade (iSharedVariable* varf) = 0;

  /**
   * Set the fading factor as a shared variable. If this factor was not previsouly 
   * set using shared variables then varf will be set to 0.
   */
  virtual void GetLODFade (iSharedVariable*& varf) const = 0;
};

/** @} */

#endif // __CS_IENGINE_LOD_H__

