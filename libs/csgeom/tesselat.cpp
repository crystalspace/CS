/*
    A General Purpose Tesselator
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
#include "csgeom/vector3.h"
#include "csgeom/tesselat.h"

#define EPS 0.0001

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

int Tesselate(const GridCell &grid, csVector3* vrt)
{
  int i;
  int cubeindex;
  csVector3 vertlist[12];

#include "csgeom/ttables.h"

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

  for(i=0;triTable[cubeindex][i]!=-1;)
  {
    vrt[i]=vertlist[triTable[cubeindex][i++]];
    vrt[i]=vertlist[triTable[cubeindex][i++]];
    vrt[i]=vertlist[triTable[cubeindex][i++]];
  }
  return i;
}
