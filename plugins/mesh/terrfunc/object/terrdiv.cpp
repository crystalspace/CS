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
#include "qsqrt.h"

//-------------- csTerrainQuadDiv ------------------------------
csTerrainQuadDiv::csTerrainQuadDiv(int depth)
{
  parent = NULL;
  parentplace = 0;
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
      children[i]->parentplace = i;
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


void csTerrainQuadDiv::SetNeighbor(int dir, csTerrainQuadDiv *neigh)
{
  neighbors[dir] = neigh;
  if(!IsLeaf())
  {
    /// call 2 children only
    int c1=0, c2=0;
    switch(dir)
    {
      case CS_QUAD_TOP: c1=CS_QUAD_TOPLEFT; c2=CS_QUAD_TOPRIGHT; break;
      case CS_QUAD_RIGHT: c1=CS_QUAD_TOPRIGHT; c2=CS_QUAD_BOTRIGHT; break;
      case CS_QUAD_BOT: c1=CS_QUAD_BOTLEFT; c2=CS_QUAD_BOTRIGHT; break;
      case CS_QUAD_LEFT: c1=CS_QUAD_TOPLEFT; c2=CS_QUAD_BOTLEFT; break;
    }
    children[c1]->SetNeighbor(dir, neigh);
    children[c2]->SetNeighbor(dir, neigh);
  }
}

csTerrainQuadDiv* csTerrainQuadDiv::GetNeighbor(int dir)
{
  if(neighbors[dir]) return neighbors[dir];
  /// find & cache it;
  if(!parent) return NULL;

  /// perhaps neighbor is sibling?
  int pchild = -1;
  if((dir==CS_QUAD_TOP) && (parentplace==CS_QUAD_BOTLEFT))
    pchild = CS_QUAD_TOPLEFT;
  if((dir==CS_QUAD_TOP) && (parentplace==CS_QUAD_BOTRIGHT))
    pchild = CS_QUAD_TOPRIGHT;
  if((dir==CS_QUAD_BOT) && (parentplace==CS_QUAD_TOPLEFT))
    pchild = CS_QUAD_BOTLEFT;
  if((dir==CS_QUAD_BOT) && (parentplace==CS_QUAD_TOPRIGHT))
    pchild = CS_QUAD_BOTRIGHT;
  if((dir==CS_QUAD_LEFT) && (parentplace==CS_QUAD_TOPRIGHT))
    pchild = CS_QUAD_TOPLEFT;
  if((dir==CS_QUAD_LEFT) && (parentplace==CS_QUAD_BOTRIGHT))
    pchild = CS_QUAD_BOTLEFT;
  if((dir==CS_QUAD_RIGHT) && (parentplace==CS_QUAD_TOPLEFT))
    pchild = CS_QUAD_TOPRIGHT;
  if((dir==CS_QUAD_RIGHT) && (parentplace==CS_QUAD_BOTLEFT))
    pchild = CS_QUAD_BOTRIGHT;
  if(pchild != -1)
  {
    /// get it and cache
    neighbors[dir] = parent->children[pchild];
    return neighbors[dir];
  }

  /// it is a child of a neighbor of parent, get parent neighbor
  csTerrainQuadDiv *parneigh = parent->GetNeighbor(dir);
  if(!parneigh) return NULL;

  if((dir==CS_QUAD_LEFT) && (parentplace==CS_QUAD_BOTLEFT))
    pchild = CS_QUAD_BOTRIGHT;
  if((dir==CS_QUAD_LEFT) && (parentplace==CS_QUAD_TOPLEFT))
    pchild = CS_QUAD_TOPRIGHT;
  if((dir==CS_QUAD_RIGHT) && (parentplace==CS_QUAD_BOTRIGHT))
    pchild = CS_QUAD_BOTLEFT;
  if((dir==CS_QUAD_RIGHT) && (parentplace==CS_QUAD_TOPRIGHT))
    pchild = CS_QUAD_TOPLEFT;
  if((dir==CS_QUAD_TOP) && (parentplace==CS_QUAD_TOPRIGHT))
    pchild = CS_QUAD_BOTRIGHT;
  if((dir==CS_QUAD_TOP) && (parentplace==CS_QUAD_TOPLEFT))
    pchild = CS_QUAD_BOTLEFT;
  if((dir==CS_QUAD_BOT) && (parentplace==CS_QUAD_BOTRIGHT))
    pchild = CS_QUAD_TOPRIGHT;
  if((dir==CS_QUAD_BOT) && (parentplace==CS_QUAD_BOTLEFT))
    pchild = CS_QUAD_TOPLEFT;

  CS_ASSERT(pchild != -1);
  /// get it and cache
  neighbors[dir] = parneigh->children[pchild];
  return neighbors[dir];
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
    min_height = height_func->GetHeight (midx, midy);
    max_height = min_height;
    h = height_func->GetHeight (minx, miny);
    if(h>max_height) max_height=h; if(h<min_height) min_height=h;
    h = height_func->GetHeight (minx, maxy);
    float h_bl = h; /// keep 
    if(h>max_height) max_height=h; if(h<min_height) min_height=h;
    h = height_func->GetHeight (maxx, miny);
    float h_tr = h; /// keep 
    if(h>max_height) max_height=h; if(h<min_height) min_height=h;
    h = height_func->GetHeight (maxx, maxy);
    if(h>max_height) max_height=h; if(h<min_height) min_height=h;

    /// compute dmax , also for leaves to get more accurate dmax up
    float h_pol = (h_tr+h_bl)*0.5; // interpolated
    h = height_func->GetHeight (midx, midy);
    dmax = ABS(h_pol - h);
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

  /// compute visible error
  /// which is heightchange / distance * cameraslant
  float e = dmax;
  /// if camera in quad: dist = 0;
  float dist = 1.;
  if( (campos.x < minx) || (campos.x > maxx) || (campos.z < miny)
    || (campos.z > maxy))
    dist = 1. / (1.+ (campos.x-midx)*(campos.x-midx) + 
      (campos.z-midy)*(campos.z-midy) );
  e *= dist;
  /// if camera in quad, camslant = 1 (full length is visible)
  float camslant = 1.0;
  if(campos.y > max_height) camslant = 1.0 / (1.+ campos.y - max_height);
  if(campos.y < min_height) camslant = 1.0 / (1.+ min_height - campos.y);
  e *= camslant;

  /// can this quad be displayed?
  bool OK = true;
  float maxerror = 1./500.;
  if(e > maxerror) OK = false;
  
  if(OK) return; // no need to divide

  /// debug prints
  if(1)
  {
    float sizex = maxx-minx;
    float sizey = maxy-miny;
    printf("LOD %gx%g dmax is %f, error is %f.\n", 
      sizex, sizey, dmax, e);
  }
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
  /// debug, create a polygon at medio height
  /// float h = (min_height + max_height)*0.5;
  float h=0.0;
  cb(userdata, 
    csVector3(minx,h,miny), csVector3(minx,h,maxy), csVector3(maxx,h,miny));
  cb(userdata, 
   csVector3(maxx,h,miny), csVector3(minx,h,maxy), csVector3(maxx,h,maxy));
}


