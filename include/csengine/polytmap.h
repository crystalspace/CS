/*
    Copyright (C) 1999-2001 by Jorrit Tyberghein
  
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

#ifndef __CS_POLYTMAP_H__
#define __CS_POLYTMAP_H__

#include "csgeom/transfrm.h"
#include "csobject/csobject.h"
#include "csobject/pobject.h"
#include "iengine/polytmap.h"

class Dumper;


SCF_VERSION (csPolyTxtPlane, 0, 0, 1);

/**
 * This class represents a texture plane. This is a plane
 * that defines the orientation and offset of a texture. It can
 * be used by several polygons to let the textures fit perfectly.
 */
class csPolyTxtPlane : public csPObject
{
  ///
  friend class csPolygon2D;
  ///
  friend class csPolyTexture;
  ///
  friend class Dumper;

private:
  /// Transformation from object to texture space.
  csMatrix3 m_obj2tex;
  /// Translation from object to texture space.
  csVector3 v_obj2tex;

  /// Transformation from world to texture space.
  csMatrix3 m_world2tex;
  /// Translation from world to texture space.
  csVector3 v_world2tex;

  /// Destructor is private.
  virtual ~csPolyTxtPlane ();

public:
  /// Constructor. Reference count is initialized to 1.
  csPolyTxtPlane ();

  /**
   * Transform this plane from object space to world space using
   * the given transform.
   */
  void ObjectToWorld (const csReversibleTransform& obj);

  /**
   * Hard transform of this plane.
   */
  void HardTransform (const csReversibleTransform& obj);

  /**
   * Transform this plane from world space to camera space using
   * the given transform. The resulting transform is put in m_cam2tex
   * and v_cam2tex.
   */
  void WorldToCamera (const csReversibleTransform& t,
  	csMatrix3& m_cam2tex, csVector3& v_cam2tex);

  /**
   * Get object to texture transformation.
   */
  void GetObjectToTexture (csMatrix3*& m_obj2tex, csVector3*& v_obj2tex)
  {
    m_obj2tex = &this->m_obj2tex;
    v_obj2tex = &this->v_obj2tex;
  }

  /**
   * Get world to texture transformation.
   */
  void GetWorldToTexture (csMatrix3*& m_wor2tex, csVector3*& v_wor2tex)
  {
    m_wor2tex = &this->m_world2tex;
    v_wor2tex = &this->v_world2tex;
  }

  ///
  void SetTextureSpace (const csVector3& v_orig,
			const csVector3& v1, float len1,
			const csVector3& v2, float len2);
  ///
  void SetTextureSpace (const csPlane3& plane_wor,
  			float xo, float yo, float zo,
			float x1, float y1, float z1,
			float len);
  ///
  void SetTextureSpace (const csPlane3& plane_wor,
  			const csVector3& v_orig,
  			const csVector3& v1, float len);
  ///
  void SetTextureSpace (const csVector3& v_orig,
  			const csVector3& v_u,
			const csVector3& v_v);
  ///
  void SetTextureSpace (float xo, float yo, float zo,
			float xu, float yu, float zu,
			float xv, float yv, float zv);
  ///
  void SetTextureSpace (float xo, float yo, float zo,
			float xu, float yu, float zu,
			float xv, float yv, float zv,
			float xw, float yw, float zw);
  ///
  void SetTextureSpace (const csMatrix3& tx_matrix,
  			const csVector3& tx_vector);

  /// Get the transformation from object to texture space.
  void GetTextureSpace (csMatrix3& tx_matrix, csVector3& tx_vector);

  CSOBJTYPE;
  DECLARE_IBASE_EXT (csPObject);

  //----------------- iPolyTxtPlane interface implementation -----------------
  struct PolyTxtPlane : public iPolyTxtPlane
  {
    DECLARE_EMBEDDED_IBASE (csPolyTxtPlane);

    virtual iObject *QueryObject()
    {
      return scfParent;
    }
    virtual csPolyTxtPlane* GetPrivateObject ()
    {
      return (csPolyTxtPlane*)scfParent;
    }
    virtual void SetTextureSpace (const csVector3& v_orig,
			const csVector3& v1, float len1,
			const csVector3& v2, float len2)
    {
      scfParent->SetTextureSpace (v_orig, v1, len1, v2, len2);
    }
    virtual void SetTextureSpace (const csMatrix3& tx_matrix,
  			const csVector3& tx_vector)
    {
      scfParent->SetTextureSpace (tx_matrix, tx_vector);
    }
    virtual void GetTextureSpace (csMatrix3& tx_matrix, csVector3& tx_vector)
    {
      scfParent->GetTextureSpace (tx_matrix, tx_vector);
    }
  } scfiPolyTxtPlane;
  friend struct PolyTxtPlane;
};

#endif // __CS_POLYTMAP_H__
