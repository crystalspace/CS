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
#include "genmaze.h"

csGenMaze::csGenMaze(int w, int h)
{
  width = w;
  height = h;
  maze = new csGenMazeNode [width*height];
  straightness = 0.5;
  cyclicalness = 0.1;
  InitMaze();
}

csGenMaze::~csGenMaze()
{
  delete[] maze;
}

void csGenMaze::InitMaze()
{
  int y, x, i;
  for(y=0; y<height; y++)
    for(x=0; x<width; x++)
    {
      GetNode(x,y).visited = false;
      for(i=0; i<4; i++)
        GetNode(x,y).opening[i] = false;
    }
}

void csGenMaze::MakeAccess(int x, int y)
{
  if(x==0) GetNode(x,y).opening[3] = true;
  if(y==0) GetNode(x,y).opening[0] = true;
  if(x==width-1) GetNode(x,y).opening[1] = true;
  if(y==height-1) GetNode(x,y).opening[2] = true;
}

void csGenMaze::GenerateMaze(int x, int y)
{
  srand(time(0));
  int direction = int(float(rand()+1)/RAND_MAX*4.0);
  VisitNode(x, y, direction);
}

void csGenMaze::VisitNode(int x, int y, int direction)
{
  /// generate an order in which to test the neighbours.
  int order[4];
  GenOrder(order, direction);
  GetNode(x,y).visited = true;
  /// take each direction and go there...
  int i;
  for(i=0; i<4; i++)
  {
    int nx, ny;
    GetNeighbor(order[i], x, y, nx, ny);
    /// don't go outside the map
    if( nx < 0 || nx >=width || ny < 0 || ny >=height )
      continue;
    /// been there?
    if(GetNode(nx,ny).visited)
    {
      // avoid the cycle?
      if( (1.0+rand())/RAND_MAX > cyclicalness )
        continue;
    }
    /// make opening
    MakeOpening(x, y, nx, ny);
    /// visit that neightbor
    if(GetNode(nx, ny).visited) continue;
    VisitNode(nx, ny, order[i]);
  }
}

void csGenMaze::GenOrder(int order[4], int direction)
{
  CS_ASSERT( direction >= 0 && direction < 4);
  bool done[4];
  int i, k;
  for(i=0; i<4; i++) done[i] = false;
  i=0;
  if( float(rand()+1)/RAND_MAX < straightness)
  {
    i=1;
    order[0] = direction;
    done[direction] = true;
  }
  for(;i<4; i++)
  {
    /// skip from 0 ... 4-i-1
    int skip = int(float(4-i)*float(rand()+1)/RAND_MAX);
    if(skip > 4-i-1) skip = 4-i-1;
    CS_ASSERT( skip >= 0 && skip <= 4-i-1 );
    /// find it
    int numskipped = 0;
    int found = 0;
    for(k=0; k<4; k++)
    {
      if(!done[k])
      {
        numskipped++;
        if(numskipped == skip+1){ found = k; break; }
      }
    }
    /// store it;
    order[i] = found;
    done[found] = true;
  }

  CS_ASSERT(done[0] && done[1] && done[2] && done[3]);
}

void csGenMaze::GetNeighbor(int dir, int x, int y, int& nx, int& ny)
{
  nx = x; ny = y;
  switch(dir) {
    case 0: ny--; break;
    case 1: nx++; break;
    case 2: ny++; break;
    case 3: nx--; break;
  }
}

void csGenMaze::MakeOpening(int x1, int y1, int x2, int y2)
{
  if(x1==x2-1)
  {
    GetNode(x1,y1).opening[1] = true;
    GetNode(x2,y2).opening[3] = true;
    return;
  }
  if(x1==x2+1)
  {
    GetNode(x1,y1).opening[3] = true;
    GetNode(x2,y2).opening[1] = true;
    return;
  }
  if(y1==y2-1)
  {
    GetNode(x1,y1).opening[2] = true;
    GetNode(x2,y2).opening[0] = true;
    return;
  }
  if(y1==y2+1)
  {
    GetNode(x1,y1).opening[0] = true;
    GetNode(x2,y2).opening[2] = true;
    return;
  }
}

bool csGenMaze::Opening(int x1, int y1, int x2, int y2)
{
  if(x1==x2-1)
  {
    CS_ASSERT( GetNode(x1,y1).opening[1] == GetNode(x2,y2).opening[3]);
    return GetNode(x1,y1).opening[1];
  }
  if(x1==x2+1)
  {
    CS_ASSERT( GetNode(x1,y1).opening[3] == GetNode(x2,y2).opening[1]);
    return GetNode(x1,y1).opening[3];
  }
  if(y1==y2-1)
  {
    CS_ASSERT( GetNode(x1,y1).opening[2] == GetNode(x2,y2).opening[0]);
    return GetNode(x1,y1).opening[2];
  }
  if(y1==y2+1)
  {
    CS_ASSERT( GetNode(x1,y1).opening[0] == GetNode(x2,y2).opening[2]);
    return GetNode(x1,y1).opening[0];
  }
  /// unreachable
  CS_ASSERT(false);
  return false;
}
