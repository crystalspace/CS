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
#include "ivideo/graph3d.h"
#include "csgeom/matrix3.h"

struct iMaterialWrapper;

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
  void (*gridcall)(iIsoSprite *, void *);
  /// grid change callback userdata
  void *gridcalldata;

  /// the mesh object
  iMeshObject *mesh;
  /// zbuf mode for the mesh
  csZBufMode zbufmode;

public:
  DECLARE_IBASE;

  ///
  csIsoMeshSprite(iBase *iParent);
  ///
  virtual ~csIsoMeshSprite();

  //-------- iIsoSprite ---------------------------------------------
  virtual int GetNumVertices() const;
  virtual void AddVertex(const csVector3& coord, float u, float v);
  virtual const csVector3& GetPosition() const {return position;}
  virtual void SetPosition(const csVector3& pos);
  virtual void MovePosition(const csVector3& delta);
  virtual void SetMaterialWrapper(iMaterialWrapper *material);
  virtual iMaterialWrapper* GetMaterialWrapper() const;
  virtual void SetMixMode(UInt mode);
  virtual UInt GetMixMode() const;
  virtual void Draw(iIsoRenderView *rview);
  virtual void SetGrid(iIsoGrid *grid);
  virtual void SetAllColors(const csColor& color);
  virtual const csVector3& GetVertexPosition(int i);
  virtual void AddToVertexColor(int i, const csColor& color);
  virtual iIsoGrid *GetGrid() const {return grid;}
  virtual void SetGridChangeCallback(GridChangeCallbackType func, void *data)
  {gridcall = func; gridcalldata = data;}
  virtual void GetGridChangeCallback(GridChangeCallbackType &func, void *&data)
    const {func = gridcall; data = gridcalldata;}
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

#endif
