/*
    Meta Surface Tesselator
    Copyright (C) 1999 by Denis Dmitriev
	Copyright (C) 2001 by Michael H. Voase.

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
#include "metagen.h"
#include "csgeom/tesselat.h"

static int _tess;
static float _iso_level;
static csVector3* _verts;
static MetaField* _fld;
static MetaBone* _mb;

// You need to set the resolution of the renderer manually
// using these three settings. Sorry, its done this way
// cuase prising the cache out significantly slows down
// processing. Mik.

#define RES_X   80
#define RES_Y   40
#define RES_Z   80

static char visited[RES_X][RES_Y][RES_Z];
static float p_c[RES_X+1][RES_Y+1][RES_Z+1];
static char where[RES_X+1][RES_Y+1][RES_Z+1];

float csMetaGen::potential(const csVector3 &p)
{
  int i, t = _fld->num_points;
  float res = -_iso_level,dx,dy,dz;
  PointCharge c;
  for (i=0;i< t;i++)
  {
	c = _fld->points[i];
    dx= c.pos.x-p.x;
    dy= c.pos.y-p.y;
    dz= c.pos.z-p.z;
    res += c.charge/(dx*dx+dy*dy+dz*dz);
  }
// Cap the result. This stops nan spewing out of the tesselator
  if ( res > 100000000 ) res = 100000000;
  if ( res < -100000000 ) res = -10000000;
  return res;
}

float csMetaGen::potential(float px, float pz, int slice)
{
  float res = -_iso_level,dx,dz;
  MetaSlice* s = _mb->slices[slice];
  int i,t = s->num_charges;
  SliceCharge c;
  for ( i = 0; i < t; i++)
  {
	c = s->charges[i];
    dx = c.pos.x - px;
    dz = c.pos.y - pz;
    res += c.charge/((dx*dx)+(dz*dz));
  }
//  printf("Pot: slice %d px %f, pz %f res %f\n",slice,px,pz,res);
// Cap the result. This stops nans spewing out of the tesselator
  if ( res > 100000000 ) res = 100000000;
  if ( res < -100000000 ) res = -10000000;
  return res;
}

void csMetaGen::_2int(const csVector3 &pos,int &x,int &y,int &z )
{
  x = int(istepx * (pos.x - XStart));
  y = int(istepy * (pos.y - YStart));
  z = int(istepz * (pos.z - ZStart));
}

void csMetaGen::_2int2(const csVector2 &pos, int &x, int &z )
{
  x = int(istepx * (pos.x - XStart));
  z = int(istepz * (pos.y - ZStart));
}

void csMetaGen::_2coord(int x,int y,int z,csVector3 &r)
{
  r.x = x * stepx + XStart;
  r.y = y * stepy + YStart;
  r.z = z * stepz + ZStart;
}

static int shift_x[8] = {0,1,1,0,0,1,1,0};
static int shift_y[8] = {1,1,1,1,0,0,0,0};
static int shift_z[8] = {1,1,0,0,1,1,0,0};

static float f_shift_x[8];
static float f_shift_y[8];
static float f_shift_z[8];

void csMetaGen::GenCell( int x, int y, int z, csTesselator::GridCell &c)
{
  csVector3 base;
  _2coord(x,y,z,base);
  int i;
  for(i=0;i<8;i++)
    c.p[i] = base + csVector3(f_shift_x[i],
		f_shift_y[i], f_shift_z[i]);
}

void csMetaGen::FillCell(int _x,int _y,int _z,csTesselator::GridCell &c)
{
  int i;
  for(i=0;i<8;i++)
  {
    int x = _x + shift_x[i];
    int y = _y + shift_y[i];
    int z = _z + shift_z[i];

    if(where[x][y][z]!=frame)
    {
      p_c[x][y][z] = c.val[i] = potential(c.p[i]);
      where[x][y][z]=frame;
    }
    else
      c.val[i]=p_c[x][y][z];
  }
}

void csMetaGen::FillCellSlice(int _x,int _y,int _z,csTesselator::GridCell &c)
{
	int i;
	for(i=0; i<8; i++)
	{
  	  int x = _x + shift_x[i];
	  int y = _y + shift_y[i];
  	  int z = _z + shift_z[i];
  	  if(where[x][y][z]!=frame)
  	  {
    	p_c[x][y][z] = c.val[i] =
		  potential(c.p[i].x, c.p[i].z, y - _mb->start_slice );
    	where[x][y][z]=frame;
  	  }
  	  else
    	c.val[i]=p_c[x][y][z];
	}
}

int csMetaGen::check_cell_assume_inside(const csTesselator::GridCell &c)
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
CS_IMPLEMENT_STATIC_VAR (GetGridCell, csTesselator::GridCell, ())

void csMetaGen::BlobCalc(int dx,int dy,int dz)
{
  static csTesselator::GridCell *_cell = GetGridCell ();
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

  if(_tess < verts->max_vertices - 15)
  {
    csVector3 dv(dx * stepx, dy * stepy, dz * stepz);

    int i;
    for(i=0;i<8;i++)
      _cell->p[i]+=dv;

    FillCell(_x,_y,_z,*_cell);

    int num=csTesselator::Tesselate(*_cell, _verts + _tess);

    if(!num)
      goto skip;

    _tess += num;

    BlobCalc(-1,0,0);
    BlobCalc(+1,0,0);
    BlobCalc(0,-1,0);
    BlobCalc(0,+1,0);
    BlobCalc(0,0,-1);
    BlobCalc(0,0,+1);

skip:
    for(i=0;i<8;i++)
      _cell->p[i]-=dv;
  }

ret_back:
  _x-=dx;
  _y-=dy;
  _z-=dz;
}

void csMetaGen::RingCalc(int dx,int dz)
{
  static csTesselator::GridCell *_cell = GetGridCell ();
  _x+=dx;
  _z+=dz;

  if (_x == -1 || _x == RES_X)
    goto ret_back;
  if (_z == -1 ||  _z == RES_Z)
	goto ret_back;

  // already done this blob
  if(visited[_x][_y][_z] == frame)
    goto ret_back;

  visited[_x][_y][_z] = frame;

  if(_tess < verts->max_vertices - 15)
  {
    csVector3 dv(dx * stepx, 0, dz * stepz);
    int i;
    for(i=0;i<8;i++)
      _cell->p[i]+=dv;

    FillCellSlice(_x,_y,_z,*_cell);
    int num = csTesselator::Tesselate(*_cell, _verts + _tess);

    if(!num)
      goto skip;

    _tess += num;

    RingCalc(-1,0);
    RingCalc(+1,0);
  	RingCalc(0,-1);
    RingCalc(0,+1);

skip:
    for (i=0; i<8; i++)
      _cell->p[i] -= dv;
  }

ret_back:
  _x -= dx;
  _z -= dz;
}

int csMetaGen::CalcBlobSurf(MetaField *field)
{
  static csTesselator::GridCell *_cell = GetGridCell ();
  int i,j;

  printf(";CalcBlobSurf - Generating with %f,%f,%f steps\n",stepx,stepy,stepz);
  printf(";Resolution set to %d x %d x %d from (%f,%f,%f) to (%f,%f,%f)\n",
    RES_X, RES_Y, RES_Z, XStart, YStart, ZStart, XFin, YFin, ZFin);

  for ( i = 0; i < 8; i++ )
  {
	f_shift_x[i] = shift_x[i] * stepx;
	f_shift_y[i] = shift_y[i] * stepy;
	f_shift_z[i] = shift_z[i] * stepz;
  }

  _verts = verts->v;
  _tess = 0;
  _fld = field;
  _iso_level = field->iso_level;
  PointCharge pc;

  for (i=0; i < field->num_points; i++)
  {
    int x,y,z;
	pc = _fld->points[i];
	if (pc.charge <= 0) continue;
	_2int(pc.pos[i],x,y,z);

  	csTesselator::GridCell cell;
  	GenCell(x,y,z,cell);
  	FillCell(x,y,z,cell);

  	while(!check_cell_assume_inside(cell))
  	{
  	  visited[x][y][z] = frame;

  	  for(j = 0; j < 8; j++)
        cell.p[j].x -= stepx;

  	  x--;
  	  FillCell(x,y,z,cell);
    }

    _x=x; _y=y; _z=z;
    *_cell=cell;
    BlobCalc(0,0,0);
  }
  return _tess;
}
// Calculate linear meta surface first draft : one
// direction only - y going up....

int csMetaGen::CalcLinSurf( MetaBone* bone )
{
  static csTesselator::GridCell *_cell = GetGridCell ();
  int i, j, k;

  printf(";CalcLinSurf - Generating with %f,%f,%f steps\n",stepx,stepy,stepz);
  printf(";Resolution set to %d x %d x %d from (%f,%f,%f) to (%f,%f,%f)\n",
    RES_X, RES_Y, RES_Z, XStart, YStart, ZStart, XFin, YFin, ZFin);

  for ( i = 0; i < 8; i++ )
  {
	f_shift_x[i] = shift_x[i] * stepx;
	f_shift_y[i] = shift_y[i] * stepy;
	f_shift_z[i] = shift_z[i] * stepz;
  }

  _verts = verts->v;
  _tess = 0;
  _iso_level = bone->iso_level;
  _mb = bone;
  MetaSlice* m;

  for (i=0; i < _mb->num_slices - 1; i++)
  {
    int x, z, y = _mb->start_slice + i;
	m = _mb->slices[i];
	for ( j = 0; j < m->num_charges; j++)
	{
	  SliceCharge c = m->charges[j];
  	  _2int2(c.pos, x, z);
  	  csTesselator::GridCell cell;
  	  GenCell( x,y,z, cell );
  	  FillCellSlice( x,y,z,cell );
  	  while(!check_cell_assume_inside(cell))
  	  {
    	visited[x][y][z] = frame;

    	for(k = 0; k < 8; k++)
      	  cell.p[k].x -= stepx;

    	x--;
    	FillCellSlice(x,y,z,cell);
  	  }

  	_x=x; _y=y; _z=z;
  	*_cell=cell;
  	RingCalc(0,0);
//	printf("------------------- Next slice %d\n",_tess);
	}
  }
  return _tess;
}

int csMetaGen::GetResX()
{
  return RES_X;
}

int csMetaGen::GetResY()
{
  return RES_Y;
}

int csMetaGen::GetResZ()
{
  return RES_Z;
}

void csMetaGen::ZeroCache()
{
  int i,j,k;
  for ( i = 0; i < RES_X; i++)
	for ( j = 0; j < RES_Y; j++)
	  for ( k = 0; k < RES_Z; k++ )
	  {
		where[i][j][k] = 0;
		p_c[i][j][k] = 0.0;
		visited[i][j][k] = 0;
	  }
  for ( i = RES_X; i < RES_X + 1; i++)
	for ( j = RES_Y; j < RES_Y + 1; j++)
	  for ( k = RES_Z; k < RES_Z + 1; k++ )
	  {
		where[i][j][k] = 0;
		p_c[i][j][k] = 0.0;
	  }
  frame = 0;
}
