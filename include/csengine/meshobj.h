/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __CS_MESHOBJ_H__
#define __CS_MESHOBJ_H__

#include "csobject/pobject.h"
#include "csobject/nobjvec.h"
#include "csengine/cssprite.h"
#include "csengine/movable.h"
#include "imeshobj.h"

class Dumper;

/**
 * The holder class for all implementations of iMeshObject.
 */
class csMeshWrapper : public csSprite
{
  friend class Dumper;

private:
  /// Bounding box for polygon trees.
  csPolyTreeBBox bbox;

  /// Mesh object corresponding with this csMeshWrapper.
  iMeshObject* mesh;

  /// Children of this object (other instances of csMeshWrapper).
  csNamedObjVector children;

protected:
  /**
   * Update this sprite in the polygon trees.
   */
  virtual void UpdateInPolygonTrees ();
  /// Update movement.
  virtual void UpdateMove ();

public:
  /// Constructor.
  csMeshWrapper (csObject* theParent, iMeshObject* mesh);
  /// Constructor.
  csMeshWrapper (csObject* theParent);
  /// Destructor.
  virtual ~csMeshWrapper ();

  /// Set the mesh object.
  void SetMeshObject (iMeshObject* mesh);
  /// Get the mesh object.
  iMeshObject* GetMeshObject () const {return mesh;}

  /**
   * Light object according to the given array of lights (i.e.
   * fill the vertex color array).
   */
  virtual void UpdateLighting (csLight** lights, int num_lights);

  /**
   * Do some initialization needed for visibility testing.
   * i.e. clear camera transformation.
   */
  virtual void VisTestReset ()
  {
    bbox.ClearTransform ();
  }

  /**
   * Draw this mesh object given a camera transformation.
   * If needed the skeleton state will first be updated.
   * Optionally update lighting if needed (DeferUpdateLighting()).
   */
  virtual void Draw (csRenderView& rview);

  /// Go the next animation frame.
  virtual void NextFrame (cs_time current_time);

  /// Returns true if this object wants to die.
  virtual bool WantToDie () { return mesh->WantToDie (); }

  /**
   * Check if this sprite is hit by this object space vector.
   * Return the collision point in object space coordinates.
   */
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr);

  /// Get the children of this mesh object.
  csNamedObjVector& GetChildren () { return children; }

  CSOBJTYPE;
};

/**
 * The holder class for all implementations of iMeshObjectFactory.
 */
class csMeshFactoryWrapper : public csObject
{
  friend class Dumper;

private:
  /// Mesh object factory corresponding with this csMeshFactoryWrapper.
  iMeshObjectFactory* meshFact;

public:
  /// Constructor.
  csMeshFactoryWrapper (iMeshObjectFactory* meshFact);
  /// Constructor.
  csMeshFactoryWrapper ();
  /// Destructor.
  virtual ~csMeshFactoryWrapper ();

  /// Set the mesh object factory.
  void SetMeshObjectFactory (iMeshObjectFactory* meshFact);

  /// Get the mesh object factory.
  iMeshObjectFactory* GetMeshObjectFactory ()
  {
    return meshFact;
  }

  /**
   * Create a new mesh object for this template.
   */
  csMeshWrapper* NewMeshObject (csObject* parent);

  CSOBJTYPE;
  DECLARE_IBASE_EXT (csObject);

  //--------------------- iMeshFactoryWrapper implementation --------------------//
  struct MeshFactoryWrapper : public iMeshFactoryWrapper
  {
    DECLARE_EMBEDDED_IBASE (csMeshFactoryWrapper);
    virtual iMeshObjectFactory* GetMeshObjectFactory ()
    {
      return scfParent->GetMeshObjectFactory ();
    }
  } scfiMeshFactoryWrapper;
};

#endif // __CS_MESHOBJ_H__
