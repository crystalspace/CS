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
  visquad = NULL;

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


void csTerrainQuadDiv::RemoveNeighbor(int dir)
{
  neighbors[dir] = NULL;
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
    children[c1]->RemoveNeighbor(dir);
    children[c2]->RemoveNeighbor(dir);
  }
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
    children[c1]->RemoveNeighbor(dir);
    children[c2]->RemoveNeighbor(dir);
  }
}

csTerrainQuadDiv* csTerrainQuadDiv::GetNeighbor(int dir)
{
  //if(!parent && neighbors[dir]) printf("qd %x dir %d %x\n", 
    //(int)this, dir, (int)neighbors[dir]);
  if(neighbors[dir]) return neighbors[dir];
  /// find & cache it;
  if(!parent) return NULL;

  /// perhaps neighbor is sibling?
  /// Note that this is prestored and should (and is) never actually used.
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

csTerrainQuad *csTerrainQuadDiv::GetVisQuad()
{
  if(visquad) return visquad;
  if(!parent) return NULL;
  csTerrainQuad *pquad = parent->GetVisQuad();
  if(!pquad) return NULL;
  visquad = pquad->GetChild(parentplace);
  return visquad;
}

void csTerrainQuadDiv::ComputeDmax(iTerrainHeightFunction* height_func,
    float minx, float miny, float maxx, float maxy)
{
  int i;
  float midx = (minx+maxx)*0.5f;
  float midy = (miny+maxy)*0.5f;
  float h;
  float cornerh[4];
  cornerh[0] = height_func->GetHeight(minx, miny);
  cornerh[1] = height_func->GetHeight(minx, maxy);
  cornerh[2] = height_func->GetHeight(maxx, maxy);
  cornerh[3] = height_func->GetHeight(maxx, miny);
  /// first establish min/max height
  if(IsLeaf())
  {
    min_height = height_func->GetHeight (midx, midy);
    max_height = min_height;
    for(i=0; i<4; i++)
    {
      h = cornerh[i];
      if(h>max_height) max_height=h; 
      if(h<min_height) min_height=h;
    }

    /// compute dmax 
    float h_pol = (cornerh[1] + cornerh[3])*0.5; // interpolated
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
    float interpol_h = (cornerh[1]+cornerh[3])*0.5;
    dmax = ABS(interpol_h - h);
    /// make sure dmax >= MAX(children dmax);
    for(i=0; i<4; i++)
    {
      if(children[i]->GetDmax() > dmax)
        dmax = children[i]->GetDmax();
    }
  }
  CS_ASSERT(max_height >= min_height);

  /// compute dmax more precise - based on edge lines
  h = height_func->GetHeight(midx, miny);
  h -= (cornerh[3] + cornerh[0])*0.5;
  h = ABS(h); if(h > dmax) dmax = h;
  h = height_func->GetHeight(midx, maxy);
  h -= (cornerh[1] + cornerh[2])*0.5;
  h = ABS(h); if(h > dmax) dmax = h;
  h = height_func->GetHeight(minx, midy);
  h -= (cornerh[0] + cornerh[1])*0.5;
  h = ABS(h); if(h > dmax) dmax = h;
  h = height_func->GetHeight(maxx, midy);
  h -= (cornerh[2] + cornerh[3])*0.5;
  h = ABS(h); if(h > dmax) dmax = h;

  //printf("size %g dmax is %g.\n", maxx-minx, dmax);
}

void csTerrainQuadDiv::ComputeLOD(int framenum, const csVector3& campos,
  float minx, float miny, float maxx, float maxy)
{
  if(GetVisQuad()) if(!GetVisQuad()->IsVisible()) return;
  float midx = (minx+maxx)*0.5f;
  float midy = (miny+maxy)*0.5f;

  /// compute visible error  (lower = more quality)
  float maxerror = 0.001f;
  float distfactor = 0.5f;
  /// which is heightchange / distance * cameraslant
  float e = dmax; 

  /// if camera in quad: dist = 1; (you can see full length)
  float distx = 1.f;
  float disty = 1.f;
  float disth = 1.f;
  if(campos.x < minx) distx = minx-campos.x;
  if(campos.x > maxx) distx = campos.x-maxx;
  if(campos.z < miny) disty = miny-campos.z;
  if(campos.z > maxy) disty = campos.z-maxy;
  if(campos.y < min_height) disth = min_height-campos.z;
  if(campos.y > max_height) disth = campos.z-max_height;
  distx*=distfactor;
  disty*=distfactor;
  disth*=distfactor;
  /// distance mult factor is thus:
  e /= 1.0f + distx*distx + disty*disty + disth*disth;

  /// if camera in quad, camslant = 1 (full length is visible)
  /**float camslant = 1.0;
  if(campos.y > max_height) camslant = 1.0 / 
    (1.+ distfactor*(campos.y - max_height));
  if(campos.y < min_height) camslant = 1.0 / 
    (1.+ distfactor*(min_height - campos.y));
  e *= camslant;**/

  /// can this quad be displayed?
  bool OK = true;
  if(e > maxerror) OK = false;

  if(OK) return; // no need to divide

  /// debug prints
  if(0)
  {
    float camslant = 1.;
    float dist = 1.0f /(1.0f + distx*distx + disty*disty + disth*disth);
    printf("LOD %g dmax is %g, dist %g, cam %g, error is %f.\n", 
      maxx-minx, dmax, dist, camslant, e);
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

int csTerrainQuadDiv::EstimateTris(int framenum)
{
  if(IsLeaf()) return 2;
  if(subdivided == framenum)
  {
    return children[0]->EstimateTris(framenum) + 
      children[1]->EstimateTris(framenum)
      + children[2]->EstimateTris(framenum) +
      children[3]->EstimateTris(framenum);
  }
  if(!HaveMoreDetailedNeighbor(framenum)) return 2;
  return 4;
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

  /// can it suffice to just create two polys?
  /// this is possible if this node is the same or more detailed
  /// than its neighbors. (the neighbors will accomodate for this node)
  float h=0.0;
  if(!HaveMoreDetailedNeighbor(framenum))
  {
    /// debug, create a polygon at medio height
    /// float h = (min_height + max_height)*0.5;
    cb(userdata, 
      csVector3(minx,h,miny), csVector3(minx,h,maxy), csVector3(maxx,h,miny));
    cb(userdata, 
     csVector3(maxx,h,miny), csVector3(minx,h,maxy), csVector3(maxx,h,maxy));
    return;
  }

  /// accomodate the more detailed neighbors
  /// by drawing starshaped triangles fitting all vertices at edges
  csVector3 center(midx,h,midy);
  csVector3 nextv;
  csVector3 oldv;

  oldv.Set(minx, h, miny);
  csTerrainQuadDiv *neigh = GetNeighbor(CS_QUAD_TOP);
  nextv.Set(maxx,h,miny);
  if(neigh) neigh->TriEdge(CS_QUAD_TOP, cb, userdata, framenum, center, oldv, 
    nextv, minx, miny, maxx, maxy);
  else { cb(userdata, center, nextv, oldv); oldv = nextv; }

  nextv.Set(maxx,h,maxy);
  neigh = GetNeighbor(CS_QUAD_RIGHT);
  if(neigh) neigh->TriEdge(CS_QUAD_RIGHT, cb, userdata, framenum, center, oldv,
    nextv, minx, miny, maxx, maxy);
  else { cb(userdata, center, nextv, oldv); oldv = nextv; }

  nextv.Set(minx,h,maxy);
  neigh = GetNeighbor(CS_QUAD_BOT);
  if(neigh) neigh->TriEdge(CS_QUAD_BOT, cb, userdata, framenum, center, oldv,
    nextv, minx, miny, maxx, maxy);
  else { cb(userdata, center, nextv, oldv); oldv = nextv; }

  nextv.Set(minx,h,miny);
  neigh = GetNeighbor(CS_QUAD_LEFT);
  if(neigh) neigh->TriEdge(CS_QUAD_LEFT, cb, userdata, framenum, center, oldv,
    nextv, minx, miny, maxx, maxy);
  else { cb(userdata, center, nextv, oldv); oldv = nextv; }

}


bool csTerrainQuadDiv::HaveMoreDetailedNeighbor(int framenum)
{
  csTerrainQuadDiv *neigh = 0;
  int dir;

  CS_ASSERT(subdivided != framenum);

  /// look in each direction
  for(dir=0; dir<4; dir++)
  {
    neigh = GetNeighbor(dir);
    if(!neigh) continue; // no neighbor - its not more detailed
    /// is neigh more detailed?
    if(neigh->subdivided == framenum)
    {
      /// more detailed than me
      return true;
    }
  }
  return false;
}


void csTerrainQuadDiv::TriEdge(int dir, void (*cb)(void *, 
  const csVector3&, const csVector3&, const csVector3&), 
  void *userdata, int framenum,
  const csVector3& center, csVector3& oldv, const csVector3& nextv,
  float minx, float miny, float maxx, float maxy)
{
  /// NOTE dir is from the caller (points from 'center' towards me)
  if(subdivided != framenum) 
  {
    /// draw singly
    cb(userdata, center, nextv, oldv);
    oldv = nextv;
    return;
  }

  /// node is divided.
  CS_ASSERT(!IsLeaf());
  csVector3 midpt = (oldv + nextv)*0.5f; // midpt along edge
  int ch = -1;
  if(dir == CS_QUAD_TOP) ch=CS_QUAD_BOTLEFT;
  if(dir == CS_QUAD_RIGHT) ch=CS_QUAD_TOPLEFT;
  if(dir == CS_QUAD_BOT) ch=CS_QUAD_TOPRIGHT;
  if(dir == CS_QUAD_LEFT) ch=CS_QUAD_BOTRIGHT;
  CS_ASSERT(ch!=-1);
  children[ch]->TriEdge(dir, cb, userdata, framenum, center, oldv,
    midpt, minx, miny, maxx, maxy);

  CS_ASSERT( oldv == midpt );

  ch=-1;
  if(dir == CS_QUAD_TOP) ch=CS_QUAD_BOTRIGHT;
  if(dir == CS_QUAD_RIGHT) ch=CS_QUAD_BOTLEFT;
  if(dir == CS_QUAD_BOT) ch=CS_QUAD_TOPLEFT;
  if(dir == CS_QUAD_LEFT) ch=CS_QUAD_TOPRIGHT;
  CS_ASSERT(ch!=-1);
  children[ch]->TriEdge(dir, cb, userdata, framenum, center, oldv,
    nextv, minx, miny, maxx, maxy);
}
