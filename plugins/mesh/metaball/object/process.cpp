/*
    Metaballs Demo
    Copyright (C) 1999 by Denis Dmitriev

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

#include "meta.h"

// -12.5 < x < 12.5
// -12.5 < y < 12.5
//   3.5 < z < 16.5

#define X_LEFT  -10.5
#define X_RIGHT 10.5
#define Y_LEFT  -10.5
#define Y_RIGHT 10.5
#define Z_LEFT  -10.5
#define Z_RIGHT 10.5

#define RES_X   26
#define RES_Y   26
#define RES_Z   26

#define EPS 0.0001

const float step_x=(X_RIGHT-X_LEFT)/RES_X;
const float step_y=(Y_RIGHT-Y_LEFT)/RES_Y;
const float step_z=(Z_RIGHT-Z_LEFT)/RES_Z;

const float inv_step_x=1/step_x;
const float inv_step_y=1/step_y;
const float inv_step_z=1/step_z;

static char visited[RES_X][RES_Y][RES_Z];
static float p_c[RES_X+1][RES_Y+1][RES_Z+1];
static char where[RES_X+1][RES_Y+1][RES_Z+1];

float csMetaBall::potential(const csVector3 &p)
{ 
  int i;
  float res = -mp.iso_level,dx,dy,dz;

  for (i=0;i<num_meta_balls;i++)
  {
    dx=meta_balls[i].center.x-p.x;
    dy=meta_balls[i].center.y-p.y;
    dz=meta_balls[i].center.z-p.z;
    res += mp.charge/(dx*dx+dy*dy+dz*dz);
  }

  return res;
}

csVector3 VertexInterp(const csVector3 &p1,const csVector3 &p2,float valp1,float valp2)
{
  float mu;
  csVector3 p;

  if(ABS (valp1)<EPS)
    return(p1);
  if(ABS (valp2)<EPS)
    return(p2);
  if(ABS (valp1-valp2)<EPS)
    return(p1);

  mu=valp1/(valp2-valp1);
  p.x=p1.x-mu*(p2.x-p1.x);
  p.y=p1.y-mu*(p2.y-p1.y);
  p.z=p1.z-mu*(p2.z-p1.z);

  return p;
}

int csMetaBall::Tesselate(const GridCell &grid,csVector3 *verts)
{
  int i, vrtx;
  int cubeindex;
  csVector3 vertlist[12];

# include "tables.h"

  cubeindex=0;

  if(grid.val[0]<0) cubeindex|=1;
  if(grid.val[1]<0) cubeindex|=2;
  if(grid.val[2]<0) cubeindex|=4;
  if(grid.val[3]<0) cubeindex|=8;
  if(grid.val[4]<0) cubeindex|=16;
  if(grid.val[5]<0) cubeindex|=32;
  if(grid.val[6]<0) cubeindex|=64;
  if(grid.val[7]<0) cubeindex|=128;

  if(!edgeTable[cubeindex])
    return 0;

  if(edgeTable[cubeindex]&1)
    vertlist[0]=VertexInterp(grid.p[0],grid.p[1],grid.val[0],grid.val[1]);
  if(edgeTable[cubeindex]&2)
    vertlist[1]=VertexInterp(grid.p[1],grid.p[2],grid.val[1],grid.val[2]);
  if(edgeTable[cubeindex]&4)
    vertlist[2]=VertexInterp(grid.p[2],grid.p[3],grid.val[2],grid.val[3]);
  if(edgeTable[cubeindex]&8)
    vertlist[3]=VertexInterp(grid.p[3],grid.p[0],grid.val[3],grid.val[0]);
  if(edgeTable[cubeindex]&16)
    vertlist[4]=VertexInterp(grid.p[4],grid.p[5],grid.val[4],grid.val[5]);
  if(edgeTable[cubeindex]&32)
    vertlist[5]=VertexInterp(grid.p[5],grid.p[6],grid.val[5],grid.val[6]);
  if(edgeTable[cubeindex]&64)
    vertlist[6]=VertexInterp(grid.p[6],grid.p[7],grid.val[6],grid.val[7]);
  if(edgeTable[cubeindex]&128)
    vertlist[7]=VertexInterp(grid.p[7],grid.p[4],grid.val[7],grid.val[4]);
  if(edgeTable[cubeindex]&256)
    vertlist[8]=VertexInterp(grid.p[0],grid.p[4],grid.val[0],grid.val[4]);
  if(edgeTable[cubeindex]&512)
    vertlist[9]=VertexInterp(grid.p[1],grid.p[5],grid.val[1],grid.val[5]);
  if(edgeTable[cubeindex]&1024)
    vertlist[10]=VertexInterp(grid.p[2],grid.p[6],grid.val[2],grid.val[6]);
  if(edgeTable[cubeindex]&2048)
    vertlist[11]=VertexInterp(grid.p[3],grid.p[7],grid.val[3],grid.val[7]);

  vrtx = 0;

  for(i=0;triTable[cubeindex][i]!=-1;i+=3)
  {
    verts[vrtx++]=vertlist[triTable[cubeindex][i]];
    verts[vrtx++]=vertlist[triTable[cubeindex][i+1]];
    verts[vrtx++]=vertlist[triTable[cubeindex][i+2]];
  }
  return vrtx;
}

void _2int(const csVector3 &pos,int &x,int &y,int &z)
{
  x=int(inv_step_x*(pos.x-X_LEFT));
  y=int(inv_step_y*(pos.y-Y_LEFT));
  z=int(inv_step_z*(pos.z-Z_LEFT));
}

void _2coord(int x,int y,int z,csVector3 &r)
{
  r.x=x*step_x+X_LEFT;
  r.y=y*step_y+Y_LEFT;
  r.z=z*step_z+Z_LEFT;
}

static int shift_x[8]={0,1,1,0,0,1,1,0};
static int shift_y[8]={1,1,1,1,0,0,0,0};
static int shift_z[8]={1,1,0,0,1,1,0,0};

static float f_shift_x[8]={0,step_x,step_x,0,0,step_x,step_x,0};
static float f_shift_y[8]={step_y,step_y,step_y,step_y,0,0,0,0};
static float f_shift_z[8]={step_z,step_z,0,0,step_z,step_z,0,0};

void GenCell(int x,int y,int z,GridCell &c)
{
  csVector3 base;
  _2coord(x,y,z,base);

  for(int i=0;i<8;i++)
    c.p[i]=base+csVector3(f_shift_x[i],f_shift_y[i],f_shift_z[i]);
}

void csMetaBall::FillCell(int _x,int _y,int _z,GridCell &c)
{
  for(int i=0;i<8;i++)
  {
    int x=_x+shift_x[i];
    int y=_y+shift_y[i];
    int z=_z+shift_z[i];

    if(where[x][y][z]!=frame)
    {
      p_c[x][y][z]=c.val[i]=potential(c.p[i]);
      where[x][y][z]=frame;
    }
    else
      c.val[i]=p_c[x][y][z];
  }
}

int csMetaBall::check_cell_assume_inside(const GridCell &c)
{
  int i,flag;

  for(flag=i=0;i<8;i++)
    if(c.val[i]>0)
      flag++;

  if(flag==8)
    return 0;
  else
    return -1;
}

static int _x,_y,_z;
static GridCell _cell;

void csMetaBall::CalculateBlob(int dx,int dy,int dz)
{
  _x+=dx;
  _y+=dy;
  _z+=dz;

  if (_x == -1 || _x == RES_X)
    goto ret_back;
  if (_y == -1 || _y == RES_Y)
    goto ret_back;
  if (_z == -1 || _z == RES_Z)
    goto ret_back;

  // already done this blob
  if(visited[_x][_y][_z] == frame)
    goto ret_back;

  visited[_x][_y][_z] = frame;

  if(vertices_tesselated < max_vertices - 15)
  {
    csVector3 dv(dx * step_x, dy * step_y, dz * step_z);

    int i;
    for(i=0;i<8;i++)
      _cell.p[i]+=dv;
    
    FillCell(_x,_y,_z,_cell);

    int num=Tesselate(_cell, mesh.vertices[0] + vertices_tesselated);

    if(!num)
      goto skip;

    vertices_tesselated+=num;

    CalculateBlob(-1,0,0);
    CalculateBlob(+1,0,0);
    CalculateBlob(0,-1,0);
    CalculateBlob(0,+1,0);
    CalculateBlob(0,0,-1);
    CalculateBlob(0,0,+1);

skip:
    for(i=0;i<8;i++)
      _cell.p[i]-=dv;
  }

ret_back:
  _x-=dx;
  _y-=dy;
  _z-=dz;
}

void csMetaBall::CalculateMetaBalls(void)
{
  frame++;
  vertices_tesselated=0;

  int i,j;

  for(i=0;i<num_meta_balls;i++)
  {
    int x,y,z;

    _2int(meta_balls[i].center,x,y,z);

    GridCell cell;
    GenCell(x,y,z,cell);
    FillCell(x,y,z,cell);

    while(!check_cell_assume_inside(cell))
    {
      visited[x][y][z]=frame;

      for(j=0;j<8;j++)
        cell.p[j].x-=step_x;

      x--;

      FillCell(x,y,z,cell);
    }

    _x=x; _y=y; _z=z;
    _cell=cell;

    CalculateBlob(0,0,0);
  }
}
