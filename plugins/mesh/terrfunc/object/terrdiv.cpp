/*
    Copyright (C) 2002 by W.C.A. Wijngaards

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
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "terrdiv.h"

//-------------- csTerrainQuadDiv ------------------------------
csTerrainQuadDiv::csTerrainQuadDiv(int depth)
{
  parent = NULL;
  int i;
  for(i=0; i<4; i++)
  {
    children[i] = NULL;
    neighbors[i] = NULL;
  }
  subdivided = 0;
  dmax = 0;
  min_height = 0;
  max_height = 0;

  /// subcreate
  if(depth > 0)
  {
    for(i=0; i<4; i++)
    {
      children[i] = new csTerrainQuadDiv(depth-1);
      children[i]->parent = this;
    }

    /// connect direct neighbors:
    ///
    ///  |---|---|
    ///  | 0 | 1 |  direct neighbors are:   0  <->  1
    ///  |   |   |                          ^       ^
    ///  |---|---|                          |       |
    ///  |   |   |                          v       v
    ///  | 2 | 3 |                          2  <->  3
    ///  |---|---|
    ///
    children[CS_QUAD_TOPLEFT]->neighbors[CS_QUAD_RIGHT] = 
      children[CS_QUAD_TOPRIGHT];
    children[CS_QUAD_BOTLEFT]->neighbors[CS_QUAD_RIGHT] = 
      children[CS_QUAD_BOTRIGHT];
    children[CS_QUAD_TOPRIGHT]->neighbors[CS_QUAD_LEFT] = 
      children[CS_QUAD_TOPLEFT];
    children[CS_QUAD_BOTRIGHT]->neighbors[CS_QUAD_LEFT] = 
      children[CS_QUAD_BOTLEFT];

    children[CS_QUAD_TOPLEFT]->neighbors[CS_QUAD_BOT] = 
      children[CS_QUAD_BOTLEFT];
    children[CS_QUAD_TOPRIGHT]->neighbors[CS_QUAD_BOT] = 
      children[CS_QUAD_BOTRIGHT];
    children[CS_QUAD_BOTLEFT]->neighbors[CS_QUAD_TOP] = 
      children[CS_QUAD_TOPLEFT];
    children[CS_QUAD_BOTRIGHT]->neighbors[CS_QUAD_TOP] = 
      children[CS_QUAD_TOPRIGHT];
  }
}

csTerrainQuadDiv::~csTerrainQuadDiv()
{
  for(int i=0; i<4; i++)
    delete children[i];
}

void csTerrainQuadDiv::ComputeDmax(iTerrainHeightFunction* height_func,
    float minx, float miny, float maxx, float maxy)
{
  int i;
  float midx = (minx+maxx)*0.5f;
  float midy = (miny+maxy)*0.5f;
  float h;
  /// first establish min/max height
  if(IsLeaf())
  {
    dmax = 0;
    min_height = height_func->GetHeight (midx, midy);
    max_height = min_height;
    h = height_func->GetHeight (minx, miny);
    if(h>max_height) max_height=h; if(h<min_height) min_height=h;
    h = height_func->GetHeight (minx, maxy);
    if(h>max_height) max_height=h; if(h<min_height) min_height=h;
    h = height_func->GetHeight (maxx, miny);
    if(h>max_height) max_height=h; if(h<min_height) min_height=h;
    h = height_func->GetHeight (maxx, maxy);
    if(h>max_height) max_height=h; if(h<min_height) min_height=h;
  }
  else
  {
    children[CS_QUAD_TOPLEFT]->ComputeDmax (height_func, 
      minx, miny, midx, midy);
    children[CS_QUAD_TOPRIGHT]->ComputeDmax(height_func, 
      midx, miny, maxx, midy);
    children[CS_QUAD_BOTLEFT]->ComputeDmax(height_func, 
      minx, midy, midx, maxy);
    children[CS_QUAD_BOTRIGHT]->ComputeDmax(height_func, 
      midx, midy, maxx, maxy);
    min_height = children[0]->GetMinHeight();
    max_height = children[0]->GetMaxHeight();
    for(i=1; i<4; i++)
    {
      h = children[i]->GetMinHeight();
      if(h<min_height) min_height=h;
      h = children[i]->GetMaxHeight();
      if(h>max_height) max_height=h;
    }
    /// compute dmax
    /// test middle vertice
    h = height_func->GetHeight(midx, midy);
    float height_tr = height_func->GetHeight(maxx, miny);
    float height_bl = height_func->GetHeight(minx, maxy);
    float interpol_h = (height_tr+height_bl)*0.5;
    dmax = ABS(interpol_h - h);
    /// make sure dmax >= MAX(children dmax);
    /// this also takes children heightchanges into account
    for(i=0; i<4; i++)
    {
      if(children[i]->GetDmax() > dmax)
        dmax = children[i]->GetDmax();
    }
  }
}

void csTerrainQuadDiv::ComputeLOD(int framenum, const csVector3& campos,
  float minx, float miny, float maxx, float maxy)
{
  float midx = (minx+maxx)*0.5f;
  float midy = (miny+maxy)*0.5f;
  if(!IsLeaf())
  {
    /// subdivide this quad
    subdivided = framenum;
    children[CS_QUAD_TOPLEFT]->ComputeLOD (framenum, campos,
      minx, miny, midx, midy);
    children[CS_QUAD_TOPRIGHT]->ComputeLOD (framenum, campos,
      midx, miny, maxx, midy);
    children[CS_QUAD_BOTLEFT]->ComputeLOD (framenum, campos,
      minx, midy, midx, maxy);
    children[CS_QUAD_BOTRIGHT]->ComputeLOD (framenum, campos,
      midx, midy, maxx, maxy);
    return;
  }
  // do nothing (yet)
}

void csTerrainQuadDiv::Triangulate(void (*cb)(void *, const csVector3&, 
  const csVector3&, const csVector3&), void *userdata, int framenum,
  float minx, float miny, float maxx, float maxy)
{
  float midx = (minx+maxx)*0.5f;
  float midy = (miny+maxy)*0.5f;
  /// subdivide?
  if(subdivided == framenum)
  {
    CS_ASSERT(!IsLeaf());
    children[CS_QUAD_TOPLEFT]->Triangulate (cb, userdata, framenum,
      minx, miny, midx, midy);
    children[CS_QUAD_TOPRIGHT]->Triangulate (cb, userdata, framenum,
      midx, miny, maxx, midy);
    children[CS_QUAD_BOTLEFT]->Triangulate (cb, userdata, framenum,
      minx, midy, midx, maxy);
    children[CS_QUAD_BOTRIGHT]->Triangulate (cb, userdata, framenum,
      midx, midy, maxx, maxy);
    return;
  }
  /// create a polygon at medio height
  float h = (min_height + max_height)*0.5;
  cb(userdata, 
    csVector3(minx,h,miny), csVector3(minx,h,maxy), csVector3(maxx,h,miny));
  cb(userdata, 
    csVector3(maxx,h,miny), csVector3(minx,h,maxy), csVector3(maxx,h,maxy));
}


