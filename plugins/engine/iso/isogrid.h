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

#ifndef __CS_ISOGRID_H__
#define __CS_ISOGRID_H__

#include "ivaria/iso.h"
#include "csgeom/box.h"
#include "csutil/refarr.h"
#include "qint.h"

class csIsoGroundMap;

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
  /// the cells in this grid, 0 means an empty cell
  iIsoCell **grid;
  /**
   * minimum gridx and gridy coordinates
   * mingridx==minworldz, mingridy==minworldx
   */
  int mingridx, mingridy;
  /// the ground map of this grid
  csIsoGroundMap *groundmap;
  /// the lights in this grid, type iIsoLight
  csRefArray<iIsoLight> lights;
  /// the dynamic lights in this grid, type iIsoLight
  csRefArray<iIsoLight> dynamiclights;
  /// recalc static lighting?
  bool recalc_staticlight;

public:
  SCF_DECLARE_IBASE;

  ///
  csIsoGrid (iBase *iParent, iIsoWorld *world, int w, int h);
  ///
  virtual ~csIsoGrid ();

  /// get a cell from the grid
  iIsoCell *GetCell(int x, int y) const
  {
    if(x<0) x=0;
    else if(x >= width) x=width-1;
    if(y<0) y=0;
    else if(y >= height) y=height-1;
    CS_ASSERT (x >= 0 && x < width);
    CS_ASSERT (y >= 0 && y < height);
    return grid[y*width+x];
  }
  /// set a cell in the grid
  void SetCell(int x, int y, iIsoCell *val)
  {
    if(x<0) x=0;
    else if(x >= width) x=width-1;
    if(y<0) y=0;
    else if(y >= height) y=height-1;
    CS_ASSERT (x >= 0 && x < width);
    CS_ASSERT (y >= 0 && y < height);
    grid[y*width+x] = val;
  }

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

  /// recalculate the static lighting
  void RecalcStaticLight();
  /// reset all lighting to static values
  void ResetAllLight();

  //------ iIsoGrid ----------------------------------------
  virtual bool Contains(const csVector3& pos);
  virtual void AddSprite(iIsoSprite *sprite);
  virtual void AddSprite(iIsoSprite *sprite, const csVector3& pos);
  virtual void RemoveSprite(iIsoSprite *sprite);
  virtual void MoveSprite(iIsoSprite *sprite, const csVector3& oldpos,
    const csVector3& newpos);
  virtual void Draw(iIsoRenderView *rview);
  virtual iIsoWorld* GetWorld() const {return world;}
  virtual void SetSpace(int minx, int minz, float miny,
    float maxy);
  virtual int GetWidth() const {return width;}
  virtual int GetHeight() const {return height;}
  virtual void GetGridOffset(int& minx, int& miny) const
  {minx = mingridx; miny = mingridy;}
  virtual void SetGroundMult(int multx, int multy);
  virtual void SetGroundValue(int x, int y, int gr_x, int gr_y, float val);
  virtual float GetGroundValue(int x, int y, int gr_x, int gr_y);
  virtual float GetGroundValue(int x, int y);
  virtual bool GroundHitBeam(const csVector3& src, const csVector3& dest);
  virtual int GetGroundMultX() const;
  virtual int GetGroundMultY() const;
  virtual void SetAllLight(const csColor& color);
  virtual void SetAllStaticLight(const csColor& color);
  virtual void RegisterLight(iIsoLight *light);
  virtual void UnRegisterLight(iIsoLight *light);
  virtual void RegisterDynamicLight(iIsoLight *light);
  virtual void UnRegisterDynamicLight(iIsoLight *light);
  virtual iIsoCell* GetGridCell(int x, int y) { return GetCell(x,y); }
  virtual const csBox3& GetBox() const {return box;}
  virtual void GetFakeLights (const csVector3& pos, csArray<iLight*>& lights);
};


/**
 * The ground map
 */
class csIsoGroundMap {
  /// the grid
  iIsoGrid *grid;
  /// muliplier - number of ground values per cell
  int multx, multy;
  /// size of map
  int width, height;
  /// ground values
  float *map;

public:
  ///
  csIsoGroundMap(iIsoGrid *grid, int multx, int multy);
  ///
  ~csIsoGroundMap();

  /// get mult x
  int GetMultX() const {return multx;}
  /// get mult y
  int GetMultY() const {return multy;}
  /// set a value
  void SetGround(int x, int y, float val)
  {
    CS_ASSERT (x >= 0 && x < width);
    CS_ASSERT (y >= 0 && y < height);
    map[y*width+x]=val;
  }
  /// get a value
  float GetGround(int x, int y) const
  {
    CS_ASSERT (x >= 0 && x < width);
    CS_ASSERT (y >= 0 && y < height);
    return map[y*width+x];
  }
  /// see if src can hit dest
  bool HitBeam(const csVector3& src, const csVector3& dest);

};

#endif // __CS_ISOGRID_H__
