/*
    Copyright (C) 2001 by Jorrit Tyberghein
  
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
#include "csgeom/polyclip.h"
#include "csgeom/poly2d.h"
#include "csgeom/vector3.h"
#include "csutil/garray.h"
#include "isys/system.h"
#include "terrvis.h"
#include "qint.h"
#include "qsqrt.h"

//------------------------------------------------------------------------

int csTerrainQuad::global_visnr = 0;

csTerrainQuad::csTerrainQuad ()
{
  children[0] = NULL;
  children[1] = NULL;
  children[2] = NULL;
  children[3] = NULL;
  visnr = -1;
}

csTerrainQuad::~csTerrainQuad ()
{
  delete children[0];
  delete children[1];
  delete children[2];
  delete children[3];
}

void csTerrainQuad::Build (int depth)
{
  if (depth <= 0) return;
  children[0] = new csTerrainQuad ();
  children[0]->Build (depth-1);
  children[1] = new csTerrainQuad ();
  children[1]->Build (depth-1);
  children[2] = new csTerrainQuad ();
  children[2]->Build (depth-1);
  children[3] = new csTerrainQuad ();
  children[3]->Build (depth-1);
}

/// smallest value for horizon
#define MININF -99999999.0

void csTerrainQuad::InitHorizon(float *horizon, int horsize)
{
  for(int i=0; i<horsize; i++)
    horizon[i] = MININF;
}

int csTerrainQuad::GetHorIndex(const csVector3& campos, float x, float z, 
  int horsize)
{
  /// @@@ could make a table to lookup the angle from x/z
  /// instead of atan
  float diffx = x - campos.x;
  float diffz = z - campos.z;

  double atn = PI/2.;
  if(diffz != 0.0) atn = atan(diffx / diffz);
  // scale from -PI/2..PI/2 to 0..horsize
  atn += PI/2.;
  atn *= float(horsize-1)/PI;
  int idx = QInt(atn);
  CS_ASSERT(idx >= 0 && idx < horsize);
  return idx;
}

void csTerrainQuad::ComputeExtent(const csVector3& campos, const csBox3& bbox,
    int horsize, int& left, int& right)
{
  // only need to test four of the vertices, since the height is not important
  // for this.
  int idx[4];
  idx[0] = GetHorIndex(campos, bbox.MinX(), bbox.MinZ(), horsize);
  idx[1] = GetHorIndex(campos, bbox.MaxX(), bbox.MinZ(), horsize);
  idx[2] = GetHorIndex(campos, bbox.MaxX(), bbox.MaxZ(), horsize);
  idx[3] = GetHorIndex(campos, bbox.MinX(), bbox.MaxZ(), horsize);
  // note: from  0-1, 1-2, 2-3, 3-0 are the edge lines

  left = right = idx[0];
  for(int i=1; i<4; i++)
  {
    // already included?
    if( (right-idx[i])%horsize <= (right-left)%horsize )
      continue;
    // extend the directions, by adding this angle to existing range.
    int ldiff = (left - idx[i] + horsize) % horsize;
    int rdiff = (idx[i] - right + horsize) % horsize;
    if(ldiff < rdiff)
      // angle is closer to left than right angle
      left = idx[i];
    else
      right = idx[i];
  }
  /// the resulting span should be less than half the horizon
  CS_ASSERT ((right - left)%horsize <= horsize/2);
}

void csTerrainQuad::ComputeMinMaxDY(const csVector3& campos, const csBox3& bbox,
    float &mindy, float &maxdy)
{
  /// idea is this:
  /// maximum dy, is the max_height / minimumdistance
  /// minimum dy, is the min_height / maximumdistance

  /// compute ground-distance to each of the four points.
  /// squared distance is used
  float sqdist[4];
  csVector2 d;
  d.Set(bbox.MinX() - campos.x, bbox.MinZ() - campos.z);
  sqdist[0] = d.SquaredNorm();
  d.Set(bbox.MaxX() - campos.x, bbox.MinZ() - campos.z);
  sqdist[1] = d.SquaredNorm();
  d.Set(bbox.MinX() - campos.x, bbox.MaxZ() - campos.z);
  sqdist[2] = d.SquaredNorm();
  d.Set(bbox.MaxX() - campos.x, bbox.MaxZ() - campos.z);
  sqdist[3] = d.SquaredNorm();

  /// compute min/max squared distance
  float mindist = sqdist[0];
  float maxdist = sqdist[0];
  for(int i=1; i<4; i++)
  {
    if(sqdist[i] < mindist) mindist = sqdist[i];
    else if(sqdist[i] > maxdist) maxdist = sqdist[i];
  }

  if(maxdist == 0.0) 
  {
    if(min_height < 0) mindy = MININF; 
    else mindy = -MININF; 
  }
  else mindy = min_height / qsqrt(maxdist);

  if(mindist == 0.0) 
  {
    if(max_height < 0) maxdy = MININF; 
    else maxdy = -MININF; 
  }
  else maxdy = max_height / qsqrt(mindist);
}

bool csTerrainQuad::CheckIfAbove(float* horizon, int horsize, int left, 
  int right, float dy)
{
  /// loop from left to right, but it can wrap by horsize
  int len = (right-left)%horsize;
  int idx = left;
  while(len--)
  {
    if( dy > horizon[idx] ) return true;
    idx = (idx+1)%horsize;
  }
  return false;
}

void csTerrainQuad::HeightenHorizon(float* horizon, int horsize, int left, 
  int right, float dy)
{
  int len = (right-left)%horsize;
  int idx = left;
  while(len--)
  {
    if( dy > horizon[idx] ) horizon[idx] = dy;
    idx = (idx+1)%horsize;
  }
}


void csTerrainQuad::ComputeVisibility(const csVector3& campos, 
  const csBox3& bbox, float* horizon, int horsize)
{
  // compute visibility for this node

  // start and end angles for projected bounding box on horizon
  int left, right;
  ComputeExtent(campos, bbox, horsize, left, right);

  // compute min and max dy values.
  float mindy, maxdy;
  ComputeMinMaxDY(campos, bbox, mindy, maxdy);

  // determine visibility
  bool vis = false;
  if(bbox.Contains(campos)) vis = true;
  else {
    // check min and max dy against the horizon
    // if the max is bigger than any value -> the node is visible
    vis = CheckIfAbove(horizon, horsize, left, right, maxdy);
  }

  if(!vis) return; // nothing to be done, nor for the children.

  // so this node is visible
  MarkVisible();
  // update horizon with minimum height
  HeightenHorizon(horizon, horsize, left, right, mindy);

  // for leaves, we are done now.
  if(IsLeaf()) return;

  // the children must be checked front-to-back...
  float midx = (bbox.MinX() + bbox.MaxX()) * 0.5;
  float midz = (bbox.MinZ() + bbox.MaxZ()) * 0.5;

  // first see where the camera lies:
  // TOP +z, BOT -z
  // LEFT -x, RIGHT +x
  // z splits top from bottom
  // x splits left from right
  int camchild = 0;
  if(campos.z < midz) camchild = CS_QUAD_TOPLEFT;
  else camchild = CS_QUAD_BOTLEFT;
  if(campos.x > midx) camchild++; // get the right quad

  // iterate back-to front
  const int fronttoback[4][4] = {
    {0,1,2,3},
    {1,0,3,2},
    {2,0,3,1},
    {3,1,2,0}
  };
  csBox3 cbox[4];
  cbox[CS_QUAD_TOPLEFT].Set (bbox.MinX(), 0, midz, midx, 0, bbox.MaxZ());
  cbox[CS_QUAD_TOPRIGHT].Set(midx, 0, midz, bbox.MaxX(), 0, bbox.MaxZ());
  cbox[CS_QUAD_BOTLEFT].Set (bbox.MinX(), 0, bbox.MinZ(), midx, 0, midz);
  cbox[CS_QUAD_BOTRIGHT].Set(midx, 0, bbox.MinZ(), bbox.MaxX(), 0, midz);
  for(int i=0; i<4; i++)
  {
    int child = fronttoback[camchild][i];
    GetChild(child)->ComputeVisibility(campos, cbox[child], horizon, horsize);
  }
}

//------------------------------------------------------------------------

