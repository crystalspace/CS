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

IMPLEMENT_IBASE (csIsoGrid)
  IMPLEMENTS_INTERFACE (iIsoGrid)
IMPLEMENT_IBASE_END

csIsoGrid::csIsoGrid (iBase *iParent, iIsoWorld *world, int w, int h)
{
  CONSTRUCT_IBASE (iParent);
  csIsoGrid::world = world;
  width = w;
  height = h;
  grid = new iIsoCell* [width*height];
  for(int i=0; i<width*height; i++) 
    grid[i] = NULL;
  mingridx = 0; mingridy = 0;
  box.Set(0,-9999,0, height,+9999,width);
  groundmap = new csIsoGroundMap(this, 1, 1);
}

csIsoGrid::~csIsoGrid ()
{
  for(int i=0; i<width*height; i++) 
    if(grid[i]) grid[i]->DecRef();
  delete[] grid;
  delete groundmap;
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
    cell = new csIsoCell(this);
    SetCell(pos, cell);
  }
  //printf("adding sprite at pos (%g, %g, %g) to cell %x\n", pos.x, 
    //pos.y, pos.z, cell);
  GetCell(pos)->AddSprite(sprite, pos);
}

void csIsoGrid::RemoveSprite(iIsoSprite *sprite)
{
  GetCell(sprite->GetPosition())->RemoveSprite(sprite, sprite->GetPosition());
}

void csIsoGrid::MoveSprite(iIsoSprite *sprite, const csVector3& oldpos,
    const csVector3& newpos)
{
  if(box.In(newpos))
  {
    //printf("Sprite moved to new pos\n");
    GetCell(oldpos)->RemoveSprite(sprite, oldpos);
    AddSprite(sprite, newpos);
    return;
  }
  // sprite not any longer in this grid
  iIsoGrid *newgrid = world->FindGrid(newpos);
  if(!newgrid) 
  {
    // uh oh sprite moved out of *all* grids
    // disallow the movement
    sprite->ForcePosition(oldpos);
    return;
  }
  printf("Grid: Sprite moved to new grid\n");
  GetCell(oldpos)->RemoveSprite(sprite, oldpos);
  sprite->SetGrid(newgrid);
  newgrid->AddSprite(sprite, newpos);
}

void csIsoGrid::Draw(iIsoRenderView *rview)
{
  // only draw all the cells in the main render pass
  if(rview->GetRenderPass() == CSISO_RENDERPASS_MAIN)
  {
    //printf("Grid::Draw() MAIN\n");
    // draw the cells in lines, worldspace z-x = c
    // which, in the grid coords, becomes x-y = c
    // for a constant which is equal to the 'depth'.

    // note: we could/should clip on the depth, but for now
    // each cell is considered infinite in height.
    // start grid x,y
    int startx, starty, scanw, scanh;
    rview->GetPrecalcGrid(startx, starty, scanw, scanh);
    startx -= mingridx;
    starty -= mingridy;
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
    // calculate dyn lighting
    SetAllLight(csColor(0.,0.,0.));
    for(int l=0; l<lights.Length(); l++)
      ((iIsoLight*)(lights[l]))->ShineGrid();
  }
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


static void setspritecolor(iIsoSprite* spr, void *dat)
{
  const csColor *col = (const csColor*)dat;
  spr->SetAllColors(*col);
}

void csIsoGrid::SetAllLight(const csColor& color)
{
  // make copy of color, since (weird usermade) iIsoSprites could violate 
  // the const that is promised in my heading.
  csColor col = color;
  for(int i=0; i<width*height; i++) 
    if(grid[i]) 
    {
      grid[i]->Traverse(setspritecolor, &col);
    }
}

void csIsoGrid::RegisterLight(iIsoLight *light)
{
  if(lights.Find(light)==-1)
    lights.Push(light);
}

void csIsoGrid::UnRegisterLight(iIsoLight *light)
{
  int idx = lights.Find(light);
  if(idx!=-1)
    lights.Delete(idx);
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
}

csIsoGroundMap::~csIsoGroundMap()
{
  delete[] map;
}

bool csIsoGroundMap::HitBeam(const csVector3& src, const csVector3& dest)
{
  /// go through each mapcell and keep track of visibility.
  csVector3 delta = dest-src; // src + delta = dest
  float dheight = delta.y;
  int mingridx = 0, mingridy = 0;
  grid->GetGridOffset(mingridx, mingridy);
  int x0 = QInt(src.z*float(multx)) - mingridx;
  int y0 = QInt(src.x*float(multy)) - mingridy;
  int x1 = QInt(dest.z*float(multx)) - mingridx;
  int y1 = QInt(dest.x*float(multy)) - mingridy;
  int dx = x1-x0;
  int dy = y1-y0;
  float m,b;
  float baseheight;

  /// check x0,y0
  if(src.y <= GetGround(x0, y0))
    return false; // start is below ground
  if (abs(dx) > abs(dy)) {            // slope < 1
    m = (float) dy / (float) dx;      // compute slope
    b = y0 - m*x0;
    dheight = dheight / float(dx);
    baseheight = src.y - dheight * x0;
    dx = (dx < 0) ? -1 : 1;
    x1-=dx; // do not check last pos
    while (x0 != x1) {
      x0 += dx;
      //check (x0, round(m*x0 + b))
      if(dheight*x0+baseheight <= GetGround(x0, QRound(m*x0 + b)))
        return false;
    }
  } else if (dy != 0) {               // slope >= 1
    m = (float) dx / (float) dy;      // compute slope
    b = x0 - m*y0;
    dheight = dheight / float(dy);
    baseheight = src.y - dheight * y0;
    dy = (dy < 0) ? -1 : 1;
    y1-=dy; // do not check last pos
    while (y0 != y1) {
      y0 += dy;
      //check (round(m*y0 + b), y0)
      if(dheight*y0+baseheight <= GetGround(QRound(m*y0 + b), y0))
        return false;
    }
  }
  return true;
}


