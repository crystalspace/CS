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

#ifndef __IENGINE_POLYTMAP_H__
#define __IENGINE_POLYTMAP_H__

#include "csutil/scf.h"

class csMatrix3;
class csVector3;
class csPolyTxtPlane;

SCF_VERSION (iPolyTxtPlane, 0, 0, 1);

/**
 * This is the interface to a texture plane. This is a plane
 * that defines the orientation and offset of a texture. It can
 * be used by several polygons to let the textures fit perfectly.
 */
struct iPolyTxtPlane : public iBase
{
  /// @@@ Ugly
  virtual csPolyTxtPlane* GetPrivateObject () = 0;
  /// Set the name of this plane.
  virtual void SetName (const char* name) = 0;
  /// Set texture space mapping.
  virtual void SetTextureSpace (const csVector3& v_orig,
			const csVector3& v1, float len1,
			const csVector3& v2, float len2) = 0;
  /// Set texture space mapping.
  virtual void SetTextureSpace (const csMatrix3& tx_matrix,
  			const csVector3& tx_vector) = 0;
  /// Get the transformation from object to texture space.
  virtual void GetTextureSpace (csMatrix3& tx_matrix, csVector3& tx_vector) = 0;
};

#endif // __IENGINE_POLYTMAP_H__

