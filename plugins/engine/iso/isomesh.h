/*
    Copyright (C) 2001 by W.C.A. Wijngaards

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

#ifndef __ISOMESH_H__
#define __ISOMESH_H__

#include "ivaria/iso.h"
#include "csutil/csobject.h"
#include "csutil/nobjvec.h"
#include "ivideo/graph3d.h"
#include "csgeom/matrix3.h"
#include "csgeom/transfrm.h"
#include "iengine/mesh.h"

struct iMaterialWrapper;
struct iMeshFactoryWrapper;
struct iMeshFactoryList;

class csIsoMeshSprite : public iIsoMeshSprite {
private:
  /// the grid this sprite lives in
  iIsoGrid *grid;

  /// semi movable information
  /// world space position
  csVector3 position;
  /// transformation
  csMatrix3 transform;

  /// the grid change callback
  iGridChangeCallback* gridcall;

  /// the mesh object
  iMeshObject *mesh;
  /// zbuf mode for the mesh
  csZBufMode zbufmode;

public:
  CS_DECLARE_STATIC_CLASSVAR (pos, GetVertexPosition, csVector3)

public:
  SCF_DECLARE_IBASE;

  ///
  csIsoMeshSprite(iBase *iParent);
  ///
  virtual ~csIsoMeshSprite();

  //-------- iIsoSprite ---------------------------------------------
  virtual int GetVertexCount() const;
  virtual void AddVertex(const csVector3& coord, float u, float v);
  virtual const csVector3& GetPosition() const {return position;}
  virtual void SetPosition(const csVector3& pos);
  virtual void MovePosition(const csVector3& delta);
  virtual void SetMaterialWrapper(iMaterialWrapper *material);
  virtual iMaterialWrapper* GetMaterialWrapper() const;
  virtual void SetMixMode(uint mode);
  virtual uint GetMixMode() const;
  virtual void Draw(iIsoRenderView *rview);
  virtual void SetGrid(iIsoGrid *grid);
  virtual void SetAllColors(const csColor& color);
  virtual const csVector3& GetVertexPosition(int i);
  virtual void AddToVertexColor(int i, const csColor& color);
  virtual iIsoGrid *GetGrid() const {return grid;}
  virtual void SetGridChangeCallback(iGridChangeCallback* func)
  {
    SCF_SET_REF (gridcall, func);
  }
  virtual iGridChangeCallback* GetGridChangeCallback() const
  {
    return gridcall;
  }
  virtual void ForcePosition(const csVector3& pos) {position = pos;}
  virtual void ResetAllColors();
  virtual void SetAllStaticColors(const csColor& color);
  virtual void AddToVertexStaticColor(int i, const csColor& color);

  //-------- iIsoMeshSprite -----------------------------------------
  virtual void SetMeshObject(iMeshObject *mesh);
  virtual iMeshObject* GetMeshObject() const {return mesh;}
  virtual void SetTransform(const csMatrix3& trans) {transform = trans;}
  virtual const csMatrix3& GetTransform() const {return transform;}
  virtual void SetZBufMode(csZBufMode mode) {zbufmode = mode;}
  virtual csZBufMode GetZBufMode() const {return zbufmode;}
};

CS_DECLARE_OBJECT_VECTOR (csIsoMeshFactoryListHelper, iMeshFactoryWrapper);

/**
 * A list of mesh factories.
 */
class csIsoMeshFactoryList : public csIsoMeshFactoryListHelper
{
public:
  SCF_DECLARE_IBASE;

  /// constructor
  csIsoMeshFactoryList ();

  class MeshFactoryList : public iMeshFactoryList
  {
    SCF_DECLARE_EMBEDDED_IBASE (csIsoMeshFactoryList);
    virtual int GetCount () const;
    virtual iMeshFactoryWrapper *Get (int n) const;
    virtual int Add (iMeshFactoryWrapper *obj);
    virtual bool Remove (iMeshFactoryWrapper *obj);
    virtual bool Remove (int n);
    virtual void RemoveAll ();
    virtual int Find (iMeshFactoryWrapper *obj) const;
    virtual iMeshFactoryWrapper *FindByName (const char *Name) const;
  } scfiMeshFactoryList;
};

SCF_VERSION (csIsoMeshFactoryWrapper, 0, 0, 1);

/**
 * The holder class for all implementations of iMeshObjectFactory.
 */
class csIsoMeshFactoryWrapper : public csObject
{
private:
  /// Mesh object factory corresponding with this csMeshFactoryWrapper.
  iMeshObjectFactory* meshFact;
  csReversibleTransform tr;	// Dummy

private:
  /// Destructor.
  virtual ~csIsoMeshFactoryWrapper ();

public:
  /// Constructor.
  csIsoMeshFactoryWrapper (iMeshObjectFactory* meshFact);
  /// Constructor.
  csIsoMeshFactoryWrapper ();

  /// Set the mesh object factory.
  void SetMeshObjectFactory (iMeshObjectFactory* meshFact);

  /// Get the mesh object factory.
  iMeshObjectFactory* GetMeshObjectFactory () const
  {
    return meshFact;
  }

  /**
   * Do a hard transform of this factory.
   * This transformation and the original coordinates are not
   * remembered but the object space coordinates are directly
   * computed (world space coordinates are set to the object space
   * coordinates by this routine). Note that some implementations
   * of mesh objects will not change the orientation of the object but
   * only the position.
   */
  void HardTransform (const csReversibleTransform& t);

  SCF_DECLARE_IBASE_EXT (csObject);

  //----------------- iMeshFactoryWrapper implementation --------------------//
  struct MeshFactoryWrapper : public iMeshFactoryWrapper
  {
    SCF_DECLARE_EMBEDDED_IBASE (csIsoMeshFactoryWrapper);
    virtual iMeshObjectFactory* GetMeshObjectFactory () const
      { return scfParent->GetMeshObjectFactory (); }
    virtual void SetMeshObjectFactory (iMeshObjectFactory* fact)
      { scfParent->SetMeshObjectFactory (fact); }
    virtual iObject *QueryObject ()
      { return scfParent; }
    virtual void HardTransform (const csReversibleTransform& t)
      { scfParent->HardTransform (t); }
    virtual iMeshWrapper* CreateMeshWrapper ()
      { return NULL; }
    virtual iMeshFactoryWrapper* GetParentContainer () const
      { return NULL; }
    virtual void SetParentContainer (iMeshFactoryWrapper *p)
      { (void)p; }
    virtual iMeshFactoryList* GetChildren ()
      { return NULL; }
    virtual csReversibleTransform& GetTransform ()
      { return scfParent->tr; }
    virtual void SetTransform (const csReversibleTransform& tr)
      { (void)tr; }
  } scfiMeshFactoryWrapper;
  friend struct MeshFactoryWrapper;
};

#endif

