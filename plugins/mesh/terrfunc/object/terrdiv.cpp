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
#include "cscsQsqrt.h"

//-------------- csTerrainQuadDiv ------------------------------
csTerrainQuadDiv::csTerrainQuadDiv(int depth)
{
  parent = 0;
  parentplace = -1;
  int i;
  for(i=0; i<4; i++)
  {
    children[i] = 0;
    neighbors[i] = 0;
    corner_height[i] = 0.0;
    corner_texuv[i].Set(0,0);
    corner_color[i].Set(1,1,1);
    corner_normal[i].Set(0,1,0);
  }
  middle_height = 0.0;
  middle_texuv.Set(0,0);
  middle_color.Set(1,1,1);
  middle_normal.Set(0,1,0);
  subdivided = 0;
  dmax = 0;
  min_height = 0;
  max_height = 0;
  visquad = 0;

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
  neighbors[dir] = 0;
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
  if(!parent) return 0;

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
  if(!parneigh) return 0;

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
  if(!parent) return 0;
  csTerrainQuad *pquad = parent->GetVisQuad();
  if(!pquad) return 0;
  visquad = pquad->GetChild(parentplace);
  return visquad;
}

void csTerrainQuadDiv::ComputeDmax(iTerrainHeightFunction* height_func,
    void (*texuv_func)(void*, csVector2&, float, float), void *texdata,
    iTerrainNormalFunction *normal_func,
    float minx, float miny, float maxx, float maxy)
{
  int i;
  float midx = (minx+maxx)*0.5f;
  float midy = (miny+maxy)*0.5f;
  float h;
  //float cornerh[4];
  //cornerh[0] = height_func->GetHeight(minx, miny);
  //cornerh[1] = height_func->GetHeight(minx, maxy);
  //cornerh[2] = height_func->GetHeight(maxx, maxy);
  //cornerh[3] = height_func->GetHeight(maxx, miny);

  corner_height[CS_QUAD_TOPLEFT] = height_func->GetHeight(minx, miny);
  corner_height[CS_QUAD_BOTLEFT] = height_func->GetHeight(minx, maxy);
  corner_height[CS_QUAD_BOTRIGHT] = height_func->GetHeight(maxx, maxy);
  corner_height[CS_QUAD_TOPRIGHT] = height_func->GetHeight(maxx, miny);
  middle_height = height_func->GetHeight(midx, midy);
  //printf("corners %g %g %g %g\n", corner_height[0], corner_height[1], corner_height[2], corner_height[3]);

  corner_normal[CS_QUAD_TOPLEFT] = normal_func->GetNormal(minx, miny);
  corner_normal[CS_QUAD_BOTLEFT] = normal_func->GetNormal(minx, maxy);
  corner_normal[CS_QUAD_BOTRIGHT] = normal_func->GetNormal(maxx, maxy);
  corner_normal[CS_QUAD_TOPRIGHT] = normal_func->GetNormal(maxx, miny);
  middle_normal = normal_func->GetNormal(midx, midy);

  texuv_func(texdata, corner_texuv[CS_QUAD_TOPLEFT], minx, miny);
  texuv_func(texdata, corner_texuv[CS_QUAD_BOTLEFT], minx, maxy);
  texuv_func(texdata, corner_texuv[CS_QUAD_BOTRIGHT], maxx, maxy);
  texuv_func(texdata, corner_texuv[CS_QUAD_TOPRIGHT], maxx, miny);
  texuv_func(texdata, middle_texuv, midx, midy);

  /// first establish min/max height
  if(IsLeaf())
  {
    min_height = middle_height;
    max_height = min_height;
    for(i=0; i<4; i++)
    {
      h = corner_height[i];
      if(h>max_height) max_height=h; 
      if(h<min_height) min_height=h;
    }

    /// compute dmax 
    float h_pol = (corner_height[CS_QUAD_BOTLEFT] + 
      corner_height[CS_QUAD_TOPRIGHT])*0.5; // interpolated
    h = middle_height;
    dmax = ABS(h_pol - h);
    //printf("isleaf, dmax %g h %g \n", dmax, h);
  }
  else
  {
    children[CS_QUAD_TOPLEFT]->ComputeDmax (height_func, texuv_func, texdata,
      normal_func, minx, miny, midx, midy);
    children[CS_QUAD_TOPRIGHT]->ComputeDmax(height_func, texuv_func, texdata,
      normal_func, midx, miny, maxx, midy);
    children[CS_QUAD_BOTLEFT]->ComputeDmax(height_func, texuv_func, texdata,
      normal_func, minx, midy, midx, maxy);
    children[CS_QUAD_BOTRIGHT]->ComputeDmax(height_func, texuv_func, texdata,
      normal_func, midx, midy, maxx, maxy);
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
    h = middle_height;
    float interpol_h = (corner_height[CS_QUAD_BOTLEFT] + 
      corner_height[CS_QUAD_TOPRIGHT])*0.5; // interpolated
    dmax = ABS(interpol_h - h);
    //printf("nonleaf, dmax %g h %g \n", dmax, h);
    /// make sure dmax >= MAX(children dmax);
    for(i=0; i<4; i++)
    {
      if(children[i]->GetDmax() > dmax)
        dmax = children[i]->GetDmax();
    }
    //printf("nonleaf, dmax %g after children \n", dmax);
  }
  CS_ASSERT(max_height >= min_height);

  /// compute dmax more precise - based on edge lines
  h = height_func->GetHeight(midx, miny);
  h -= (corner_height[CS_QUAD_TOPLEFT] + corner_height[CS_QUAD_TOPRIGHT])*0.5;
  h = ABS(h); if(h > dmax) dmax = h;
  h = height_func->GetHeight(midx, maxy);
  h -= (corner_height[CS_QUAD_BOTLEFT] + corner_height[CS_QUAD_BOTRIGHT])*0.5;
  h = ABS(h); if(h > dmax) dmax = h;
  h = height_func->GetHeight(minx, midy);
  h -= (corner_height[CS_QUAD_TOPLEFT] + corner_height[CS_QUAD_BOTLEFT])*0.5;
  h = ABS(h); if(h > dmax) dmax = h;
  h = height_func->GetHeight(maxx, midy);
  h -= (corner_height[CS_QUAD_TOPRIGHT] + corner_height[CS_QUAD_BOTRIGHT])*0.5;
  h = ABS(h); if(h > dmax) dmax = h;

  //printf("  dmax %g after rest \n", dmax);
  //printf("size %g dmax is %g.\n", maxx-minx, dmax);
}

void csTerrainQuadDiv::ComputeLOD(int framenum, const csVector3& campos,
  void (*light_func)(void*, csColor&, float, float, const csVector3&), 
  void *lightdata, float minx, float miny, float maxx, float maxy)
{
  if(GetVisQuad()) if(!GetVisQuad()->IsVisible()) return;
  float midx = (minx+maxx)*0.5f;
  float midy = (miny+maxy)*0.5f;

  /// if we arrive at this point then the quad will be visible, so
  /// (re)compute lighting for it
  if(parent && parentplace == CS_QUAD_TOPLEFT)
    corner_color[CS_QUAD_TOPLEFT] = parent->corner_color[CS_QUAD_TOPLEFT];
  else if(parent && parentplace == CS_QUAD_BOTRIGHT)
    corner_color[CS_QUAD_TOPLEFT] = parent->middle_color;
  else light_func(lightdata, corner_color[CS_QUAD_TOPLEFT], minx, miny,
    corner_normal[CS_QUAD_TOPLEFT]);

  if(parent && parentplace == CS_QUAD_BOTLEFT)
    corner_color[CS_QUAD_BOTLEFT] = parent->corner_color[CS_QUAD_BOTLEFT];
  else if(parent && parentplace == CS_QUAD_TOPRIGHT)
    corner_color[CS_QUAD_BOTLEFT] = parent->middle_color;
  else light_func(lightdata, corner_color[CS_QUAD_BOTLEFT], minx, maxy,
    corner_normal[CS_QUAD_BOTLEFT]);

  if(parent && parentplace == CS_QUAD_BOTRIGHT)
    corner_color[CS_QUAD_BOTRIGHT] = parent->corner_color[CS_QUAD_BOTRIGHT];
  else if(parent && parentplace == CS_QUAD_TOPLEFT)
    corner_color[CS_QUAD_BOTRIGHT] = parent->middle_color;
  else light_func(lightdata, corner_color[CS_QUAD_BOTRIGHT], maxx, maxy,
    corner_normal[CS_QUAD_BOTRIGHT]);

  if(parent && parentplace == CS_QUAD_TOPRIGHT)
    corner_color[CS_QUAD_TOPRIGHT] = parent->corner_color[CS_QUAD_TOPRIGHT];
  else if(parent && parentplace == CS_QUAD_BOTLEFT)
    corner_color[CS_QUAD_TOPRIGHT] = parent->middle_color;
  light_func(lightdata, corner_color[CS_QUAD_TOPRIGHT], maxx, miny,
    corner_normal[CS_QUAD_TOPRIGHT]);

  light_func(lightdata, middle_color, midx, midy, middle_normal);

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
      light_func, lightdata, minx, miny, midx, midy);
    children[CS_QUAD_TOPRIGHT]->ComputeLOD (framenum, campos,
      light_func, lightdata, midx, miny, maxx, midy);
    children[CS_QUAD_BOTLEFT]->ComputeLOD (framenum, campos,
      light_func, lightdata, minx, midy, midx, maxy);
    children[CS_QUAD_BOTRIGHT]->ComputeLOD (framenum, campos,
      light_func, lightdata, midx, midy, maxx, maxy);
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
  const csVector3&, const csVector3&, const csVector2&, const csVector2&, 
  const csVector2&, const csColor&, const csColor&, const csColor&), 
  void *userdata, int framenum, 
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
  if(!HaveMoreDetailedNeighbor(framenum))
  {
    /// debug, create a polygon at medio height
    /// float h = (min_height + max_height)*0.5;
    cb(userdata, 
      csVector3(minx,corner_height[CS_QUAD_TOPLEFT],miny), 
      csVector3(minx,corner_height[CS_QUAD_BOTLEFT],maxy), 
      csVector3(maxx,corner_height[CS_QUAD_TOPRIGHT],miny),
      corner_texuv[CS_QUAD_TOPLEFT], corner_texuv[CS_QUAD_BOTLEFT],
      corner_texuv[CS_QUAD_TOPRIGHT], corner_color[CS_QUAD_TOPLEFT],
      corner_color[CS_QUAD_BOTLEFT], corner_color[CS_QUAD_TOPRIGHT]);
    cb(userdata, 
     csVector3(maxx,corner_height[CS_QUAD_TOPRIGHT],miny), 
     csVector3(minx,corner_height[CS_QUAD_BOTLEFT],maxy), 
     csVector3(maxx,corner_height[CS_QUAD_BOTRIGHT],maxy),
     corner_texuv[CS_QUAD_TOPRIGHT], corner_texuv[CS_QUAD_BOTLEFT],
     corner_texuv[CS_QUAD_BOTRIGHT], corner_color[CS_QUAD_TOPRIGHT],
     corner_color[CS_QUAD_BOTLEFT], corner_color[CS_QUAD_BOTRIGHT]);
    return;
  }

  /// accomodate the more detailed neighbors
  /// by drawing starshaped triangles fitting all vertices at edges
  csVector3 center(midx,middle_height,midy);
  csVector2& center_uv = middle_texuv;
  csColor& center_col = middle_color;

  csVector3 nextv;
  csVector3 oldv;

  csVector2 old_uv = corner_texuv[CS_QUAD_TOPLEFT];
  csColor old_col = corner_color[CS_QUAD_TOPLEFT];
  oldv.Set(minx, corner_height[CS_QUAD_TOPLEFT], miny);
  csTerrainQuadDiv *neigh = GetNeighbor(CS_QUAD_TOP);
  nextv.Set(maxx,corner_height[CS_QUAD_TOPRIGHT],miny);
  csVector2 next_uv = corner_texuv[CS_QUAD_TOPRIGHT];
  csColor next_col = corner_color[CS_QUAD_TOPRIGHT];
  if(neigh) neigh->TriEdge(CS_QUAD_TOP, cb, userdata, framenum, center, oldv, 
    nextv, center_uv, old_uv, next_uv, center_col, old_col, next_col, 
    minx, miny, maxx, maxy);
  else { cb(userdata, center, nextv, oldv, center_uv, next_uv, old_uv,
    center_col, next_col, old_col); 
    oldv = nextv; old_uv=next_uv; old_col=next_col;}

  nextv.Set(maxx,corner_height[CS_QUAD_BOTRIGHT],maxy);
  next_uv = corner_texuv[CS_QUAD_BOTRIGHT];
  next_col = corner_color[CS_QUAD_BOTRIGHT];
  neigh = GetNeighbor(CS_QUAD_RIGHT);
  if(neigh) neigh->TriEdge(CS_QUAD_RIGHT, cb, userdata, framenum, center, oldv,
    nextv, center_uv, old_uv, next_uv, center_col, old_col, next_col, 
    minx, miny, maxx, maxy);
  else { cb(userdata, center, nextv, oldv, center_uv, next_uv, old_uv,
    center_col, next_col, old_col); 
    oldv = nextv; old_uv=next_uv; old_col=next_col;}

  nextv.Set(minx,corner_height[CS_QUAD_BOTLEFT],maxy);
  next_uv = corner_texuv[CS_QUAD_BOTLEFT];
  next_col = corner_color[CS_QUAD_BOTLEFT];
  neigh = GetNeighbor(CS_QUAD_BOT);
  if(neigh) neigh->TriEdge(CS_QUAD_BOT, cb, userdata, framenum, center, oldv,
    nextv, center_uv, old_uv, next_uv, center_col, old_col, next_col, 
    minx, miny, maxx, maxy);
  else { cb(userdata, center, nextv, oldv, center_uv, next_uv, old_uv,
    center_col, next_col, old_col); 
    oldv = nextv; old_uv=next_uv; old_col=next_col;}

  nextv.Set(minx,corner_height[CS_QUAD_TOPLEFT],miny);
  next_uv = corner_texuv[CS_QUAD_TOPLEFT];
  next_col = corner_color[CS_QUAD_TOPLEFT];
  neigh = GetNeighbor(CS_QUAD_LEFT);
  if(neigh) neigh->TriEdge(CS_QUAD_LEFT, cb, userdata, framenum, center, oldv,
    nextv, center_uv, old_uv, next_uv, center_col, old_col, next_col, 
    minx, miny, maxx, maxy);
  else { cb(userdata, center, nextv, oldv, center_uv, next_uv, old_uv,
    center_col, next_col, old_col); 
    oldv = nextv; old_uv=next_uv; old_col=next_col;}

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
  const csVector3&, const csVector3&, const csVector3&, const csVector2&, 
  const csVector2&, const csVector2&, const csColor&, const csColor&, 
  const csColor&), void *userdata, int framenum,
  const csVector3& center, csVector3& oldv, const csVector3& nextv,
  const csVector2& center_uv, csVector2& old_uv, const csVector2& next_uv, 
  const csColor& center_col, csColor& old_col, const csColor& next_col,
  float minx, float miny, float maxx, float maxy)
{
  /// NOTE dir is from the caller (points from 'center' towards me)
  if(subdivided != framenum) 
  {
    /// draw singly
    cb(userdata, center, nextv, oldv, center_uv, next_uv, old_uv,
      center_col, next_col, old_col);
    oldv = nextv;
    old_uv = next_uv;
    old_col = next_col;
    return;
  }

  /// node is divided.
  CS_ASSERT(!IsLeaf());
  csVector3 midpt = (oldv + nextv)*0.5f; // midpt along edge
  int ch = -1;
  int corner = -1;
  if(dir == CS_QUAD_TOP) {ch=CS_QUAD_BOTLEFT; corner=CS_QUAD_BOTRIGHT;}
  if(dir == CS_QUAD_RIGHT) {ch=CS_QUAD_TOPLEFT; corner=CS_QUAD_BOTLEFT;}
  if(dir == CS_QUAD_BOT) {ch=CS_QUAD_TOPRIGHT; corner=CS_QUAD_TOPLEFT;}
  if(dir == CS_QUAD_LEFT) {ch=CS_QUAD_BOTRIGHT; corner=CS_QUAD_TOPRIGHT;}
  CS_ASSERT(ch!=-1);
  CS_ASSERT(corner!=-1);
  /// get middle point height from children
  midpt.y = children[ch]->corner_height[corner];
  /// middle point texture, color from children of caller...
  int origdir = -1, origchild = -1, origcorner = -1;
  if(dir == CS_QUAD_TOP) {origdir=CS_QUAD_BOT, origchild=CS_QUAD_TOPLEFT;
    origcorner=CS_QUAD_TOPRIGHT;}
  if(dir == CS_QUAD_RIGHT) {origdir=CS_QUAD_LEFT; origchild=CS_QUAD_TOPRIGHT;
    origcorner=CS_QUAD_BOTRIGHT;}
  if(dir == CS_QUAD_BOT) {origdir=CS_QUAD_TOP; origchild=CS_QUAD_BOTRIGHT;
    origcorner=CS_QUAD_BOTLEFT;}
  if(dir == CS_QUAD_LEFT) {origdir=CS_QUAD_RIGHT; origchild=CS_QUAD_BOTLEFT;
    origcorner=CS_QUAD_TOPLEFT;}
  csTerrainQuadDiv *n= GetNeighbor(origdir);
  CS_ASSERT(n);
  csVector2 mid_uv = n->children[origchild]->corner_texuv[origcorner];
  // csColor mid_col = n->children[origchild]->corner_color[origcorner];

  csColor mid_col = children[ch]->corner_color[corner];

  children[ch]->TriEdge(dir, cb, userdata, framenum, center, oldv,
    midpt, center_uv, old_uv, mid_uv, center_col, old_col, mid_col,
    minx, miny, maxx, maxy);

  CS_ASSERT( oldv == midpt );
  CS_ASSERT( old_uv == mid_uv );

  ch=-1;
  if(dir == CS_QUAD_TOP) ch=CS_QUAD_BOTRIGHT;
  if(dir == CS_QUAD_RIGHT) ch=CS_QUAD_BOTLEFT;
  if(dir == CS_QUAD_BOT) ch=CS_QUAD_TOPLEFT;
  if(dir == CS_QUAD_LEFT) ch=CS_QUAD_TOPRIGHT;
  CS_ASSERT(ch!=-1);
  children[ch]->TriEdge(dir, cb, userdata, framenum, center, oldv,
    nextv, center_uv, old_uv, next_uv, center_col, old_col, next_col,
    minx, miny, maxx, maxy);
}
