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

#ifndef __CS_ISOSPR_H__
#define __CS_ISOSPR_H__

#include "ivaria/iso.h"
#include "ivideo/graph3d.h"
#include "csgeom/poly2d.h"
#include "csgeom/poly3d.h"

struct iMaterialWrapper;

class csIsoSprite : public iIsoSprite {
private:
  /// the grid this sprites lives in
  iIsoGrid *grid;
  /// world space position
  csVector3 position;
  /// material to draw
  iMaterialWrapper *material;
  /// the 3d polygon in objectspace for the sprite
  csPoly3D poly;
  /// the (u,v) coordinates for the polygon (x=u, y=v)
  csPoly2D uv;
  /// the (dynamic) colors of the vertices (x=red, y=green, z=blue)
  csPoly3D colors;
  /// the static (precalc) colors of the vertices (x=red, y=green, z=blue)
  csPoly3D static_colors;
  /// drawing prealloc
  G3DPolygonDPFX g3dpolyfx;
  /// the grid change callback
  csRef<iGridChangeCallback> gridcall;

public:
  SCF_DECLARE_IBASE;

  ///
  csIsoSprite(iBase *iParent);
  ///
  virtual ~csIsoSprite();

  //-------- iIsoSprite ---------------------------------------------
  virtual int GetVertexCount() const;
  virtual void AddVertex(const csVector3& coord, float u, float v);
  virtual const csVector3& GetPosition() const {return position;}
  virtual void SetPosition(const csVector3& pos);
  virtual void MovePosition(const csVector3& delta);
  virtual void SetMaterialWrapper(iMaterialWrapper *material)
  {csIsoSprite::material = material;}
  virtual iMaterialWrapper* GetMaterialWrapper() const {return material;}
  virtual void SetMixMode(uint mode) {g3dpolyfx.mixmode = mode;}
  virtual uint GetMixMode() const {return g3dpolyfx.mixmode;}
  virtual void Draw(iIsoRenderView *rview);
  virtual void SetGrid(iIsoGrid *grid);
  virtual void SetAllColors(const csColor& color);
  virtual const csVector3& GetVertexPosition(int i)
  {
    CS_ASSERT (i >= 0 && i < poly.GetVertexCount ());
    return poly[i];
  }
  virtual void AddToVertexColor(int i, const csColor& color);
  virtual iIsoGrid *GetGrid() const {return grid;}
  virtual void SetGridChangeCallback (iGridChangeCallback* func)
  {
    gridcall = func;
  }
  virtual iGridChangeCallback* GetGridChangeCallback () const
  {
    return gridcall;
  }
  virtual void ForcePosition(const csVector3& pos) {position = pos;}
  virtual void ResetAllColors();
  virtual void SetAllStaticColors(const csColor& color);
  virtual void AddToVertexStaticColor(int i, const csColor& color);

};

#endif // __CS_ISOSPR_H__
