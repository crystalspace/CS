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

#ifndef __ISOGRID_H__
#define __ISOGRID_H__

#include "ivaria/iso.h"
#include "csgeom/box.h"
#include "qint.h"

/**
 *
*/
class csIsoGrid : public iIsoGrid {
private:
  /// The world the grid is part of
  iIsoWorld *world;
  /// the part of the world occupied by the grid
  csBox3 box;

  /// size of the grid
  int width, height;
  /// the cells in this grid, NULL means an empty cell
  iIsoCell **grid;
  /** 
   * minimum gridx and gridy coordinates
   * mingridx==minworldz, mingridy==minworldx
   */
  int mingridx, mingridy;

public:
  DECLARE_IBASE;

  ///
  csIsoGrid (iBase *iParent, iIsoWorld *world, int w, int h);
  ///
  virtual ~csIsoGrid ();

  /// get a cell from the grid
  iIsoCell *GetCell(int x, int y) const {return grid[y*width+x];}
  /// set a cell in the grid
  void SetCell(int x, int y, iIsoCell *val) {grid[y*width+x] = val;}

  /** 
   *  Get the Cell for a given position in world space
   *  Assumes that grid.Contains(position).
   */
  inline iIsoCell *GetCell(const csVector3& pos) const
  { return GetCell(QInt(pos.z)-mingridx, QInt(pos.x)-mingridy); }
  /** 
   *  Set the Cell for a given position in world space
   *  Assumes that grid.Contains(position).
   */
  inline void SetCell(const csVector3& pos, iIsoCell *x)
  { SetCell(QInt(pos.z)-mingridx, QInt(pos.x)-mingridy, x); }

  //------ iIsoGrid ----------------------------------------
  virtual bool Contains(const csVector3& pos);
  virtual void AddSprite(iIsoSprite *sprite);
  virtual void AddSprite(iIsoSprite *sprite, const csVector3& pos);
  virtual void RemoveSprite(iIsoSprite *sprite);
  virtual void MoveSprite(iIsoSprite *sprite, const csVector3& oldpos,
    const csVector3& newpos);
  virtual void Draw(iIsoRenderView *rview);
  virtual iIsoWorld* GetWorld() const {return world;}
  virtual void SetSpace(int minx, int minz, float miny = -1.0, 
    float maxy = +10.0);

};

#endif
