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

#include "cssysdef.h"
#include "isogrid.h"
#include "isocell.h"
#include "qsqrt.h"
#include "qint.h"

SCF_IMPLEMENT_IBASE (csIsoGrid)
  SCF_IMPLEMENTS_INTERFACE (iIsoGrid)
SCF_IMPLEMENT_IBASE_END

csIsoGrid::csIsoGrid (iBase *iParent, iIsoWorld *world, int w, int h)
{
  SCF_CONSTRUCT_IBASE (iParent);
  csIsoGrid::world = world;
  width = w;
  height = h;
  grid = new iIsoCell* [width*height];
  int i;
  for(i=0; i<width*height; i++)
    grid[i] = 0;
  mingridx = 0; mingridy = 0;
  box.Set(0,-9999,0, height,+9999,width);
  groundmap = new csIsoGroundMap(this, 1, 1);
  recalc_staticlight = true;
}

csIsoGrid::~csIsoGrid ()
{
  int i;
  for(i=0; i<width*height; i++)
    if(grid[i]) grid[i]->DecRef();

  delete[] grid;
  delete groundmap;
  SCF_DESTRUCT_IBASE();
}

bool csIsoGrid::Contains(const csVector3& pos)
{
  return box.In(pos);
}

void csIsoGrid::AddSprite(iIsoSprite *sprite)
{
  AddSprite(sprite, sprite->GetPosition());
}

void csIsoGrid::AddSprite(iIsoSprite *sprite, const csVector3& pos)
{
  iIsoCell *cell = GetCell(pos);
  if(!cell)
  {
    //printf("new cell\n");
    cell = new csIsoCell(0);
    SetCell(pos, cell);
  }
  //printf("adding sprite at pos (%g, %g, %g) to cell %x\n", pos.x,
    //pos.y, pos.z, cell);
  GetCell(pos)->AddSprite(sprite, pos);

  if(!recalc_staticlight)
  {
    // avoid recalc of entire grid by lighting this sprite only.
    // if recalc of entire grid is scheduled anyway, it isn't needed.
    // Shine static lights on sprite
    sprite->SetAllStaticColors(csColor(0,0,0));
    for(int l=0; l<lights.Length(); l++)
      lights[l]->ShineSprite(sprite);
  }
}

void csIsoGrid::RemoveSprite(iIsoSprite *sprite)
{
  iIsoCell *cell = GetCell(sprite->GetPosition());
  if(cell) cell->RemoveSprite(sprite, sprite->GetPosition());
}

void csIsoGrid::MoveSprite(iIsoSprite *sprite, const csVector3& oldpos,
    const csVector3& newpos)
{
  //printf("IsoGrid::MoveSprite\n");
  /// same as below, but detect edging errors.
  //if(box.In(newpos)), must be epsilon from the border at least.
  if( (newpos.x - box.MinX() > EPSILON)
    && (newpos.y - box.MinY() > EPSILON)
    && (newpos.z - box.MinZ() > EPSILON)
    && (box.MaxX() - newpos.x > EPSILON)
    && (box.MaxY() - newpos.y > EPSILON)
    && (box.MaxZ() - newpos.z > EPSILON)
    )
  {
    //printf("Sprite moved to new pos\n");
    // prevent to destruct the sprite when moving
    sprite->IncRef ();
    iIsoCell *oldcell = GetCell(oldpos);
    if(oldcell) oldcell->RemoveSprite(sprite, oldpos);
    AddSprite(sprite, newpos);
    sprite->DecRef ();
    return;
  }
  // sprite not any longer in this grid
  iIsoGrid *newgrid = world->FindGrid(newpos);
  if(!newgrid)
  {
    // uh oh sprite moved out of *all* grids
    // disallow the movement
    //printf("Grid: no grid, disallowed movement\n");
    sprite->ForcePosition(oldpos);
    return;
  }
  //printf("Grid: Sprite moved to new grid\n");
  sprite->IncRef ();
  iIsoCell *prevcell = GetCell(oldpos);
  if(prevcell) prevcell->RemoveSprite(sprite, oldpos);
  sprite->SetGrid(newgrid);
  newgrid->AddSprite(sprite, newpos);
  sprite->DecRef ();
}

void csIsoGrid::Draw(iIsoRenderView *rview)
{
  //printf("IsoGrid::Draw pass %d\n", rview->GetRenderPass());
  // only draw all the cells in the main render pass
  if((rview->GetRenderPass() == CSISO_RENDERPASS_MAIN)
    || (rview->GetRenderPass() == CSISO_RENDERPASS_FG))
  {
    //printf("Grid::Draw() MAIN\n");
    // draw the cells in lines, worldspace z-x = c
    // which, in the grid coords, becomes x-y = c
    // for a constant which is equal to the 'depth'.

    // note: we could/should clip on the depth, but for now
    // each cell is considered infinite in height.
    // start grid x,y
    int startx, starty, scanw, scanh;
    float celpery;
    rview->GetPrecalcGrid(startx, starty, scanw, scanh, celpery);
    startx -= mingridx;
    starty -= mingridy;
    int extratop = QInt(box.Min().y*celpery+0.9);
    startx += extratop;
    starty -= extratop;
    scanh += extratop + QInt(box.Max().y*celpery+0.9);
    //if( (startx < 0) || (starty < 0))
      //return; // nothing to do, we are not visible.
    iIsoCell *cell = 0;
    int i;
    int hpos, wpos, posx, posy;
    /// scan each cell in the region.
    int sw, ew;

    //// select optimized(1) or clean(0) code
#if 1
    hpos = 0;
    if((width-0) < startx-starty)
      hpos = (startx-starty - (width-0))/2;
    if(0-height > startx-starty-scanh*2)
      scanh -= (0-height-(startx-starty-scanh*2))/2;
    for(; hpos < scanh; hpos++)
    {
      sw = 0; ew = scanw;
      i=(startx-hpos)-width+1; if(i>sw) sw=i;
      i=(starty+hpos)-height+1; if(i>sw) sw=i;
      i=(startx-hpos-ew)+1; if(i<0) ew+=i;
      i=(starty+hpos-ew)+1; if(i<0) ew+=i;
      posx = startx - hpos - sw;
      posy = starty + hpos - sw;
      //wpos = ew - sw;
      //if(wpos>0)
      //while(wpos--);
      for(wpos = sw; wpos < ew; wpos++)
      {
        //posx = startx - hpos - wpos;
	//posy = starty + hpos - wpos;
        //if( (posx<0) || (posx>=width) || (posy<0) || (posy>=height))
	  //printf("OOPS\n");
	cell = GetCell(posx, posy);
	//printf("cell %d %d %x\n", posx, posy, (int)cell);
	if(cell) cell->Draw(rview);
	posx--; posy--;
      }
      sw = 0; ew = scanw;
      i=(startx-hpos)-width+1; if(i>sw) sw=i;
      i=(starty+hpos+1)-height+1; if(i>sw) sw=i;
      i=(startx-hpos-ew)+1; if(i<0) ew+=i;
      i=(starty+hpos-ew+1)+1; if(i<0) ew+=i;
      posx = startx - hpos - sw;
      posy = starty + hpos - sw + 1;
      //wpos = ew - sw;
      //if(wpos>0)
      //while(wpos--);
      for(wpos = sw; wpos < ew; wpos++)
      {
        //posx = startx - hpos - wpos;
	//posy = starty + hpos - wpos + 1;
        //if( (posx<0) || (posx>=width) || (posy<0) || (posy>=height))
	  //printf("OOPS\n");
	cell = GetCell(posx, posy);
	if(cell) cell->Draw(rview);
	posx--; posy--;
      }
    }

    return;

#else
    /// alternative (cleaner code)
    for(hpos = 0; hpos < scanh; hpos++)
    {
      for(wpos = 0; wpos < scanw; wpos++)
      {
        posx = startx - hpos - wpos;
	posy = starty + hpos - wpos;
        if( (posx<0) || (posx>=width) || (posy<0) || (posy>=height))
	  continue;
	cell = GetCell(posx, posy);
	//printf("cell %d %d %x\n", posx, posy, (int)cell);
	if(cell) cell->Draw(rview);
      }
      for(wpos = 0; wpos < scanw; wpos++)
      {
        posx = startx - hpos - wpos;
	posy = starty + hpos - wpos + 1;
        if( (posx<0) || (posx>=width) || (posy<0) || (posy>=height))
	  continue;
	cell = GetCell(posx, posy);
	if(cell) cell->Draw(rview);
      }
    }
    return;
#endif
  }
  // other render passes?
  if(rview->GetRenderPass() == CSISO_RENDERPASS_PRE)
  {
    // check lower bound on Z.
    float myminz = box.MinZ() - box.MaxX() - 10.;
    if(myminz < rview->GetMinZ()) rview->SetMinZ(myminz);

    // calc static lights if needed
    if(recalc_staticlight) RecalcStaticLight();
    // calculate dyn lighting
    ResetAllLight();
    int l;
    for(l=0; l<dynamiclights.Length(); l++)
      dynamiclights[l]->ShineGrid();
  }
}

void csIsoGrid::RecalcStaticLight()
{
  SetAllStaticLight(csColor(0.,0.,0.));
  int l;
  for(l=0; l<lights.Length(); l++)
    lights[l]->ShineGrid();
  recalc_staticlight = false;
}

void csIsoGrid::SetSpace(int minx, int minz, float miny = -1.0,
  float maxy = +10.0)
{
  mingridx = minz;
  mingridy = minx;
  box.Set(minx,miny,minz, minx+height,maxy,minz+width);
}

void csIsoGrid::SetGroundMult(int multx, int multy)
{
  delete groundmap;
  groundmap = new csIsoGroundMap(this, multx, multy);
}

void csIsoGrid::SetGroundValue(int x, int y, int gr_x, int gr_y, float val)
{
  groundmap->SetGround(x*groundmap->GetMultX()+gr_x,
    y*groundmap->GetMultY()+gr_y, val);
}

float csIsoGrid::GetGroundValue(int x, int y, int gr_x, int gr_y)
{
  return groundmap->GetGround(x*groundmap->GetMultX()+gr_x,
    y*groundmap->GetMultY()+gr_y);
}

float csIsoGrid::GetGroundValue(int x, int y)
{
  return groundmap->GetGround(x, y);
}

bool csIsoGrid::GroundHitBeam(const csVector3& src, const csVector3& dest)
{
  return groundmap->HitBeam(src, dest);
}

int csIsoGrid::GetGroundMultX() const
{
  return groundmap->GetMultX();
}

int csIsoGrid::GetGroundMultY() const
{
  return groundmap->GetMultY();
}

struct ResetSpriteLight : public iIsoCellTraverseCallback
{
  SCF_DECLARE_IBASE;
  ResetSpriteLight () { SCF_CONSTRUCT_IBASE (0); }
  virtual ~ResetSpriteLight () { SCF_DESTRUCT_IBASE(); }
  virtual void Traverse (iIsoSprite* spr);
};

SCF_IMPLEMENT_IBASE (ResetSpriteLight)
  SCF_IMPLEMENTS_INTERFACE (iIsoCellTraverseCallback)
SCF_IMPLEMENT_IBASE_END

void ResetSpriteLight::Traverse (iIsoSprite* spr)
{
  spr->ResetAllColors();
}

void csIsoGrid::ResetAllLight()
{
  ResetSpriteLight* rs = new ResetSpriteLight ();
  int i;
  for(i=0; i<width*height; i++)
    if(grid[i])
      grid[i]->Traverse (rs);
  rs->DecRef ();
}

struct SetSpriteColor : public iIsoCellTraverseCallback
{
  const csColor* col;
  SCF_DECLARE_IBASE;
  SetSpriteColor () { SCF_CONSTRUCT_IBASE (0); }
  virtual ~SetSpriteColor () { SCF_DESTRUCT_IBASE(); }
  virtual void Traverse (iIsoSprite* spr);
};

SCF_IMPLEMENT_IBASE (SetSpriteColor)
  SCF_IMPLEMENTS_INTERFACE (iIsoCellTraverseCallback)
SCF_IMPLEMENT_IBASE_END

void SetSpriteColor::Traverse (iIsoSprite* spr)
{
  spr->SetAllColors(*col);
}

void csIsoGrid::SetAllLight(const csColor& color)
{
  // make copy of color, since (weird usermade) iIsoSprites could violate
  // the const that is promised in my heading.
  SetSpriteColor* sp = new SetSpriteColor ();
  csColor col = color;
  sp->col = &col;
  int i;
  for(i=0; i<width*height; i++)
    if(grid[i])
    {
      grid[i]->Traverse (sp);
    }
  sp->DecRef ();
}

struct SetSpriteStaticColor : public iIsoCellTraverseCallback
{
  const csColor* col;
  SCF_DECLARE_IBASE;
  SetSpriteStaticColor () { SCF_CONSTRUCT_IBASE (0); }
  virtual ~SetSpriteStaticColor () { SCF_DESTRUCT_IBASE(); }
  virtual void Traverse (iIsoSprite* spr);
};

SCF_IMPLEMENT_IBASE (SetSpriteStaticColor)
  SCF_IMPLEMENTS_INTERFACE (iIsoCellTraverseCallback)
SCF_IMPLEMENT_IBASE_END

void SetSpriteStaticColor::Traverse (iIsoSprite* spr)
{
  spr->SetAllColors(*col);
  spr->SetAllStaticColors(*col);
}

void csIsoGrid::SetAllStaticLight(const csColor& color)
{
  csColor col = color;
  SetSpriteStaticColor* sp = new SetSpriteStaticColor ();
  sp->col = &col;
  int i;
  for(i=0; i<width*height; i++)
    if(grid[i])
    {
      grid[i]->Traverse (sp);
    }
  sp->DecRef ();
}


void csIsoGrid::RegisterLight(iIsoLight *light)
{
  recalc_staticlight = true;
  if(lights.Find(light)==-1)
  {
    lights.Push(light);
  }
}

void csIsoGrid::UnRegisterLight(iIsoLight *light)
{
  int idx = lights.Find(light);
  if(idx!=-1)
  {
    lights.DeleteIndex (idx);
    recalc_staticlight = true;
  }
}

void csIsoGrid::RegisterDynamicLight(iIsoLight *light)
{
  if(dynamiclights.Find(light)==-1)
  {
    dynamiclights.Push(light);
  }
}

void csIsoGrid::UnRegisterDynamicLight(iIsoLight *light)
{
  dynamiclights.Delete (light);
}

void csIsoGrid::GetFakeLights (const csVector3& pos, csArray<iLight*>& flights)
{
  flights.Empty ();
  // fill the current array
  int l;

  int visx = QInt( (pos.z - float(mingridx) )*groundmap->GetMultX());
  int visy = QInt( (pos.x - float(mingridy) )*groundmap->GetMultY());

  for(l=0; l<lights.Length(); l++)
  {
    if(lights[l]->GetVis(visx, visy) > 0.0 )
      flights.Push (lights[l]->GetFakeLight());
  }
  for(l=0; l<dynamiclights.Length(); l++)
  {
    if(dynamiclights[l]->GetVis(visx, visy) > 0.0 )
      flights.Push (dynamiclights[l]->GetFakeLight());
  }

}

//-------------- csIsoGroundMap -------------------------------------------
csIsoGroundMap::csIsoGroundMap(iIsoGrid *grid, int multx, int multy)
{
  csIsoGroundMap::grid = grid;
  csIsoGroundMap::multx = multx;
  csIsoGroundMap::multy = multy;
  width = grid->GetWidth() * multx;
  height = grid->GetHeight() * multy;
  map = new float[width*height];
  int i;
  for(i=0; i<width*height; i++)
    map[i] = 0.0f;
}

csIsoGroundMap::~csIsoGroundMap()
{
  delete[] map;
}

bool csIsoGroundMap::HitBeam(const csVector3& gsrc, const csVector3& gdest)
{
  csVector3 src = gsrc, dest=gdest;
  /// go through each mapcell and keep track of visibility.
  int mingridx = 0, mingridy = 0;
  grid->GetGridOffset(mingridx, mingridy);

  /// shift src/dest so that they both fall in the grid
  csBox3 box = grid->GetBox();
  box.Set(box.Min().x, -99999, box.Min().z, box.Max().x, +99999, box.Max().z);
  csVector3 isect;
  csSegment3 seg(src, dest);
  if(!box.In(src))
    if(csIntersect3::BoxSegment(box, seg, isect) > -1)
    {
      src = isect;
      seg.SetStart(isect + 0.001*(dest-src)); // avoid 2nd hit on src
    }
  if(!box.In(dest))
    if(csIntersect3::BoxSegment(box, seg, isect) > -1)
    {
      dest = isect;
      //seg.SetEnd(isect);
    }

  csVector3 delta = dest-src; // src + delta = dest

  /// check each square along groundsquare size steps...
  if(delta.IsZero()) return true;
  float len = 2.0*qsqrt(delta.z*delta.z*float(multx*multx) +
    delta.x*delta.x*float(multy*multy));
  csVector3 m = delta/len;
  m.z *= float(multx);
  m.x *= float(multy);
  int steps = QInt(len);
  csVector3 pos = src;
  pos.z *= float(multx);
  pos.x *= float(multy);
  pos.z -= mingridx*multx;
  pos.x -= mingridy*multy;
  int x,z;
  while(steps--)
  {
    //x = QInt(pos.z) - multminx;
    //y = QInt(pos.x) - multminy;
    //printf("Checking %d,%d (%g,%g,%g) %g\n", x,y, pos.x, pos.y, pos.z,
      //GetGround(x,y));
    z = QInt(pos.z);
    x = QInt(pos.x);
    if(x >= 0 && z >= 0 && z<width && x<height)
      if(pos.y <= GetGround(QInt(pos.z), QInt(pos.x)))
        return false;
    pos += m;
  }
  return true;
}


