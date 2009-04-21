/*
    Copyright  (C) 2004 by Hristo Hristov

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or  (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_IMESH_GMESHSKEL2_H__
#define __CS_IMESH_GMESHSKEL2_H__

/**\file
 * General mesh object skeletal animation
 */ 

#include "csutil/scf.h"

#include "imesh/genmesh.h"

/**\addtogroup meshplugins
 * @{ */

struct iSkeleton;

class csReversibleTransform;
class csVector2;
class csVector3;


/**
 * This interface describes the API for setting up the skeleton animation
 * control as implemented by the 'gmeshskelanim' plugin. The objects that
 * implement iGenMeshSkeletonControlState also implement this interface.
 */
struct iGenMeshSkeletonControlState : public virtual iBase
{
  SCF_INTERFACE(iGenMeshSkeletonControlState, 2, 0, 0);

  /**
   * Get animated vertices 
   */
  virtual csVector3 *GetAnimatedVertices() = 0;

  /**
   * Get animated vertices count
   */
  virtual int GetAnimatedVerticesCount() = 0;

  /**
   * Get animated face normals
   */
  virtual csVector3 *GetAnimatedFaceNormals() = 0;

  /**
   * Get animated face normals count
   */
  virtual int GetAnimatedFaceNormalsCount() = 0;

  /**
   * Get animated vertices normals
   */
  virtual csVector3 *GetAnimatedVertNormals() = 0;

  /**
   * Get animated vertices normals count
   */
  virtual int GetAnimatedVertNormalsCount() = 0;

  virtual iSkeleton * GetSkeleton() = 0;

};

/** @} */

#endif // __CS_IMESH_GMESHSKEL2_H__
