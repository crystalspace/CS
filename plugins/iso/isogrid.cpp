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
}

csIsoGrid::~csIsoGrid ()
{
  for(int i=0; i<width*height; i++) 
    if(grid[i]) grid[i]->DecRef();
  delete[] grid;
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
    // uh oh sprite not any longer in *any* grid
    sprite->SetGrid(NULL); // so the sprite will not call me back
    sprite->SetPosition(oldpos);
    sprite->SetGrid(this);
    // disallow the movement
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
  // do nothing
}

void csIsoGrid::SetSpace(int minx, int minz, float miny = -1.0,
  float maxy = +10.0)
{
  mingridx = minz; 
  mingridy = minx;
  box.Set(minx,miny,minz, minx+height,maxy,minz+width);
}

