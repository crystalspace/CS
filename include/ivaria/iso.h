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

#ifndef __IISO_H__
#define __IISO_H__

#include "csutil/scf.h"
#include "isys/plugin.h"
#include "csutil/csrect.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"

struct iSystem;
struct iEvent;
struct iGraphics2D;
struct iGraphics3D;
struct iTextureManager;
struct iMaterialHandle;
struct iClipper2D;
struct iIsoWorld;
struct iIsoView;
struct iIsoRenderView;
struct iIsoSprite;
struct iIsoGrid;
struct iIsoCell;

SCF_VERSION (iIsoEngine, 0, 0, 1);

/**
 * SCF Interface to the isometric engine.
*/
struct iIsoEngine : public iPlugIn
{
  /// Get the system
  virtual iSystem* GetSystem() const = 0;
  /// Get the 2d canvas
  virtual iGraphics2D* GetG2D() const = 0;
  /// Get the 3d renderer
  virtual iGraphics3D* GetG3D() const = 0;
  /// Get the texture manager
  virtual iTextureManager* GetTextureManager() const = 0;

  /// flags to pass to g3d->BeginDraw()
  virtual int GetBeginDrawFlags () const = 0;

  /// Create a new world
  virtual iIsoWorld* CreateWorld() = 0;
  /// Create new view on the given world
  virtual iIsoView* CreateView(iIsoWorld *world) = 0;
  /// Create new sprite
  virtual iIsoSprite* CreateSprite() = 0;
  /// (convenience) create new floor/ceiling tile.
  virtual iIsoSprite* CreateFloorSprite(const csVector3& pos, float w, 
    float h) = 0;
  /// (convenience) create new front-facing sprite (for objects).
  virtual iIsoSprite* CreateFrontSprite(const csVector3& pos, float w, 
    float h) = 0;
};


SCF_VERSION (iIsoWorld, 0, 0, 1);

/**
 * The isometric world, contains all the sprites to render the world.
*/
struct iIsoWorld : public iBase
{
  /// Add a sprite to this world
  virtual void AddSprite(iIsoSprite *sprite) = 0;
  /// Remove a sprite from this world
  virtual void RemoveSprite(iIsoSprite *sprite) = 0;
  /// Move a sprite already in this world, give previous and new position
  virtual void MoveSprite(iIsoSprite *sprite, const csVector3& oldpos,
    const csVector3& newpos) = 0;

  /// Create a new isoGrid in this world.
  virtual iIsoGrid* CreateGrid(int width, int height) = 0;
  /// Find an isoGrid that contains given position, can be NULL.
  virtual iIsoGrid* FindGrid(const csVector3& pos) = 0;

  /// Draw using given renderview
  virtual void Draw(iIsoRenderView *rview) = 0;
};

SCF_VERSION (iIsoGrid, 0, 0, 1);

/**
 * a grid - part of the world. Contains a number of cells.
 * contains sort of a screenfull of world space.
 * it has a width in number of cells - laying in the z direction.
 * and it has a height in number of cells - laying in the x direction.
*/
struct iIsoGrid : public iBase
{
  /// the the world that this grid is part of.
  virtual iIsoWorld* GetWorld() const = 0;
  /**
   * set the minumum x and minimum z world coordinates for this grid.
   * After the call the grid occupies the space in the world from
   * (minx, miny, minz) to (minx+height, maxy, minz+width)
   * Because cells are aligned at the whole numbers, minx and minz must
   * be whole numbers too. Contents are not shifted, so use when empty.
   */
  virtual void SetSpace(int minx, int minz, float miny = -1.0, 
    float maxy = +10.0) = 0;
  /// does this grid contain given position?
  virtual bool Contains(const csVector3& pos) = 0;
  /// get the width of the grid
  virtual int GetWidth() const = 0;
  /// get the height of the grid
  virtual int GetHeight() const = 0;
  /// get the grid offset
  virtual void GetGridOffset(int& minx, int& miny) const = 0;

  /// Add a sprite to this grid
  virtual void AddSprite(iIsoSprite *sprite) = 0;
  /// Add a sprite to this grid at a given position (used internally)
  virtual void AddSprite(iIsoSprite *sprite, const csVector3& pos) = 0;
  /// Remove a sprite from this grid
  virtual void RemoveSprite(iIsoSprite *sprite) = 0;
  /** 
    * Move a sprite already in this grid, give previous and new position 
    * (used internally by IsoSprite).
    */
  virtual void MoveSprite(iIsoSprite *sprite, const csVector3& oldpos,
    const csVector3& newpos) = 0;
    
  /// Draw using given renderview
  virtual void Draw(iIsoRenderView *rview) = 0;
};

SCF_VERSION (iIsoCell, 0, 0, 1);

/**
 * a grid cell. Size is 1.000 x 1.000 in (x,z) world space.
*/
struct iIsoCell : public iBase
{
  /// Add a sprite to this cell at pos (used internally)
  virtual void AddSprite(iIsoSprite *sprite, const csVector3& pos) = 0;
  /// Remove a sprite from this cell from pos (used internally)
  virtual void RemoveSprite(iIsoSprite *sprite, const csVector3& pos) = 0;
  /// Draw using given renderview
  virtual void Draw(iIsoRenderView *rview) = 0;
};

SCF_VERSION (iIsoView, 0, 0, 1);

/**
 * A view on the isometric world. Can be used to render a specific portion
 * of an isoWorld onto a particular part of the screen.

 * The axes are aligned like this
 *  +y
 *  |     ./+z
 *  |   ./
 *  | ./
 *  |/
 *   \.
 *     \.
 *       \.
 *         \+x
*/
struct iIsoView : public iBase
{
  /// Set world to use
  virtual void SetWorld(iIsoWorld* world) = 0;
  /// Get world shown in view
  virtual iIsoWorld* GetWorld() const = 0;

  /// Set the rectangle on the screen to draw to
  virtual void SetRect(const csRect& rect) = 0;
  /// Get the rectangle on the screen to draw to
  virtual const csRect& GetRect() const = 0;

  /** set the axis of the view on the world.
   *  The scale is in pixels, and must be > 0. Default is g3d->height/16.
   *  The skew values determine the angle of the z and x axis.
   *  it is the amount of vertical change per horizontal change.
   *  1.0 gives perfect isometrical view, 0.5 flattens the lines.
   *  skew values must be > 0.
   */
  virtual void SetAxes(float xscale, float yscale, float zscale,
    float zskew, float xskew) = 0;

  /// See the world position scrolled to (shown in center of view)
  virtual const csVector2& GetScroll() const = 0;
  /// Set the scroll position. Show world space pos as screen space coord.
  virtual void SetScroll(const csVector3& worldpos, const csVector2& coord) = 0;
  /// Move the scroll position by delta (in world space).
  virtual void MoveScroll(const csVector3& delta) = 0;

  /// Transform world space coordinate into a screen coordinate.
  virtual void W2S(const csVector3& world, csVector2& screen) = 0;
  /** 
   *  Transform world space coordinate into a screen coordinate.
   *  The returned z value can be used for a zbuffer, is bigger when
   *  further away.
   */
  virtual void W2S(const csVector3& world, csVector3& screen) = 0;
  /**
   *  Transform screen coordinate into a world space coordinate.
   *  The world space coordinate will have y=0.
   */
  virtual void S2W(const csVector2& screen, csVector3& world) = 0;

  /** 
   * draw the view onto the screen.
   * Call this when in 3d mode (with the engine->getBeginDrawFlags passed)
   */
  virtual void Draw() = 0;

};

SCF_VERSION (iIsoRenderView, 0, 0, 1);

/// time to do pre-rendering calculations
#define CSISO_RENDERPASS_PRE 	0
/// this pass is when the background items (nontransparent) are drawn.
#define CSISO_RENDERPASS_BG 	1
/// this pass do the main drawing stuff.
#define CSISO_RENDERPASS_MAIN 	2
/// this pass is when the transparent items, the foreground is drawn.
#define CSISO_RENDERPASS_FG 	3
/// post drawing calcs if needed
#define CSISO_RENDERPASS_POST 	4

/**
 * a view being rendered onto the screen.
*/
struct iIsoRenderView : public iBase
{
  /// get the view for this rendering
  virtual iIsoView* GetView() const = 0;
  /// get g3d
  virtual iGraphics3D* GetG3D() const = 0;
  /// get the pass number, see CSISO_RENDERPASS_... defines
  virtual int GetRenderPass() const = 0;
  /// get the clipper
  virtual iClipper2D* GetClipper() const = 0;
  /// get precalc grid values
  virtual void GetPrecalcGrid(int& startx, int& starty, int& scanw, 
    int& scanh) const = 0;
};

SCF_VERSION (iIsoSprite, 0, 0, 1);

/**
 * A sprite for the isometric engine.
*/
struct iIsoSprite : public iBase
{
  /// get the number of vertices
  virtual int GetNumVertices() const = 0;
  /// add a new vertex to the polygon
  virtual void AddVertex(const csVector3& coord, float u, float v) = 0;

  /// Get the world position of the sprite
  virtual const csVector3& GetPosition() const = 0;
  /// Set the position. In world space.
  virtual void SetPosition(const csVector3& pos) = 0;
  /// Move the position by delta.
  virtual void MovePosition(const csVector3& delta) = 0;

  /// Set the materialhandle to use
  virtual void SetMaterialHandle(iMaterialHandle *material) = 0;
  /// Get the materialhandle
  virtual iMaterialHandle* GetMaterialHandle() const = 0;
  /// Set the mixmode
  virtual void SetMixmode(UInt mode) = 0;
  /// Get the mixmode
  virtual UInt GetMixmode() const = 0;

  /// Draw using given renderview
  virtual void Draw(iIsoRenderView *rview) = 0;

  /// Set the grid this sprite is part of (used as notification by grid/world)
  virtual void SetGrid(iIsoGrid *grid) = 0;
};


#endif
